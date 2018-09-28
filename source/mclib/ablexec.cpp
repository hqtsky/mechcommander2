//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
//***************************************************************************
//
//								ABLEXEC.CPP
//
//***************************************************************************
#include "stdinc.h"

//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>

#ifndef ABLGEN_H
#include "ablgen.h"
#endif

#ifndef ABLERR_H
#include "ablerr.h"
#endif

#ifndef ABLSCAN_H
#include "ablscan.h"
#endif

#ifndef ABLEXEC_H
#include "ablexec.h"
#endif

#ifndef ABLDBUG_H
#include "abldbug.h"
#endif

//***************************************************************************

//--------
// GLOBALS
PSTR codeBuffer		   = nullptr;
PSTR codeBufferPtr	 = nullptr;
PSTR codeSegmentPtr	= nullptr;
PSTR codeSegmentLimit  = nullptr;
PSTR statementStartPtr = nullptr;

TokenCodeType codeToken;
int32_t execLineNumber;
int32_t execStatementCount = 0;

StackItem* stack			   = nullptr;
StackItemPtr tos			   = nullptr;
StackItemPtr stackFrameBasePtr = nullptr;
StackItemPtr StaticDataPtr	 = nullptr;
int32_t* StaticVariablesSizes  = nullptr;
int32_t* EternalVariablesSizes = nullptr;
int32_t eternalOffset		   = 0;
int32_t MaxStaticVariables	 = 0;
int32_t MaxEternalVariables	= 0;
int32_t NumStaticVariables	 = 0;
int32_t NumOrderCalls		   = 1;
int32_t NumStateHandles		   = 0;
StateHandleInfo StateHandleList[MAX_STATE_HANDLES_PER_MODULE];
int32_t CurModuleHandle		= 0;
int32_t MaxCodeBufferSize   = 0;
bool CallModuleInit			= false;
bool AutoReturnFromOrders   = false;
int32_t MaxLoopIterations   = 100001;
bool AssertEnabled			= false;
bool PrintEnabled			= true;
bool StringFunctionsEnabled = true;
bool DebugCodeEnabled		= false;
bool IncludeDebugInfo		= true;
bool ProfileABL				= false;
bool Crunch					= true;

char SetStateDebugStr[256];

//----------
// EXTERNALS

extern SymTableNodePtr CurRoutineIdPtr;

extern ModuleEntryPtr ModuleRegistry;
extern ABLModulePtr* ModuleInstanceRegistry;
extern ABLModulePtr CurModule;
extern ABLModulePtr CurLibrary;
extern int32_t NumStateTransitions;

extern TokenCodeType curToken;
extern int32_t lineNumber;
extern int32_t FileNumber;
extern int32_t level;
extern TypePtr IntegerTypePtr;
extern TypePtr CharTypePtr;
extern TypePtr RealTypePtr;
extern TypePtr BooleanTypePtr;

extern StackItem returnValue;

extern bool ExitWithReturn;
extern bool ExitFromTacOrder;

extern DebuggerPtr debugger;
extern bool NewStateSet;

extern void (*ABLEndlessStateCallback)(UserFile* log);

//***************************************************************************
// CRUNCH/DECRUNCH routines
//***************************************************************************

void crunchToken(void)
{
	if (!Crunch)
		return;
	if (codeBufferPtr >= (codeBuffer + MaxCodeBufferSize - 100))
		syntaxError(ABL_ERR_SYNTAX_CODE_SEGMENT_OVERFLOW);
	else
	{
		*codeBufferPtr = (char)curToken;
		codeBufferPtr++;
	}
}

//***************************************************************************

void crunchSymTableNodePtr(SymTableNodePtr nodePtr)
{
	if (!Crunch)
		return;
	if (codeBufferPtr >= (codeBuffer + MaxCodeBufferSize - 100))
		syntaxError(ABL_ERR_SYNTAX_CODE_SEGMENT_OVERFLOW);
	else
	{
		SymTableNodePtr* nodePtrPtr = (SymTableNodePtr*)codeBufferPtr;
		*nodePtrPtr					= nodePtr;
		codeBufferPtr += sizeof(SymTableNodePtr);
	}
}

//***************************************************************************

void crunchStatementMarker(void)
{
	if (!Crunch)
		return;
	if (codeBufferPtr >= (codeBuffer + MaxCodeBufferSize - 100))
		syntaxError(ABL_ERR_SYNTAX_CODE_SEGMENT_OVERFLOW);
	else
	{
		char saveCode  = *(--codeBufferPtr);
		*codeBufferPtr = (char)TKN_STATEMENT_MARKER;
		codeBufferPtr++;
		if (IncludeDebugInfo)
		{
			*((puint8_t)codeBufferPtr) = (uint8_t)FileNumber;
			codeBufferPtr += sizeof(uint8_t);
			*((int32_t*)codeBufferPtr) = lineNumber;
			codeBufferPtr += sizeof(int32_t);
		}
		*codeBufferPtr = saveCode;
		codeBufferPtr++;
	}
}

