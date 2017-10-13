//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//
/*
This header and class is not used
*/

#pragma once

#ifndef PTR_HPP
#define PTR_HPP

namespace Stuff
{

// Initialized_Ptr: An "initialized pointer" class (I wanted to call it just
// "Pointer" or "Ptr" but this caused some conflicts.  This is just like a
// normal pointer, except that it must be explicitly constructed.  This is very
// handy as it prevents uninitialized-pointer mishaps.

template <class T> class Initialized_Ptr
{
  public:
	operator T*(void)const { return (m_ptr); }

	T& operator*(void)const { return (*m_ptr); }

	T* operator->(void)const { return (m_ptr); }

	Initialized_Ptr() : m_ptr(0) {}

	explicit Initialized_Ptr(T* ptr) : m_ptr(ptr) {}

	Initialized_Ptr& operator=(T* ptr) { m_ptr = ptr; }

  private:
	T* m_ptr;
};

}; // namespace Stuff

#endif // PTR_HPP