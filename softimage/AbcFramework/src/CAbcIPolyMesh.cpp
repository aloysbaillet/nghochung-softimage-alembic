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
// CAbcIPolyMesh
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CAbcIPolyMesh::CAbcIPolyMesh( Alembic::Abc::IObject in_object ) : CAbcISchemaObjectImpl< IPolyMesh, IAbcIPolyMesh, EIObject_Polymesh >( in_object, kWrapExisting )
{
}

EAbcTopoVariance CAbcIPolyMesh::GetTopologyVariance() const
{
	return (EAbcTopoVariance)(unsigned int)m_Schema.getTopologyVariance();
}

bool CAbcIPolyMesh::IsConstant() const
{
	return m_Schema.isConstant();
}	

EAbcResult CAbcIPolyMesh::GetUVs( bool in_bExpand, IAbcISampleSelector* in_pSampleSelector, IAbcSampleBuffer** out_ppBuffer )
{
	if ( !out_ppBuffer ) 
		return EResult_InvalidPtr;

	IV2fGeomParam l_uvParams = m_Schema.getUVsParam();
	if ( l_uvParams.valid() )
	{
		ISampleSelector l_Selector;
		if ( in_pSampleSelector )
			( ( CAbcISampleSelector* )in_pSampleSelector )->GetSampleSelector( l_Selector );

		IV2fGeomParam::Sample l_uvSamp = in_bExpand ? l_uvParams.getExpandedValue( l_Selector ) : l_uvParams.getIndexedValue( l_Selector );
		V2fArraySamplePtr l_values = l_uvSamp.getVals();

		return CreateBuffer<V2fArraySamplePtr, V2fArraySample>( l_values, (EAbcGeomScope)l_uvParams.getScope(), l_uvParams.isIndexed(), out_ppBuffer );
	}
	return EResult_InvalidProp;
}

EAbcResult CAbcIPolyMesh::GetNormals( bool in_bExpand, IAbcISampleSelector* in_pSampleSelector, IAbcSampleBuffer** out_ppBuffer )
{
	if ( !out_ppBuffer ) 
		return EResult_InvalidPtr;

	IN3fGeomParam l_normalParam = m_Schema.getNormalsParam();
	if ( l_normalParam.valid() )
	{
		ISampleSelector l_Selector;
		if ( in_pSampleSelector )
			( ( CAbcISampleSelector* )in_pSampleSelector )->GetSampleSelector( l_Selector );

		IN3fGeomParam::Sample l_normalSamp = in_bExpand ? l_normalParam.getExpandedValue( l_Selector ) : l_normalParam.getIndexedValue( l_Selector );	
		N3fArraySamplePtr l_values = l_normalSamp.getVals();

		return CreateBuffer<N3fArraySamplePtr, N3fArraySample>( l_values, (EAbcGeomScope)l_normalParam.getScope(), l_normalParam.isIndexed(), out_ppBuffer );
	}
	return EResult_InvalidProp;
}

EAbcResult CAbcIPolyMesh::GetPositions( IAbcISampleSelector* in_pSampleSelector, IAbcSampleBuffer** out_ppBuffer )
{
	if ( !out_ppBuffer ) 
		return EResult_InvalidPtr;
	IP3fArrayProperty l_positionsProp = m_Schema.getPositionsProperty();
	
	if ( l_positionsProp.valid() )
	{
		ISampleSelector l_Selector;
		if ( in_pSampleSelector )
			( ( CAbcISampleSelector* )in_pSampleSelector )->GetSampleSelector( l_Selector );

		IP3fArrayProperty::sample_ptr_type l_positionsSamp;
		l_positionsProp.get( l_positionsSamp, l_Selector );
		return CreateBuffer<IP3fArrayProperty::sample_ptr_type, IP3fArrayProperty::sample_type>( l_positionsSamp, (EAbcGeomScope)GetGeometryScope( l_positionsProp.getMetaData() ), true, out_ppBuffer );
	}

	return EResult_InvalidProp;
}

