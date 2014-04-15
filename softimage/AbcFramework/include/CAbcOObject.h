//*****************************************************************************
/*!
	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*

#ifndef CABCOOBJECT_H
#define CABCOOBJECT_H

#include "CAbcOutput_Helpers.h"
#include "CRefCount.h"
#include <Alembic/Abc/OArchive.h>

class CAbcOObject : public CAbcOObjectImpl<Alembic::Abc::OObject, IAbcOObject>, protected CRefCount
{
	IMPL_REFCOUNT;
public:
	typedef CAbcOObjectImpl< Alembic::Abc::OObject, IAbcOObject > TheBaseClass;

	CAbcOObject( Alembic::Abc::OArchive& in_OArchive )
		: TheBaseClass( in_OArchive.getTop() )
	{

	}

	EAbcOObjectType GetType() const { return EOObject_Unknown;}
};

#endif