//===========================================================================//
// File:	angle.hh                                                         //
// Contents: Interface specification for angle classes                       //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once

#ifndef _ANGLE_HPP_
#define _ANGLE_HPP_

#include <stuff/scalar.hpp>

namespace Stuff
{
	class Radian;
	class Degree;
	class SinCosPair;
}

#if !defined(Spew)
void Spew(PCSTR group, const Stuff::Radian& angle);
void Spew(PCSTR group, const Stuff::Degree& angle);
void Spew(PCSTR group, const Stuff::SinCosPair& angle);
#endif

namespace Stuff
{

	class Degree;
	class SinCosPair;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Radian ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	class Radian
	{
	public:
		float angle;

		//
		// Constructors
		//
		Radian(void)							{}
		Radian(float a)
		{
			angle = a;
		}
		Radian(const Radian& a)
		{
			angle = a.angle;
		}
		explicit Radian(const Degree& degree)
		{
			*this = degree;
		}
		explicit Radian(const SinCosPair& pair)
		{
			*this = pair;
		}

		//
		// Assignment operators
		//
		Radian& operator=(float angle)
		{
			//Check_Pointer(this);
			this->angle = angle;
			return *this;
		}
		Radian& operator=(const Radian& radian)
		{
			//Check_Pointer(this);
			Check_Object(&radian);
			angle = radian.angle;
			return *this;
		}
		Radian& operator=(const Degree& degree);
		Radian& operator=(const SinCosPair& pair);

		//
		// Casting
		//
		operator float(void) const
		{
			// Check_Object(this);
			return angle;
		}

		float getdata(void) const
		{
			// Check_Object(this);
			return angle;
		}
		//
		// These comparator functions are not designed to make exact comparisons
		// of Scalaring point numbers, but rather to compare them to within some
		// specified error threshold
		//
		bool operator!(void) const
		{
			// Check_Object(this);
			return Small_Enough(angle);
		}

		bool operator==(const Radian& r) const
		{
			// Check_Object(this);
			Check_Object(&r);
			return Close_Enough(angle, r.angle);
		}
		bool operator==(float r) const
		{
			// Check_Object(this);
			return Close_Enough(angle, r);
		}

		bool operator!=(const Radian& r) const
		{
			// Check_Object(this);
			Check_Object(&r);
			return !Close_Enough(angle, r.angle);
		}
		bool operator!=(float r) const
		{
			// Check_Object(this);
			return !Close_Enough(angle, r);
		}

		//
		// Math operators
		//
		Radian& Negate(float r)
		{
			//Check_Pointer(this);
			angle = -r;
			return *this;
		}

		Radian& Add(float r1, float r2)
		{
			//Check_Pointer(this);
			angle = r1 + r2;
			return *this;
		}
		Radian& operator+=(float r)
		{
			// Check_Object(this);
			angle += r;
			return *this;
		}

		Radian& Subtract(float r1, float r2)
		{
			//Check_Pointer(this);
			angle = r1 - r2;
			return *this;
		}
		Radian& operator-=(float r)
		{
			// Check_Object(this);
			angle -= r;
			return *this;
		}

		Radian& Multiply(float r1, float r2)
		{
			//Check_Pointer(this);
			angle = r1 * r2;
			return *this;
		}
		Radian& operator*=(float r)
		{
			// Check_Object(this);
			angle *= r;
			return *this;
		}

		Radian&
		Divide(float r1, float r2)
		{
			//Check_Pointer(this);
			Verify(!Small_Enough(r2));
			angle = r1 / r2;
			return *this;
		}
		Radian&
		operator/=(float r)
		{
			// Check_Object(this);
			Verify(!Small_Enough(r));
			angle /= r;
			return *this;
		}

		//
		// Template support
		//
		Radian&
		Lerp(
			const Radian& a,
			const Radian& b,
			float t
		);

