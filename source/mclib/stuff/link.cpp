//===========================================================================//
// File:	link.cc                                                          //
// Contents: Implementation details of base Link                             //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include "stdafx.h"
//#include "stuffheaders.hpp"

//#include <gameos.hpp>
#include <stuff/plug.hpp>
#include <stuff/link.hpp>

using namespace Stuff;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Link ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//
//#############################################################################
// Link
//#############################################################################
//
Link::Link(Socket* socket, Plug* plug)
{
	this->socket = socket;
	this->plug   = plug;
	//
	//----------------------------------------------------
	// Add this new link to the plugs current set of links
	//----------------------------------------------------
	//
	AddToPlug(plug);
}

//
//#############################################################################
// ~Link
//#############################################################################
//
Link::~Link()
{
	Check_Signature(this);
	//
	//-----------------------------------------------------
	// Remove this link from the plugs current set of links
	//-----------------------------------------------------
	//
	Verify(!nextLink);
	Verify(!prevLink);
	Check_Object(plug);
	Verify(plug->linkHead != this);
}

//
//#############################################################################
// TestInstance
//#############################################################################
//
void Link::TestInstance()
{
	Check_Signature(socket);
	Check_Signature(plug);
}

//
//#############################################################################
// ReleaseFromPlug
//#############################################################################
//
void Link::ReleaseFromPlug()
{
	// Check_Object(this);
	//
	//-----------------------------------------------------
	// Remove this link from the plugs current set of links
	//-----------------------------------------------------
	//
	Check_Object(plug);
	if (plug->linkHead == this)
	{
		plug->linkHead = nextLink;
	}
	if (prevLink != nullptr)
	{
		Check_Object(prevLink);
		prevLink->nextLink = nextLink;
	}
	if (nextLink != nullptr)
	{
		Check_Object(nextLink);
		nextLink->prevLink = prevLink;
	}
	prevLink = nextLink = nullptr;
}

//
//#############################################################################
// AddToPlug
//#############################################################################
//
void Link::AddToPlug(Plug* plug)
{
	// Check_Object(this);
	Check_Object(plug);
	if ((nextLink = plug->linkHead) != nullptr)
	{
		Check_Object(nextLink);
		nextLink->prevLink = this;
	}
	this->prevLink = nullptr;
	plug->linkHead = this;
}
