//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once

#ifndef MLR_MLRAMBIENTLIGHT_HPP
#define MLR_MLRAMBIENTLIGHT_HPP

#include "mlr/mlrlight.h"

namespace MidLevelRenderer
{

//##########################################################################
//######################    MLRAmbientLight    #############################
//##########################################################################

class MLRAmbientLight : public MLRLight
{
public:
	static void __stdcall InitializeClass(void);
	static void __stdcall TerminateClass(void);

	MLRAmbientLight(void);
	MLRAmbientLight(std::iostream stream, uint32_t version);
	// MLRAmbientLight(Stuff::Page* page);
	~MLRAmbientLight(void);

	virtual void LightVertex(const MLRVertexData&);

	virtual LightType GetLightType()
	{
		// Check_Object(this);
		return AmbientLight;
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
};
} // namespace MidLevelRenderer
#endif
