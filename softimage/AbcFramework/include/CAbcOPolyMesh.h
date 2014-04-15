//*****************************************************************************
/*!
	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*

#ifndef CABCOPOLYMESH_H
#define CABCOPOLYMESH_H

#include "AbcFrameworkImportedTypes.h"
#include <Alembic/AbcGeom/OPolyMesh.h>
#include "CAbcOObject.h"
#include "CRefCount.h"

class CAbcOPolyMesh : public CAbcOSchemaObjectImpl< Alembic::AbcGeom::OPolyMesh, IAbcOPolyMesh >, protected CRefCount
{
	IMPL_REFCOUNT;
public:
	typedef CAbcOSchemaObjectImpl< Alembic::AbcGeom::OPolyMesh, IAbcOPolyMesh > TheBaseClass;

	CAbcOPolyMesh( IAbcOObject* in_pParent, Alembic::Abc::OObject& in_ParentOObj, const char* in_pszName )
		: TheBaseClass( in_pParent, in_ParentOObj, in_pszName )
	{ }

	EAbcOObjectType GetType() const { return EOObject_Polymesh;}

	virtual EAbcResult CreateFaceSet( IAbcOFaceSet** out_ppOFaceSet, const char* in_pszName );
	
	virtual EAbcResult GetFaceSet( IAbcOFaceSet** out_ppOFaceSet, const char* in_pszName );

	void AddSample( 
		const float* in_pfVertexPositions, int in_iVertexPositionsCount,
		const Alembic::Abc::Box3d& in_BBox
	);

	void AddSample( 
		const float* in_pfVertexPositions, int in_iVertexPositionsCount,
		const int* in_pFaceVertices, int in_iFaceVerticesSize,
		const int* in_pFaceCounts, int in_iFaceCountsSize,
		const Alembic::Abc::Box3d& in_BBox
		);

	
	void AddSample( 
		const float* in_pfVertexPositions, int in_iVertexPositionsCount,
		const int* in_pFaceVertices, int in_iFaceVerticesSize,
		const int* in_pFaceCounts, int in_iFaceCountsSize,
		const float* in_pUVs, int in_iUVCount,
		const unsigned int* in_pNodeIndices, int in_iNodeIndicesCount,
		const unsigned int* in_pVertexNormalIndices, int in_iVertexNormalIndicesCount,
		const float* in_pNormals, int in_iNormalCount,
		const Alembic::Abc::Box3d& in_BBox
	);
};
#endif