//***************************************************************************

void uncrunchStatementMarker(void)
{
	//-------------------------
	// Pull code off the buffer
	codeBufferPtr--;
	//-------------------------------
	// Pull debug info off the buffer
	if (IncludeDebugInfo)
		codeBufferPtr -= (sizeof(uint8_t) + sizeof(int32_t));
	//-------------------------------------
	// Pull statement marker off the buffer
	codeBufferPtr--;
}

//***************************************************************************

PSTR crunchAddressMarker(Address address)
{
	if (!Crunch)
		return (nullptr);
	PSTR saveCodeBufferPtr = nullptr;
	if (codeBufferPtr >= (codeBuffer + MaxCodeBufferSize - 100))
		syntaxError(ABL_ERR_SYNTAX_CODE_SEGMENT_OVERFLOW);
	else
	{
		char saveCode  = *(--codeBufferPtr);
		*codeBufferPtr = (char)TKN_ADDRESS_MARKER;
		codeBufferPtr++;
		saveCodeBufferPtr		   = codeBufferPtr;
		*((Address*)codeBufferPtr) = address;
		codeBufferPtr += sizeof(Address);
		*codeBufferPtr = saveCode;
		codeBufferPtr++;
	}
	return (saveCodeBufferPtr);
}

//***************************************************************************

PSTR fixupAddressMarker(Address address)
{
	if (!Crunch)
		return (nullptr);
	PSTR oldAddress		 = *((Address*)address);
	*((int32_t*)address) = codeBufferPtr - address;
	return (oldAddress);
}

//***************************************************************************

void crunchInteger(int32_t value)
{
	if (!Crunch)
		return;
	if (codeBufferPtr >= (codeBuffer + MaxCodeBufferSize - 100))
		syntaxError(ABL_ERR_SYNTAX_CODE_SEGMENT_OVERFLOW);
	else
	{
		*((int32_t*)codeBufferPtr) = value;
		codeBufferPtr += sizeof(int32_t);
	}
}

//***************************************************************************

void crunchByte(uint8_t value)
{
	if (!Crunch)
		return;
	if (codeBufferPtr >= (codeBuffer + MaxCodeBufferSize - 100))
		syntaxError(ABL_ERR_SYNTAX_CODE_SEGMENT_OVERFLOW);
	else
	{
		*((puint8_t)codeBufferPtr) = value;
		codeBufferPtr += sizeof(uint8_t);
	}
}

//***************************************************************************

void crunchOffset(Address address)
{
	if (!Crunch)
		return;
	if (codeBufferPtr >= (codeBuffer + MaxCodeBufferSize - 100))
		syntaxError(ABL_ERR_SYNTAX_CODE_SEGMENT_OVERFLOW);
	else
	{
		*((int32_t*)codeBufferPtr) = address - codeBufferPtr;
		codeBufferPtr += sizeof(int32_t);
	}
}

//***************************************************************************

PSTR createCodeSegment(int32_t& codeSegmentSize)
{
	codeSegmentSize  = codeBufferPtr - codeBuffer + 1;
	PSTR codeSegment = (PSTR)ABLCodeMallocCallback(codeSegmentSize);
	if (!codeSegment)
		ABL_Fatal(0, " ABL: Unable to AblCodeHeap->malloc code segment ");
	for (size_t i = 0; i < codeSegmentSize; i++)
		codeSegment[i] = codeBuffer[i];
	codeBufferPtr = codeBuffer;
	return (codeSegment);
}

//***************************************************************************

SymTableNodePtr getCodeSymTableNodePtr(void)
{
	SymTableNodePtr* nodePtrPtr = (SymTableNodePtr*)codeSegmentPtr;
	SymTableNodePtr nodePtr		= *nodePtrPtr;
	codeSegmentPtr += sizeof(SymTableNodePtr);
	return (nodePtr);
}

//***************************************************************************

int32_t getCodeStatementMarker(void)
{
	//------------------------------------------
	// NOTE: If there's a problem, we return -1.
	int32_t lineNum = -1;
	if (codeToken == TKN_STATEMENT_MARKER)
	{
		if (IncludeDebugInfo)
		{
			FileNumber = *((puint8_t)codeSegmentPtr);
			codeSegmentPtr += sizeof(uint8_t);
			lineNum = *((int32_t*)codeSegmentPtr);
			codeSegmentPtr += sizeof(int32_t);
		}
	}
	return (lineNum);
}

