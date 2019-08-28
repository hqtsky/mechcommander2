//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once

#ifndef MLR_MLRLIGHT_HPP
#define MLR_MLRLIGHT_HPP

#include "stuff/linearmatrix.h"
#include "stuff/color.h"
#include "stuff/registeredclass.h"
// #include "stuff/mstring.h"
#include "mlr/mlrstate.h"
#include "mlr/mlr.h"

namespace Stuff
{
class Point3D;
class Vector3D;
class RGBAcolour;
class RGBcolour;
class Page;
} // namespace Stuff

namespace MidLevelRenderer
{

class GOSVertexPool;
class MLRLightMap;

struct MLRVertexData
{
	Stuff::Point3D* point;
#if COLOR_AS_DWORD
	uint32_t* color;
#else
	Stuff::RGBAcolour* color;
#endif
	Stuff::Vector3D* normal;
	size_t index;
};

//##########################################################################
//######################### MLRLight #################################
//##########################################################################

class MLRLight // : public Stuff::RegisteredClass
{
public:
	static void __stdcall InitializeClass(void);
	static void __stdcall TerminateClass(void);

	MLRLight(ClassData* class_data);
	MLRLight(ClassData* class_data, std::iostream stream, uint32_t version);
	MLRLight(ClassData* class_data, Stuff::Page* page);
	~MLRLight(void);

	static MLRLight* Make(std::iostream stream, uint32_t version);
	static MLRLight* Make(Stuff::Page* page);
	virtual void Save(std::iostream stream);
	virtual void Write(Stuff::Page* page);

	enum LightType
	{
		AmbientLight = 0,
		InfiniteLight,
		InfiniteLightWithFallOff,
		PointLight,
		SpotLight,
		LookUpLight
	};

	virtual LightType GetLightType(void) = 0;
	virtual void LightVertex(const MLRVertexData&) = 0;

	void SetIntensity(float _int)
	{
		// Check_Object(this);
		intensity = _int;
	};
	float GetIntensity(void)
	{
		// Check_Object(this);
		return intensity;
	};
	void Setcolour(Stuff::RGBcolour col)
	{
		// Check_Object(this);
		color = col;
	}
	void Setcolour(float, float, float);
	void Getcolour(Stuff::RGBcolour& col)
	{
		// Check_Object(this);
		col = color;
	};
	void Getcolour(float&, float&, float&);
	void GetInWorldPosition(Stuff::Point3D& pos)
	{
		// Check_Object(this);
		pos = lightToWorld;
	};
	void GetInWorldDirection(Stuff::UnitVector3D& dir)
	{
		// Check_Object(this);
		lightToWorld.GetLocalForwardInWorld(&dir);
	};
	void GetInShapePosition(Stuff::Point3D& pos)
	{
		// Check_Object(this);
		pos = lightToShape;
	};
	void GetInShapePosition(Stuff::LinearMatrix4D& pos)
	{
		// Check_Object(this);
		pos = lightToShape;
	};
	void GetInShapeDirection(Stuff::UnitVector3D& dir)
	{
		// Check_Object(this);
		lightToShape.GetLocalForwardInWorld(&dir);
	};
	void SetLightToWorldMatrix(const Stuff::LinearMatrix4D&);
	const Stuff::LinearMatrix4D& GetLightToWorldMatrix(void)
	{
		// Check_Object(this);
		return lightToWorld;
	};

	virtual void SetLightToShapeMatrix(const Stuff::LinearMatrix4D&);
	const Stuff::LinearMatrix4D& GetLightToShapeMatrix(void)
	{
		// Check_Object(this);
		return lightToShape;
	};

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Light Map Support
	//
	virtual MLRLightMap* GetLightMap(void)
	{
		// Check_Object(this);
		return nullptr;
	}
	uint32_t GetLightMask(void)
	{
		// Check_Object(this);
		return lightMask;
	}
	void SetDynamicLight(void)
	{
		// Check_Object(this);
		lightMask |= MLRState::TerrainLightingMode;
	}
	void SetStaticLight(void)
	{
		// Check_Object(this);
		lightMask &= ~MLRState::TerrainLightingMode;
	}
	void SetName(std::wstring& name)
	{
		// Check_Object(this);
		lightName = name;
	}
	std::wstring& GetName(void)
	{
		// Check_Object(this);
		return lightName;
	}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Class Data Support
	//
public:
	static ClassData* DefaultData;

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Testing
	//
public:
	void TestInstance(void);

protected:
	float intensity;
	Stuff::RGBcolour color;
	Stuff::LinearMatrix4D lightToWorld;
	Stuff::LinearMatrix4D lightToShape;
	uint32_t lightMask;
	std::wstring lightName;
};
} // namespace MidLevelRenderer
#endif
