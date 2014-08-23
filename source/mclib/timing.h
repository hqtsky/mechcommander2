//******************************************************************************************
//	timing.h - This file contains the externs for the timing variables
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once

#ifndef TIMING_H
#define TIMING_H

// #include "dstd.h"

//----------------------------------------------------------------------------------
extern bool		gamePaused;			//Is the game paused?
extern int32_t 	turn;				//What frame of the scenario is it?
extern float	frameLength;		//Duration of last frame in seconds.
extern float	scenarioTime;		//Time scenario has been running.
extern ULONG	LastTimeGetTime;	//Stores Interval since timeGetTime last called.
extern bool		dynamicFrameTiming;	//This flag determines if we are using frameLength
									//To time a frame.  This is FALSE when something
									//puches a frame length dramatically over 4fps.
									//Like a cache hit.  It insures all frames get played.

//----------------------------------------------------------------------------------
// Macro Definitions
#define MAX_TIMERS			32

//----------------------------------------------------------------------------------
class Timer
{
	protected:
		float startTime;
		float currentTime;
		bool expired;
		bool updateWhilePaused;
		
	public:
	
		Timer (void)
		{
			init();
		}
		
		~Timer (void)
		{
			destroy();
		}
		
		void init (void)
		{
			startTime = 0.0;
			currentTime = 0.0;
			expired = TRUE;
			updateWhilePaused = FALSE;
		}
		
		void destroy (void)
		{
		}
		
		void update (void)
		{
			if (updateWhilePaused || !gamePaused)
			{
				currentTime -= frameLength;
				if (currentTime < 0.0)
					expired = TRUE;
				else
					expired = FALSE;
			}
		}
		
		bool isExpired (void)
		{
			return expired;
		}
		
		void setTimer (float newTime)
		{
			startTime = newTime;
			currentTime = newTime;
			expired = (newTime > 0.0);
		}
		
		void setUpdateWhilePaused (bool val)
		{
			updateWhilePaused = val;
		}
		
		float getCurrentTime (void)
		{
			return currentTime;
		}
};		

typedef Timer *TimerPtr;
//----------------------------------------------------------------------------------
class TimerManager
{
	protected:
		Timer					timers[MAX_TIMERS];
		
	public:
		
		TimerManager (void)
		{
			init();
		}
		
		~TimerManager (void)
		{
			destroy();
		}
		
		void init (void)
		{
			for (int32_t i=0;i<MAX_TIMERS;i++)
			{
				timers[i].init();
			}
		}
		
		void destroy (void)
		{
			for (int32_t i=0;i<MAX_TIMERS;i++)
			{
				timers[i].init();
			}
		}
		
		TimerPtr getTimer (int32_t index)
		{
			if ((index >= 0) && (index < MAX_TIMERS))
				return &(timers[index]);
				
			return NULL;
		}
		
		void update (void)
		{
			for (int32_t i=0;i<MAX_TIMERS;i++)
			{
				timers[i].update();
			}
		}
};

typedef TimerManager *TimerManagerPtr;
//----------------------------------------------------------------------------------
extern TimerManagerPtr timerManager;			

extern PSTR monthName[];
//----------------------------------------------------------------------------------
// Find out actual system time
typedef struct _MC_SYSTEMTIME
{
    ULONG dwYear; 
	ULONG dwMonth; 
	ULONG dwDayOfWeek; 
	ULONG dwDay; 
	ULONG dwHour; 
	ULONG dwMinute; 
	ULONG dwSecond; 
	ULONG dwMilliseconds;

	void copyFromSystemTime (PVOID systemTime);

} MC_SYSTEMTIME;

ULONG MCTiming_GetTimeZoneInforation(PVOID timeData);

ULONG MCTiming_GetTimeZoneInformationSize (void);

void MCTiming_GetUTCSystemTimeFromInformation(ULONG daylightInfo, PVOID timeData, MC_SYSTEMTIME *systemTime);
//----------------------------------------------------------------------------------
#endif
