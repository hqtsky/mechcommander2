//***************************************************************************
//
//	dmultplyr.h -- File contains the MoverGroup Definition
//
//	MechCommander 2
//
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#ifndef DMULTPLYR_H
#define DMULTPLYR_H

//---------------------------------------------------------------------------

#define	WORLDCHUNK_TYPE_BITS				4
#define	WORLDCHUNK_TERRAINBLOCK_BITS		8
#define	WORLDCHUNK_TERRAINVERTEX_BITS		9
#define	WORLDCHUNK_TERRAINITEM_BITS			3
#define	WORLDCHUNK_CELLPOS_BITS				10
#define	WORLDCHUNK_MINE_BITS				3
#define	WORLDCHUNK_TEAM_BITS				4
#define	WORLDCHUNK_SECONDS_BITS				6
#define	WORLDCHUNK_ARTILLERY_TYPE_BITS		3
#define	WORLDCHUNK_ARTILLERY_TIME_BITS		5
#define	WORLDCHUNK_TERROBJ_ALIGNMENT_BITS	2
#define	WORLDCHUNK_MISSIONSCRIPT_CODE_BITS	8
#define	WORLDCHUNK_MISSIONSCRIPT_PARM_BITS	16
#define	WORLDCHUNK_PILOT_INDEX_BITS			7
#define	WORLDCHUNK_VEHICLE_CLASS_BITS		3
#define	WORLDCHUNK_SCORE_BITS				16
#define	WORLDCHUNK_COMMANDER_BITS			4
#define	WORLDCHUNK_SUBTYPE_BITS				2
#define	WORLDCHUNK_CAPTURE_TIME_BITS		20

#define	WORLDCHUNK_TYPE_MASK				0x0000000F
#define	WORLDCHUNK_TERRAINBLOCK_MASK		0x000000FF
#define	WORLDCHUNK_TERRAINVERTEX_MASK		0x000001FF
#define	WORLDCHUNK_TERRAINITEM_MASK			0x00000007
#define	WORLDCHUNK_CELLPOS_MASK				0x000003FF
#define	WORLDCHUNK_MINE_MASK				0x00000007
#define	WORLDCHUNK_TEAMID_MASK				0x0000000F
#define	WORLDCHUNK_SECONDS_MASK				0x0000003F
#define	WORLDCHUNK_ARTILLERY_TYPE_MASK		0x00000007
#define	WORLDCHUNK_ARTILLERY_TIME_MASK		0x0000001F
#define	WORLDCHUNK_TERROBJ_ALIGNMENT_MASK	0x00000003
#define	WORLDCHUNK_MISSIONSCRIPT_CODE_MASK	0x000000FF
#define	WORLDCHUNK_MISSIONSCRIPT_PARM_MASK	0x0000FFFF
#define	WORLDCHUNK_PILOT_INDEX_MASK			0x0000007F
#define	WORLDCHUNK_VEHICLE_CLASS_MASK		0x00000007
#define	WORLDCHUNK_SCORE_MASK				0x0000FFFF
#define	WORLDCHUNK_COMMANDER_MASK			0x0000000F
#define	WORLDCHUNK_SUBTYPE_MASK				0x00000003
#define	WORLDCHUNK_CAPTURE_TIME_MASK		0x0003FFFF

typedef enum {
	WORLDCHUNK_MINE,
	WORLDCHUNK_FIRE,
	WORLDCHUNK_ARTILLERY_COMMANDER0,		// We pass the TEAM id now, NOT the commander
	WORLDCHUNK_ARTILLERY_COMMANDER1,		// We need for 8 player MPlayer
	WORLDCHUNK_ARTILLERY_COMMANDER2,		// -fs
	WORLDCHUNK_ARTILLERY_COMMANDER3,
	WORLDCHUNK_ARTILLERY_COMMANDER4,
	WORLDCHUNK_ARTILLERY_COMMANDER5,
	WORLDCHUNK_ARTILLERY_COMMANDER6,
	WORLDCHUNK_ARTILLERY_COMMANDER7,
	WORLDCHUNK_MISSION_SCRIPT_MESSAGE,
	WORLDCHUNK_PILOT_KILL_STAT,
	WORLDCHUNK_SCORE,
	WORLDCHUNK_KILL_LOSS,
	WORLDCHUNK_CAPTURE_BUILDING,
	WORLDCHUNK_END_MISSION,
	NUM_WORLDCHUNK_TYPES
};

//---------------------------------------------------------------------------

class WorldChunk;
typedef WorldChunk* WorldChunkPtr;

#endif

//***************************************************************************

