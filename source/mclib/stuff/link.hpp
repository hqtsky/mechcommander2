//===========================================================================//
// File:	link.hh                                                          //
// Contents: Interface specification of base Link                            //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once

#ifndef _LINK_HPP_
#define _LINK_HPP_

#include <stuff/style.hpp>

namespace Stuff {

	class Socket;
	class Plug;
	// class PlugIterator;
	// class Node;

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Link ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	class Link
#if defined(_ARMOR)
		: public Stuff::Signature
#endif
	{
		friend class Plug;
		friend class PlugIterator;

	public:
		//
		//--------------------------------------------------------------------
		//--------------------------------------------------------------------
		//
		virtual ~Link(void);
		void TestInstance(void);

		//
		//--------------------------------------------------------------------
		//--------------------------------------------------------------------
		//
		Socket* GetSocket(void)	{return socket;}
		Plug* GetPlug(void)		{ return plug; }

	protected:
		//
		//--------------------------------------------------------------------
		//--------------------------------------------------------------------
		//
		Link(Socket* socket, Plug* plug);

		//
		//--------------------------------------------------------------------
		//--------------------------------------------------------------------
		//
		void ReleaseFromPlug(void);

		//
		//--------------------------------------------------------------------
		//--------------------------------------------------------------------
		//
		Socket*	socket;
		Plug*	plug;

	private:
		//
		//--------------------------------------------------------------------
		//--------------------------------------------------------------------
		//
		void AddToPlug(Plug* plug);

		//
		//--------------------------------------------------------------------
		//--------------------------------------------------------------------
		//
		Link* nextLink;
		Link* prevLink;
	};

}

#endif
