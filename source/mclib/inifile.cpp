//---------------------------------------------------------------------------
//
// inifile.cpp - This file contains the class declaration for the FitIni Files
//
//				The FitIni file is a ini style file.
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

//---------------------------------------------------------------------------
// Include files

#ifndef INIFILE_H
#include "inifile.h"
#endif

#ifndef HEAP_H
#include "heap.H"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>

#include "err.h"

#ifndef _MBCS
#include <gameos.hpp>
#else
#include <assert.h>
#define gosASSERT assert
#define gos_Malloc malloc
#define gos_Free free
#endif

//---------------------------------------------------------------------------
// Static Globals
char fitIniHeader[7] = "FITini";
char fitIniFooter[7] = "FITend";

#undef isspace		//Macro Chokes under Intel Compiler!!

//---------------------------------------------------------------------------
// class FitIniFile
FitIniFile::FitIniFile (void) : File()
{
	totalBlocks = 0;
	fileBlocks = NULL;
	
	currentBlockId = NULL;
	currentBlockOffset = 0;
	currentBlockSize = 0;
}

//---------------------------------------------------------------------------
FitIniFile::~FitIniFile (void)
{
	close();
}

//---------------------------------------------------------------------------
int32_t FitIniFile::findNextBlockStart (PSTR line, ULONG lineLen)
{
	char thisLine[255];
	PSTR common = NULL;
	
	do
	{
		if (line)
		{
			readLine((PUCHAR)line,lineLen);
			common = line;
		}
		else
		{
			readLine((PUCHAR)thisLine,254);
			common = thisLine;
		}
	}
	while(!eof() && (common[0] != '['));
	
	if (eof())
	{
		return(NO_MORE_BLOCKS);
	}

	return(NO_ERROR);
}	

//---------------------------------------------------------------------------
int32_t FitIniFile::countBlocks (void)
{
	int32_t count = 0;
	int32_t oldPosition = logicalPosition;
	
	while (findNextBlockStart() != NO_MORE_BLOCKS)
	{
		count++;
	}
	
	//----------------------------------
	// Move back to where we were.
	seek(oldPosition);
	
	return(count);
}

//---------------------------------------------------------------------------
int32_t FitIniFile::getNextWord (PSTR &line, PSTR buffer, ULONG bufLen)
{
	//--------------------------------------------------
	// Check to see if we are at end of line
	if (*line == '\0')
		return(GET_NEXT_LINE);
		
	//--------------------------------------------------
	// Check to see if the rest of the line is comments
	if (*line == '/')
		return(GET_NEXT_LINE);
		
	//------------------------------------------
	// Find start of word from current location
	while ((*line != '\0') && ((*line == ' ') || (*line == '\t') || (*line == ',')))
	{
		line++;
	}
	
	//--------------------------------------------------
	// Check to see if we are at end of line
	if (*line == '\0')
		return(GET_NEXT_LINE);
		
	//--------------------------------------------------
	// Check to see if the rest of the line is comments
	if (*line == '/')
		return(GET_NEXT_LINE);
		
	//-------------------------------------------
	// Find length of word from current location
	PSTR startOfWord = line;
	ULONG wordLength = 0;
	while ((*line != '\0') && ((*line != ' ') && (*line != '\t') && (*line != ',')))
	{
		line++;
		wordLength++;
	}
	
	if (wordLength > bufLen)
		return(BUFFER_TOO_SMALL);
		
	strncpy(buffer, startOfWord, wordLength);
	buffer[wordLength] = '\0';
	
	return(NO_ERROR);
}

//---------------------------------------------------------------------------
int32_t FitIniFile::afterOpen (void)
{
	char line[255];
	
	//-------------------------------------------------------
	// Check if we opened this with CREATE and write the
	// FITini Header and position to Write Start.
	if (fileMode == CREATE && parent == NULL)
	{
		sprintf(line,"%s \r\n",fitIniHeader);
		write((PUCHAR)line,strlen(line));

		totalBlocks = 0;
	}
	else
	{
		//------------------------------------------------------
		// Check if this is a FitIniFile by looking for header.
		char chkHeader[12];
	
		readLine((PUCHAR)chkHeader,11);
		if (strstr(chkHeader,fitIniHeader) == NULL)
			return(NOT_A_FITINIFILE);

				//------------------------------------------------------
		// Find out how many blocks we have	
		totalBlocks = countBlocks();
		
		//--------------------------------------------------------------------------
		// Allocate RAM for the BlockInfoNodes.  Check if system Heap is available
		fileBlocks = (IniBlockNode *)systemHeap->Malloc(sizeof(IniBlockNode) * totalBlocks);
		
		gosASSERT(fileBlocks != NULL);

		memset(fileBlocks,0,sizeof(IniBlockNode) * totalBlocks);
		
		//--------------------------------------------------------------------------
		// Put Info into fileBlocks.
		
		ULONG currentBlockNum = 0;
		while (findNextBlockStart(line,254) != NO_MORE_BLOCKS)
		{
			//----------------------------------------------------
			// If we write too many fileBlocks, we will trash RAM
			// Shouldn't be able to happen but...
			if (currentBlockNum == totalBlocks)
				return(TOO_MANY_BLOCKS);

			int32_t count = 1;
			while (line[count] != ']' && line[count] != '\n')
			{
				fileBlocks[currentBlockNum].blockId[count-1] = line[count];
				count++;
			}
			if (count >= 49)
				STOP(("BlockId To large in Fit File %s",fileName));

			if (line[count] == '\n')
			{
				char error[256];
				sprintf( error, "couldn't resolve block %s in file %s", line, getFilename() );
				Assert( 0, 0, error );
				return SYNTAX_ERROR;
			}
			
			fileBlocks[currentBlockNum].blockId[count-1] = '\0';
			
			//----------------------------------------------------------------------
			// Since we just read all of last line, we now point to start of data
			fileBlocks[currentBlockNum].blockOffset = logicalPosition;
			currentBlockNum++;
		}

		//------------------------------------------------------
		// If we didn't read in enough, CD-ROM error?
		if (currentBlockNum != totalBlocks)
			return(NOT_ENOUGH_BLOCKS);
	}

	return(NO_ERROR);
}

//---------------------------------------------------------------------------
void FitIniFile::atClose (void)
{
	//------------------------------------------------------------
	// Check if we are in create mode and if so, write the footer
	char line[200];
	if (fileMode == CREATE)
	{
		seek(0,SEEK_END);
		sprintf(line,"%s \r\n",fitIniFooter);
		write((PUCHAR)line,strlen(line));
	}

	//-----------------------------
	// Free up the fileBlocks
	systemHeap->Free(fileBlocks);
	fileBlocks = NULL;
}

//---------------------------------------------------------------------------
float FitIniFile::textToFloat (PCSTR num)
{
	float result = atof(num);
	return(result);
}

//---------------------------------------------------------------------------
double FitIniFile::textToDouble (PCSTR num)
{
	double result = atof(num);
	return(result);
}

//---------------------------------------------------------------------------
int32_t FitIniFile::textToLong (PCSTR num)
{
	int32_t result = 0;
	
	//------------------------------------
	// Check if Hex Number
	PSTR hexOffset = (PSTR )strstr(num,"0x");
	if (hexOffset == NULL)
	{
		result = atol(num);
	}
	else
	{
		hexOffset += 2;
		int32_t numDigits = strlen(hexOffset)-1;
		for (int i=0; i<=numDigits; i++)
		{
			if (!isalnum(hexOffset[i]) || (isalpha(hexOffset[i]) && toupper(hexOffset[i]) > 'F'))
			{
				hexOffset[i] = 0;	// we've reach a "wrong" character. Either start of a comment or something illegal. Either way, stop evaluation here.
				break;
			}
		}
		numDigits = strlen(hexOffset)-1;
		int32_t power = 0;
		for (int32_t count = numDigits;count >= 0;count--,power++)
		{
			uint8_t currentDigit = toupper(hexOffset[count]);
			
			if (currentDigit >= 'A' && currentDigit <= 'F')
			{
				result += (currentDigit - 'A' + 10)<<(4*power);
			}
			else if (currentDigit >= '0' && currentDigit <= '9')
			{
				result += (currentDigit - '0')<<(4*power);
			}
			else
			{
				//---------------------------------------------------------
				// There is a digit in here I don't understand.  Return 0.
				result = 0;
				break;
			}
		}
	}

	
	return(result);
}


