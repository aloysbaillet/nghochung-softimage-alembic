//*****************************************************************************
/*!
	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*

#include "CAbcDataTypeHelper.h"
#include <Alembic/Abc/TypedPropertyTraits.h>
#include <Alembic/AbcGeom/OCamera.h>
#include <Alembic/AbcGeom/OPoints.h>
#include <Alembic/AbcGeom/OPolyMesh.h>
#include <Alembic/AbcGeom/OSubD.h>
#include <Alembic/AbcGeom/OXform.h>

#define INIT_TRAITS_INFO( ENUM, TRAITS ) m_traits[ENUM].Init( TRAITS::interpretation(), TRAITS::dataType() );

class AbcTraitsTable
{
public:
	AbcTraitsTable()
	{
		INIT_TRAITS_INFO( EDataTraits_Bool, Alembic::Abc::BooleanTPTraits );
		INIT_TRAITS_INFO( EDataTraits_UInt8, Alembic::Abc::Uint8TPTraits );
		INIT_TRAITS_INFO( EDataTraits_Int8, Alembic::Abc::Int8TPTraits );
		INIT_TRAITS_INFO( EDataTraits_UInt16, Alembic::Abc::Uint16TPTraits );
		INIT_TRAITS_INFO( EDataTraits_Int16, Alembic::Abc::Int16TPTraits );
		INIT_TRAITS_INFO( EDataTraits_UInt32, Alembic::Abc::Uint32TPTraits );
		INIT_TRAITS_INFO( EDataTraits_Int32, Alembic::Abc::Int32TPTraits );
		INIT_TRAITS_INFO( EDataTraits_UInt64, Alembic::Abc::Uint64TPTraits );
		INIT_TRAITS_INFO( EDataTraits_Int64, Alembic::Abc::Int64TPTraits );
		INIT_TRAITS_INFO( EDataTraits_Float16, Alembic::Abc::Float16TPTraits );
		INIT_TRAITS_INFO( EDataTraits_Float32, Alembic::Abc::Float32TPTraits );
		INIT_TRAITS_INFO( EDataTraits_Float64, Alembic::Abc::Float64TPTraits );
		INIT_TRAITS_INFO( EDataTraits_StringPtr, Alembic::Abc::StringTPTraits );
		INIT_TRAITS_INFO( EDataTraits_WStringPtr, Alembic::Abc::WstringTPTraits );

		INIT_TRAITS_INFO( EDataTraits_V2s, Alembic::Abc::V2sTPTraits );
		INIT_TRAITS_INFO( EDataTraits_V2i, Alembic::Abc::V2iTPTraits );
		INIT_TRAITS_INFO( EDataTraits_V2f, Alembic::Abc::V2fTPTraits );
		INIT_TRAITS_INFO( EDataTraits_V2d, Alembic::Abc::V2dTPTraits );

		INIT_TRAITS_INFO( EDataTraits_V3s, Alembic::Abc::V3sTPTraits );
		INIT_TRAITS_INFO( EDataTraits_V3i, Alembic::Abc::V3iTPTraits );
		INIT_TRAITS_INFO( EDataTraits_V3f, Alembic::Abc::V3fTPTraits );
		INIT_TRAITS_INFO( EDataTraits_V3d, Alembic::Abc::V3dTPTraits );

		INIT_TRAITS_INFO( EDataTraits_P2s, Alembic::Abc::P2sTPTraits );
		INIT_TRAITS_INFO( EDataTraits_P2i, Alembic::Abc::P2iTPTraits );
		INIT_TRAITS_INFO( EDataTraits_P2f, Alembic::Abc::P2fTPTraits );
		INIT_TRAITS_INFO( EDataTraits_P2d, Alembic::Abc::P2dTPTraits );

		INIT_TRAITS_INFO( EDataTraits_P3s, Alembic::Abc::P3sTPTraits );
		INIT_TRAITS_INFO( EDataTraits_P3i, Alembic::Abc::P3iTPTraits );
		INIT_TRAITS_INFO( EDataTraits_P3f, Alembic::Abc::P3fTPTraits );
		INIT_TRAITS_INFO( EDataTraits_P3d, Alembic::Abc::P3dTPTraits );

		INIT_TRAITS_INFO( EDataTraits_Box2s, Alembic::Abc::Box2sTPTraits );
		INIT_TRAITS_INFO( EDataTraits_Box2i, Alembic::Abc::Box2iTPTraits );
		INIT_TRAITS_INFO( EDataTraits_Box2f, Alembic::Abc::Box2fTPTraits );
		INIT_TRAITS_INFO( EDataTraits_Box2d, Alembic::Abc::Box2dTPTraits );

		INIT_TRAITS_INFO( EDataTraits_Box3s, Alembic::Abc::Box3sTPTraits );
		INIT_TRAITS_INFO( EDataTraits_Box3i, Alembic::Abc::Box3iTPTraits );
		INIT_TRAITS_INFO( EDataTraits_Box3f, Alembic::Abc::Box3fTPTraits );
		INIT_TRAITS_INFO( EDataTraits_Box3d, Alembic::Abc::Box3dTPTraits );

		INIT_TRAITS_INFO( EDataTraits_M33f, Alembic::Abc::M33fTPTraits );
		INIT_TRAITS_INFO( EDataTraits_M33d, Alembic::Abc::M33dTPTraits );
		INIT_TRAITS_INFO( EDataTraits_M44f, Alembic::Abc::M44fTPTraits );
		INIT_TRAITS_INFO( EDataTraits_M44d, Alembic::Abc::M44dTPTraits );

		INIT_TRAITS_INFO( EDataTraits_Quatf, Alembic::Abc::QuatfTPTraits );
		INIT_TRAITS_INFO( EDataTraits_Quatd, Alembic::Abc::QuatdTPTraits );

		//-*****************************************************************************
		// colors. For now, using "rgb"/"rgba" as the interpretation
		INIT_TRAITS_INFO( EDataTraits_C3h, Alembic::Abc::C3hTPTraits );
		INIT_TRAITS_INFO( EDataTraits_C3f, Alembic::Abc::C3fTPTraits );
		INIT_TRAITS_INFO( EDataTraits_C3c, Alembic::Abc::C3cTPTraits );

		INIT_TRAITS_INFO( EDataTraits_C4h, Alembic::Abc::C4hTPTraits );
		INIT_TRAITS_INFO( EDataTraits_C4f, Alembic::Abc::C4fTPTraits );
		INIT_TRAITS_INFO( EDataTraits_C4c, Alembic::Abc::C4cTPTraits );

		//-*****************************************************************************
		// Normals.

		INIT_TRAITS_INFO( EDataTraits_N2f, Alembic::Abc::N2fTPTraits );
		INIT_TRAITS_INFO( EDataTraits_N2d, Alembic::Abc::N2dTPTraits );

		// N3f and N3d is typedefed in Foundation from V3f and V3d
		INIT_TRAITS_INFO( EDataTraits_N3f, Alembic::Abc::N3fTPTraits );
		INIT_TRAITS_INFO( EDataTraits_N3d, Alembic::Abc::N3dTPTraits );

		INIT_TRAITS_INFO( EDataTraits_V4f, AbcFramework::V4fTPTraits );
		INIT_TRAITS_INFO( EDataTraits_Rotf, AbcFramework::VRotationfTraits );
	}


	EAbcDataTraits GetDataType( const Alembic::AbcCoreAbstract::PropertyHeader& in_PropHeader )
	{
		TraitsInfo l_traits( in_PropHeader );
		for (int i=0; i<EDataTraits_NumDataTypes; ++i)
		{
			if ( m_traits[i] == l_traits )
				return (EAbcDataTraits)i;
		}
		return EDataTraits_Unknown;
	}

	static AbcTraitsTable& Instance() { return ms_Instance;}

private:
	static AbcTraitsTable ms_Instance;

	struct TraitsInfo 
	{
		std::string m_Interp;
		Alembic::AbcCoreAbstract::DataType m_DataType;

		TraitsInfo( )
		{
		}

		TraitsInfo( const Alembic::AbcCoreAbstract::PropertyHeader& in_PropHeader )
		{
			m_Interp = in_PropHeader.getMetaData().get( "interpretation" );
			m_DataType = in_PropHeader.getDataType();
		}

		void Init( const std::string& in_Name, const Alembic::AbcCoreAbstract::DataType& in_Type )
		{
			m_Interp = in_Name;
			m_DataType.setPod( in_Type.getPod() );
			m_DataType.setExtent( in_Type.getExtent() );
		}

		bool operator==( const TraitsInfo& in_other ) const
		{
			return m_Interp == in_other.m_Interp && m_DataType == in_other.m_DataType;
		}
	};
	
	TraitsInfo m_traits[EDataTraits_NumDataTypes];
};

AbcTraitsTable AbcTraitsTable::ms_Instance;

EAbcOObjectType GetOObjectType( const Alembic::Abc::ObjectHeader& in_ObjHeader )
{
	const Alembic::Abc::MetaData& l_metaData = in_ObjHeader.getMetaData();

	if ( Alembic::AbcGeom::OPolyMeshSchema::matches(l_metaData) )
		return EOObject_Polymesh;
	else if ( Alembic::AbcGeom::OPointsSchema::matches(l_metaData) )
		return EOObject_Points;
	else if ( Alembic::AbcGeom::OSubDSchema::matches(l_metaData) )
		return EOObject_Subd;
	else if ( Alembic::AbcGeom::OXformSchema::matches(l_metaData) )
		return EOObject_Xform;
	else if ( Alembic::AbcGeom::OCameraSchema::matches(l_metaData) )
		return EOObject_Camera;
	else
		return EOObject_Unknown;
}

EAbcPodType GetPodType( const Alembic::AbcCoreAbstract::DataType& in_DataType )
{
	return (EAbcPodType)in_DataType.getPod();
}

EAbcDataTraits GetDataTraits( const Alembic::AbcCoreAbstract::PropertyHeader& in_PropHeader )
{
	return AbcTraitsTable::Instance().GetDataType( in_PropHeader );
}
