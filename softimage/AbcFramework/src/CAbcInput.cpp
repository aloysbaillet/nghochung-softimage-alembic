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

EAbcIObjectType GetEIObjectType( Alembic::Abc::IObject& in_object )
{
	AbcA::MetaData l_md = in_object.getMetaData();

	if ( IPolyMesh::matches( l_md ) )
		return EIObject_Polymesh;
	else if ( ISubD::matches( l_md ) )
		return EIObject_Subd;
	else if ( IPoints::matches( l_md ) )
		return EIObject_Points;
	else if ( IMaterial::matches( l_md ) )
		return EIObject_Material;
	else if ( ICamera::matches( l_md ) )
		return EIObject_Camera;
	else if ( IXform::matches( l_md ) )
		return EIObject_Xform;
	else if ( ICurves::matches( l_md ) )
		return EIObject_Curves;
	else if ( INuPatch::matches( l_md ) )
		return EIObject_NuPatch;
	else if ( IFaceSet::matches( l_md ) )
		return EIObject_FaceSet;
	else
		return EIObject_Unknown;
}


EAbcResult CreateFromInternal( Alembic::Abc::IObject in_object, IAbcIObject** out_ppObject )
{
	EAbcIObjectType l_eType = GetEIObjectType( in_object );
	switch( l_eType )
	{
	case EIObject_Polymesh:
		{
			CAbcIPolyMesh* l_pPolyMesh = new CAbcIPolyMesh( in_object );
			if ( l_pPolyMesh )
			{
				*out_ppObject = l_pPolyMesh;
				l_pPolyMesh->AddRef();
				return EResult_Success;
			}
			return EResult_OutOfMemory;
		}
		break;
	case EIObject_Xform:
		{
			CAbcIXform* l_pTransform = new CAbcIXform( in_object );
			if ( l_pTransform )
			{
				*out_ppObject = l_pTransform;
				l_pTransform->AddRef();
				return EResult_Success;
			}
			return EResult_OutOfMemory;
		}
		break;
	case EIObject_Points:
		{
			CAbcIPoints* l_pPoints = new CAbcIPoints( in_object );
			if ( l_pPoints )
			{
				*out_ppObject = l_pPoints;
				l_pPoints->AddRef();
				return EResult_Success;
			}
			return EResult_OutOfMemory;
		}
	case EIObject_Camera:
		{
			CAbcICamera* l_pCamera = new CAbcICamera( in_object );
			if ( l_pCamera )
			{
				*out_ppObject = l_pCamera;
				l_pCamera->AddRef();
				return EResult_Success;
			}
			return EResult_OutOfMemory;
		}
	case EIObject_FaceSet:
		{
			CAbcIFaceSet* l_pFaceSet = new CAbcIFaceSet( in_object );
			if ( l_pFaceSet )
			{
				*out_ppObject = l_pFaceSet;
				l_pFaceSet->AddRef();
				return EResult_Success;
			}
			return EResult_OutOfMemory;
		}
	case EIObject_Unknown:
		// TODO:
	case EIObject_Subd:
	case EIObject_Material:
	case EIObject_Curves:
		{
			CAbcIObject* l_pObj = new CAbcIObject( in_object );
			if ( l_pObj )
			{
				*out_ppObject = l_pObj;
				l_pObj->AddRef();
				return EResult_Success;
			}
			return EResult_OutOfMemory;
		}


	default:
		return EResult_Fail; // Not supported yet
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CAbcIObject
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CAbcIObject::CAbcIObject( IObject in_object ) : CAbcIObjectImpl< IObject, IAbcIObject >( in_object )
{

}

EAbcIObjectType CAbcIObject::GetType() const
{
	AbcA::MetaData l_md = m_Object.getMetaData();

	if ( IPolyMesh::matches( l_md ) )
		return EIObject_Polymesh;
	else if ( ISubD::matches( l_md ) )
		return EIObject_Subd;
	else if ( IPoints::matches( l_md ) )
		return EIObject_Points;
	else if ( IMaterial::matches( l_md ) )
		return EIObject_Material;
	else if ( ICamera::matches( l_md ) )
		return EIObject_Camera;
	else if ( IXform::matches( l_md ) )
		return EIObject_Xform;
	else if ( ICurves::matches( l_md ) )
		return EIObject_Curves;
	else if ( INuPatch::matches( l_md ) )
		return EIObject_NuPatch;
	else if ( IFaceSet::matches( l_md ) )
		return EIObject_FaceSet;
	else
		return EIObject_Unknown;
}

EAbcResult CAbcIObject::TransformInto( EAbcIObjectType in_objType, IAbcIObject** out_ppObject )
{
	if ( !out_ppObject )
		return EResult_InvalidPtr;

	if ( GetType() == in_objType )
	{
		this->AddRef();
		*out_ppObject = this;
		return EResult_Success;
	}
	return EResult_InvalidCast;
}

EAbcResult CAbcIObject::GetSchemaObject( IAbcISchemaObject** out_ppObject ) const
{
	if ( !out_ppObject )
		return EResult_InvalidPtr;
	return CreateFromInternal( m_Object, (IAbcIObject**)out_ppObject );
}