//---------------------------------------------------------------------------
short FitIniFile::textToShort (PCSTR num)
{
	short result = 0;
	
	//------------------------------------
	// Check if Hex Number
	PSTR hexOffset = (PSTR )strstr(num,"0x");
	if (hexOffset == NULL)
	{
		result = atol(num);
	}
	else
	{
		hexOffset += 2;
		int32_t numDigits = strlen(hexOffset)-1;
		for (int i=0; i<=numDigits; i++)
		{
			if (!isalnum(hexOffset[i]) || (isalpha(hexOffset[i]) && toupper(hexOffset[i]) > 'F'))
			{
				hexOffset[i] = 0;	// we've reach a "wrong" character. Either start of a comment or something illegal. Either way, stop evaluation here.
				break;
			}
		}
		numDigits = strlen(hexOffset)-1;
		int32_t power = 0;
		for (int32_t count = numDigits;count >= 0;count--,power++)
		{
			uint8_t currentDigit = toupper(hexOffset[count]);
			
			if (currentDigit >= 'A' && currentDigit <= 'F')
			{
				result += (currentDigit - 'A' + 10)<<(4*power);
			}
			else if (currentDigit >= '0' && currentDigit <= '9')
			{
				result += (currentDigit - '0')<<(4*power);
			}
			else
			{
				//---------------------------------------------------------
				// There is a digit in here I don't understand.  Return 0.
				result = 0;
				break;
			}
		}
	}
	
	return(result);
}


//---------------------------------------------------------------------------
char FitIniFile::textToChar (PCSTR num)
{
	char result = 0;
	
	//------------------------------------
	// Check if Hex Number
	PSTR hexOffset = (PSTR )strstr(num,"0x");
	if (hexOffset == NULL)
	{
		result = atol(num);
	}
	else
	{
		hexOffset += 2;
		int32_t numDigits = strlen(hexOffset)-1;
		for (int i=0; i<=numDigits; i++)
		{
			if (!isalnum(hexOffset[i]) || (isalpha(hexOffset[i]) && toupper(hexOffset[i]) > 'F'))
			{
				hexOffset[i] = 0;	// we've reach a "wrong" character. Either start of a comment or something illegal. Either way, stop evaluation here.
				break;
			}
		}
		numDigits = strlen(hexOffset)-1;
		int32_t power = 0;
		for (int32_t count = numDigits;count >= 0;count--,power++)
		{
			uint8_t currentDigit = toupper(hexOffset[count]);
			
			if (currentDigit >= 'A' && currentDigit <= 'F')
			{
				result += (currentDigit - 'A' + 10)<<(4*power);
			}
			else if (currentDigit >= '0' && currentDigit <= '9')
			{
				result += (currentDigit - '0')<<(4*power);
			}
			else
			{
				//---------------------------------------------------------
				// There is a digit in here I don't understand.  Return 0.
				result = 0;
				break;
			}
		}
	}
	
	return(result);
}

//---------------------------------------------------------------------------
ULONG FitIniFile::textToULong (PCSTR num)
{
	ULONG result = 0;
	
	//------------------------------------
	// Check if Hex Number
	PSTR hexOffset = (PSTR )strstr(num,"0x");
	if (hexOffset == NULL)
	{
		result = atol(num);
	}
	else
	{
		hexOffset += 2;
		int32_t numDigits = strlen(hexOffset)-1;
		for (int i=0; i<=numDigits; i++)
		{
			if (!isalnum(hexOffset[i]) || (isalpha(hexOffset[i]) && toupper(hexOffset[i]) > 'F'))
			{
				hexOffset[i] = 0;	// we've reach a "wrong" character. Either start of a comment or something illegal. Either way, stop evaluation here.
				break;
			}
		}
		numDigits = strlen(hexOffset)-1;
		int32_t power = 0;
		for (int32_t count = numDigits;count >= 0;count--,power++)
		{
			uint8_t currentDigit = toupper(hexOffset[count]);
			
			if (currentDigit >= 'A' && currentDigit <= 'F')
			{
				result += (currentDigit - 'A' + 10)<<(4*power);
			}
			else if (currentDigit >= '0' && currentDigit <= '9')
			{
				result += (currentDigit - '0')<<(4*power);
			}
			else
			{
				//---------------------------------------------------------
				// There is a digit in here I don't understand.  Return 0.
				result = 0;
				break;
			}
		}
	}
	
	return(result);
}


//---------------------------------------------------------------------------
uint16_t FitIniFile::textToUShort (PCSTR num)
{
	uint16_t result = 0;
	
	//------------------------------------
	// Check if Hex Number
	PSTR hexOffset = (PSTR )strstr(num,"0x");
	if (hexOffset == NULL)
	{
		result = atol(num);
	}
	else
	{
		hexOffset += 2;
		int32_t numDigits = strlen(hexOffset)-1;
		for (int i=0; i<=numDigits; i++)
		{
			if (!isalnum(hexOffset[i]) || (isalpha(hexOffset[i]) && toupper(hexOffset[i]) > 'F'))
			{
				hexOffset[i] = 0;	// we've reach a "wrong" character. Either start of a comment or something illegal. Either way, stop evaluation here.
				break;
			}
		}
		numDigits = strlen(hexOffset)-1;
		int32_t power = 0;
		for (int32_t count = numDigits;count >= 0;count--,power++)
		{
			uint8_t currentDigit = toupper(hexOffset[count]);
			
			if (currentDigit >= 'A' && currentDigit <= 'F')
			{
				result += (currentDigit - 'A' + 10)<<(4*power);
			}
			else if (currentDigit >= '0' && currentDigit <= '9')
			{
				result += (currentDigit - '0')<<(4*power);
			}
			else
			{
				//---------------------------------------------------------
				// There is a digit in here I don't understand.  Return 0.
				result = 0;
				break;
			}
		}
	}
	
	return(result);
}


//---------------------------------------------------------------------------
uint8_t FitIniFile::textToUChar (PCSTR num)
{
	uint8_t result = 0;
	
	//------------------------------------
	// Check if Hex Number
	PSTR hexOffset = (PSTR )strstr(num,"0x");
	if (hexOffset == NULL)
	{
		result = atol(num);
	}
	else
	{
		hexOffset += 2;
		int32_t numDigits = strlen(hexOffset)-1;
		for (int i=0; i<=numDigits; i++)
		{
			if (!isalnum(hexOffset[i]) || (isalpha(hexOffset[i]) && toupper(hexOffset[i]) > 'F'))
			{
				hexOffset[i] = 0;	// we've reach a "wrong" character. Either start of a comment or something illegal. Either way, stop evaluation here.
				break;
			}
		}
		numDigits = strlen(hexOffset)-1;
		int32_t power = 0;
		for (int32_t count = numDigits;count >= 0;count--,power++)
		{
			uint8_t currentDigit = toupper(hexOffset[count]);
			
			if (currentDigit >= 'A' && currentDigit <= 'F')
			{
				result += (currentDigit - 'A' + 10)<<(4*power);
			}
			else if (currentDigit >= '0' && currentDigit <= '9')
			{
				result += (currentDigit - '0')<<(4*power);
			}
			else
			{
				//---------------------------------------------------------
				// There is a digit in here I don't understand.  Return 0.
				result = 0;
				break;
			}
		}
	}
	
	return(result);
}

//---------------------------------------------------------------------------
bool FitIniFile::booleanToLong (PCSTR num)
{
char testChar = 0;
	while (num[testChar] && isspace(num[testChar]))
		testChar++;

	if ((toupper(num[testChar]) == 'F') || (toupper(num[testChar]) == '0'))
		return FALSE;
	else
		return(TRUE);
}	

//---------------------------------------------------------------------------
int32_t FitIniFile::floatToText (PSTR result, float num, ULONG bufLen)
{
	char temp[250];
	sprintf(temp,"%f4",num);

	ULONG numLength = strlen(temp);
	if (numLength >= bufLen)
		return(BUFFER_TOO_SMALL);

	strncpy(result,temp,numLength);
	result[numLength] = '\0';
	
	return(NO_ERROR);
}			

//---------------------------------------------------------------------------
int32_t FitIniFile::longToTextDec (PSTR result, int32_t num, ULONG bufLen)
{
	char temp[250];
	sprintf(temp,"%d",num);

	ULONG numLength = strlen(temp);
	if (numLength >= bufLen)
		return(BUFFER_TOO_SMALL);

	strncpy(result,temp,numLength);
	result[numLength] = '\0';
	
	return(NO_ERROR);
}	

//---------------------------------------------------------------------------
int32_t FitIniFile::longToTextHex (PSTR result, int32_t num, ULONG bufLen)
{
	char temp[250];
	sprintf(temp,"0x%x",num);

	ULONG numLength = strlen(temp);
	if (numLength >= bufLen)
		return(BUFFER_TOO_SMALL);

	strncpy(result,temp,numLength);
	result[numLength] = '\0';
	
	return(NO_ERROR);
}	

//---------------------------------------------------------------------------
int32_t FitIniFile::shortToTextDec (PSTR result, short num, ULONG bufLen)
{
	char temp[250];
	sprintf(temp,"%d",num);

	ULONG numLength = strlen(temp);
	if (numLength >= bufLen)
		return(BUFFER_TOO_SMALL);

	strncpy(result,temp,numLength);
	result[numLength] = '\0';
	
	return(NO_ERROR);
}	

//---------------------------------------------------------------------------
int32_t FitIniFile::shortToTextHex (PSTR result, short num, ULONG bufLen)
{
	char temp[250];
	sprintf(temp,"0x%x",num);

	ULONG numLength = strlen(temp);
	if (numLength >= bufLen)
		return(BUFFER_TOO_SMALL);

	strncpy(result,temp,numLength);
	result[numLength] = '\0';
	
	return(NO_ERROR);
}	

