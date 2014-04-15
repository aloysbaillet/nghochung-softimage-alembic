//*****************************************************************************
/*!
	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*

#include "CAbcInput.h"
#include <assert.h>

// Alembic Includes
#include <Alembic/AbcGeom/All.h>
#include <Alembic/AbcMaterial/All.h>
// This is required to tell Alembic which implementation to use.  In this case,
// the HDF5 implementation, currently the only one available.
#include <Alembic/AbcCoreHDF5/All.h>

using namespace Alembic;
using namespace Alembic::Abc;
using namespace Alembic::AbcGeom;
using namespace Alembic::AbcMaterial;
using namespace Alembic::AbcCoreAbstract;

CAbcIFaceSet::CAbcIFaceSet( Alembic::Abc::IObject in_object ) : CAbcISchemaObjectImpl< IFaceSet, IAbcIFaceSet, EIObject_FaceSet >( in_object, kWrapExisting )
{
}

// IFaceSetSchema
bool	CAbcIFaceSet::IsConstant() const
{
	return m_Schema.isConstant();
}

EAbcIFaceSetExclusivity	CAbcIFaceSet::GetExclusivity() const
{
	return (EAbcIFaceSetExclusivity)m_Schema.getFaceExclusivity();
}

// Standard Accessors
EAbcResult CAbcIFaceSet::GetSample( IAbcISampleSelector* in_pSampleSelector, IAbcSampleBuffer** out_ppSample )
{
	if ( out_ppSample == NULL )
		return EResult_InvalidPtr;

	ISampleSelector l_ss;
	if ( in_pSampleSelector != NULL )
		((CAbcISampleSelector*)in_pSampleSelector)->GetSampleSelector( l_ss );

	IFaceSetSchema::Sample l_sample;
	m_Schema.get( l_sample, l_ss );
	Int32ArraySamplePtr l_faces =  l_sample.getFaces();
	*out_ppSample = new CSampleBuffer<Int32ArraySamplePtr, Int32ArraySample>( l_faces, EGeomScope_FaceVarying, false );
	if ( !(*out_ppSample) )
	{
		return EResult_OutOfMemory;
	}
	(*out_ppSample)->AddRef();
	return EResult_Success;
}

