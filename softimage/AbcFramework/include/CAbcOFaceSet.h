//*****************************************************************************
/*!
	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*

#ifndef CABCOFACESET_H
#define CABCOFACESET_H

#include "AbcFrameworkImportedTypes.h"
#include <Alembic/AbcGeom/OFaceSet.h>
#include "CAbcOObject.h"
#include "CRefCount.h"

class CAbcOFaceSet : public CAbcOSchemaObjectImpl< Alembic::AbcGeom::OFaceSet, IAbcOFaceSet >, protected CRefCount
{
	IMPL_REFCOUNT;
public:
	typedef CAbcOSchemaObjectImpl< Alembic::AbcGeom::OFaceSet, IAbcOFaceSet > TheBaseClass;

	CAbcOFaceSet( IAbcOObject* in_pParent, Alembic::Abc::OObject& in_ParentOObj, const char* in_pszName )
		: TheBaseClass( in_pParent, in_ParentOObj, in_pszName )
	{

	}

	EAbcOObjectType GetType() const { return EOObject_FaceSet;}

	virtual void AddSample( const Alembic::Util::int32_t* in_pIndices, int in_iIndicesCount );
};

#endif