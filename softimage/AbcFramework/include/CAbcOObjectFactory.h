//*****************************************************************************
/*!
	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*

#ifndef CABCOOBJECTFACTORY_H
#define CABCOOBJECTFACTORY_H

#include "IAbcFramework.h"
#include "IAbcOutput.h"
#include <Alembic/Abc/OObject.h>

class CAbcOObjectFactory
{
public:
	static EAbcResult CreateOObject( IAbcOObject** out_ppObj, IAbcOObject* in_pParent, EAbcOObjectType in_type, Alembic::Abc::OObject& in_Parent, const char* in_pszName );
};

#endif