		//
		// Support functions
		//
		static float
		Normalize(float Value);
		Radian&
		Normalize(void);

#if !defined(Spew)
		friend void
		::Spew(
			PCSTR group,
			const Radian& angle
		);
#endif
		void TestInstance(void) const {}
		static bool TestClass(void);
	};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Degree ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	class Degree
	{
	public:
		float
		angle;

		//
		// constructors
		//
		Degree()
		{}
		Degree(float a)
		{
			angle = a;
		}
		Degree(const Degree& a)
		{
			angle = a.angle;
		}
		explicit Degree(const Radian& radian)
		{
			*this = radian;
		}

		//
		// Assignment operators
		//
		Degree&
		operator=(const Degree& degree)
		{
			// Check_Object(this);
			Check_Object(&degree);
			angle = degree.angle;
			return *this;
		}
		Degree&
		operator=(float angle)
		{
			// Check_Object(this);
			this->angle = angle;
			return *this;
		}
		Degree&
		operator=(const Radian& radian)
		{
			// Check_Object(this);
			Check_Object(&radian);
			angle = radian.angle * Degrees_Per_Radian;
			return *this;
		}

		//
		// Support functions
		//
#if !defined(Spew)
		friend void
		::Spew(
			PCSTR group,
			const Degree& angle
		);
#endif
		void TestInstance(void) const {}
		static bool TestClass(void);
	};

	inline Radian&
	Radian::operator=(const Degree& degree)
	{
		//Check_Pointer(this);
		Check_Object(&degree);
		angle = degree.angle * Radians_Per_Degree;
		return *this;
	}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~ SinCosPair ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	class SinCosPair
	{
	public:
		float
		sine,
		cosine;

		//
		// Constructors
		//
		SinCosPair()
		{}
		SinCosPair(float sin, float cos)
		{
			//Check_Pointer(this);
			sine = sin;
			cosine = cos;
			// Check_Object(this);
		}
		SinCosPair(const SinCosPair& pair)
		{
			//Check_Pointer(this);
			Check_Object(&pair);
			sine = pair.sine;
			cosine = pair.cosine;
		}
		explicit SinCosPair(const Radian& radian)
		{
			*this = radian;
		}

		//
		// Assignment operators
		//
		SinCosPair&
		operator=(const SinCosPair& pair)
		{
			//Check_Pointer(this);
			Check_Object(&pair);
			sine = pair.sine;
			cosine = pair.cosine;
			return *this;
		}
		SinCosPair&
		operator=(const Radian& radian)
		{
			//Check_Pointer(this);
			Check_Object(&radian);
			(void)radian;	// 4100
#if USE_ASSEMBLER_CODE
			float* f = &sine;
			_asm
			{
				push	ebx
				push	edx

				mov		ebx, f
				mov		edx, [edx]radian.angle

				fld		dword ptr [edx]
				fsincos
				fstp	dword ptr [ebx + 4]
				fstp	dword ptr [ebx]

				pop		edx
				pop		ebx
			}
#else
			cosine = cos(radian);
			sine = sin(radian);
#endif
			// Check_Object(this);
			return *this;
		}

		//
		// Support functions
		//
#if !defined(Spew)
		friend void
		::Spew(
			PCSTR group,
			const SinCosPair& angle
		);
#endif
		void TestInstance(void) const {}
		static bool TestClass(void);
	};

	inline Radian&
	Radian::operator=(const SinCosPair& pair)
	{
		//Check_Pointer(this);
		Check_Object(&pair);
		angle = Arctan(pair.sine, pair.cosine);
		return *this;
	}

}

namespace MemoryStreamIO
{

	inline Stuff::MemoryStream&
	Read(
		Stuff::MemoryStream* stream,
		Stuff::Radian* output
	)
	{
		return stream->ReadBytes(output, sizeof(*output));
	}
	inline Stuff::MemoryStream&
	Write(
		Stuff::MemoryStream* stream,
		const Stuff::Radian* input
	)
	{
		return stream->WriteBytes(input, sizeof(*input));
	}

}
#endif
