//===========================================================================//
// File:	mtrxstk.hh                                                       //
// Contents: Interface specification for the matrix class                    //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
/*
This header and class is not used
*/

#pragma once

#ifndef _MATRIXSTACK_HPP_
#define _MATRIXSTACK_HPP_

#include <stuff/memoryblock.hpp>
#include <stuff/matrix.hpp>
#include <stuff/linearmatrix.hpp>

namespace Stuff
{

//~~~~~~~~~~~~~~~~~~~~~~~ AffinerMatrix4DStack ~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class AffinerMatrix4DStack : public MemoryStackOf<AffineMatrix4D>
{
  public:
	AffinerMatrix4DStack(size_t start, size_t delta, PCSTR name)
		: MemoryStackOf<AffineMatrix4D>(start, delta, name)
	{
	}

	AffinerMatrix4DStack& Concatenate(const AffineMatrix4D& matrix);

	AffineMatrix4D& Push(const AffineMatrix4D& matrix);

	operator AffineMatrix4D&()
	{
		// Check_Object(this);
		return *Peek();
	}
};

//~~~~~~~~~~~~~~~~~~~~~~~ LinearMatrix4DStack ~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class LinearMatrix4DStack : public MemoryStackOf<LinearMatrix4D>
{
  public:
	LinearMatrix4DStack(size_t start, size_t delta, PCSTR name)
		: MemoryStackOf<LinearMatrix4D>(start, delta, name)
	{
	}

	LinearMatrix4DStack& Concatenate(const LinearMatrix4D& matrix);

	LinearMatrix4D& Push(const LinearMatrix4D& matrix);

	operator LinearMatrix4D&(void)
	{
		// Check_Object(this);
		return *Peek();
	}
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~ Matrix4DStack ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class Matrix4DStack : public MemoryStackOf<Matrix4D>
{
  public:
	Matrix4DStack(size_t start, size_t delta, PCSTR name)
		: MemoryStackOf<Matrix4D>(start, delta, name)
	{
	}

	Matrix4DStack& Concatenate(const Matrix4D& matrix);
	Matrix4DStack& Concatenate(const AffineMatrix4D& matrix);

	Matrix4D& Push(const Matrix4D& matrix);
	Matrix4D& Push(const AffineMatrix4D& matrix);

	operator Matrix4D&(void)
	{
		// Check_Object(this);
		return *Peek();
	}
};
}
#endif