//---------------------------------------------------------------------------
int32_t FitIniFile::byteToTextDec (PSTR result, byte num, ULONG bufLen)
{
	char temp[250];
	sprintf(temp,"%d",num);

	ULONG numLength = strlen(temp);
	if (numLength >= bufLen)
		return(BUFFER_TOO_SMALL);

	strncpy(result,temp,numLength);
	result[numLength] = '\0';
	
	return(NO_ERROR);
}	

//---------------------------------------------------------------------------
int32_t FitIniFile::byteToTextHex (PSTR result, byte num, ULONG bufLen)
{
	char temp[250];
	sprintf(temp,"0x%x",num);

	ULONG numLength = strlen(temp);
	if (numLength >= bufLen)
		return(BUFFER_TOO_SMALL);

	strncpy(result,temp,numLength);
	result[numLength] = '\0';
	
	return(NO_ERROR);
}	

//---------------------------------------------------------------------------
int32_t FitIniFile::open (PCSTR fName, FileMode _mode, int32_t numChild)
{
	int32_t result = File::open(fName,_mode,numChild);
	if (result != NO_ERROR)
		return(result);
		
	seek(0);

	result = afterOpen();
	
	return(result);
}

//---------------------------------------------------------------------------
int32_t FitIniFile::open (FilePtr _parent, ULONG fileSize, int32_t numChild)
{
	numChild = -1;		//Force all parented FitINIs to load from RAM.
	int32_t result = File::open(_parent,fileSize,numChild);
	if (result != NO_ERROR)
		return(result);
		
	result = afterOpen();
	
	return(result);
}

//---------------------------------------------------------------------------
int32_t FitIniFile::create (PSTR fName)
{
	int32_t result = File::create(fName);
	afterOpen();
	return(result);
}

int32_t FitIniFile::createWithCase( PSTR fName )
{
	int32_t result = File::createWithCase( fName );
	afterOpen();
	return result;
}

//---------------------------------------------------------------------------
void FitIniFile::close (void)
{
	if (isOpen())
	{
		atClose();
		File::close();
	}
}

//---------------------------------------------------------------------------
HRESULT FitIniFile::seekBlock(PCSTR blockId)
{
	ULONG blockNum = 0;
	
	while ((blockNum < totalBlocks) && (strcmp(fileBlocks[blockNum].blockId,blockId) != 0))
	{
		blockNum++;
	}
	
	if (blockNum == totalBlocks)
	{
		return(BLOCK_NOT_FOUND);
	}

	//----------------------------------------
	// Block was found, seek to that position
	seek(fileBlocks[blockNum].blockOffset);
	
	//----------------------------------------
	// Setup all current Block Info
	currentBlockId = fileBlocks[blockNum].blockId;
	currentBlockOffset = fileBlocks[blockNum].blockOffset;
	
	blockNum++;
	if (blockNum == totalBlocks)
	{
		currentBlockSize = getLength() - currentBlockOffset;
	}
	else
	{
		currentBlockSize = fileBlocks[blockNum].blockOffset - currentBlockOffset;
	}
		
	return(NO_ERROR);
}

//---------------------------------------------------------------------------
int32_t FitIniFile::readIdFloat (PCSTR varName, float &value)
{
	char line[255];
	char searchString[255];
	
	//--------------------------------
	// Always read from top of Block.
	seek(currentBlockOffset);
	ULONG endOfBlock = currentBlockOffset+currentBlockSize;
	
	//------------------------
	// Put prefix on varName.
	sprintf(searchString,"f %s",varName);
	
	//--------------------------------
	// Search line by line for varName
	int	testy;
	do
	{
		readLine((PUCHAR)line,254);
		testy = strnicmp(line, searchString, strlen(searchString));
		if (testy == 0)
		{
		PSTR tc = &line[strlen(searchString)];
			while (isspace(*tc))
				tc++;
			if (*tc != '=')
				testy = 1;
		}
	}
	while((testy != 0) && (logicalPosition < endOfBlock));
	
	if (logicalPosition >= endOfBlock)
	{
		value = 0.0;
		return(VARIABLE_NOT_FOUND);
	}
	
	PSTR equalSign = strstr(line, "=");
	if (equalSign)
	{
		equalSign++;
		value = textToFloat(equalSign);
	}
	else
	{
		return(SYNTAX_ERROR);
	}
	
	return(NO_ERROR);
}

//---------------------------------------------------------------------------
int32_t FitIniFile::readIdDouble (PCSTR varName, double &value)
{
	char line[255];
	char searchString[255];
	
	//--------------------------------
	// Always read from top of Block.
	seek(currentBlockOffset);
	ULONG endOfBlock = currentBlockOffset+currentBlockSize;
	
	//------------------------
	// Put prefix on varName.
	sprintf(searchString,"f %s",varName);
	
	//--------------------------------
	// Search line by line for varName
	int	testy;
	do
	{
		readLine((PUCHAR)line,254);
		testy = strnicmp(line, searchString, strlen(searchString));
		if (testy == 0)
		{
		PSTR tc = &line[strlen(searchString)];
			while (isspace(*tc))
				tc++;
			if (*tc != '=')
				testy = 1;
		}
	}
	while((testy != 0) && (logicalPosition < endOfBlock));
	
	if (logicalPosition >= endOfBlock)
	{
		value = 0.0;
		return(VARIABLE_NOT_FOUND);
	}
	
	PSTR equalSign = strstr(line, "=");
	if (equalSign)
	{
		equalSign++;
		value = textToDouble(equalSign);
	}
	else
	{
		return(SYNTAX_ERROR);
	}
	
	return(NO_ERROR);
}

//---------------------------------------------------------------------------
int32_t FitIniFile::readIdLong (PCSTR varName, int32_t &value)
{
	char line[255];
	char searchString[255];
	
	//--------------------------------
	// Always read from top of Block.
	seek(currentBlockOffset);
	ULONG endOfBlock = currentBlockOffset+currentBlockSize;
	
	//------------------------
	// Put prefix on varName.
	sprintf(searchString,"l %s",varName);
	
	//--------------------------------
	// Search line by line for varName
	int	testy;
	do
	{
		readLine((PUCHAR)line,254);
		testy = strnicmp(line, searchString, strlen(searchString));
		if (testy == 0)
		{
		PSTR tc = &line[strlen(searchString)];
			while (isspace(*tc))
				tc++;
			if (*tc != '=')
				testy = 1;
		}
	}
	while((testy != 0) && (logicalPosition < endOfBlock));
	
	if (logicalPosition >= endOfBlock)
	{
		value = 0;
		return(VARIABLE_NOT_FOUND);
	}
	
	PSTR equalSign = strstr(line, "=");
	if (equalSign)
	{
		equalSign++;
		value = textToLong(equalSign);
	}
	else
	{
		return(SYNTAX_ERROR);
	}
	
	return(NO_ERROR);
}

//---------------------------------------------------------------------------
int32_t FitIniFile::readIdBoolean (PCSTR varName, bool &value)
{
	char line[255];
	char searchString[255];
	
	//--------------------------------
	// Always read from top of Block.
	seek(currentBlockOffset);
	ULONG endOfBlock = currentBlockOffset+currentBlockSize;
	
	//------------------------
	// Put prefix on varName.
	sprintf(searchString,"b %s",varName);
	
	//--------------------------------
	// Search line by line for varName
	int	testy;
	do
	{
		readLine((PUCHAR)line,254);
		testy = strnicmp(line, searchString, strlen(searchString));
		if (testy == 0)
		{
		PSTR tc = &line[strlen(searchString)];
			while (isspace(*tc))
				tc++;
			if (*tc != '=')
				testy = 1;
		}
	}
	while((testy != 0) && (logicalPosition < endOfBlock));
	
	if (logicalPosition >= endOfBlock)
	{
		value = 0;
		return(VARIABLE_NOT_FOUND);
	}
	
	PSTR equalSign = strstr(line, "=");
	if (equalSign)
	{
		equalSign++;
		value = booleanToLong(equalSign);
	}
	else
	{
		return(SYNTAX_ERROR);
	}
	
	return(NO_ERROR);
}

//---------------------------------------------------------------------------
int32_t FitIniFile::readIdShort (PCSTR varName, short &value)
{
	char line[255];
	char searchString[255];
	
	//--------------------------------
	// Always read from top of Block.
	seek(currentBlockOffset);
	ULONG endOfBlock = currentBlockOffset+currentBlockSize;
	
	//------------------------
	// Put prefix on varName.
	sprintf(searchString,"s %s",varName);
	
	//--------------------------------
	// Search line by line for varName
	int	testy;
	do
	{
		readLine((PUCHAR)line,254);
		testy = strnicmp(line, searchString, strlen(searchString));
		if (testy == 0)
		{
		PSTR tc = &line[strlen(searchString)];
			while (isspace(*tc))
				tc++;
			if (*tc != '=')
				testy = 1;
		}
	}
	while((testy != 0) && (logicalPosition < endOfBlock));
	
	if (logicalPosition >= endOfBlock)
	{
		value = 0;
		return(VARIABLE_NOT_FOUND);
	}
	
	PSTR equalSign = strstr(line, "=");
	if (equalSign)
	{
		equalSign++;
		value = textToShort(equalSign);
	}
	else
	{
		return(SYNTAX_ERROR);
	}
	
	return(NO_ERROR);
}

