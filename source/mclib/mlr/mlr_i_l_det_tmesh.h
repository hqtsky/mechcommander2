//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once

#ifndef MLR_MLR_I_L_DET_TMESH_HPP
#define MLR_MLR_I_L_DET_TMESH_HPP

#include "mlr/mlr_i_c_det_tmesh.h"

namespace MidLevelRenderer
{

class MLR_I_L_TMesh;
class MLR_I_L_DeT_PMesh;

//##########################################################################
//###### MLRIndexedTriMesh with color and lighting and detail texture ######
//##########################################################################

class MLR_I_L_DeT_TMesh : public MLR_I_C_DeT_TMesh
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
	MLR_I_L_DeT_TMesh(ClassData* class_data, std::iostream stream, uint32_t version);
	~MLR_I_L_DeT_TMesh(void);

public:
	MLR_I_L_DeT_TMesh(ClassData* class_data = MLR_I_L_DeT_TMesh::DefaultData);

	static MLR_I_L_DeT_TMesh* Make(std::iostream stream, uint32_t version);

	void Save(std::iostream stream);

public:
	virtual void SetNormalData(const Stuff::Vector3D* array, size_t point_count);
	virtual void GetNormalData(Stuff::Vector3D** array, size_t* point_count);

#if COLOR_AS_DWORD
	virtual void SetcolourData(const uint32_t* array, size_t point_count);
#else
	virtual void SetcolourData(const Stuff::RGBAcolour* array, size_t point_count);
#endif

	virtual void Lighting(MLRLight* const*, uint32_t nrLights);

	virtual void
#if COLOR_AS_DWORD
	PaintMe(const uint32_t* paintMe);
#else
	PaintMe(const Stuff::RGBAcolour* paintMe);
#endif

	virtual uint32_t TransformAndClip(
		Stuff::Matrix4D*, MLRClippingState, GOSVertexPool*, bool = false);

	virtual void TransformNoClip(Stuff::Matrix4D*, GOSVertexPool*, bool = false);

	void Copy(MLR_I_L_TMesh*, MLRState detailState, float xOff, float yOff, float xFac, float yFac);

	bool Copy(MLR_I_L_DeT_PMesh*);

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
		size_t ret = MLR_I_C_DeT_TMesh::GetSize();
		ret += normals.GetSize();
		ret += litcolours.GetSize();
		return ret;
	}

protected:
	std::vector<Stuff::Vector3D> normals; // Base address of normal list

#if COLOR_AS_DWORD
	std::vector<uint32_t> litcolours;
#else
	std::vector<Stuff::RGBAcolour> litcolours;
#endif
};

//	MLR_I_L_DeT_TMesh*
//		CreateIndexedTriCube_Nocolour_NoLit(float, MLRState*);
MLRShape*
CreateIndexedTriIcosahedron_colour_Lit_DetTex(IcoInfo&, MLRState*, MLRState*);
} // namespace MidLevelRenderer
#endif