EAbcResult CAbcIPolyMesh::GetVelocities( IAbcISampleSelector* in_pSampleSelector, IAbcSampleBuffer** out_ppBuffer )
{
	if ( !out_ppBuffer ) 
		return EResult_InvalidPtr;
	IV3fArrayProperty l_velProp = m_Schema.getVelocitiesProperty();
	
	if ( l_velProp.valid() )
	{
		ISampleSelector l_Selector;
		if ( in_pSampleSelector )
			( ( CAbcISampleSelector* )in_pSampleSelector )->GetSampleSelector( l_Selector );

		IV3fArrayProperty::sample_ptr_type l_velSamp;
		l_velProp.get( l_velSamp, l_Selector );
		return CreateBuffer<IV3fArrayProperty::sample_ptr_type, IV3fArrayProperty::sample_type>( l_velSamp, EGeomScope_Vertex, true, out_ppBuffer );
	}

	return EResult_InvalidProp;
}

EAbcResult CAbcIPolyMesh::GetFaceCounts( IAbcISampleSelector* in_pSampleSelector, IAbcSampleBuffer** out_ppBuffer )
{
	if ( !out_ppBuffer ) 
		return EResult_InvalidPtr;

	IInt32ArrayProperty l_faceCountsProp = m_Schema.getFaceCountsProperty();

	if ( l_faceCountsProp.valid() )
	{
		ISampleSelector l_Selector;
		if ( in_pSampleSelector )
			( ( CAbcISampleSelector* )in_pSampleSelector )->GetSampleSelector( l_Selector );

		IInt32ArrayProperty::sample_ptr_type l_faceCountsSamp;
		l_faceCountsProp.get( l_faceCountsSamp, l_Selector );
		return CreateBuffer<IInt32ArrayProperty::sample_ptr_type, IInt32ArrayProperty::sample_type>( l_faceCountsSamp, (EAbcGeomScope)GetGeometryScope( l_faceCountsProp.getMetaData() ), false, out_ppBuffer );
	}
	return EResult_InvalidProp;
}

EAbcResult CAbcIPolyMesh::GetFaceIndices( IAbcISampleSelector* in_pSampleSelector, IAbcSampleBuffer** out_ppBuffer )
{
	if ( !out_ppBuffer ) 
		return EResult_InvalidPtr;

	IInt32ArrayProperty l_faceIndicesProp = m_Schema.getFaceIndicesProperty();

	if ( l_faceIndicesProp.valid() )
	{
		ISampleSelector l_Selector;
		if ( in_pSampleSelector )
			( ( CAbcISampleSelector* )in_pSampleSelector )->GetSampleSelector( l_Selector );

		IInt32ArrayProperty::sample_ptr_type l_faceIndicesSamp;
		l_faceIndicesProp.get( l_faceIndicesSamp, l_Selector );
		return CreateBuffer<IInt32ArrayProperty::sample_ptr_type, IInt32ArrayProperty::sample_type>( l_faceIndicesSamp, (EAbcGeomScope)GetGeometryScope( l_faceIndicesProp.getMetaData() ), false, out_ppBuffer );
	}
	return EResult_InvalidProp;
}

EAbcResult CAbcIPolyMesh::GetUVsParam( IAbcIGeomParam** out_ppGeomParam )
{
	if ( !out_ppGeomParam )
		return EResult_InvalidPtr;

	IV2fGeomParam l_uvParams = m_Schema.getUVsParam();
	if ( l_uvParams.valid() )
	{
		*out_ppGeomParam = new CAbcIGeomParamImpl<Alembic::Abc::V2fTPTraits>( l_uvParams.getParent(), l_uvParams.getName() );
		(*out_ppGeomParam)->AddRef();
		return EResult_Success;
	}
	
	return EResult_InvalidProp;
}

EAbcResult CAbcIPolyMesh::GetNormalsParam( IAbcIGeomParam** out_ppGeomParam )
{
	if ( !out_ppGeomParam )
		return EResult_InvalidPtr;

	IN3fGeomParam l_normalParams = m_Schema.getNormalsParam();
	if ( l_normalParams.valid() )
	{
		*out_ppGeomParam = new CAbcIGeomParamImpl<Alembic::Abc::N3fTPTraits>( l_normalParams.getParent(), l_normalParams.getName() );
		(*out_ppGeomParam)->AddRef();
		return EResult_Success;
	}

	return EResult_InvalidProp;
}