//---------------------------------------------------------------------------
int32_t FitIniFile::readIdChar (PCSTR varName, char &value)
{
	char line[255];
	char searchString[255];
	
	//--------------------------------
	// Always read from top of Block.
	seek(currentBlockOffset);
	ULONG endOfBlock = currentBlockOffset+currentBlockSize;
	
	//------------------------
	// Put prefix on varName.
	sprintf(searchString,"c %s",varName);
	
	//--------------------------------
	// Search line by line for varName
	int	testy;
	do
	{
		readLine((PUCHAR)line,254);
		testy = strnicmp(line, searchString, strlen(searchString));
		if (testy == 0)
		{
		PSTR tc = &line[strlen(searchString)];
			while (isspace(*tc))
				tc++;
			if (*tc != '=')
				testy = 1;
		}
	}
	while((testy != 0) && (logicalPosition < endOfBlock));
	
	if (logicalPosition >= endOfBlock)
	{
		value = 0;
		return(VARIABLE_NOT_FOUND);
	}
	
	PSTR equalSign = strstr(line, "=");
	if (equalSign)
	{
		equalSign++;
		value = textToChar(equalSign);
	}
	else
	{
		return(SYNTAX_ERROR);
	}
	
	return(NO_ERROR);
}

//---------------------------------------------------------------------------
int32_t FitIniFile::readIdULong (PCSTR varName, ULONG &value)
{
	char line[255];
	char searchString[255];
	
	//--------------------------------
	// Always read from top of Block.
	seek(currentBlockOffset);
	ULONG endOfBlock = currentBlockOffset+currentBlockSize;
	
	//------------------------
	// Put prefix on varName.
	sprintf(searchString,"ul %s",varName);
	
	//--------------------------------
	// Search line by line for varName
	int	testy;
	do
	{
		readLine((PUCHAR)line,254);
		testy = strnicmp(line, searchString, strlen(searchString));
		if (testy == 0)
		{
			PSTR tc = &line[strlen(searchString)];
			
			while (isspace(*tc))
				tc++;
				
			if (*tc != '=')
				testy = 1;
		}
	}
	while((testy != 0) && (logicalPosition < endOfBlock));
	
	if (logicalPosition >= endOfBlock)
	{
		value = 0;
		return(VARIABLE_NOT_FOUND);
	}
	
	PSTR equalSign = strstr(line, "=");
	if (equalSign)
	{
		equalSign++;
		value = textToULong(equalSign);
	}
	else
	{
		return(SYNTAX_ERROR);
	}
	
	return(NO_ERROR);
}

//---------------------------------------------------------------------------
int32_t FitIniFile::readIdUShort (PCSTR varName, uint16_t &value)
{
	char line[255];
	char searchString[255];
	
	//--------------------------------
	// Always read from top of Block.
	seek(currentBlockOffset);
	ULONG endOfBlock = currentBlockOffset+currentBlockSize;
	
	//------------------------
	// Put prefix on varName.
	sprintf(searchString,"us %s",varName);
	
	//--------------------------------
	// Search line by line for varName
	int	testy;
	do
	{
		readLine((PUCHAR)line,254);
		testy = strnicmp(line, searchString, strlen(searchString));
		if (testy == 0)
		{
		PSTR tc = &line[strlen(searchString)];
			while (isspace(*tc))
				tc++;
			if (*tc != '=')
				testy = 1;
		}
	}
	while((testy != 0) && (logicalPosition < endOfBlock));
	
	if (logicalPosition >= endOfBlock)
	{
		value = 0;
		return(VARIABLE_NOT_FOUND);
	}
	
	PSTR equalSign = strstr(line, "=");
	if (equalSign)
	{
		equalSign++;
		value = textToUShort(equalSign);
	}
	else
	{
		return(SYNTAX_ERROR);
	}
	
	return(NO_ERROR);
}

//---------------------------------------------------------------------------
int32_t FitIniFile::readIdUChar (PCSTR varName, uint8_t &value)
{
	char line[255];
	char searchString[255];
	
	//--------------------------------
	// Always read from top of Block.
	seek(currentBlockOffset);
	ULONG endOfBlock = currentBlockOffset+currentBlockSize;
	
	//------------------------
	// Put prefix on varName.
	sprintf(searchString,"uc %s",varName);
	
	//--------------------------------
	// Search line by line for varName
	int	testy;
	do
	{
		readLine((PUCHAR)line,254);
		testy = strnicmp(line, searchString, strlen(searchString));
		if (testy == 0)
		{
		PSTR tc = &line[strlen(searchString)];
			while (isspace(*tc))
				tc++;
			if (*tc != '=')
				testy = 1;
		}
	}
	while((testy != 0) && (logicalPosition < endOfBlock));
	
	if (logicalPosition >= endOfBlock)
	{
		value = 0;
		return(VARIABLE_NOT_FOUND);
	}
	
	PSTR equalSign = strstr(line, "=");
	if (equalSign)
	{
		equalSign++;
		value = textToUChar(equalSign);
	}
	else
	{
		return(SYNTAX_ERROR);
	}
	
	return(NO_ERROR);
}

//---------------------------------------------------------------------------
int32_t FitIniFile::copyString (PSTR dest, PSTR src, ULONG bufLen)
{
	ULONG offset = 0;
	//---------------------
	// Find starting Quote
	while (*src != '"')
	{
		src++;
	}
	
	src++;		//One more to get to first character past quote.
	
	//---------------------------------------
	// Copy each character until close quote
	while (*src != '"' && offset < bufLen)
	{
		dest[offset] = *src;
		src++;
		offset++;
	}

	//----------------------------------------------------
	// If this string is longer than buffer, let em know.
	if (offset == bufLen)
	{
		return(BUFFER_TOO_SMALL);
	}

	//---------------------------------
	// otherwise, NULL term and return	
	dest[offset] = '\0';
	return(NO_ERROR);
}

//---------------------------------------------------------------------------
int32_t FitIniFile::readIdString (PCSTR varName, PSTR result, ULONG bufferSize)
{
	char line[2048];
	char searchString[255];
	
	//--------------------------------
	// Always read from top of Block.
	seek(currentBlockOffset);
	ULONG endOfBlock = currentBlockOffset+currentBlockSize;
	
	//------------------------
	// Put prefix on varName.
	sprintf(searchString,"st %s",varName);
	
	//--------------------------------
	// Search line by line for varName
	int	testy;
	do
	{
		readLine((PUCHAR)line,2047);
		testy = strnicmp(line, searchString, strlen(searchString));
		if (testy == 0)
		{
		PSTR tc = &line[strlen(searchString)];
			while (isspace(*tc))
				tc++;
			if (*tc != '=')
				testy = 1;
		}
	}
	while((testy != 0) && (logicalPosition < endOfBlock));
	
	if (logicalPosition >= endOfBlock)
	{
		return(VARIABLE_NOT_FOUND);
	}
	
	PSTR equalSign = strstr(line, "=");
	if (equalSign)
	{
		equalSign++;

		

		PSTR pFound = NULL;
		PSTR pFirstEqual = strstr( equalSign, "\"" );
		// strings can span more than one line, make sure there is another equal sign
		if ( pFirstEqual && !strstr( pFirstEqual+1, "\"" ) ) 
		{
			int32_t curLen = strlen( equalSign );
			char tmpLine[2047];
			do
			{
				readLine((PUCHAR)tmpLine,2047);
				pFound = strstr( tmpLine, "\"" );

				int32_t addedLen = strlen( tmpLine ) + 1;
				if ( curLen + addedLen < 2048 )
				{
					strcat( equalSign, tmpLine );
					strcat( equalSign, "\n" );
					curLen += addedLen;
				}
				else
					break;

			}while( !pFound && (logicalPosition < endOfBlock) );
		}
		
		int32_t errorCode = copyString(result,equalSign,bufferSize);
		if (errorCode != NO_ERROR)
			return(errorCode);
	}
	else
	{
		return(SYNTAX_ERROR);
	}
	
	
	return(NO_ERROR);
}

//---------------------------------------------------------------------------
int32_t FitIniFile::getIdStringLength (PCSTR varName)
{
	char line[255];
	char searchString[255];
	
	//--------------------------------
	// Always read from top of Block.
	seek(currentBlockOffset);
	ULONG endOfBlock = currentBlockOffset+currentBlockSize;
	
	//------------------------
	// Put prefix on varName.
	sprintf(searchString,"st %s",varName);
	
	//--------------------------------
	// Search line by line for varName
	int	testy;
	do
	{
		readLine((PUCHAR)line,254);
		testy = strnicmp(line, searchString, strlen(searchString));
		if (testy == 0)
		{
		PSTR tc = &line[strlen(searchString)];
			while (isspace(*tc))
				tc++;
			if (*tc != '=')
				testy = 1;
		}
	}
	while((testy != 0) && (logicalPosition < endOfBlock));
	
	if (logicalPosition >= endOfBlock)
	{
		return(VARIABLE_NOT_FOUND);
	}
	
	PSTR equalSign = strchr(line, '"');
	if (equalSign)
	{
		equalSign++;
		PSTR end = equalSign;
		while (*end != '"' && *end != 0)
			end++;
		if (*end)
			return (end - equalSign + 1);	// + for terminator
		else
			return(SYNTAX_ERROR);
	}

	return(SYNTAX_ERROR);
}

