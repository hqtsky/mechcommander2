//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once

#ifndef MLR_MLR_I_C_PMESH_HPP
#define MLR_MLR_I_C_PMESH_HPP

#include "mlr/mlr_i_pmesh.h"

namespace MidLevelRenderer
{

//##########################################################################
//#### MLRIndexedPolyMesh with color but no lighting one texture layer  ####
//##########################################################################

class MLR_I_C_PMesh : public MLR_I_PMesh
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
	MLR_I_C_PMesh(ClassData* class_data, std::iostream stream, uint32_t version);
	~MLR_I_C_PMesh(void);

public:
	MLR_I_C_PMesh(ClassData* class_data = MLR_I_C_PMesh::DefaultData);

	static MLR_I_C_PMesh* Make(std::iostream stream, uint32_t version);

	void Save(std::iostream stream);

public:
	//		void Copy(MLRIndexedPolyMesh*);

#if COLOR_AS_DWORD
	virtual void SetcolourData(const uint32_t* array, size_t point_count);
	virtual void GetcolourData(uint32_t** array, size_t* point_count);
#else
	virtual void SetcolourData(const Stuff::RGBAcolour* array, size_t point_count);
	virtual void GetcolourData(Stuff::RGBAcolour** array, size_t* point_count);
#endif

	virtual void
#if COLOR_AS_DWORD
	PaintMe(const uint32_t* paintMe);
#else
	PaintMe(const Stuff::RGBAcolour* paintMe);
#endif

	virtual uint32_t TransformAndClip(
		Stuff::Matrix4D*, MLRClippingState, GOSVertexPool*, bool = false);

	virtual void TransformNoClip(Stuff::Matrix4D*, GOSVertexPool*, bool = false);

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
		size_t ret = MLR_I_PMesh::GetSize();
		ret += colors.GetSize();
		return ret;
	}

protected:
#if COLOR_AS_DWORD
	std::vector<uint32_t> colors; // Base address of color list
#else
	std::vector<Stuff::RGBAcolour> colors; // Base address of color list
#endif
};

MLR_I_C_PMesh*
CreateIndexedCube_colour_NoLit(float, Stuff::RGBAcolour*, MLRState*);

MLR_I_PMesh*
CreateIndexedViewFrustrum_colour_NoLit(float near_clip, float far_clip, float left_clip,
	float right_clip, float top_clip, float bottom_clip, Stuff::RGBAcolour&, MLRState*);

MLRShape*
CreateIndexedIcosahedron_colour_NoLit(IcoInfo&, MLRState*);
} // namespace MidLevelRenderer
#endif
