//*****************************************************************************
/*!
	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*

#ifndef CABCOXFORM_H
#define CABCOXFORM_H

#include <Alembic/AbcGeom/OXform.h>
#include "CAbcOObject.h"
#include "CRefCount.h"

class CAbcOXform : public CAbcOSchemaObjectImpl<Alembic::AbcGeom::OXform, IAbcOXform>, protected CRefCount
{
	IMPL_REFCOUNT;
public:
	typedef CAbcOSchemaObjectImpl< Alembic::AbcGeom::OXform, IAbcOXform > TheBaseClass;

	CAbcOXform( IAbcOObject* in_pParent, Alembic::Abc::OObject& in_ParentOObj, const char* in_pszName )
		: TheBaseClass( in_pParent, in_ParentOObj, in_pszName )
	{

	}

	EAbcOObjectType GetType() const { return EOObject_Xform;}
	
	virtual void AddSampleFromMatrix( const Alembic::Abc::M44d& in_matLocal );
	virtual void AddSampleFromMatrix( const Alembic::Abc::M44d& in_matGlobal, const Alembic::Abc::M44d& in_matParentGlobal );
};

#endif