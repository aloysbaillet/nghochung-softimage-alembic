//*****************************************************************************
/*!
	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*

#ifndef CABCOPOINTS_H
#define CABCOPOINTS_H

#include "AbcFrameworkImportedTypes.h"
#include <Alembic/AbcGeom/OPoints.h>
#include "CAbcOObject.h"
#include "CRefCount.h"

class CAbcOPoints : public CAbcOSchemaObjectImpl< Alembic::AbcGeom::OPoints, IAbcOPoints>, protected CRefCount
{
	IMPL_REFCOUNT;
public:
	typedef CAbcOSchemaObjectImpl< Alembic::AbcGeom::OPoints, IAbcOPoints > TheBaseClass;

	CAbcOPoints( IAbcOObject* in_pParent, Alembic::Abc::OObject& in_ParentOObj, const char* in_pszName )
		: TheBaseClass( in_pParent, in_ParentOObj, in_pszName )
	{

	}

	EAbcOObjectType GetType() const { return EOObject_Points;}

	virtual unsigned int GetInstancedGeomIndex( unsigned int in_ulDBID )
	{
		DBIDToInfoMap::const_iterator it = m_InstanceGeomDBIDToIndex.find( in_ulDBID );
		if ( it == m_InstanceGeomDBIDToIndex.end() )
		{
			unsigned int l_ulIdx = (unsigned int)m_InstanceGeomDBIDToIndex.size();
			m_InstanceGeomDBIDToIndex[ in_ulDBID ] = l_ulIdx;
			return l_ulIdx;
		}
		else
		{
			return it->second.m_ulIndex;
		}
	}

	virtual void		SetHierarchyInstancedGeom( unsigned int in_ulDBID )
	{
		DBIDToInfoMap::iterator it = m_InstanceGeomDBIDToIndex.find( in_ulDBID );
		if ( it == m_InstanceGeomDBIDToIndex.end() )
		{
			unsigned int l_ulIdx = (unsigned int)m_InstanceGeomDBIDToIndex.size();
			m_InstanceGeomDBIDToIndex[ in_ulDBID ] = InstancedGeomInfo( l_ulIdx, true );
		}
		else
		{
			it->second.m_bHierarchy = true;
		}
	}

	virtual unsigned int		GetNbInstancedGeom() const
	{
		return ( unsigned int )m_InstanceGeomDBIDToIndex.size();
	}

	virtual EAbcResult	GetInstancedGeom( unsigned int** out_ppDBIDs, bool** out_ppInstancedHierarchy ) const
	{
		if ( out_ppDBIDs )
		{
			for ( DBIDToInfoMap::const_iterator it = m_InstanceGeomDBIDToIndex.begin(); it != m_InstanceGeomDBIDToIndex.end(); ++it )
			{
				(*out_ppDBIDs)[it->second.m_ulIndex] = it->first;
			}
		}

		if ( out_ppInstancedHierarchy )
		{
			for ( DBIDToInfoMap::const_iterator it = m_InstanceGeomDBIDToIndex.begin(); it != m_InstanceGeomDBIDToIndex.end(); ++it )
			{
				(*out_ppInstancedHierarchy)[it->second.m_ulIndex] = it->second.m_bHierarchy;
			}
		}


		return EResult_Success;
	}

	virtual EAbcResult GetInstancedGeomProperty( IAbcOProperty** out_ppProp )
	{
		if ( out_ppProp == NULL )
			return EResult_InvalidPtr;

		CAbcPtr< IAbcOTypedArrayProperty<Alembic::Abc::StringPODTraits> > l_spInstancedGeomsProp;
		CAbcPtr< IAbcOCompoundProperty > l_spUserProps;
		GetUserProperties( &l_spUserProps );

		if ( l_spUserProps->GetProperty( (IAbcOProperty**)&l_spInstancedGeomsProp, PROPNAME_SHAPEINSTANCEDGEOMS ) == EResult_Success )
		{
			l_spInstancedGeomsProp->AddRef();
			*out_ppProp = l_spInstancedGeomsProp.GetPtr();
		}
		else
		{
			l_spUserProps->CreateProperty( (IAbcOProperty**)&l_spInstancedGeomsProp, PROPNAME_SHAPEINSTANCEDGEOMS, EPropertyType_Array, EDataTraits_StringPtr );
			l_spInstancedGeomsProp->AddRef();
			*out_ppProp = l_spInstancedGeomsProp.GetPtr();
		}

		return EResult_Success;
	}

	void AddSample( const float* in_pfPointPositions, int in_iPointCount, const Alembic::Abc::Box3d& in_BBox );

	void AddSample( 
		const float* in_pfPointPositions, int in_iPointCount,
		const unsigned long long* in_pIds, int in_iIdCount,
		const Alembic::Abc::Box3d& in_BBox
		);

	void AddSample( 
		const float* in_pfPointPositions, int in_iPointCount,
		const float* in_pVelocities, int in_iVelocityCount,
		const float* in_pWidths, int in_iWidthCount,
		const Alembic::Abc::Box3d& in_BBox
		);

	void AddSample( 
		const float* in_pfPointPositions, int in_iPointCount,
		const unsigned long long* in_pIds, int in_iIdCount,
		const float* in_pVelocities, int in_iVelocityCount,
		const float* in_pWidths, int in_iWidthCount,
		const Alembic::Abc::Box3d& in_BBox
		);

private:
	struct InstancedGeomInfo
	{
		InstancedGeomInfo( ) : m_ulIndex( UINT_MAX ), m_bHierarchy( false ) {}
		InstancedGeomInfo( unsigned int in_ulIndex ) : m_ulIndex( in_ulIndex ), m_bHierarchy( false ) {}
		InstancedGeomInfo( unsigned int in_ulIndex, bool in_bHierarchy ) : m_ulIndex( in_ulIndex ), m_bHierarchy( in_bHierarchy ) {}

		unsigned int m_ulIndex;
		bool m_bHierarchy;
	};

	typedef std::map<unsigned int, InstancedGeomInfo> DBIDToInfoMap;
	DBIDToInfoMap m_InstanceGeomDBIDToIndex;
};
#endif