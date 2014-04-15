//*****************************************************************************
/*!
	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*

#include "CAbcOPolyMesh.h"
#include <Alembic/AbcCoreAbstract/TimeSampling.h>

void CAbcOPolyMesh::AddSample( 
	const float* in_pfVertexPositions, int in_iVertexPositionsCount, 
	const int* in_pFaceVertices, int in_iFaceVerticesSize, 
	const int* in_pFaceCounts, int in_iFaceCountsSize,
	const Alembic::Abc::Box3d& in_BBox
	)
{
	Alembic::AbcGeom::OPolyMeshSchema& l_meshSchema = GetSchema();

	Alembic::AbcGeom::OPolyMeshSchema::Sample l_meshSample(
		Alembic::Abc::P3fArraySample( (const Alembic::Abc::V3f*)in_pfVertexPositions, in_iVertexPositionsCount ),
		Alembic::Abc::Int32ArraySample( in_pFaceVertices, in_iFaceVerticesSize ),
		Alembic::Abc::Int32ArraySample( in_pFaceCounts, in_iFaceCountsSize )
	);

	l_meshSample.setSelfBounds( in_BBox );

	l_meshSchema.set( l_meshSample );
}

void CAbcOPolyMesh::AddSample( 
	const float* in_pfVertexPositions, int in_iVertexPositionsCount,
	const int* in_pFaceVertices, int in_iFaceVerticesSize,
	const int* in_pFaceCounts, int in_iFaceCountsSize,
	const float* in_pUVs, int in_iUVCount,
	const unsigned int* in_pNodeIndices, int in_iNodeIndicesCount,
	const unsigned int* in_pVertexNormalIndices, int in_iVertexNormalIndicesCount,
	const float* in_pNormals, int in_iNormalCount,
	const Alembic::Abc::Box3d& in_BBox
	)
{
	Alembic::AbcGeom::OPolyMeshSchema& l_meshSchema = GetInternalObject().getSchema();

	Alembic::AbcGeom::OV2fGeomParam::Sample l_UVs;
	if ( in_iUVCount > 0 )
	{
		l_UVs.setScope( Alembic::AbcGeom::kFacevaryingScope );
		l_UVs.setVals( Alembic::Abc::V2fArraySample( (const Alembic::Abc::V2f*)in_pUVs, in_iUVCount ) );

		if ( in_iNodeIndicesCount > 0 )
		{
			l_UVs.setIndices( Alembic::Abc::UInt32ArraySample( in_pNodeIndices, in_iNodeIndicesCount ) );
		}
	}
	
	Alembic::AbcGeom::ON3fGeomParam::Sample l_Normals;
	if ( in_iNormalCount > 0 )
	{
		l_Normals.setScope( Alembic::AbcGeom::kFacevaryingScope );
		l_Normals.setVals( Alembic::Abc::N3fArraySample( (const Alembic::Abc::N3f*)in_pNormals, in_iNormalCount ) ); 
		
		if ( in_iVertexNormalIndicesCount > 0 )
		{
			l_Normals.setIndices( Alembic::Abc::UInt32ArraySample( in_pVertexNormalIndices, in_iVertexNormalIndicesCount ) );
		}
	}

	Alembic::AbcGeom::OPolyMeshSchema::Sample l_meshSample(
		Alembic::Abc::P3fArraySample( (const Alembic::Abc::V3f*)in_pfVertexPositions, in_iVertexPositionsCount ),
		Alembic::Abc::Int32ArraySample( in_pFaceVertices, in_iFaceVerticesSize ),
		Alembic::Abc::Int32ArraySample( in_pFaceCounts, in_iFaceCountsSize ),
		l_UVs,
		l_Normals
	);

	l_meshSample.setSelfBounds( in_BBox );

	l_meshSchema.set( l_meshSample );
}

void CAbcOPolyMesh::AddSample( 
	const float* in_pfVertexPositions, int in_iVertexPositionsCount, 
	const Alembic::Abc::Box3d& in_BBox
	)
{
	Alembic::AbcGeom::OPolyMeshSchema& l_meshSchema = GetInternalObject().getSchema();

	Alembic::AbcGeom::OPolyMeshSchema::Sample l_meshSample(
		Alembic::Abc::P3fArraySample( (const Alembic::Abc::V3f*)in_pfVertexPositions, in_iVertexPositionsCount )
	);

	l_meshSample.setSelfBounds( in_BBox );

	l_meshSchema.set( l_meshSample );
}

EAbcResult CAbcOPolyMesh::CreateFaceSet( IAbcOFaceSet** out_ppOFaceSet, const char* in_pszName )
{
	return CreateChild( (IAbcOObject**)out_ppOFaceSet, EOObject_FaceSet, in_pszName );
}

EAbcResult CAbcOPolyMesh::GetFaceSet( IAbcOFaceSet** out_ppOFaceSet, const char* in_pszName )
{
	if ( in_pszName == NULL || out_ppOFaceSet == NULL )
		return EResult_InvalidPtr;

	CAbcPtr<IAbcOObject> l_spChild;
	if ( GetChild( &l_spChild, in_pszName ) == EResult_Success )
	{
		if ( l_spChild->GetType() == EOObject_FaceSet )
		{
			l_spChild->AddRef();
			*out_ppOFaceSet = (IAbcOFaceSet*)l_spChild.GetPtr();

			return EResult_Success;
		}
	}

	return EResult_Fail;
}