//***************************************************************************

PSTR getCodeAddressMarker(void)
{
	Address address = nullptr;
	if (codeToken == TKN_ADDRESS_MARKER)
	{
		address = *((int32_t*)codeSegmentPtr) + codeSegmentPtr - 1;
		codeSegmentPtr += sizeof(Address);
	}
	return (address);
}

//***************************************************************************

int32_t getCodeInteger(void)
{
	int32_t value = *((int32_t*)codeSegmentPtr);
	codeSegmentPtr += sizeof(int32_t);
	return (value);
}

//***************************************************************************

uint8_t getCodeByte(void)
{
	uint8_t value = *((puint8_t)codeSegmentPtr);
	codeSegmentPtr += sizeof(uint8_t);
	return (value);
}

//***************************************************************************

PSTR getCodeAddress(void)
{
	Address address = *((int32_t*)codeSegmentPtr) + codeSegmentPtr - 1;
	codeSegmentPtr += sizeof(int32_t);
	return (address);
}

//***************************************************************************
// STACK routines
//***************************************************************************

void pop(void) { --tos; }

//***************************************************************************

void getCodeToken(void)
{
	codeToken = (TokenCodeType)*codeSegmentPtr;
	codeSegmentPtr++;
}

//***************************************************************************

void pushInteger(int32_t value)
{
	StackItemPtr valuePtr = ++tos;
	if (valuePtr >= &stack[MAXSIZE_STACK])
		runtimeError(ABL_ERR_RUNTIME_STACK_OVERFLOW);
	valuePtr->integer = value;
}

//***************************************************************************

void pushReal(float value)
{
	StackItemPtr valuePtr = ++tos;
	if (valuePtr >= &stack[MAXSIZE_STACK])
		runtimeError(ABL_ERR_RUNTIME_STACK_OVERFLOW);
	valuePtr->real = value;
}

//***************************************************************************

void pushByte(char value)
{
	StackItemPtr valuePtr = ++tos;
	if (valuePtr >= &stack[MAXSIZE_STACK])
		runtimeError(ABL_ERR_RUNTIME_STACK_OVERFLOW);
	valuePtr->byte = value;
}

//***************************************************************************

void pushAddress(Address address)
{
	StackItemPtr valuePtr = ++tos;
	if (valuePtr >= &stack[MAXSIZE_STACK])
		runtimeError(ABL_ERR_RUNTIME_STACK_OVERFLOW);
	valuePtr->address = address;
}

//***************************************************************************

void pushBoolean(bool value)
{
	StackItemPtr valuePtr = ++tos;
	if (valuePtr >= &stack[MAXSIZE_STACK])
		runtimeError(ABL_ERR_RUNTIME_STACK_OVERFLOW);
	valuePtr->integer = (value ? 1 : 0);
}

//***************************************************************************
void pushStackFrameHeader(int32_t oldLevel, int32_t newLevel)
{
	//-----------------------------------
	// Make space for the return value...
	pushInteger(0);
	StackFrameHeaderPtr headerPtr = (StackFrameHeaderPtr)stackFrameBasePtr;
	//----------------------------------------------------------------------
	// STATIC LINK
	// Currently, let's not allow functions defined within functions. Assume
	// the old scope level equals the new scope level, for now.
	if (newLevel == -1)
	{
		//--------------------------------------------------------------------
		// Calling a library function, so push a nullptr static link since
		// it's scope is in a different module than the calling function.
		// Note that global variables in libraries should be STATIC, otherwise
		// they're never in scope! Weird "feature" which we may want
		// to fix later...
		pushAddress(nullptr);
	}
	else if (newLevel == oldLevel + 1)
	{
		//----------------------------------------------------------------
		// Calling a routine nested within the caller, so push the pointer
		// to the caller's stack frame. In ABL, as currently defined
		// (2/22/96), this should only be when a module's code section is
		// calling a function...
		pushAddress((Address)headerPtr);
	}
	else if (newLevel == oldLevel)
	{
		//---------------------------------------------------------------
		// Calling a function at the same scope level. We like that! Push
		// pointer to stack frame of their common parent...
		pushAddress(headerPtr->staticLink.address);
	}
	else
	{
		//-------------------------------------------------------
		// Oops. We don't want nested functions, for now, in ABL.
		runtimeError(ABL_ERR_RUNTIME_NESTED_FUNCTION_CALL);
	}
	pushAddress((Address)stackFrameBasePtr);
	//---------------------------
	// Push the return address...
	pushAddress(0);
}

