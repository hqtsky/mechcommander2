//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once

#ifndef MLR_MLR_I_TMESH_HPP
#define MLR_MLR_I_TMESH_HPP

#include "stuff/plane.h"
#include "mlr/mlrindexedprimitivebase.h"

//#include "mlr/mlr_i_pmesh.h"

namespace Stuff
{
class Plane;
}

namespace MidLevelRenderer
{

class MLR_I_PMesh;

//##########################################################################
//#### MLRIndexedTriMesh with no color no lighting one texture layer #####
//##########################################################################

class MLR_I_TMesh : public MLRIndexedPrimitiveBase
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
	MLR_I_TMesh(ClassData* class_data, std::iostream stream, uint32_t version);
	~MLR_I_TMesh(void);

public:
	MLR_I_TMesh(ClassData* class_data = MLR_I_TMesh::DefaultData);

	static MLR_I_TMesh* Make(std::iostream stream, uint32_t version);

	void Save(std::iostream stream);

public:
	virtual void InitializeDrawPrimitive(uint8_t, int32_t = 0);

	virtual size_t GetNumPrimitives(void)
	{
		// Check_Object(this);
		return numOfTriangles;
	}

	virtual void SetSubprimitiveLengths(uint8_t* length_array, size_t subprimitive_count)
	{
		// Check_Object(this);
		(void)length_array;
		// _ASSERT(gos_GetCurrentHeap() == Heap);
		numOfTriangles = subprimitive_count;
		testList.SetLength(numOfTriangles);
		facePlanes.SetLength(numOfTriangles);
	}

	void FindFacePlanes(void);

	virtual int32_t FindBackFace(const Stuff::Point3D&);

	const Stuff::Plane* GetTrianglePlane(size_t index)
	{
		// Check_Object(this);
		_ASSERT(index < facePlanes.GetLength());
		return &facePlanes[index];
	}

	virtual void Lighting(MLRLight* const*, uint32_t nrLights);

	virtual void LightMapLighting(MLRLight*);

#if COLOR_AS_DWORD
	virtual void PaintMe(const uint32_t* paintMe)
	{
		(void)paintMe;
	};
#else
	virtual void PaintMe(const Stuff::RGBAcolour* paintMe)
	{
		(void)paintMe;
	};
#endif

	virtual uint32_t TransformAndClip(
		Stuff::Matrix4D*, MLRClippingState, GOSVertexPool*, bool = false);

	bool CastRay(Stuff::Line3D* line, Stuff::Normal3D* normal);

	// void
	// Transform(Stuff::Matrix4D*);

	virtual void TransformNoClip(Stuff::Matrix4D*, GOSVertexPool*, bool = false);

	// Initializes the visibility test list
	void ResetTestList(void);

	// find which vertices are visible which not - returns nr of visible
	// vertices the result is stored in the visibleIndexedVertices array
	int32_t FindVisibleVertices(void);

	bool Copy(MLR_I_PMesh*);

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
		size_t ret = MLRIndexedPrimitiveBase::GetSize();
		ret += testList.GetSize();
		ret += facePlanes.GetSize();
		return ret;
	}

protected:
	size_t numOfTriangles;
	std::vector<uint8_t> testList;
	std::vector<Stuff::Plane> facePlanes;
};

#define ICO_X 0.525731112119133606f
#define ICO_Z 0.850650808352039932f

extern float vdata[12][3];
extern uint32_t tindices[20][3];
extern uint32_t triDrawn;
MLR_I_TMesh*
CreateIndexedTriCube_Nocolour_NoLit(float, MLRState*);
MLRShape*
CreateIndexedTriIcosahedron_Nocolour_NoLit(IcoInfo&, MLRState*);
} // namespace MidLevelRenderer
#endif
