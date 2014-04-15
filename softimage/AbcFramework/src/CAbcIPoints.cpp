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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CAbcIPoints
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CAbcIPoints::CAbcIPoints( Alembic::Abc::IObject in_object ) : CAbcISchemaObjectImpl< IPoints, IAbcIPoints, EIObject_Points >( in_object, kWrapExisting )
{
}

// IPointsSchema
bool CAbcIPoints::IsConstant() const
{
	return m_Schema.isConstant();
}
	
// Standard Accessors
EAbcResult CAbcIPoints::GetPositions( IAbcISampleSelector* in_pSampleSelector, IAbcSampleBuffer** out_ppBuffer )
{
	if ( out_ppBuffer == NULL )
		return EResult_InvalidPtr;

	ISampleSelector l_ss;
	if ( in_pSampleSelector )
		( ( CAbcISampleSelector* )in_pSampleSelector )->GetSampleSelector( l_ss );

	IPointsSchema::Sample l_Sample;
	l_Sample = m_Schema.getValue( l_ss );

	P3fArraySamplePtr l_SamplePtr = l_Sample.getPositions();
	return CreateBuffer<P3fArraySamplePtr, P3fArraySample>( l_SamplePtr, out_ppBuffer );
}

EAbcResult CAbcIPoints::GetVelocities( IAbcISampleSelector* in_pSampleSelector, IAbcSampleBuffer** out_ppBuffer )
{
	if ( out_ppBuffer == NULL )
		return EResult_InvalidPtr;

	ISampleSelector l_ss;
	if ( in_pSampleSelector )
		( ( CAbcISampleSelector* )in_pSampleSelector )->GetSampleSelector( l_ss );

	IPointsSchema::Sample l_Sample;
	l_Sample = m_Schema.getValue( l_ss );

	V3fArraySamplePtr l_SamplePtr = l_Sample.getVelocities();
	return CreateBuffer<V3fArraySamplePtr, V3fArraySample>( l_SamplePtr, out_ppBuffer );
}

EAbcResult CAbcIPoints::GetIds( IAbcISampleSelector* in_pSampleSelector, IAbcSampleBuffer** out_ppBuffer )
{
	if ( out_ppBuffer == NULL )
		return EResult_InvalidPtr;

	ISampleSelector l_ss;
	if ( in_pSampleSelector )
		( ( CAbcISampleSelector* )in_pSampleSelector )->GetSampleSelector( l_ss );

	IPointsSchema::Sample l_Sample;
	l_Sample = m_Schema.getValue( l_ss );

	UInt64ArraySamplePtr l_SamplePtr = l_Sample.getIds();
	return CreateBuffer<UInt64ArraySamplePtr, UInt64ArraySample>( l_SamplePtr, out_ppBuffer );
}

template<typename TSamplePtrType, typename TSampleType >
static EAbcResult CAbcIPoints::CreateBuffer( TSamplePtrType& in_ptr, IAbcSampleBuffer** out_ppBuffer )
{
	*out_ppBuffer = new CSampleBuffer<TSamplePtrType, TSampleType>( in_ptr, EGeomScope_Vertex, false );
	if ( !(*out_ppBuffer) )
	{
		return EResult_OutOfMemory;
	}
	(*out_ppBuffer)->AddRef();
	return EResult_Success;
}
