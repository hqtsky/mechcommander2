//===========================================================================//
// File:	 Globals.hpp													 //
// Contents: global variables												 //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once

//System Setup
extern ULONG TerminateGame;

extern ULONG gHardwareMouse;


//
// Processor information
//
extern int HasCPUID;			// 1 when at least a Pentium is being used.
extern int HasMMX;				// 1 when MMX instructions are present
extern int HasRDTSC;			// 1 when RDTSC instruction is present
extern int HasKatmai;			// 1 when Katmai instructions available
extern int Has3DNow;			// 1 when 3DNox instructions available
extern UCHAR CPUID0[17];		// CPUID 0 information (+ terminating 0)
extern UCHAR CPUID1[16];		// CPUID 1 information
extern UCHAR CPUID2[16];		// CPUID 2 information

//
// Global window 
//
extern UCHAR gActive;
extern UCHAR gGotFocus;
extern ULONG MouseInWindow;		// 1=Mouse over window in a window mode



//
//
//
void __stdcall CheckThreads(void);
void __stdcall DestroyThreads(void);
