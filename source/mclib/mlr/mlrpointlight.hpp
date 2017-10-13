//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once

#ifndef MLR_MLRPOINTLIGHT_HPP
#define MLR_MLRPOINTLIGHT_HPP

#include <mlr/mlrinfinitelightwithfalloff.hpp>

namespace MidLevelRenderer
{

//##########################################################################
//######################    MLRPointLight    ###############################
//##########################################################################

class MLRPointLight : public MLRInfiniteLightWithFalloff
{
  public:
	static void __stdcall InitializeClass(void);
	static void __stdcall TerminateClass(void);

	MLRPointLight(void);
	MLRPointLight(Stuff::MemoryStream* stream, uint32_t version);
	MLRPointLight(Stuff::Page* page);
	~MLRPointLight(void);

	void Save(Stuff::MemoryStream* stream);
	void Write(Stuff::Page* page);

	virtual void LightVertex(const MLRVertexData&);

	virtual LightType GetLightType()
	{
		// Check_Object(this);
		return PointLight;
	}

	void SetLightMap(MLRLightMap* light_map);

	virtual MLRLightMap* GetLightMap()
	{
		// Check_Object(this);
		return lightMap;
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
	MLRLightMap* lightMap;
};
}
#endif
