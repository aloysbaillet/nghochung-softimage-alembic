//*****************************************************************************
/*!
	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*

#include "CAbcOFaceSet.h"

void CAbcOFaceSet::AddSample( const Alembic::Util::int32_t* in_pIndices, int in_iIndicesCount )
{
	Alembic::AbcGeom::OFaceSetSchema l_faceSetSchema = GetInternalObject().getSchema();

	Alembic::AbcGeom::OFaceSetSchema::Sample l_faceSetSample( 
		Alembic::Abc::Int32ArraySample( in_pIndices, in_iIndicesCount)
		);

	l_faceSetSchema.set( l_faceSetSample );
}