//***************************************************************************

void allocLocal(TypePtr typePtr)
{
	if (typePtr == IntegerTypePtr)
		pushInteger(0);
	else if (typePtr == RealTypePtr)
		pushReal((float)0.0);
	else if (typePtr == BooleanTypePtr)
		pushByte(0);
	else if (typePtr == CharTypePtr)
		pushByte(0);
	else
		switch (typePtr->form)
		{
		case FRM_ENUM:
			pushInteger(0);
			break;
			// NOTE: We currently are not supporting sub ranges, until
			// we really want 'em...
			//			case FRM_SUBRANGE:
			//				allocLocal(typePtr->info.subrange.rangeTypePtr);
			//				break;
		case FRM_ARRAY:
			PSTR ptr = (PSTR)ABLStackMallocCallback(typePtr->size);
			if (!ptr)
				ABL_Fatal(0, " ABL: Unable to AblStackHeap->malloc local array ");
			pushAddress((Address)ptr);
			break;
		}
}

//***************************************************************************

void freeLocal(SymTableNodePtr idPtr)
{
	//---------------------------------------
	// Frees data allocated on local stack...
	TypePtr typePtr		 = (TypePtr)(idPtr->typePtr);
	StackItemPtr itemPtr = nullptr;
	if (((typePtr->form == FRM_ARRAY) /* || (typePtr->form == FRM_RECORD)*/) &&
		(idPtr->defn.key != DFN_REFPARAM))
	{
		switch (idPtr->defn.info.data.varType)
		{
		case VAR_TYPE_NORMAL:
			itemPtr = stackFrameBasePtr + idPtr->defn.info.data.offset;
			break;
			//			case VAR_TYPE_ETERNAL:
			//				itemPtr = stack + idPtr->defn.info.data.offset;
			//				break;
			//			case VAR_TYPE_STATIC:
			//				itemPtr = StaticDataPtr +
			// idPtr->defn.info.data.offset; 				break;
		}
		if (!itemPtr)
			runtimeError(0);
		else
			ABLStackFreeCallback(itemPtr->address);
	}
}

//***************************************************************************
// FUNCTION ENTRY/EXIT routines
//***************************************************************************

void routineEntry(SymTableNodePtr routineIdPtr)
{
	if (debugger)
		debugger->traceRoutineEntry(routineIdPtr);
	memset(&returnValue, 0, sizeof(StackItem));
	//------------------------------
	// Switch to new code segment...
	codeSegmentPtr = routineIdPtr->defn.info.routine.codeSegment;
	//----------------------------------------------
	// Allocate local variables onto system stack...
	for (SymTableNodePtr varIdPtr	  = (SymTableNodePtr)(routineIdPtr->defn.info.routine.locals);
		 varIdPtr != nullptr; varIdPtr = varIdPtr->next)
		if (varIdPtr->defn.info.data.varType == VAR_TYPE_NORMAL)
			allocLocal((TypePtr)(varIdPtr->typePtr));
}

//***************************************************************************

void routineExit(SymTableNodePtr routineIdPtr)
{
	if (debugger)
		debugger->traceRoutineExit(routineIdPtr);
	//-----------------------------------------
	// De-alloc parameters & local variables...
	SymTableNodePtr idPtr;
	for (idPtr = (SymTableNodePtr)(routineIdPtr->defn.info.routine.params); idPtr != nullptr;
		 idPtr = idPtr->next)
		freeLocal(idPtr);
	for (idPtr = (SymTableNodePtr)(routineIdPtr->defn.info.routine.locals); idPtr != nullptr;
		 idPtr = idPtr->next)
		if (idPtr->defn.info.data.varType == VAR_TYPE_NORMAL)
			freeLocal(idPtr);
	StackFrameHeaderPtr headerPtr = (StackFrameHeaderPtr)stackFrameBasePtr;
	codeSegmentPtr				  = headerPtr->returnAddress.address;
	if (routineIdPtr->typePtr == nullptr)
		tos = stackFrameBasePtr - 1;
	else
		tos = stackFrameBasePtr;
	stackFrameBasePtr = (StackItemPtr)headerPtr->dynamicLink.address;
}

//***************************************************************************