//---------------------------------------------------------------------------
int32_t FitIniFile::readIdFloatArray (PCSTR varName, float *result, ULONG numElements)
{
	char line[255];
	char frontSearch[10];
	char searchString[255];
	
	//--------------------------------
	// Always read from top of Block.
	seek(currentBlockOffset);
	ULONG endOfBlock = currentBlockOffset+currentBlockSize;
	
	//------------------------------------------------------------------
	// Create two search strings so that we can match any number in []
	sprintf(frontSearch,"f[");
	sprintf(searchString,"] %s",varName);
	
	//--------------------------------
	// Search line by line for varName
	PSTR fSearch = NULL;
	PSTR bSearch = NULL;
	
	do
	{
		readLine((PUCHAR)line,254);
		
		fSearch = strstr(line,frontSearch);
		bSearch = strstr(line,searchString);
	}
	while(((fSearch == NULL) || (bSearch == NULL)) && (logicalPosition < endOfBlock));
	
	if (logicalPosition >= endOfBlock)
	{
		return(VARIABLE_NOT_FOUND);
	}

	//--------------------------------------
	// Get number of elements in array.
	char elementString[10];
	ULONG actualElements;
	
	fSearch += 2;												//Move pointer to first number in brackets.
	int32_t numDigits = bSearch - fSearch;

	if (numDigits > 9)
		return(TOO_MANY_ELEMENTS);

	strncpy(elementString,fSearch,numDigits);
	elementString[numDigits] = '\0';
	
	actualElements = textToULong(elementString);	

	if (actualElements > numElements)
		return(USER_ARRAY_TOO_SMALL);
		
	//------------------------------
	// Parse out the elements here.
	PSTR equalSign = strstr(line, "=");
	ULONG elementsRead = 0;
	if (equalSign)
	{
		equalSign++; //Move to char past equal sign.

		//--------------------------------------------------------------------------------
		// Now, loop until we reach the end of block or we've read in all of the elements
		while ((logicalPosition < endOfBlock) && (elementsRead < actualElements))
		{
			int32_t errorCode = getNextWord(equalSign,elementString,9);
			if (errorCode == GET_NEXT_LINE)
			{
				readLine((PUCHAR)line,254);
				equalSign = line;
				continue;
			}

			if (errorCode != NO_ERROR)
			{
				return(errorCode);
			}
			
			result[elementsRead] = textToFloat(elementString);
			elementsRead++;
		}
		
		if (logicalPosition >= endOfBlock && elementsRead < actualElements)
			return(NOT_ENOUGH_ELEMENTS_FOR_ARRAY);
	}
	else
	{
		return(SYNTAX_ERROR);
	}
		
	return(NO_ERROR);
}
		
//---------------------------------------------------------------------------
int32_t FitIniFile::readIdLongArray (PCSTR varName, int32_t *result, ULONG numElements)
{
	char line[255];
	char frontSearch[10];
	char searchString[255];
	
	//--------------------------------
	// Always read from top of Block.
	seek(currentBlockOffset);
	ULONG endOfBlock = currentBlockOffset+currentBlockSize;
	
	//------------------------------------------------------------------
	// Create two search strings so that we can match any number in []
	sprintf(frontSearch,"l[");
	sprintf(searchString,"] %s",varName);
	
	//--------------------------------
	// Search line by line for varName
	PSTR fSearch = NULL;
	PSTR bSearch = NULL;
	
	do
	{
		readLine((PUCHAR)line,254);
		
		fSearch = strstr(line,frontSearch);
		bSearch = strstr(line,searchString);
	}
	while(((fSearch == NULL) || (bSearch == NULL)) && (logicalPosition < endOfBlock));
	
	if (logicalPosition >= endOfBlock)
	{
		return(VARIABLE_NOT_FOUND);
	}

	//--------------------------------------
	// Get number of elements in array.
	char elementString[10];
	ULONG actualElements;
	
	fSearch += 2;												//Move pointer to first number in brackets.
	int32_t numDigits = bSearch - fSearch;

	if (numDigits > 9)
		return(TOO_MANY_ELEMENTS);

	strncpy(elementString,fSearch,numDigits);
	elementString[numDigits] = '\0';
	
	actualElements = textToULong(elementString);	

	if (actualElements > numElements)
		return(USER_ARRAY_TOO_SMALL);
		
	//------------------------------
	// Parse out the elements here.
	PSTR equalSign = strstr(line, "=");
	ULONG elementsRead = 0;
	if (equalSign)
	{
		equalSign++; //Move to char past equal sign.

		//--------------------------------------------------------------------------------
		// Now, loop until we reach the end of block or we've read in all of the elements
		while ((logicalPosition < endOfBlock) && (elementsRead < actualElements))
		{
			int32_t errorCode = getNextWord(equalSign,elementString,9);
			if (errorCode == GET_NEXT_LINE)
			{
				readLine((PUCHAR)line,254);
				equalSign = line;
				continue;
			}

			if (errorCode != NO_ERROR)
			{
				return(errorCode);
			}
			
			result[elementsRead] = textToLong(elementString);
			elementsRead++;
		}
		
		if (logicalPosition >= endOfBlock && elementsRead < actualElements)
			return(NOT_ENOUGH_ELEMENTS_FOR_ARRAY);
	}
	else
	{
		return(SYNTAX_ERROR);
	}
		
	return(NO_ERROR);
}

//---------------------------------------------------------------------------
int32_t FitIniFile::readIdULongArray (PCSTR varName, ULONG *result, ULONG numElements)
{
	char line[255];
	char frontSearch[10];
	char searchString[255];
	
	//--------------------------------
	// Always read from top of Block.
	seek(currentBlockOffset);
	ULONG endOfBlock = currentBlockOffset+currentBlockSize;
	
	//------------------------------------------------------------------
	// Create two search strings so that we can match any number in []
	sprintf(frontSearch,"ul[");
	sprintf(searchString,"] %s",varName);
	
	//--------------------------------
	// Search line by line for varName
	PSTR fSearch = NULL;
	PSTR bSearch = NULL;
	
	do
	{
		readLine((PUCHAR)line,254);
		
		fSearch = strstr(line,frontSearch);
		bSearch = strstr(line,searchString);
	}
	while(((fSearch == NULL) || (bSearch == NULL)) && (logicalPosition < endOfBlock));
	
	if (logicalPosition >= endOfBlock)
	{
		return(VARIABLE_NOT_FOUND);
	}

	//--------------------------------------
	// Get number of elements in array.
	char elementString[10];
	ULONG actualElements;
	
	fSearch += 3;												//Move pointer to first number in brackets.
	int32_t numDigits = bSearch - fSearch;

	if (numDigits > 9)
		return(TOO_MANY_ELEMENTS);

	strncpy(elementString,fSearch,numDigits);
	elementString[numDigits] = '\0';
	
	actualElements = textToULong(elementString);	

	if (actualElements > numElements)
		return(USER_ARRAY_TOO_SMALL);
		
	//------------------------------
	// Parse out the elements here.
	PSTR equalSign = strstr(line, "=");
	ULONG elementsRead = 0;
	if (equalSign)
	{
		equalSign++; //Move to char past equal sign.

		//--------------------------------------------------------------------------------
		// Now, loop until we reach the end of block or we've read in all of the elements
		while ((logicalPosition < endOfBlock) && (elementsRead < actualElements))
		{
			int32_t errorCode = getNextWord(equalSign,elementString,9);
			if (errorCode == GET_NEXT_LINE)
			{
				readLine((PUCHAR)line,254);
				equalSign = line;
				continue;
			}

			if (errorCode != NO_ERROR)
			{
				return(errorCode);
			}
			
			result[elementsRead] = textToULong(elementString);
			elementsRead++;
		}
		
		if (logicalPosition >= endOfBlock && elementsRead < actualElements)
			return(NOT_ENOUGH_ELEMENTS_FOR_ARRAY);
	}
	else
	{
		return(SYNTAX_ERROR);
	}
		
	return(NO_ERROR);
}
		
