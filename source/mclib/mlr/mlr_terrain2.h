//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once

#ifndef MLR_MLR_TERRAIN2_HPP
#define MLR_MLR_TERRAIN2_HPP

#include "mlr/mlr_i_det_tmesh.h"

namespace MidLevelRenderer
{

//##########################################################################
//#### MLRIndexedPolyMesh with no color no lighting w/ detail texture  #####
//##########################################################################

class MLR_Terrain2 : public MLR_I_DeT_TMesh
{
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Initialization
	//
public:
	static void __stdcall InitializeClass(void);
	static void __stdcall TerminateClass(void);

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Constructors/Destructors
	//
protected:
	MLR_Terrain2(ClassData* class_data, std::iostream stream, uint32_t version);
	~MLR_Terrain2(void);

public:
	MLR_Terrain2(ClassData* class_data = MLR_Terrain2::DefaultData);

	static MLR_Terrain2* Make(std::iostream stream, uint32_t version);

	void Save(std::iostream stream);

public:
	virtual uint32_t TransformAndClip(
		Stuff::Matrix4D*, MLRClippingState, GOSVertexPool*, bool = false);

	virtual void TransformNoClip(Stuff::Matrix4D*, GOSVertexPool*, bool = false);

	void SetCurrentDepth(uint8_t d);

	uint8_t GetCurrentDepth()
	{
		// Check_Object(this);
		return currentDepth;
	}

	void SetDepthData(uint8_t md, uint8_t mad)
	{
		// Check_Object(this);
		maxDepth = md;
		maxAllDepth = mad;
	}

	void SetTileData(uint8_t tx, uint8_t tz)
	{
		// Check_Object(this);
		tileX = tx;
		tileZ = tz;
	}

	void SetFrame(int32_t res, float xMin, float zMin, float xMax, float zMax)
	{
		// Check_Object(this);
		frame[res][0] = xMin;
		frame[res][1] = zMin;
		frame[res][2] = xMax;
		frame[res][3] = zMax;
	}

	float GetFrame(int32_t res, int32_t p)
	{
		// Check_Object(this);
		return frame[res][p];
	}

	void SetBorderPixel(float bp)
	{
		// Check_Object(this);
		borderPixelFun = bp;
	}

	void CalculateUVs(void);

	void SetLevelTexture(int32_t lev, int32_t handle);

	int32_t GetLevelTexture(int32_t lev)
	{
		// Check_Object(this);
		_ASSERT(lev >= 0 && lev < 8);
		return textures[lev];
	}

	void LightMapLighting(MLRLight*);

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Class Data Support
	//
public:
	static ClassData* DefaultData;

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Testing
	//
public:
	void TestInstance(void) const;

	virtual size_t GetSize(void)
	{
		// Check_Object(this);
		size_t ret = MLR_I_DeT_TMesh::GetSize();
		return ret;
	}

protected:
	int32_t textures[8];
	float frame[8][4];

	uint8_t tileX, tileZ;
	uint8_t currentDepth, maxDepth, maxAllDepth;

	float borderPixelFun;

	static std::vector<Stuff::Vector2DScalar>*
		detailTexCoords; // Base address of texture coordinate list
};
} // namespace MidLevelRenderer
#endif
