//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once

#if !defined(MLR_GOSIMAGEPOOL_HPP)
#define MLR_GOSIMAGEPOOL_HPP

#include <mlr/mlr.hpp>

namespace MidLevelRenderer {

	class GOSImagePool
#if defined(_ARMOR)
		: public Stuff::Signature
#endif
	{
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Constructors/Destructors
	//
	public:
		GOSImagePool(void);
		virtual ~GOSImagePool(void);

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Image handling
	//
	public:
		GOSImage* GetImage(PCSTR imageName);
		GOSImage* GetImage(PCSTR imageName, gos_TextureFormat format, int size, gos_TextureHints hints);

		virtual bool LoadImage(GOSImage *image, int=0)=0;
		void RemoveImage(GOSImage *image);
		void UnLoadImages(void);
		void GetTexturePath(Stuff::MString* pName) const
		{
			Check_Object(this);
			*pName = texturePath;
		}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Testing
	//
	public:
		void TestInstance(void) const
		{
		}

	protected:
		Stuff::HashOf<GOSImage*, Stuff::MString> imageHash;
		Stuff::MString texturePath;
	};

	class TGAFilePool:
		public GOSImagePool
	{
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Constructors/Destructors
	//
	public:
		TGAFilePool(PCSTR path);

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Image handling
	//
	public:
		bool LoadImage(GOSImage *image, int=0);
	};
}
#endif