//---------------------------------------------------------------------------
int32_t FitIniFile::readIdShortArray (PCSTR varName, short *result, ULONG numElements)
{
	char line[255];
	char frontSearch[10];
	char searchString[255];
	
	//--------------------------------
	// Always read from top of Block.
	seek(currentBlockOffset);
	ULONG endOfBlock = currentBlockOffset+currentBlockSize;
	
	//------------------------------------------------------------------
	// Create two search strings so that we can match any number in []
	sprintf(frontSearch,"s[");
	sprintf(searchString,"] %s",varName);
	
	//--------------------------------
	// Search line by line for varName
	PSTR fSearch = NULL;
	PSTR bSearch = NULL;
	
	do
	{
		readLine((PUCHAR)line,254);
		
		fSearch = strstr(line,frontSearch);
		bSearch = strstr(line,searchString);
	}
	while(((fSearch == NULL) || (bSearch == NULL)) && (logicalPosition < endOfBlock));
	
	if (logicalPosition >= endOfBlock)
	{
		return(VARIABLE_NOT_FOUND);
	}

	//--------------------------------------
	// Get number of elements in array.
	char elementString[10];
	ULONG actualElements;
	
	fSearch += 2;												//Move pointer to first number in brackets.
	int32_t numDigits = bSearch - fSearch;

	if (numDigits > 9)
		return(TOO_MANY_ELEMENTS);

	strncpy(elementString,fSearch,numDigits);
	elementString[numDigits] = '\0';
	
	actualElements = textToULong(elementString);	

	if (actualElements > numElements)
		return(USER_ARRAY_TOO_SMALL);
		
	//------------------------------
	// Parse out the elements here.
	PSTR equalSign = strstr(line, "=");
	ULONG elementsRead = 0;
	if (equalSign)
	{
		equalSign++; //Move to char past equal sign.

		//--------------------------------------------------------------------------------
		// Now, loop until we reach the end of block or we've read in all of the elements
		while ((logicalPosition < endOfBlock) && (elementsRead < actualElements))
		{
			int32_t errorCode = getNextWord(equalSign,elementString,9);
			if (errorCode == GET_NEXT_LINE)
			{
				readLine((PUCHAR)line,254);
				equalSign = line;
				continue;
			}

			if (errorCode != NO_ERROR)
			{
				return(errorCode);
			}
			
			result[elementsRead] = textToShort(elementString);
			elementsRead++;
		}
		
		if (logicalPosition >= endOfBlock && elementsRead < actualElements)
			return(NOT_ENOUGH_ELEMENTS_FOR_ARRAY);
	}
	else
	{
		return(SYNTAX_ERROR);
	}
		
	return(NO_ERROR);
}

//---------------------------------------------------------------------------
int32_t FitIniFile::readIdUShortArray (PCSTR varName, puint16_t result, ULONG numElements)
{
	char line[255];
	char frontSearch[10];
	char searchString[255];
	
	//--------------------------------
	// Always read from top of Block.
	seek(currentBlockOffset);
	ULONG endOfBlock = currentBlockOffset+currentBlockSize;
	
	//------------------------------------------------------------------
	// Create two search strings so that we can match any number in []
	sprintf(frontSearch,"us[");
	sprintf(searchString,"] %s",varName);
	
	//--------------------------------
	// Search line by line for varName
	PSTR fSearch = NULL;
	PSTR bSearch = NULL;
	
	do
	{
		readLine((PUCHAR)line,254);
		
		fSearch = strstr(line,frontSearch);
		bSearch = strstr(line,searchString);
	}
	while(((fSearch == NULL) || (bSearch == NULL)) && (logicalPosition < endOfBlock));
	
	if (logicalPosition >= endOfBlock)
	{
		return(VARIABLE_NOT_FOUND);
	}

	//--------------------------------------
	// Get number of elements in array.
	char elementString[10];
	ULONG actualElements;
	
	fSearch += 3;												//Move pointer to first number in brackets.
	int32_t numDigits = bSearch - fSearch;

	if (numDigits > 9)
		return(TOO_MANY_ELEMENTS);

	strncpy(elementString,fSearch,numDigits);
	elementString[numDigits] = '\0';
	
	actualElements = textToULong(elementString);	

	if (actualElements > numElements)
		return(USER_ARRAY_TOO_SMALL);
		
	//------------------------------
	// Parse out the elements here.
	PSTR equalSign = strstr(line, "=");
	ULONG elementsRead = 0;
	if (equalSign)
	{
		equalSign++; //Move to char past equal sign.

		//--------------------------------------------------------------------------------
		// Now, loop until we reach the end of block or we've read in all of the elements
		while ((logicalPosition < endOfBlock) && (elementsRead < actualElements))
		{
			int32_t errorCode = getNextWord(equalSign,elementString,9);
			if (errorCode == GET_NEXT_LINE)
			{
				readLine((PUCHAR)line,254);
				equalSign = line;
				continue;
			}

			if (errorCode != NO_ERROR)
			{
				return(errorCode);
			}
			
			result[elementsRead] = textToUShort(elementString);
			elementsRead++;
		}
		
		if (logicalPosition >= endOfBlock && elementsRead < actualElements)
			return(NOT_ENOUGH_ELEMENTS_FOR_ARRAY);
	}
	else
	{
		return(SYNTAX_ERROR);
	}
		
	return(NO_ERROR);
}
		
//---------------------------------------------------------------------------
int32_t FitIniFile::readIdCharArray (PCSTR varName, PSTR result, ULONG numElements)
{
	char line[255];
	char frontSearch[10];
	char searchString[255];
	
	//--------------------------------
	// Always read from top of Block.
	seek(currentBlockOffset);
	ULONG endOfBlock = currentBlockOffset+currentBlockSize;
	
	//------------------------------------------------------------------
	// Create two search strings so that we can match any number in []
	sprintf(frontSearch,"c[");
	sprintf(searchString,"] %s",varName);
	
	//--------------------------------
	// Search line by line for varName
	PSTR fSearch = NULL;
	PSTR bSearch = NULL;
	
	do
	{
		readLine((PUCHAR)line,254);
		
		fSearch = strstr(line,frontSearch);
		bSearch = strstr(line,searchString);
	}
	while(((fSearch == NULL) || (bSearch == NULL)) && (logicalPosition < endOfBlock));
	
	if (logicalPosition >= endOfBlock)
	{
		return(VARIABLE_NOT_FOUND);
	}

	//--------------------------------------
	// Get number of elements in array.
	char elementString[10];
	ULONG actualElements;
	
	fSearch += 2;												//Move pointer to first number in brackets.
	int32_t numDigits = bSearch - fSearch;

	if (numDigits > 9)
		return(TOO_MANY_ELEMENTS);

	strncpy(elementString,fSearch,numDigits);
	elementString[numDigits] = '\0';
	
	actualElements = textToULong(elementString);	

	if (actualElements > numElements)
		return(USER_ARRAY_TOO_SMALL);
		
	//------------------------------
	// Parse out the elements here.
	PSTR equalSign = strstr(line, "=");
	ULONG elementsRead = 0;
	if (equalSign)
	{
		equalSign++; //Move to char past equal sign.

		//--------------------------------------------------------------------------------
		// Now, loop until we reach the end of block or we've read in all of the elements
		while ((logicalPosition < endOfBlock) && (elementsRead < actualElements))
		{
			int32_t errorCode = getNextWord(equalSign,elementString,9);
			if (errorCode == GET_NEXT_LINE)
			{
				readLine((PUCHAR)line,254);
				equalSign = line;
				continue;
			}

			if (errorCode != NO_ERROR)
			{
				return(errorCode);
			}
			
			result[elementsRead] = textToChar(elementString);
			elementsRead++;
		}
		
		if (logicalPosition >= endOfBlock && elementsRead < actualElements)
			return(NOT_ENOUGH_ELEMENTS_FOR_ARRAY);
	}
	else
	{
		return(SYNTAX_ERROR);
	}
		
	return(NO_ERROR);
}

//---------------------------------------------------------------------------
int32_t FitIniFile::readIdUCharArray (PCSTR varName, puint8_t result, ULONG numElements)
{
	char line[255];
	char frontSearch[10];
	char searchString[255];
	
	//--------------------------------
	// Always read from top of Block.
	seek(currentBlockOffset);
	ULONG endOfBlock = currentBlockOffset+currentBlockSize;
	
	//------------------------------------------------------------------
	// Create two search strings so that we can match any number in []
	sprintf(frontSearch,"uc[");
	sprintf(searchString,"] %s",varName);
	
	//--------------------------------
	// Search line by line for varName
	PSTR fSearch = NULL;
	PSTR bSearch = NULL;
	
	do
	{
		readLine((PUCHAR)line,254);
		
		fSearch = strstr(line,frontSearch);
		bSearch = strstr(line,searchString);
	}
	while(((fSearch == NULL) || (bSearch == NULL)) && (logicalPosition < endOfBlock));
	
	if (logicalPosition >= endOfBlock)
	{
		return(VARIABLE_NOT_FOUND);
	}

	//--------------------------------------
	// Get number of elements in array.
	char elementString[10];
	ULONG actualElements;
	
	fSearch += 3;												//Move pointer to first number in brackets.
	int32_t numDigits = bSearch - fSearch;

	if (numDigits > 9)
		return(TOO_MANY_ELEMENTS);

	strncpy(elementString,fSearch,numDigits);
	elementString[numDigits] = '\0';
	
	actualElements = textToULong(elementString);	

	if (actualElements > numElements)
		return(USER_ARRAY_TOO_SMALL);
		
	//------------------------------
	// Parse out the elements here.
	PSTR equalSign = strstr(line, "=");
	ULONG elementsRead = 0;
	if (equalSign)
	{
		equalSign++; //Move to char past equal sign.

		//--------------------------------------------------------------------------------
		// Now, loop until we reach the end of block or we've read in all of the elements
		while ((logicalPosition < endOfBlock) && (elementsRead < actualElements))
		{
			int32_t errorCode = getNextWord(equalSign,elementString,9);
			if (errorCode == GET_NEXT_LINE)
			{
				readLine((PUCHAR)line,254);
				equalSign = line;
				continue;
			}

			if (errorCode != NO_ERROR)
			{
				return(errorCode);
			}
			
			result[elementsRead] = textToUChar(elementString);
			elementsRead++;
		}
		
		if (logicalPosition >= endOfBlock && elementsRead < actualElements)
			return(NOT_ENOUGH_ELEMENTS_FOR_ARRAY);
	}
	else
	{
		return(SYNTAX_ERROR);
	}
		
	return(NO_ERROR);
}
		
