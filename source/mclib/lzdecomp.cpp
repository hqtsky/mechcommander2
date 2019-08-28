//--------------------------------------------------------------------------
// LZ Decompress Routine
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

//#include "stdinc.h"
#include "lz.h"

//---------------------------------------------------------------------------
// Static Globals

enum class __lzdecomp_const
{
	HASH_CLEAR = 256, // clear hash table command code
	HASH_EOF = 257, // End Of Data command code
	HASH_FREE = 258, // First Hash Table Chain Offset value
	BASE_BITS = 9,
	MAX_BIT_INDEX = (1 << BASE_BITS),
	NO_RAM_FOR_LZ_DECOMP = 0xCBCB0002,
};

#ifndef nullptr
#define nullptr 0
#endif

typedef uint8_t* uint8_t*;

typedef struct HashStruct
{
	uint16_t Chain;
	uint8_t Suffix;
} HashStruct;
typedef HashStruct* HashStructPtr;

HashStructPtr LZOldChain = nullptr; // Old Chain value Found
HashStructPtr LZChain = nullptr; // Current Chain value Found
uint32_t LZMaxIndex = 0; // Max index value in Hash Table
uint32_t LZCodeMask = 0;
uint32_t LZFreeIndex = 0; // Current Free index into Hash Table
uint8_t* LZSrcBufEnd = nullptr; // ptr to 3rd from last byte in src buffer
uint8_t* LZOrigDOSBuf = nullptr; // original offset to start of src buffer
wchar_t LZHashBuffer[16384];
wchar_t LZOldSuffix = 0; // Current Suffix value found

//-----------------------------

//-------------------------------------------------------------------------------
// LZ DeCompress Routine
// Takes a pointer to dest buffer, a pointer to source buffer and len of source.
// returns length of decompressed image.
size_t
LZDecomp(uint8_t* dest, uint8_t* src, size_t srcLen)
{
	size_t result = 0;
	__asm {
		mov		esi, src
		mov		ebx, srcLen

		mov		edi, dest
		xor		eax, eax

		xor		ecx, ecx // CH and CL used
		lea		ebx, [ebx+esi-3]

		mov		LZOldChain, eax
		mov		LZSrcBufEnd, ebx

		mov		LZChain, eax
		mov		LZMaxIndex, eax

		mov		LZCodeMask, eax
		mov		LZFreeIndex, eax

		mov		LZCodeMask, MAX_BIT_INDEX-1
		mov		LZMaxIndex, MAX_BIT_INDEX // max index for 9 bits == 512

		mov		LZFreeIndex, HASH_FREE // set index to 258
		mov		LZOldSuffix, al

		mov		ch, BASE_BITS
		jmp		GetCode

				//--------------------------------------------------------------------------
				//
				// ClearHash restarts decompression assuming that it is starting
				// from the
				//           beginning
				//
				//--------------------------------------------------------------------------

		ClearHash:
		mov		ch, BASE_BITS // set up for nine bit codes
		mov		LZCodeMask, MAX_BIT_INDEX-1

		mov		LZMaxIndex, MAX_BIT_INDEX // max index for 9 bits == 512
		mov		LZFreeIndex, HASH_FREE // set index to 258

		cmp		esi, LZSrcBufEnd
		ja		error

		mov		eax, [esi+0]
		xor		ebx, ebx

		shr		eax, cl

		add		cl, ch
		mov		edx, LZCodeMask

		mov		bl, cl
		and   	cl, 07h

		shr   	ebx, 3
		and		eax, edx

		add		esi, ebx
		nop

		mov		LZOldChain, eax // previous Chain Offset.
		mov		LZOldSuffix, al

		mov		[edi], al
		inc		edi
				//-------------------------------------------------------------------------
				// ReadCode gets the next hash code (9 BITS) from LZDOSBuff
				//         this WILL ReadFile more data if the buffer is empty
				//-------------------------------------------------------------------------
		GetCode:
		cmp		esi, LZSrcBufEnd
		ja		error // Read Passed End?

		mov		eax, [esi+0]
		xor		ebx, ebx

		shr		eax, cl

		add		cl, ch
		mov		edx, LZCodeMask

		mov		bl, cl
		and   	cl, 07h

		shr   	ebx, 3
		and		eax, edx

		add		esi, ebx
		nop

		cmp		eax, HASH_EOF
		je		eof

		cmp		eax, HASH_CLEAR // are we to clear out hash table?
		je		ClearHash
				//---------------------------------------------------------------------------
				//
				// handle Chain acts on two types of Codes, A previously tabled
				// one and a new one. On a previously tabled one, the chain
				// value and suffix for that code are preserved into OldSuffix
				// and OldChain. The block operates on searching backward in the
				// chains until a chain offset of 0-255 is found (meaning the
				// terminal character has been reached.) Each character in the
				// chain is saved on the stack.
				//
				//---------------------------------------------------------------------------

				// HandleChain:
		mov		edx, esp
		dec		esp

		mov		LZChain, eax // Save new chain as well
		lea		ebx, [LZHashBuffer+eax+eax*2]

		cmp		eax, LZFreeIndex // is code in HASH TABLE already?
		jl		InTable // if yes, then process chain

		mov		al, LZOldSuffix // get back the old suffix and plant it
		mov		[esp], al // onto the stack for processing later
		dec		esp
		mov		[ebx+2], al
		mov		eax, LZOldChain // get Old chain for creation of Old Chain
		mov		[ebx], ax
		lea		ebx, [LZHashBuffer+eax+eax*2]

		InTable:
		test	ah, ah //(ax<255) is current chain a character?
		jz		ChainEnd // if yes, then begin Print out

		mov		al, [ebx+2] // push suffix onto stack
		mov		[esp], al // onto the stack for processing later

		dec		esp
		movzx	eax, word ptr [ebx] // get chain to this code
		lea		ebx, [LZHashBuffer+eax+eax*2]
		jmp   InTable // and keep filling up

		ChainEnd:
		mov		LZOldSuffix, al // save last character in chain
		mov		[esp], al // onto the stack for processing later

		sub		edx, esp
		mov		ebx, LZFreeIndex // get new code number index

		send_bytes:
		mov		al, [esp]
		inc		esp

		mov		[edi], al
		inc		edi

		dec		edx
		jnz		send_bytes

						//---------------------------------------------------------------------------
						//
						// Here we add another chain to the hash table so that we
						// continually use it for more decompressions.
						//
						//---------------------------------------------------------------------------

		mov		al, LZOldSuffix
		mov		edx, LZOldChain

		mov		byte ptr [LZHashBuffer+ebx+ebx*2+2], al
		mov		word ptr [LZHashBuffer+ebx+ebx*2], dx

		inc		ebx
		mov		eax, LZChain

		mov		LZFreeIndex, ebx
		mov		edx, LZMaxIndex

		mov		LZOldChain, eax
		cmp		ebx, edx

		jl    	GetCode

		cmp		ch, 12
		je		GetCode

		inc		ch
		mov		ebx, LZCodeMask

		mov		eax, LZMaxIndex
		add		ebx, ebx

		add		eax, eax
		or		ebx, 1

		mov		LZMaxIndex, eax
		mov		LZCodeMask, ebx

		jmp   	GetCode


		error:
		eof:
		sub		edi, dest
		mov		result, edi
	}
	return (result);
}