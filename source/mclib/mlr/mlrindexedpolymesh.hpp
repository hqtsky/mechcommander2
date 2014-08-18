//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once
#define MLR_MLRINDEXEDPOLYMESH_HPP

#if !defined(MLR_MLR_HPP)
	#include <mlr/mlr.hpp>
#endif

#if !defined(MLR_MLRINDEXEDPRIMITIVE_HPP)
	#include <mlr/mlrindexedprimitive.hpp>
#endif

namespace MidLevelRenderer {

	//##########################################################################
	//####################    MLRIndexedPolyMesh    ############################
	//##########################################################################


	class MLRIndexedPolyMesh:
		public MLRIndexedPrimitive
	{
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Initialization
	//
	public:
		static void
			InitializeClass(void);
		static void
			TerminateClass(void);

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Constructors/Destructors
	//
	protected:
		MLRIndexedPolyMesh(
			Stuff::MemoryStream *stream,
			int version
		);
		~MLRIndexedPolyMesh(void);

	public:
		MLRIndexedPolyMesh(void);

		static MLRIndexedPolyMesh*
			Make(
				Stuff::MemoryStream *stream,
				int version
			);

		void
			Save(Stuff::MemoryStream *stream);

		void*
			operator new(size_t size)
				{
					Verify(size == sizeof(MLRIndexedPolyMesh));
					return AllocatedMemory->New(void);
				}
		void
			operator delete(void *where)
				{AllocatedMemory->Delete(where);}

	private:
		static Stuff::MemoryBlock
			*AllocatedMemory;

	public:
		virtual	void	InitializeDrawPrimitive(int, int=0);

		virtual void	SetPrimitiveLength(PUCHAR , int);
		virtual void	GetPrimitiveLength(PUCHAR *, int*);

		void	FindFacePlanes(void);

		virtual int	FindBackFace(const Stuff::Point3D&);

		const Stuff::Plane *GetPolygonPlane(int i)
			{
				Check_Object(this);
				Verify(i<facePlanes.GetLength(void));

				return &facePlanes[i];
			}

		virtual void	Lighting(MLRLight**, int nrLights);

		MLRPrimitive *LightMapLighting(MLRLight*);

		virtual int	Clip(MLRClippingState, GOSVertexPool*);

		bool
			CastRay(
				Stuff::Line3D *line,
				Stuff::Normal3D *normal
			);

		void
			Transform(Stuff::Matrix4D*);

		virtual void
			TransformNoClip(Stuff::Matrix4D*, GOSVertexPool*);

	//	Initializes the visibility test list
		void
			ResetTestList(void);

	//	find which vertices are visible which not - returns nr of visible vertices
	//	the result is stored in the visibleIndexedVertices array
		int
			FindVisibleVertices(void);

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Class Data Support
	//
	public:
		static ClassData
			*DefaultData;

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Testing
	//
	public:
		void
			TestInstance(void) const;

	virtual int
			GetSize(void)
		{ 
			Check_Object(this);
			int ret = MLRIndexedPrimitive::GetSize(void);
			ret += testList.GetSize(void);
			ret += facePlanes.GetSize(void);

			return ret;
		}

	protected:
		Stuff::DynamicArrayOf<uint8_t>	testList;
		
		Stuff::DynamicArrayOf<Stuff::Plane> facePlanes;
		
	};

	MLRIndexedPolyMesh*
		CreateIndexedCube(Stuff::Scalar, Stuff::RGBAColor*, Stuff::Vector3D*, MLRState*);

}