void execute(SymTableNodePtr routineIdPtr)
{
	SymTableNodePtr thisRoutineIdPtr = CurRoutineIdPtr;
	CurRoutineIdPtr					 = routineIdPtr;
	routineEntry(routineIdPtr);
	//----------------------------------------------------
	// Now, search this module for the function we want...
	if (CallModuleInit)
	{
		CallModuleInit					  = false;
		SymTableNodePtr initFunctionIdPtr = searchSymTable("init",
			ModuleRegistry[CurModule->getHandle()].moduleIdPtr->defn.info.routine.localSymTable);
		if (initFunctionIdPtr)
		{
			execRoutineCall(initFunctionIdPtr, false);
			//-------------------------------------------------------------------------
			// Since we're calling the function directly, we need to compensate
			// for the codeSegmentPtr being incremented by 1 in the normal
			// execRoutineCall...
			codeSegmentPtr--;
		}
	}
	if (routineIdPtr->defn.info.routine.flags & ROUTINE_FLAG_FSM)
	{
		NewStateSet = true;
		static char stateList[60][256];
		strcpy(SetStateDebugStr, "--");
		while (NewStateSet)
		{
			NumStateTransitions++;
			sprintf(stateList[NumStateTransitions], "%s (%s)", CurModule->getState()->name,
				SetStateDebugStr);
			if (NumStateTransitions == 50)
			{
				UserFile* userFile = UserFile::getNewFile();
				char errStr[512];
				if (userFile)
				{
					int32_t err = userFile->open("endless.log");
					if (!err)
					{
						// char s[1024];
						// sprintf(s, "Current Date: %s\n", GetTime());
						// userFile->write(s);
						userFile->write(ModuleRegistry[CurModule->getHandle()].fileName);
						for (size_t i = 1; i < 51; i++)
							userFile->write(stateList[i]);
						userFile->write(" ");
						if (ABLEndlessStateCallback)
							(*ABLEndlessStateCallback)(userFile);
						userFile->close();
					}
				}
				sprintf(errStr, " ABL endless state loop in %s [%s:%s] ",
					ModuleRegistry[CurModule->getHandle()].fileName, CurModule->getState()->name,
					CurModule->getPrevState()->name);
#if 0
				ABL_Fatal(NumStateTransitions, errStr);
#else
				NewStateSet = false;
#endif
			}
			else
			{
				NewStateSet				 = false;
				SymTableNodePtr curState = CurModule->getState();
				if (!curState)
					ABL_Fatal(0, " ABL.execute: nullptr state in FSM ");
				execRoutineCall(curState, false);
				codeSegmentPtr--;
			}
			//---------------------------------------------
			// In case we exited with a return statement...
			ExitWithReturn   = false;
			ExitFromTacOrder = false;
		}
	}
	else
	{
		getCodeToken();
		execStatement();
		//---------------------------------------------
		// In case we exited with a return statement...
		ExitWithReturn   = false;
		ExitFromTacOrder = false;
	}
	routineExit(routineIdPtr);
	CurRoutineIdPtr = thisRoutineIdPtr;
}

//***************************************************************************

void executeChild(SymTableNodePtr routineIdPtr, SymTableNodePtr childRoutineIdPtr)
{
	// THIS DOES NOT SUPPORT CALLING FUNCTIONS WITH PARAMETERS YET!
	SymTableNodePtr thisRoutineIdPtr = CurRoutineIdPtr;
	CurRoutineIdPtr					 = routineIdPtr;
	routineEntry(routineIdPtr);
	//----------------------------------------------------
	// Now, search this module for the function we want...
	SymTableNodePtr initFunctionIdPtr = nullptr;
	if (CallModuleInit)
	{
		CallModuleInit	= false;
		initFunctionIdPtr = searchSymTable("init", routineIdPtr->defn.info.routine.localSymTable);
		if (initFunctionIdPtr)
		{
			execRoutineCall(initFunctionIdPtr, false);
			//-------------------------------------------------------------------------
			// Since we're calling the function directly, we need to compensate
			// for the codeSegmentPtr being incremented by 1 in the normal
			// execRoutineCall...
			codeSegmentPtr--;
		}
	}
	if (initFunctionIdPtr != childRoutineIdPtr)
	{
		//-----------------------------------------------------------------------
		// If we're calling the module's init function, and we just did above,
		// don't call it again! That's why we make the check on the above
		// line...
		execRoutineCall(childRoutineIdPtr, false);
		//-------------------------------------------------------------------------
		// Since we're calling the function directly, we need to compensate for
		// the codeSegmentPtr being incremented by 1 in the normal
		// execRoutineCall...
		codeSegmentPtr--;
	}
	//---------------------------------------------
	// In case we exited with a return statement...
	ExitWithReturn   = false;
	ExitFromTacOrder = false;
	routineExit(routineIdPtr);
	CurRoutineIdPtr = thisRoutineIdPtr;
}

//***************************************************************************
// MISC routines
//***************************************************************************