//---------------------------------------------------------------------------
ULONG FitIniFile::getIdFloatArrayElements (PCSTR varName)
{
	char line[255];
	char frontSearch[10];
	char searchString[255];
	
	//--------------------------------
	// Always read from top of Block.
	seek(currentBlockOffset);
	ULONG endOfBlock = currentBlockOffset+currentBlockSize;
	
	//------------------------------------------------------------------
	// Create two search strings so that we can match any number in []
	sprintf(frontSearch,"f[");
	sprintf(searchString,"] %s",varName);
	
	//--------------------------------
	// Search line by line for varName
	PSTR fSearch = NULL;
	PSTR bSearch = NULL;
	
	do
	{
		readLine((PUCHAR)line,254);
		
		fSearch = strstr(line,frontSearch);
		bSearch = strstr(line,searchString);
	}
	while(((fSearch == NULL) || (bSearch == NULL)) && (logicalPosition < endOfBlock));
	
	if (logicalPosition >= endOfBlock)
	{
		return(VARIABLE_NOT_FOUND);
	}

	//--------------------------------------
	// Get number of elements in array.
	char elementString[10];
	ULONG actualElements;
	
	fSearch += 2;												//Move pointer to first number in brackets.
	int32_t numDigits = bSearch - fSearch;

	if (numDigits > 9)
		return(TOO_MANY_ELEMENTS);

	strncpy(elementString,fSearch,numDigits);
	elementString[numDigits] = '\0';
	
	actualElements = textToULong(elementString);	
	
	return(actualElements);
}	
	
//---------------------------------------------------------------------------
ULONG FitIniFile::getIdLongArrayElements (PCSTR varName)
{
	char line[255];
	char frontSearch[10];
	char searchString[255];
	
	//--------------------------------
	// Always read from top of Block.
	seek(currentBlockOffset);
	ULONG endOfBlock = currentBlockOffset+currentBlockSize;
	
	//------------------------------------------------------------------
	// Create two search strings so that we can match any number in []
	sprintf(frontSearch,"l[");
	sprintf(searchString,"] %s",varName);
	
	//--------------------------------
	// Search line by line for varName
	PSTR fSearch = NULL;
	PSTR bSearch = NULL;
	
	do
	{
		readLine((PUCHAR)line,254);
		
		fSearch = strstr(line,frontSearch);
		bSearch = strstr(line,searchString);
	}
	while(((fSearch == NULL) || (bSearch == NULL)) && (logicalPosition < endOfBlock));
	
	if (logicalPosition >= endOfBlock)
	{
		return(VARIABLE_NOT_FOUND);
	}

	//--------------------------------------
	// Get number of elements in array.
	char elementString[10];
	ULONG actualElements;
	
	fSearch += 2;												//Move pointer to first number in brackets.
	int32_t numDigits = bSearch - fSearch;

	if (numDigits > 9)
		return(TOO_MANY_ELEMENTS);

	strncpy(elementString,fSearch,numDigits);
	elementString[numDigits] = '\0';
	
	actualElements = textToULong(elementString);	
	
	return(actualElements);
}

//---------------------------------------------------------------------------
ULONG FitIniFile::getIdULongArrayElements (PCSTR varName)
{
	char line[255];
	char frontSearch[10];
	char searchString[255];
	
	//--------------------------------
	// Always read from top of Block.
	seek(currentBlockOffset);
	ULONG endOfBlock = currentBlockOffset+currentBlockSize;
	
	//------------------------------------------------------------------
	// Create two search strings so that we can match any number in []
	sprintf(frontSearch,"ul[");
	sprintf(searchString,"] %s",varName);
	
	//--------------------------------
	// Search line by line for varName
	PSTR fSearch = NULL;
	PSTR bSearch = NULL;
	
	do
	{
		readLine((PUCHAR)line,254);
		
		fSearch = strstr(line,frontSearch);
		bSearch = strstr(line,searchString);
	}
	while(((fSearch == NULL) || (bSearch == NULL)) && (logicalPosition < endOfBlock));
	
	if (logicalPosition >= endOfBlock)
	{
		return(VARIABLE_NOT_FOUND);
	}

	//--------------------------------------
	// Get number of elements in array.
	char elementString[10];
	ULONG actualElements;
	
	fSearch += 3;												//Move pointer to first number in brackets.
	int32_t numDigits = bSearch - fSearch;

	if (numDigits > 9)
		return(TOO_MANY_ELEMENTS);

	strncpy(elementString,fSearch,numDigits);
	elementString[numDigits] = '\0';
	
	actualElements = textToULong(elementString);	
	
	return(actualElements);
}
											 
//---------------------------------------------------------------------------
ULONG FitIniFile::getIdShortArrayElements (PCSTR varName)
{
	char line[255];
	char frontSearch[10];
	char searchString[255];
	
	//--------------------------------
	// Always read from top of Block.
	seek(currentBlockOffset);
	ULONG endOfBlock = currentBlockOffset+currentBlockSize;
	
	//------------------------------------------------------------------
	// Create two search strings so that we can match any number in []
	sprintf(frontSearch,"s[");
	sprintf(searchString,"] %s",varName);
	
	//--------------------------------
	// Search line by line for varName
	PSTR fSearch = NULL;
	PSTR bSearch = NULL;
	
	do
	{
		readLine((PUCHAR)line,254);
		
		fSearch = strstr(line,frontSearch);
		bSearch = strstr(line,searchString);
	}
	while(((fSearch == NULL) || (bSearch == NULL)) && (logicalPosition < endOfBlock));
	
	if (logicalPosition >= endOfBlock)
	{
		return(VARIABLE_NOT_FOUND);
	}

	//--------------------------------------
	// Get number of elements in array.
	char elementString[10];
	ULONG actualElements;
	
	fSearch += 2;												//Move pointer to first number in brackets.
	int32_t numDigits = bSearch - fSearch;

	if (numDigits > 9)
		return(TOO_MANY_ELEMENTS);

	strncpy(elementString,fSearch,numDigits);
	elementString[numDigits] = '\0';
	
	actualElements = textToULong(elementString);	
	
	return(actualElements);
}

//---------------------------------------------------------------------------
ULONG FitIniFile::getIdUShortArrayElements (PCSTR varName)
{
	char line[255];
	char frontSearch[10];
	char searchString[255];
	
	//--------------------------------
	// Always read from top of Block.
	seek(currentBlockOffset);
	ULONG endOfBlock = currentBlockOffset+currentBlockSize;
	
	//------------------------------------------------------------------
	// Create two search strings so that we can match any number in []
	sprintf(frontSearch,"us[");
	sprintf(searchString,"] %s",varName);
	
	//--------------------------------
	// Search line by line for varName
	PSTR fSearch = NULL;
	PSTR bSearch = NULL;
	
	do
	{
		readLine((PUCHAR)line,254);
		
		fSearch = strstr(line,frontSearch);
		bSearch = strstr(line,searchString);
	}
	while(((fSearch == NULL) || (bSearch == NULL)) && (logicalPosition < endOfBlock));
	
	if (logicalPosition >= endOfBlock)
	{
		return(VARIABLE_NOT_FOUND);
	}

	//--------------------------------------
	// Get number of elements in array.
	char elementString[10];
	ULONG actualElements;
	
	fSearch += 3;												//Move pointer to first number in brackets.
	int32_t numDigits = bSearch - fSearch;

	if (numDigits > 9)
		return(TOO_MANY_ELEMENTS);

	strncpy(elementString,fSearch,numDigits);
	elementString[numDigits] = '\0';
	
	actualElements = textToULong(elementString);	
	
	return(actualElements);
}

