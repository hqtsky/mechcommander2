/*******************************************************************************
 Copyright (c) 2011-2014, Jerker Back. All rights reserved.

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the following
 conditions are met (OSI approved BSD 2-clause license):

 1. Redistributions of source code must retain the above copyright notice,
	this list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright notice,
	this list of conditions and the following disclaimer in the documentation
	and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/
/*******************************************************************************
 gameplatform.cpp - gameos entry point functions

 MechCommander 2 source code

 2014-07-24 jerker_back, created

 $LastChangedBy$

================================================================================
 RcsID = $Id$ */

#include <gameos.hpp>
#include <platform.hpp>


extern "C" int WINAPI 
WinMain(_In_ HINSTANCE hInstance,_In_opt_ HINSTANCE hPrevInstance,_In_ PSTR pCmdLine,_In_ int nShowCmd)
{
	Platform = Platform_Game;
	RunFromWinMain(hInstance, hPrevInstance, pCmdLine, nShowCmd);
	return 0;
}

#if _CONSIDERED_OBSOLETE
extern bool gViaNew;

PVOID __cdecl operator new(size_t nSize, HGOSHEAP HeapBlock)
{
	gViaNew = true;
	return gos_Malloc(nSize, HeapBlock);
}

PVOID __cdecl operator new(size_t nSize)
{
	gViaNew = true;
	return gos_Malloc(nSize, (HGOSHEAP)-1);
}

void __cdecl operator delete(PVOID block) { gos_Free(block); }

void __cdecl operator delete[](PVOID block) { operator delete(block); }
#endif
