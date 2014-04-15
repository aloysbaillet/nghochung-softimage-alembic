//*****************************************************************************
/*!
	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*

#include "CAbcOXform.h"
#include <Alembic/AbcGeom/OXform.h>
#include <Alembic/AbcCoreAbstract/TimeSampling.h>

void CAbcOXform::AddSampleFromMatrix( const Alembic::Abc::M44d& in_matLocal )
{
	Alembic::AbcGeom::OXformSchema& l_pSchema = GetSchema();

	Alembic::AbcGeom::XformSample sample;
	sample.setMatrix( in_matLocal );
	l_pSchema.set( sample );
}

void CAbcOXform::AddSampleFromMatrix( const Alembic::Abc::M44d& in_matGlobal, const Alembic::Abc::M44d& in_matParentGlobal )
{
	Alembic::Abc::M44d l_matLocal;
	Alembic::Abc::M44d::multiply( in_matGlobal, in_matParentGlobal.inverse(), l_matLocal );

	Alembic::AbcGeom::OXformSchema& l_pSchema = GetSchema();
	
	Alembic::AbcGeom::XformSample sample;
	sample.setMatrix( l_matLocal );
	l_pSchema.set( sample );
}