//---------------------------------------------------------------------------
ULONG FitIniFile::getIdCharArrayElements (PCSTR varName)
{
	char line[255];
	char frontSearch[10];
	char searchString[255];
	
	//--------------------------------
	// Always read from top of Block.
	seek(currentBlockOffset);
	ULONG endOfBlock = currentBlockOffset+currentBlockSize;
	
	//------------------------------------------------------------------
	// Create two search strings so that we can match any number in []
	sprintf(frontSearch,"c[");
	sprintf(searchString,"] %s",varName);
	
	//--------------------------------
	// Search line by line for varName
	PSTR fSearch = NULL;
	PSTR bSearch = NULL;
	
	do
	{
		readLine((PUCHAR)line,254);
		
		fSearch = strstr(line,frontSearch);
		bSearch = strstr(line,searchString);
	}
	while(((fSearch == NULL) || (bSearch == NULL)) && (logicalPosition < endOfBlock));
	
	if (logicalPosition >= endOfBlock)
	{
		return(VARIABLE_NOT_FOUND);
	}

	//--------------------------------------
	// Get number of elements in array.
	char elementString[10];
	ULONG actualElements;
	
	fSearch += 2;												//Move pointer to first number in brackets.
	int32_t numDigits = bSearch - fSearch;

	if (numDigits > 9)
		return(TOO_MANY_ELEMENTS);

	strncpy(elementString,fSearch,numDigits);
	elementString[numDigits] = '\0';
	
	actualElements = textToULong(elementString);	
	
	return(actualElements);
}

//---------------------------------------------------------------------------
ULONG FitIniFile::getIdUCharArrayElements (PCSTR varName)
{
	char line[255];
	char frontSearch[10];
	char searchString[255];
	
	//--------------------------------
	// Always read from top of Block.
	seek(currentBlockOffset);
	ULONG endOfBlock = currentBlockOffset+currentBlockSize;
	
	//------------------------------------------------------------------
	// Create two search strings so that we can match any number in []
	sprintf(frontSearch,"uc[");
	sprintf(searchString,"] %s",varName);
	
	//--------------------------------
	// Search line by line for varName
	PSTR fSearch = NULL;
	PSTR bSearch = NULL;
	
	do
	{
		readLine((PUCHAR)line,254);
		
		fSearch = strstr(line,frontSearch);
		bSearch = strstr(line,searchString);
	}
	while(((fSearch == NULL) || (bSearch == NULL)) && (logicalPosition < endOfBlock));
	
	if (logicalPosition >= endOfBlock)
	{
		return(VARIABLE_NOT_FOUND);
	}

	//--------------------------------------
	// Get number of elements in array.
	char elementString[10];
	ULONG actualElements;
	
	fSearch += 3;												//Move pointer to first number in brackets.
	int32_t numDigits = bSearch - fSearch;

	if (numDigits > 9)
		return(TOO_MANY_ELEMENTS);

	strncpy(elementString,fSearch,numDigits);
	elementString[numDigits] = '\0';
	
	actualElements = textToULong(elementString);	
	
	return(actualElements);
}
	
//---------------------------------------------------------------------------
int32_t FitIniFile::writeBlock (PCSTR blockId)
{
	char thisLine[255];
	sprintf(thisLine,"\r\n[%s]\r\n",blockId);
	
	int32_t result = write((PUCHAR)thisLine,strlen(thisLine));
	return(result);
}

//---------------------------------------------------------------------------
int32_t FitIniFile::writeIdFloat (PCSTR varName, float value)
{
	char thisLine[255];
	sprintf(thisLine,"f %s = %f\r\n",varName,value);
	
	int32_t result = write((PUCHAR)thisLine,strlen(thisLine));
	return(result);
}

//---------------------------------------------------------------------------
int32_t FitIniFile::writeIdBoolean (PCSTR varName, bool value)
{
	char thisLine[255];
	if (value)
		sprintf(thisLine,"b %s = %s\r\n",varName,"TRUE");
	else
		sprintf(thisLine,"b %s = %s\r\n",varName,"FALSE");
	
	int32_t result = write((PUCHAR)thisLine,strlen(thisLine));
	return(result);
}

//---------------------------------------------------------------------------
int32_t FitIniFile::writeIdLong (PCSTR varName, int32_t value)
{
	char thisLine[255];
	sprintf(thisLine,"l %s = %d\r\n",varName,value);
	
	int32_t result = write((PUCHAR)thisLine,strlen(thisLine));
	return(result);
}

//---------------------------------------------------------------------------
int32_t FitIniFile::writeIdShort (PCSTR varName, short value)
{
	char thisLine[255];
	sprintf(thisLine,"s %s = %d\r\n",varName,value);
	
	int32_t result = write((PUCHAR)thisLine,strlen(thisLine));
	return(result);
}

//---------------------------------------------------------------------------
int32_t FitIniFile::writeIdChar (PCSTR varName, char value)
{
	char thisLine[255];
	sprintf(thisLine,"c %s = %d\r\n",varName,value);
	
	int32_t result = write((PUCHAR)thisLine,strlen(thisLine));
	return(result);
}

//---------------------------------------------------------------------------
int32_t FitIniFile::writeIdULong (PCSTR varName, ULONG value)
{
	char thisLine[255];
	sprintf(thisLine,"ul %s = %d\r\n",varName,value);
	
	int32_t result = write((PUCHAR)thisLine,strlen(thisLine));
	return(result);
}

//---------------------------------------------------------------------------
int32_t FitIniFile::writeIdUShort (PCSTR varName, uint16_t value)
{
	char thisLine[255];
	sprintf(thisLine,"us %s = %d\r\n",varName,value);
	
	int32_t result = write((PUCHAR)thisLine,strlen(thisLine));
	return(result);
}

//---------------------------------------------------------------------------
int32_t FitIniFile::writeIdUChar (PCSTR varName, uint8_t value)
{
	char thisLine[255];
	sprintf(thisLine,"uc %s = %d\r\n",varName,value);
	
	int32_t result = write((PUCHAR)thisLine,strlen(thisLine));
	return(result);
}

//---------------------------------------------------------------------------
int32_t FitIniFile::writeIdString (PCSTR varName, PCSTR result)
{
	char thisLine[4096];
	char tmpString[4000];
	memset(tmpString,0,4000);
	strncpy(tmpString,result,3999);

	if (strlen(result) >= 4000)
		PAUSE(("String passed to WriteIdString is longer then 4000 characters"));

	sprintf(thisLine,"st %s = \"%s\"\r\n",varName,result);
	
	int32_t bytesWritten = write((PUCHAR)thisLine,strlen(thisLine));
	return(bytesWritten);
}

//---------------------------------------------------------------------------
int32_t FitIniFile::writeIdUShortArray (PCSTR varName, puint16_t array, ULONG numElements)
{
	char thisLine[255];
	sprintf(thisLine,"us[%d] %s = %d,",numElements,varName,array[0]);
	
	int32_t bytesWritten = write((PUCHAR)thisLine,strlen(thisLine));

	for (int32_t i=1;i<(int32_t)numElements;i++)
	{
		sprintf(thisLine,"%d,",array[i]);
		bytesWritten += write((PUCHAR)thisLine,strlen(thisLine));
	}
	sprintf(thisLine,"\r\n");
	bytesWritten += write((PUCHAR)thisLine,strlen(thisLine));

	return (bytesWritten);
}

//---------------------------------------------------------------------------
int32_t FitIniFile::writeIdLongArray (PCSTR varName, int32_t *array, ULONG numElements)
{
	char thisLine[255];
	sprintf(thisLine,"l[%d] %s = %d,",numElements,varName,array[0]);
	
	int32_t bytesWritten = write((PUCHAR)thisLine,strlen(thisLine));

	for (int32_t i=1;i<(int32_t)numElements;i++)
	{
		sprintf(thisLine,"%d,",array[i]);
		bytesWritten += write((PUCHAR)thisLine,strlen(thisLine));
	}
	sprintf(thisLine,"\r\n");
	bytesWritten += write((PUCHAR)thisLine,strlen(thisLine));

	return (bytesWritten);
}

//---------------------------------------------------------------------------
int32_t FitIniFile::writeIdFloatArray (PCSTR varName, float *array, ULONG numElements)
{
	char thisLine[255];
	sprintf(thisLine,"f[%d] %s = %.2f,",numElements,varName,array[0]);
	
	int32_t bytesWritten = write((PUCHAR)thisLine,strlen(thisLine));

	for (int32_t i=1;i<(int32_t)numElements;i++)
	{
		sprintf(thisLine," %.2f,",array[i]);
		bytesWritten += write((PUCHAR)thisLine,strlen(thisLine));
	}
	sprintf(thisLine,"\r\n");
	bytesWritten += write((PUCHAR)thisLine,strlen(thisLine));

	return (bytesWritten);
}

//---------------------------------------------------------------------------
int32_t FitIniFile::writeIdUCharArray (PCSTR varName, puint8_t array, ULONG numElements)
{
	char thisLine[255];
	sprintf(thisLine,"uc[%d] %s = %d,",numElements,varName,array[0]);
	
	int32_t bytesWritten = write((PUCHAR)thisLine,strlen(thisLine));

	for (int32_t i=1;i<(int32_t)numElements;i++)
	{
		sprintf(thisLine," %d,",array[i]);
		bytesWritten += write((PUCHAR)thisLine,strlen(thisLine));
	}
	sprintf(thisLine,"\r\n");
	bytesWritten += write((PUCHAR)thisLine,strlen(thisLine));

	return (bytesWritten);
}

//---------------------------------------------------------------------------
//
// Edit log
//
//---------------------------------------------------------------------------
