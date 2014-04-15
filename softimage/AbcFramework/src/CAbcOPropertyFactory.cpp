//*****************************************************************************
/*!
	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*

#include "CAbcDataTypeHelper.h"
#include "CAbcOutput_Helpers.h"
#include "CAbcOPropertyFactory.h"
#include "CAbcOProperty.h"
#include "CAbcOTypedScalarProperty.h"
#include "CAbcOTypedArrayProperty.h"
#include "CAbcOCompoundProperty.h"
#include "CAbcOTypedGeomParam.h"

#include <boost/smart_ptr/intrusive_ptr.hpp>

EAbcResult CAbcOPropertyFactory::CreateOProperty( 
	IAbcOProperty** out_ppProp, 
	Alembic::Abc::OCompoundProperty& in_parent, 
	const char* in_pszName, 
	EAbcPropertyType in_propType, 
	EAbcDataTraits in_DataType
)
{
	IAbcOProperty* l_pProperty = NULL;

#define CASE_CREATE_SCALAR_PROP( ETYPE, TRAITS ) case ETYPE: l_pProperty = new CAbcOTypedScalarProperty< TRAITS >( in_parent, in_pszName ); break
#define CASE_CREATE_ARRAY_PROP( ETYPE, TRAITS ) case ETYPE: l_pProperty = new CAbcOTypedArrayProperty< TRAITS >( in_parent, in_pszName ); break

	switch( in_propType )
	{
	case EPropertyType_Scalar:
		{
			switch ( in_DataType )
			{
				CASE_CREATE_SCALAR_PROP( EDataTraits_Bool, Alembic::Abc::BooleanTPTraits );
				CASE_CREATE_SCALAR_PROP( EDataTraits_UInt8, Alembic::Abc::Uint8TPTraits );
				CASE_CREATE_SCALAR_PROP( EDataTraits_Int8, Alembic::Abc::Int8TPTraits );
				CASE_CREATE_SCALAR_PROP( EDataTraits_UInt16, Alembic::Abc::Uint16TPTraits);
				CASE_CREATE_SCALAR_PROP( EDataTraits_Int16, Alembic::Abc::Int16TPTraits );
				CASE_CREATE_SCALAR_PROP( EDataTraits_UInt32, Alembic::Abc::Uint32TPTraits );
				CASE_CREATE_SCALAR_PROP( EDataTraits_Int32, Alembic::Abc::Int32TPTraits );
				CASE_CREATE_SCALAR_PROP( EDataTraits_UInt64, Alembic::Abc::Uint64TPTraits);
				CASE_CREATE_SCALAR_PROP( EDataTraits_Int64, Alembic::Abc::Int64TPTraits );
				CASE_CREATE_SCALAR_PROP( EDataTraits_Float16, Alembic::Abc::Float16TPTraits );
				CASE_CREATE_SCALAR_PROP( EDataTraits_Float32, Alembic::Abc::Float32TPTraits );
				CASE_CREATE_SCALAR_PROP( EDataTraits_Float64, Alembic::Abc::Float64TPTraits );

				CASE_CREATE_SCALAR_PROP( EDataTraits_StringPtr, Alembic::Abc::StringTPTraits );
				CASE_CREATE_SCALAR_PROP( EDataTraits_WStringPtr, Alembic::Abc::WstringTPTraits );

				CASE_CREATE_SCALAR_PROP( EDataTraits_V2s , Alembic::Abc::V2sTPTraits );
				CASE_CREATE_SCALAR_PROP( EDataTraits_V2i , Alembic::Abc::V2iTPTraits );
				CASE_CREATE_SCALAR_PROP( EDataTraits_V2f , Alembic::Abc::V2fTPTraits );
				CASE_CREATE_SCALAR_PROP( EDataTraits_V2d , Alembic::Abc::V2dTPTraits );
				
				CASE_CREATE_SCALAR_PROP( EDataTraits_V3s , Alembic::Abc::V3sTPTraits );
				CASE_CREATE_SCALAR_PROP( EDataTraits_V3i , Alembic::Abc::V3iTPTraits );
				CASE_CREATE_SCALAR_PROP( EDataTraits_V3f , Alembic::Abc::V3fTPTraits );
				CASE_CREATE_SCALAR_PROP( EDataTraits_V3d , Alembic::Abc::V3dTPTraits );

				CASE_CREATE_SCALAR_PROP( EDataTraits_P2s, Alembic::Abc::P2sTPTraits );
				CASE_CREATE_SCALAR_PROP( EDataTraits_P2i, Alembic::Abc::P2iTPTraits );  
				CASE_CREATE_SCALAR_PROP( EDataTraits_P2f, Alembic::Abc::P2fTPTraits );  
				CASE_CREATE_SCALAR_PROP( EDataTraits_P2d, Alembic::Abc::P2dTPTraits );  

				CASE_CREATE_SCALAR_PROP( EDataTraits_P3s, Alembic::Abc::P3sTPTraits );  
				CASE_CREATE_SCALAR_PROP( EDataTraits_P3i, Alembic::Abc::P3iTPTraits );  
				CASE_CREATE_SCALAR_PROP( EDataTraits_P3f, Alembic::Abc::P3fTPTraits );  
				CASE_CREATE_SCALAR_PROP( EDataTraits_P3d, Alembic::Abc::P3dTPTraits );  

				CASE_CREATE_SCALAR_PROP( EDataTraits_Box2s, Alembic::Abc::Box2sTPTraits );
				CASE_CREATE_SCALAR_PROP( EDataTraits_Box2i, Alembic::Abc::Box2iTPTraits );
				CASE_CREATE_SCALAR_PROP( EDataTraits_Box2f, Alembic::Abc::Box2fTPTraits );
				CASE_CREATE_SCALAR_PROP( EDataTraits_Box2d, Alembic::Abc::Box2dTPTraits );
				
				CASE_CREATE_SCALAR_PROP( EDataTraits_Box3s, Alembic::Abc::Box3sTPTraits );
				CASE_CREATE_SCALAR_PROP( EDataTraits_Box3i, Alembic::Abc::Box3iTPTraits );
				CASE_CREATE_SCALAR_PROP( EDataTraits_Box3f, Alembic::Abc::Box3fTPTraits );
				CASE_CREATE_SCALAR_PROP( EDataTraits_Box3d, Alembic::Abc::Box3dTPTraits );
				
				CASE_CREATE_SCALAR_PROP( EDataTraits_M33f, Alembic::Abc::M33fTPTraits );
				CASE_CREATE_SCALAR_PROP( EDataTraits_M33d, Alembic::Abc::M33dTPTraits );
				CASE_CREATE_SCALAR_PROP( EDataTraits_M44f, Alembic::Abc::M44fTPTraits );
				CASE_CREATE_SCALAR_PROP( EDataTraits_M44d, Alembic::Abc::M44dTPTraits );
				
				CASE_CREATE_SCALAR_PROP( EDataTraits_Quatf, Alembic::Abc::QuatfTPTraits );
				CASE_CREATE_SCALAR_PROP( EDataTraits_Quatd, Alembic::Abc::QuatdTPTraits );
				
				CASE_CREATE_SCALAR_PROP( EDataTraits_C3h , Alembic::Abc::C3hTPTraits );
				CASE_CREATE_SCALAR_PROP( EDataTraits_C3f , Alembic::Abc::C3fTPTraits );
				CASE_CREATE_SCALAR_PROP( EDataTraits_C3c , Alembic::Abc::C3cTPTraits );
				
				CASE_CREATE_SCALAR_PROP( EDataTraits_C4h , Alembic::Abc::C4hTPTraits );
				CASE_CREATE_SCALAR_PROP( EDataTraits_C4f , Alembic::Abc::C4fTPTraits );
				CASE_CREATE_SCALAR_PROP( EDataTraits_C4c , Alembic::Abc::C4cTPTraits );

				CASE_CREATE_SCALAR_PROP( EDataTraits_N2f, Alembic::Abc::N2fTPTraits );
				CASE_CREATE_SCALAR_PROP( EDataTraits_N2d , Alembic::Abc::N2dTPTraits );

				CASE_CREATE_SCALAR_PROP( EDataTraits_N3f, Alembic::Abc::N3fTPTraits );
				CASE_CREATE_SCALAR_PROP( EDataTraits_N3d , Alembic::Abc::N3dTPTraits );

				CASE_CREATE_SCALAR_PROP( EDataTraits_V4f , AbcFramework::V4fTPTraits );
				CASE_CREATE_SCALAR_PROP( EDataTraits_Rotf, AbcFramework::VRotationfTraits );
			}
		}
		break;
	case EPropertyType_Array:
		{
			switch ( in_DataType )
			{
				CASE_CREATE_ARRAY_PROP( EDataTraits_Bool, Alembic::Abc::BooleanTPTraits );
				CASE_CREATE_ARRAY_PROP( EDataTraits_UInt8, Alembic::Abc::Uint8TPTraits );
				CASE_CREATE_ARRAY_PROP( EDataTraits_Int8, Alembic::Abc::Int8TPTraits );
				CASE_CREATE_ARRAY_PROP( EDataTraits_UInt16, Alembic::Abc::Uint16TPTraits);
				CASE_CREATE_ARRAY_PROP( EDataTraits_Int16, Alembic::Abc::Int16TPTraits );
				CASE_CREATE_ARRAY_PROP( EDataTraits_UInt32, Alembic::Abc::Uint32TPTraits );
				CASE_CREATE_ARRAY_PROP( EDataTraits_Int32, Alembic::Abc::Int32TPTraits );
				CASE_CREATE_ARRAY_PROP( EDataTraits_UInt64, Alembic::Abc::Uint64TPTraits);
				CASE_CREATE_ARRAY_PROP( EDataTraits_Int64, Alembic::Abc::Int64TPTraits );
				CASE_CREATE_ARRAY_PROP( EDataTraits_Float16, Alembic::Abc::Float16TPTraits );
				CASE_CREATE_ARRAY_PROP( EDataTraits_Float32, Alembic::Abc::Float32TPTraits );
				CASE_CREATE_ARRAY_PROP( EDataTraits_Float64, Alembic::Abc::Float64TPTraits );

				CASE_CREATE_ARRAY_PROP( EDataTraits_StringPtr, Alembic::Abc::StringTPTraits );
				CASE_CREATE_ARRAY_PROP( EDataTraits_WStringPtr, Alembic::Abc::WstringTPTraits );
						
				CASE_CREATE_ARRAY_PROP( EDataTraits_V2s , Alembic::Abc::V2sTPTraits );
				CASE_CREATE_ARRAY_PROP( EDataTraits_V2i , Alembic::Abc::V2iTPTraits );
				CASE_CREATE_ARRAY_PROP( EDataTraits_V2f , Alembic::Abc::V2fTPTraits );
				CASE_CREATE_ARRAY_PROP( EDataTraits_V2d , Alembic::Abc::V2dTPTraits );
						
				CASE_CREATE_ARRAY_PROP( EDataTraits_V3s , Alembic::Abc::V3sTPTraits );
				CASE_CREATE_ARRAY_PROP( EDataTraits_V3i , Alembic::Abc::V3iTPTraits );
				CASE_CREATE_ARRAY_PROP( EDataTraits_V3f , Alembic::Abc::V3fTPTraits );
				CASE_CREATE_ARRAY_PROP( EDataTraits_V3d , Alembic::Abc::V3dTPTraits );

				CASE_CREATE_ARRAY_PROP( EDataTraits_P2s, Alembic::Abc::P2sTPTraits );
				CASE_CREATE_ARRAY_PROP( EDataTraits_P2i, Alembic::Abc::P2iTPTraits );  
				CASE_CREATE_ARRAY_PROP( EDataTraits_P2f, Alembic::Abc::P2fTPTraits );  
				CASE_CREATE_ARRAY_PROP( EDataTraits_P2d, Alembic::Abc::P2dTPTraits );  
							
				CASE_CREATE_ARRAY_PROP( EDataTraits_P3s, Alembic::Abc::P3sTPTraits );  
				CASE_CREATE_ARRAY_PROP( EDataTraits_P3i, Alembic::Abc::P3iTPTraits );  
				CASE_CREATE_ARRAY_PROP( EDataTraits_P3f, Alembic::Abc::P3fTPTraits );  
				CASE_CREATE_ARRAY_PROP( EDataTraits_P3d, Alembic::Abc::P3dTPTraits );  
						
				CASE_CREATE_ARRAY_PROP( EDataTraits_Box2s, Alembic::Abc::Box2sTPTraits );
				CASE_CREATE_ARRAY_PROP( EDataTraits_Box2i, Alembic::Abc::Box2iTPTraits );
				CASE_CREATE_ARRAY_PROP( EDataTraits_Box2f, Alembic::Abc::Box2fTPTraits );
				CASE_CREATE_ARRAY_PROP( EDataTraits_Box2d, Alembic::Abc::Box2dTPTraits );
						
				CASE_CREATE_ARRAY_PROP( EDataTraits_Box3s, Alembic::Abc::Box3sTPTraits );
				CASE_CREATE_ARRAY_PROP( EDataTraits_Box3i, Alembic::Abc::Box3iTPTraits );
				CASE_CREATE_ARRAY_PROP( EDataTraits_Box3f, Alembic::Abc::Box3fTPTraits );
				CASE_CREATE_ARRAY_PROP( EDataTraits_Box3d, Alembic::Abc::Box3dTPTraits );
						
				CASE_CREATE_ARRAY_PROP( EDataTraits_M33f, Alembic::Abc::M33fTPTraits );
				CASE_CREATE_ARRAY_PROP( EDataTraits_M33d, Alembic::Abc::M33dTPTraits );
				CASE_CREATE_ARRAY_PROP( EDataTraits_M44f, Alembic::Abc::M44fTPTraits );
				CASE_CREATE_ARRAY_PROP( EDataTraits_M44d, Alembic::Abc::M44dTPTraits );
						
				CASE_CREATE_ARRAY_PROP( EDataTraits_Quatf, Alembic::Abc::QuatfTPTraits );
				CASE_CREATE_ARRAY_PROP( EDataTraits_Quatd, Alembic::Abc::QuatdTPTraits );
						
				CASE_CREATE_ARRAY_PROP( EDataTraits_C3h , Alembic::Abc::C3hTPTraits );
				CASE_CREATE_ARRAY_PROP( EDataTraits_C3f , Alembic::Abc::C3fTPTraits );
				CASE_CREATE_ARRAY_PROP( EDataTraits_C3c , Alembic::Abc::C3cTPTraits );
						
				CASE_CREATE_ARRAY_PROP( EDataTraits_C4h , Alembic::Abc::C4hTPTraits );
				CASE_CREATE_ARRAY_PROP( EDataTraits_C4f , Alembic::Abc::C4fTPTraits );
				CASE_CREATE_ARRAY_PROP( EDataTraits_C4c , Alembic::Abc::C4cTPTraits );

				CASE_CREATE_ARRAY_PROP( EDataTraits_N2f, Alembic::Abc::N2fTPTraits );
				CASE_CREATE_ARRAY_PROP( EDataTraits_N2d , Alembic::Abc::N2dTPTraits );
							
				CASE_CREATE_ARRAY_PROP( EDataTraits_N3f, Alembic::Abc::N3fTPTraits );
				CASE_CREATE_ARRAY_PROP( EDataTraits_N3d , Alembic::Abc::N3dTPTraits );

				CASE_CREATE_ARRAY_PROP( EDataTraits_V4f , AbcFramework::V4fTPTraits );
				CASE_CREATE_ARRAY_PROP( EDataTraits_Rotf, AbcFramework::VRotationfTraits );
			}
		}
		break;	

	case EPropertyType_Compound:
		{
			l_pProperty = new CAbcOCompoundProperty( in_parent, in_pszName );
		}
		break;
	}

#undef CASE_CREATE_SCALAR_PROP
#undef CASE_CREATE_ARRAY_PROP

	if ( l_pProperty )
	{
		l_pProperty->AddRef();
		*out_ppProp = l_pProperty;

		return EResult_Success;
	}

	return EResult_Fail;
}

EAbcResult CAbcOPropertyFactory::CreateOGeomParam( 
	IAbcOGeomParamBase** out_ppGeomParam, 
	Alembic::Abc::OCompoundProperty& in_parent, 
	const char* in_pszName, 
	bool in_bIsIndexed,
	EAbcDataTraits in_DataType, 
	EAbcGeomScope in_geomScope,
	size_t in_szExtent 
)
{
	IAbcOGeomParamBase* l_pGeomParam = NULL;

#define CASE_CREATE_GEOMPARAM( ETYPE, TRAITS ) case ETYPE: l_pGeomParam = new CAbcOTypedGeomParam< TRAITS >( in_parent, in_pszName, in_bIsIndexed, in_geomScope, in_szExtent ); break
	switch ( in_DataType )
	{
		CASE_CREATE_GEOMPARAM( EDataTraits_Bool, Alembic::Abc::BooleanTPTraits );
		CASE_CREATE_GEOMPARAM( EDataTraits_UInt8, Alembic::Abc::Uint8TPTraits );
		CASE_CREATE_GEOMPARAM( EDataTraits_Int8, Alembic::Abc::Int8TPTraits );
		CASE_CREATE_GEOMPARAM( EDataTraits_UInt16, Alembic::Abc::Uint16TPTraits);
		CASE_CREATE_GEOMPARAM( EDataTraits_Int16, Alembic::Abc::Int16TPTraits );
		CASE_CREATE_GEOMPARAM( EDataTraits_UInt32, Alembic::Abc::Uint32TPTraits );
		CASE_CREATE_GEOMPARAM( EDataTraits_Int32, Alembic::Abc::Int32TPTraits );
		CASE_CREATE_GEOMPARAM( EDataTraits_UInt64, Alembic::Abc::Uint64TPTraits);
		CASE_CREATE_GEOMPARAM( EDataTraits_Int64, Alembic::Abc::Int64TPTraits );
		CASE_CREATE_GEOMPARAM( EDataTraits_Float16, Alembic::Abc::Float16TPTraits );
		CASE_CREATE_GEOMPARAM( EDataTraits_Float32, Alembic::Abc::Float32TPTraits );
		CASE_CREATE_GEOMPARAM( EDataTraits_Float64, Alembic::Abc::Float64TPTraits );

		CASE_CREATE_GEOMPARAM( EDataTraits_StringPtr, Alembic::Abc::StringTPTraits );
		CASE_CREATE_GEOMPARAM( EDataTraits_WStringPtr, Alembic::Abc::WstringTPTraits );
				
		CASE_CREATE_GEOMPARAM( EDataTraits_V2s , Alembic::Abc::V2sTPTraits );
		CASE_CREATE_GEOMPARAM( EDataTraits_V2i , Alembic::Abc::V2iTPTraits );
		CASE_CREATE_GEOMPARAM( EDataTraits_V2f , Alembic::Abc::V2fTPTraits );
		CASE_CREATE_GEOMPARAM( EDataTraits_V2d , Alembic::Abc::V2dTPTraits );
				
		CASE_CREATE_GEOMPARAM( EDataTraits_V3s , Alembic::Abc::V3sTPTraits );
		CASE_CREATE_GEOMPARAM( EDataTraits_V3i , Alembic::Abc::V3iTPTraits );
		CASE_CREATE_GEOMPARAM( EDataTraits_V3f , Alembic::Abc::V3fTPTraits );
		CASE_CREATE_GEOMPARAM( EDataTraits_V3d , Alembic::Abc::V3dTPTraits );

		CASE_CREATE_GEOMPARAM( EDataTraits_P2s, Alembic::Abc::P2sTPTraits );
		CASE_CREATE_GEOMPARAM( EDataTraits_P2i, Alembic::Abc::P2iTPTraits );  
		CASE_CREATE_GEOMPARAM( EDataTraits_P2f, Alembic::Abc::P2fTPTraits );  
		CASE_CREATE_GEOMPARAM( EDataTraits_P2d, Alembic::Abc::P2dTPTraits );  
					
		CASE_CREATE_GEOMPARAM( EDataTraits_P3s, Alembic::Abc::P3sTPTraits );  
		CASE_CREATE_GEOMPARAM( EDataTraits_P3i, Alembic::Abc::P3iTPTraits );  
		CASE_CREATE_GEOMPARAM( EDataTraits_P3f, Alembic::Abc::P3fTPTraits );  
		CASE_CREATE_GEOMPARAM( EDataTraits_P3d, Alembic::Abc::P3dTPTraits );  
				
		CASE_CREATE_GEOMPARAM( EDataTraits_Box2s, Alembic::Abc::Box2sTPTraits );
		CASE_CREATE_GEOMPARAM( EDataTraits_Box2i, Alembic::Abc::Box2iTPTraits );
		CASE_CREATE_GEOMPARAM( EDataTraits_Box2f, Alembic::Abc::Box2fTPTraits );
		CASE_CREATE_GEOMPARAM( EDataTraits_Box2d, Alembic::Abc::Box2dTPTraits );
				
		CASE_CREATE_GEOMPARAM( EDataTraits_Box3s, Alembic::Abc::Box3sTPTraits );
		CASE_CREATE_GEOMPARAM( EDataTraits_Box3i, Alembic::Abc::Box3iTPTraits );
		CASE_CREATE_GEOMPARAM( EDataTraits_Box3f, Alembic::Abc::Box3fTPTraits );
		CASE_CREATE_GEOMPARAM( EDataTraits_Box3d, Alembic::Abc::Box3dTPTraits );
				
		CASE_CREATE_GEOMPARAM( EDataTraits_M33f, Alembic::Abc::M33fTPTraits );
		CASE_CREATE_GEOMPARAM( EDataTraits_M33d, Alembic::Abc::M33dTPTraits );
		CASE_CREATE_GEOMPARAM( EDataTraits_M44f, Alembic::Abc::M44fTPTraits );
		CASE_CREATE_GEOMPARAM( EDataTraits_M44d, Alembic::Abc::M44dTPTraits );
				
		CASE_CREATE_GEOMPARAM( EDataTraits_Quatf, Alembic::Abc::QuatfTPTraits );
		CASE_CREATE_GEOMPARAM( EDataTraits_Quatd, Alembic::Abc::QuatdTPTraits );
				
		CASE_CREATE_GEOMPARAM( EDataTraits_C3h , Alembic::Abc::C3hTPTraits );
		CASE_CREATE_GEOMPARAM( EDataTraits_C3f , Alembic::Abc::C3fTPTraits );
		CASE_CREATE_GEOMPARAM( EDataTraits_C3c , Alembic::Abc::C3cTPTraits );
				
		CASE_CREATE_GEOMPARAM( EDataTraits_C4h , Alembic::Abc::C4hTPTraits );
		CASE_CREATE_GEOMPARAM( EDataTraits_C4f , Alembic::Abc::C4fTPTraits );
		CASE_CREATE_GEOMPARAM( EDataTraits_C4c , Alembic::Abc::C4cTPTraits );

		CASE_CREATE_GEOMPARAM( EDataTraits_N2f, Alembic::Abc::N2fTPTraits );
		CASE_CREATE_GEOMPARAM( EDataTraits_N2d , Alembic::Abc::N2dTPTraits );

		CASE_CREATE_GEOMPARAM( EDataTraits_N3f, Alembic::Abc::N3fTPTraits );
		CASE_CREATE_GEOMPARAM( EDataTraits_N3d , Alembic::Abc::N3dTPTraits );

		CASE_CREATE_GEOMPARAM( EDataTraits_V4f , AbcFramework::V4fTPTraits );
		CASE_CREATE_GEOMPARAM( EDataTraits_Rotf, AbcFramework::VRotationfTraits );
	}
#undef CASE_CREATE_GEOMPARAM

	if ( l_pGeomParam )
	{
		l_pGeomParam->AddRef();
		*out_ppGeomParam = l_pGeomParam;

		return EResult_Success;
	}

	return EResult_Fail;
}

EAbcResult CAbcOPropertyFactory::WrapOProperty( IAbcOProperty** out_ppProp, Alembic::AbcCoreAbstract::BasePropertyWriterPtr in_ptr )
{
	IAbcOProperty* l_pProperty = NULL;

#define CASE_WRAP_SCALAR_PROP( ETYPE, TRAITS ) case ETYPE: l_pProperty = new CAbcOTypedScalarProperty< TRAITS >( l_ptr ); break
#define CASE_WRAP_ARRAY_PROP( ETYPE, TRAITS ) case ETYPE: l_pProperty = new CAbcOTypedArrayProperty< TRAITS >( l_ptr ); break

	if ( in_ptr->isSimple() )
	{
		if ( in_ptr->isScalar() )
		{
			Alembic::AbcCoreAbstract::ScalarPropertyWriterPtr l_ptr = Alembic::Util::static_pointer_cast<Alembic::AbcCoreAbstract::ScalarPropertyWriter, Alembic::AbcCoreAbstract::BasePropertyWriter>( in_ptr );
			switch ( GetDataTraits(in_ptr->getHeader()) )
			{
				CASE_WRAP_SCALAR_PROP( EDataTraits_Bool, Alembic::Abc::BooleanTPTraits );
				CASE_WRAP_SCALAR_PROP( EDataTraits_UInt8, Alembic::Abc::Uint8TPTraits );
				CASE_WRAP_SCALAR_PROP( EDataTraits_Int8, Alembic::Abc::Int8TPTraits );
				CASE_WRAP_SCALAR_PROP( EDataTraits_UInt16, Alembic::Abc::Uint16TPTraits);
				CASE_WRAP_SCALAR_PROP( EDataTraits_Int16, Alembic::Abc::Int16TPTraits );
				CASE_WRAP_SCALAR_PROP( EDataTraits_UInt32, Alembic::Abc::Uint32TPTraits );
				CASE_WRAP_SCALAR_PROP( EDataTraits_Int32, Alembic::Abc::Int32TPTraits );
				CASE_WRAP_SCALAR_PROP( EDataTraits_UInt64, Alembic::Abc::Uint64TPTraits);
				CASE_WRAP_SCALAR_PROP( EDataTraits_Int64, Alembic::Abc::Int64TPTraits );
				CASE_WRAP_SCALAR_PROP( EDataTraits_Float16, Alembic::Abc::Float16TPTraits );
				CASE_WRAP_SCALAR_PROP( EDataTraits_Float32, Alembic::Abc::Float32TPTraits );
				CASE_WRAP_SCALAR_PROP( EDataTraits_Float64, Alembic::Abc::Float64TPTraits );

				CASE_WRAP_SCALAR_PROP( EDataTraits_StringPtr, Alembic::Abc::StringTPTraits );
				CASE_WRAP_SCALAR_PROP( EDataTraits_WStringPtr, Alembic::Abc::WstringTPTraits );
					
				CASE_WRAP_SCALAR_PROP( EDataTraits_V2s , Alembic::Abc::V2sTPTraits );
				CASE_WRAP_SCALAR_PROP( EDataTraits_V2i , Alembic::Abc::V2iTPTraits );
				CASE_WRAP_SCALAR_PROP( EDataTraits_V2f , Alembic::Abc::V2fTPTraits );
				CASE_WRAP_SCALAR_PROP( EDataTraits_V2d , Alembic::Abc::V2dTPTraits );
					
				CASE_WRAP_SCALAR_PROP( EDataTraits_V3s , Alembic::Abc::V3sTPTraits );
				CASE_WRAP_SCALAR_PROP( EDataTraits_V3i , Alembic::Abc::V3iTPTraits );
				CASE_WRAP_SCALAR_PROP( EDataTraits_V3f , Alembic::Abc::V3fTPTraits );
				CASE_WRAP_SCALAR_PROP( EDataTraits_V3d , Alembic::Abc::V3dTPTraits );

				CASE_WRAP_SCALAR_PROP( EDataTraits_P2s, Alembic::Abc::P2sTPTraits );
				CASE_WRAP_SCALAR_PROP( EDataTraits_P2i, Alembic::Abc::P2iTPTraits );  
				CASE_WRAP_SCALAR_PROP( EDataTraits_P2f, Alembic::Abc::P2fTPTraits );  
				CASE_WRAP_SCALAR_PROP( EDataTraits_P2d, Alembic::Abc::P2dTPTraits );  
				
				CASE_WRAP_SCALAR_PROP( EDataTraits_P3s, Alembic::Abc::P3sTPTraits );  
				CASE_WRAP_SCALAR_PROP( EDataTraits_P3i, Alembic::Abc::P3iTPTraits );  
				CASE_WRAP_SCALAR_PROP( EDataTraits_P3f, Alembic::Abc::P3fTPTraits );  
				CASE_WRAP_SCALAR_PROP( EDataTraits_P3d, Alembic::Abc::P3dTPTraits );  
					
				CASE_WRAP_SCALAR_PROP( EDataTraits_Box2s, Alembic::Abc::Box2sTPTraits );
				CASE_WRAP_SCALAR_PROP( EDataTraits_Box2i, Alembic::Abc::Box2iTPTraits );
				CASE_WRAP_SCALAR_PROP( EDataTraits_Box2f, Alembic::Abc::Box2fTPTraits );
				CASE_WRAP_SCALAR_PROP( EDataTraits_Box2d, Alembic::Abc::Box2dTPTraits );
					
				CASE_WRAP_SCALAR_PROP( EDataTraits_Box3s, Alembic::Abc::Box3sTPTraits );
				CASE_WRAP_SCALAR_PROP( EDataTraits_Box3i, Alembic::Abc::Box3iTPTraits );
				CASE_WRAP_SCALAR_PROP( EDataTraits_Box3f, Alembic::Abc::Box3fTPTraits );
				CASE_WRAP_SCALAR_PROP( EDataTraits_Box3d, Alembic::Abc::Box3dTPTraits );
					
				CASE_WRAP_SCALAR_PROP( EDataTraits_M33f, Alembic::Abc::M33fTPTraits );
				CASE_WRAP_SCALAR_PROP( EDataTraits_M33d, Alembic::Abc::M33dTPTraits );
				CASE_WRAP_SCALAR_PROP( EDataTraits_M44f, Alembic::Abc::M44fTPTraits );
				CASE_WRAP_SCALAR_PROP( EDataTraits_M44d, Alembic::Abc::M44dTPTraits );
					
				CASE_WRAP_SCALAR_PROP( EDataTraits_Quatf, Alembic::Abc::QuatfTPTraits );
				CASE_WRAP_SCALAR_PROP( EDataTraits_Quatd, Alembic::Abc::QuatdTPTraits );
					
				CASE_WRAP_SCALAR_PROP( EDataTraits_C3h , Alembic::Abc::C3hTPTraits );
				CASE_WRAP_SCALAR_PROP( EDataTraits_C3f , Alembic::Abc::C3fTPTraits );
				CASE_WRAP_SCALAR_PROP( EDataTraits_C3c , Alembic::Abc::C3cTPTraits );
					
				CASE_WRAP_SCALAR_PROP( EDataTraits_C4h , Alembic::Abc::C4hTPTraits );
				CASE_WRAP_SCALAR_PROP( EDataTraits_C4f , Alembic::Abc::C4fTPTraits );
				CASE_WRAP_SCALAR_PROP( EDataTraits_C4c , Alembic::Abc::C4cTPTraits );

				CASE_WRAP_SCALAR_PROP( EDataTraits_N2f, Alembic::Abc::N2fTPTraits );
				CASE_WRAP_SCALAR_PROP( EDataTraits_N2d , Alembic::Abc::N2dTPTraits );

				CASE_WRAP_SCALAR_PROP( EDataTraits_N3f, Alembic::Abc::N3fTPTraits );
				CASE_WRAP_SCALAR_PROP( EDataTraits_N3d , Alembic::Abc::N3dTPTraits );

				CASE_WRAP_SCALAR_PROP( EDataTraits_V4f , AbcFramework::V4fTPTraits );
				CASE_WRAP_SCALAR_PROP( EDataTraits_Rotf, AbcFramework::VRotationfTraits );
			}
		}
		else
		{
			Alembic::AbcCoreAbstract::ArrayPropertyWriterPtr l_ptr = Alembic::Util::static_pointer_cast<Alembic::AbcCoreAbstract::ArrayPropertyWriter, Alembic::AbcCoreAbstract::BasePropertyWriter>(in_ptr);
			switch ( GetDataTraits(in_ptr->getHeader()) )
			{
				CASE_WRAP_ARRAY_PROP( EDataTraits_Bool, Alembic::Abc::BooleanTPTraits );
				CASE_WRAP_ARRAY_PROP( EDataTraits_UInt8, Alembic::Abc::Uint8TPTraits );
				CASE_WRAP_ARRAY_PROP( EDataTraits_Int8, Alembic::Abc::Int8TPTraits );
				CASE_WRAP_ARRAY_PROP( EDataTraits_UInt16, Alembic::Abc::Uint16TPTraits);
				CASE_WRAP_ARRAY_PROP( EDataTraits_Int16, Alembic::Abc::Int16TPTraits );
				CASE_WRAP_ARRAY_PROP( EDataTraits_UInt32, Alembic::Abc::Uint32TPTraits );
				CASE_WRAP_ARRAY_PROP( EDataTraits_Int32, Alembic::Abc::Int32TPTraits );
				CASE_WRAP_ARRAY_PROP( EDataTraits_UInt64, Alembic::Abc::Uint64TPTraits);
				CASE_WRAP_ARRAY_PROP( EDataTraits_Int64, Alembic::Abc::Int64TPTraits );
				CASE_WRAP_ARRAY_PROP( EDataTraits_Float16, Alembic::Abc::Float16TPTraits );
				CASE_WRAP_ARRAY_PROP( EDataTraits_Float32, Alembic::Abc::Float32TPTraits );
				CASE_WRAP_ARRAY_PROP( EDataTraits_Float64, Alembic::Abc::Float64TPTraits );

				CASE_WRAP_ARRAY_PROP( EDataTraits_StringPtr, Alembic::Abc::StringTPTraits );
				CASE_WRAP_ARRAY_PROP( EDataTraits_WStringPtr, Alembic::Abc::WstringTPTraits );
					
				CASE_WRAP_ARRAY_PROP( EDataTraits_V2s , Alembic::Abc::V2sTPTraits );
				CASE_WRAP_ARRAY_PROP( EDataTraits_V2i , Alembic::Abc::V2iTPTraits );
				CASE_WRAP_ARRAY_PROP( EDataTraits_V2f , Alembic::Abc::V2fTPTraits );
				CASE_WRAP_ARRAY_PROP( EDataTraits_V2d , Alembic::Abc::V2dTPTraits );
					
				CASE_WRAP_ARRAY_PROP( EDataTraits_V3s , Alembic::Abc::V3sTPTraits );
				CASE_WRAP_ARRAY_PROP( EDataTraits_V3i , Alembic::Abc::V3iTPTraits );
				CASE_WRAP_ARRAY_PROP( EDataTraits_V3f , Alembic::Abc::V3fTPTraits );
				CASE_WRAP_ARRAY_PROP( EDataTraits_V3d , Alembic::Abc::V3dTPTraits );

				CASE_WRAP_ARRAY_PROP( EDataTraits_P2s, Alembic::Abc::P2sTPTraits );
				CASE_WRAP_ARRAY_PROP( EDataTraits_P2i, Alembic::Abc::P2iTPTraits );  
				CASE_WRAP_ARRAY_PROP( EDataTraits_P2f, Alembic::Abc::P2fTPTraits );  
				CASE_WRAP_ARRAY_PROP( EDataTraits_P2d, Alembic::Abc::P2dTPTraits );  
						  
				CASE_WRAP_ARRAY_PROP( EDataTraits_P3s, Alembic::Abc::P3sTPTraits );  
				CASE_WRAP_ARRAY_PROP( EDataTraits_P3i, Alembic::Abc::P3iTPTraits );  
				CASE_WRAP_ARRAY_PROP( EDataTraits_P3f, Alembic::Abc::P3fTPTraits );  
				CASE_WRAP_ARRAY_PROP( EDataTraits_P3d, Alembic::Abc::P3dTPTraits );  
					
				CASE_WRAP_ARRAY_PROP( EDataTraits_Box2s, Alembic::Abc::Box2sTPTraits );
				CASE_WRAP_ARRAY_PROP( EDataTraits_Box2i, Alembic::Abc::Box2iTPTraits );
				CASE_WRAP_ARRAY_PROP( EDataTraits_Box2f, Alembic::Abc::Box2fTPTraits );
				CASE_WRAP_ARRAY_PROP( EDataTraits_Box2d, Alembic::Abc::Box2dTPTraits );
					
				CASE_WRAP_ARRAY_PROP( EDataTraits_Box3s, Alembic::Abc::Box3sTPTraits );
				CASE_WRAP_ARRAY_PROP( EDataTraits_Box3i, Alembic::Abc::Box3iTPTraits );
				CASE_WRAP_ARRAY_PROP( EDataTraits_Box3f, Alembic::Abc::Box3fTPTraits );
				CASE_WRAP_ARRAY_PROP( EDataTraits_Box3d, Alembic::Abc::Box3dTPTraits );
					
				CASE_WRAP_ARRAY_PROP( EDataTraits_M33f, Alembic::Abc::M33fTPTraits );
				CASE_WRAP_ARRAY_PROP( EDataTraits_M33d, Alembic::Abc::M33dTPTraits );
				CASE_WRAP_ARRAY_PROP( EDataTraits_M44f, Alembic::Abc::M44fTPTraits );
				CASE_WRAP_ARRAY_PROP( EDataTraits_M44d, Alembic::Abc::M44dTPTraits );
					
				CASE_WRAP_ARRAY_PROP( EDataTraits_Quatf, Alembic::Abc::QuatfTPTraits );
				CASE_WRAP_ARRAY_PROP( EDataTraits_Quatd, Alembic::Abc::QuatdTPTraits );
					
				CASE_WRAP_ARRAY_PROP( EDataTraits_C3h , Alembic::Abc::C3hTPTraits );
				CASE_WRAP_ARRAY_PROP( EDataTraits_C3f , Alembic::Abc::C3fTPTraits );
				CASE_WRAP_ARRAY_PROP( EDataTraits_C3c , Alembic::Abc::C3cTPTraits );
					
				CASE_WRAP_ARRAY_PROP( EDataTraits_C4h , Alembic::Abc::C4hTPTraits );
				CASE_WRAP_ARRAY_PROP( EDataTraits_C4f , Alembic::Abc::C4fTPTraits );
				CASE_WRAP_ARRAY_PROP( EDataTraits_C4c , Alembic::Abc::C4cTPTraits );

				CASE_WRAP_ARRAY_PROP( EDataTraits_N2f, Alembic::Abc::N2fTPTraits );
				CASE_WRAP_ARRAY_PROP( EDataTraits_N2d , Alembic::Abc::N2dTPTraits );
					 	  
				CASE_WRAP_ARRAY_PROP( EDataTraits_N3f, Alembic::Abc::N3fTPTraits );
				CASE_WRAP_ARRAY_PROP( EDataTraits_N3d , Alembic::Abc::N3dTPTraits );

				CASE_WRAP_ARRAY_PROP( EDataTraits_V4f , AbcFramework::V4fTPTraits );
				CASE_WRAP_ARRAY_PROP( EDataTraits_Rotf, AbcFramework::VRotationfTraits );
			}
		}
	}
	else
	{
		Alembic::AbcCoreAbstract::CompoundPropertyWriterPtr l_ptr = Alembic::Util::static_pointer_cast<Alembic::AbcCoreAbstract::CompoundPropertyWriter, Alembic::AbcCoreAbstract::BasePropertyWriter>(in_ptr);
		l_pProperty = new CAbcOCompoundProperty( l_ptr );
	}

#undef CASE_WRAP_SCALAR_PROP
#undef CASE_WRAP_ARRAY_PROP 

	if ( l_pProperty )
	{
		l_pProperty->AddRef();
		*out_ppProp = l_pProperty;

		return EResult_Success;
	}

	return EResult_Fail;
}

EAbcResult CAbcOPropertyFactory::WrapOGeomParam( IAbcOGeomParamBase** out_ppGeomParam, IAbcOProperty* in_pProp )
{
	IAbcOGeomParamBase* l_pGeomParam = NULL;

#define CASE_WRAP_GEOMPARAM_INDEXED( ETYPE, TRAITS ) case ETYPE:																				\
	{																																			\
		l_pGeomParam = new CAbcOTypedGeomParam< TRAITS >( l_spCompound->GetInternal().getPtr(), in_pProp );										\
	}																																			\
	break

#define CASE_WRAP_GEOMPARAM_NON_INDEXED( ETYPE, TRAITS ) case ETYPE:																			\
	{																																			\
		CAbcPtr< CAbcOTypedArrayProperty<TRAITS> > l_spArray = static_cast< CAbcOTypedArrayProperty<TRAITS>* >(in_pProp);						\
		l_pGeomParam = new CAbcOTypedGeomParam< TRAITS >( l_spArray->GetInternal().getPtr(), in_pProp );										\
	}																																			\
	break
	
	if ( in_pProp->GetType() == EPropertyType_Compound )
	{
		CAbcPtr<CAbcOCompoundProperty> l_spCompound = static_cast<CAbcOCompoundProperty*>(in_pProp);
		CAbcPtr<IAbcOProperty> l_spValsProp;
		l_spCompound->GetProperty( (IAbcOProperty**)&l_spValsProp, ".vals" );
		
		CAbcPtr<IAbcOProperty> l_spIndicesProp;
		l_spCompound->GetProperty( (IAbcOProperty**)&l_spIndicesProp, ".indices" );

		if ( l_spValsProp != NULL && l_spIndicesProp != NULL )
		{
			switch ( l_spValsProp->GetDataTraits() )
			{
				CASE_WRAP_GEOMPARAM_INDEXED( EDataTraits_Bool, Alembic::Abc::BooleanTPTraits );
				CASE_WRAP_GEOMPARAM_INDEXED( EDataTraits_UInt8, Alembic::Abc::Uint8TPTraits );
				CASE_WRAP_GEOMPARAM_INDEXED( EDataTraits_Int8, Alembic::Abc::Int8TPTraits );
				CASE_WRAP_GEOMPARAM_INDEXED( EDataTraits_UInt16, Alembic::Abc::Uint16TPTraits);
				CASE_WRAP_GEOMPARAM_INDEXED( EDataTraits_Int16, Alembic::Abc::Int16TPTraits );
				CASE_WRAP_GEOMPARAM_INDEXED( EDataTraits_UInt32, Alembic::Abc::Uint32TPTraits );
				CASE_WRAP_GEOMPARAM_INDEXED( EDataTraits_Int32, Alembic::Abc::Int32TPTraits );
				CASE_WRAP_GEOMPARAM_INDEXED( EDataTraits_UInt64, Alembic::Abc::Uint64TPTraits);
				CASE_WRAP_GEOMPARAM_INDEXED( EDataTraits_Int64, Alembic::Abc::Int64TPTraits );
				CASE_WRAP_GEOMPARAM_INDEXED( EDataTraits_Float16, Alembic::Abc::Float16TPTraits );
				CASE_WRAP_GEOMPARAM_INDEXED( EDataTraits_Float32, Alembic::Abc::Float32TPTraits );
				CASE_WRAP_GEOMPARAM_INDEXED( EDataTraits_Float64, Alembic::Abc::Float64TPTraits );

				CASE_WRAP_GEOMPARAM_INDEXED( EDataTraits_StringPtr, Alembic::Abc::StringTPTraits );
				CASE_WRAP_GEOMPARAM_INDEXED( EDataTraits_WStringPtr, Alembic::Abc::WstringTPTraits );

				CASE_WRAP_GEOMPARAM_INDEXED( EDataTraits_V2s , Alembic::Abc::V2sTPTraits );
				CASE_WRAP_GEOMPARAM_INDEXED( EDataTraits_V2i , Alembic::Abc::V2iTPTraits );
				CASE_WRAP_GEOMPARAM_INDEXED( EDataTraits_V2f , Alembic::Abc::V2fTPTraits );
				CASE_WRAP_GEOMPARAM_INDEXED( EDataTraits_V2d , Alembic::Abc::V2dTPTraits );

				CASE_WRAP_GEOMPARAM_INDEXED( EDataTraits_V3s , Alembic::Abc::V3sTPTraits );
				CASE_WRAP_GEOMPARAM_INDEXED( EDataTraits_V3i , Alembic::Abc::V3iTPTraits );
				CASE_WRAP_GEOMPARAM_INDEXED( EDataTraits_V3f , Alembic::Abc::V3fTPTraits );
				CASE_WRAP_GEOMPARAM_INDEXED( EDataTraits_V3d , Alembic::Abc::V3dTPTraits );

				CASE_WRAP_GEOMPARAM_INDEXED( EDataTraits_P2s, Alembic::Abc::P2sTPTraits );
				CASE_WRAP_GEOMPARAM_INDEXED( EDataTraits_P2i, Alembic::Abc::P2iTPTraits );  
				CASE_WRAP_GEOMPARAM_INDEXED( EDataTraits_P2f, Alembic::Abc::P2fTPTraits );  
				CASE_WRAP_GEOMPARAM_INDEXED( EDataTraits_P2d, Alembic::Abc::P2dTPTraits );  

				CASE_WRAP_GEOMPARAM_INDEXED( EDataTraits_P3s, Alembic::Abc::P3sTPTraits );  
				CASE_WRAP_GEOMPARAM_INDEXED( EDataTraits_P3i, Alembic::Abc::P3iTPTraits );  
				CASE_WRAP_GEOMPARAM_INDEXED( EDataTraits_P3f, Alembic::Abc::P3fTPTraits );  
				CASE_WRAP_GEOMPARAM_INDEXED( EDataTraits_P3d, Alembic::Abc::P3dTPTraits );  

				CASE_WRAP_GEOMPARAM_INDEXED( EDataTraits_Box2s, Alembic::Abc::Box2sTPTraits );
				CASE_WRAP_GEOMPARAM_INDEXED( EDataTraits_Box2i, Alembic::Abc::Box2iTPTraits );
				CASE_WRAP_GEOMPARAM_INDEXED( EDataTraits_Box2f, Alembic::Abc::Box2fTPTraits );
				CASE_WRAP_GEOMPARAM_INDEXED( EDataTraits_Box2d, Alembic::Abc::Box2dTPTraits );

				CASE_WRAP_GEOMPARAM_INDEXED( EDataTraits_Box3s, Alembic::Abc::Box3sTPTraits );
				CASE_WRAP_GEOMPARAM_INDEXED( EDataTraits_Box3i, Alembic::Abc::Box3iTPTraits );
				CASE_WRAP_GEOMPARAM_INDEXED( EDataTraits_Box3f, Alembic::Abc::Box3fTPTraits );
				CASE_WRAP_GEOMPARAM_INDEXED( EDataTraits_Box3d, Alembic::Abc::Box3dTPTraits );

				CASE_WRAP_GEOMPARAM_INDEXED( EDataTraits_M33f, Alembic::Abc::M33fTPTraits );
				CASE_WRAP_GEOMPARAM_INDEXED( EDataTraits_M33d, Alembic::Abc::M33dTPTraits );
				CASE_WRAP_GEOMPARAM_INDEXED( EDataTraits_M44f, Alembic::Abc::M44fTPTraits );
				CASE_WRAP_GEOMPARAM_INDEXED( EDataTraits_M44d, Alembic::Abc::M44dTPTraits );

				CASE_WRAP_GEOMPARAM_INDEXED( EDataTraits_Quatf, Alembic::Abc::QuatfTPTraits );
				CASE_WRAP_GEOMPARAM_INDEXED( EDataTraits_Quatd, Alembic::Abc::QuatdTPTraits );

				CASE_WRAP_GEOMPARAM_INDEXED( EDataTraits_C3h , Alembic::Abc::C3hTPTraits );
				CASE_WRAP_GEOMPARAM_INDEXED( EDataTraits_C3f , Alembic::Abc::C3fTPTraits );
				CASE_WRAP_GEOMPARAM_INDEXED( EDataTraits_C3c , Alembic::Abc::C3cTPTraits );

				CASE_WRAP_GEOMPARAM_INDEXED( EDataTraits_C4h , Alembic::Abc::C4hTPTraits );
				CASE_WRAP_GEOMPARAM_INDEXED( EDataTraits_C4f , Alembic::Abc::C4fTPTraits );
				CASE_WRAP_GEOMPARAM_INDEXED( EDataTraits_C4c , Alembic::Abc::C4cTPTraits );

				CASE_WRAP_GEOMPARAM_INDEXED( EDataTraits_N2f, Alembic::Abc::N2fTPTraits );
				CASE_WRAP_GEOMPARAM_INDEXED( EDataTraits_N2d , Alembic::Abc::N2dTPTraits );

				CASE_WRAP_GEOMPARAM_INDEXED( EDataTraits_N3f, Alembic::Abc::N3fTPTraits );
				CASE_WRAP_GEOMPARAM_INDEXED( EDataTraits_N3d , Alembic::Abc::N3dTPTraits );

				CASE_WRAP_GEOMPARAM_INDEXED( EDataTraits_V4f , AbcFramework::V4fTPTraits );
				CASE_WRAP_GEOMPARAM_INDEXED( EDataTraits_Rotf, AbcFramework::VRotationfTraits );
			}
		}
	}
	else if ( in_pProp->GetType() == EPropertyType_Array )
	{
		switch ( in_pProp->GetDataTraits() )
		{
			CASE_WRAP_GEOMPARAM_NON_INDEXED( EDataTraits_Bool, Alembic::Abc::BooleanTPTraits );
			CASE_WRAP_GEOMPARAM_NON_INDEXED( EDataTraits_UInt8, Alembic::Abc::Uint8TPTraits );
			CASE_WRAP_GEOMPARAM_NON_INDEXED( EDataTraits_Int8, Alembic::Abc::Int8TPTraits );
			CASE_WRAP_GEOMPARAM_NON_INDEXED( EDataTraits_UInt16, Alembic::Abc::Uint16TPTraits);
			CASE_WRAP_GEOMPARAM_NON_INDEXED( EDataTraits_Int16, Alembic::Abc::Int16TPTraits );
			CASE_WRAP_GEOMPARAM_NON_INDEXED( EDataTraits_UInt32, Alembic::Abc::Uint32TPTraits );
			CASE_WRAP_GEOMPARAM_NON_INDEXED( EDataTraits_Int32, Alembic::Abc::Int32TPTraits );
			CASE_WRAP_GEOMPARAM_NON_INDEXED( EDataTraits_UInt64, Alembic::Abc::Uint64TPTraits);
			CASE_WRAP_GEOMPARAM_NON_INDEXED( EDataTraits_Int64, Alembic::Abc::Int64TPTraits );
			CASE_WRAP_GEOMPARAM_NON_INDEXED( EDataTraits_Float16, Alembic::Abc::Float16TPTraits );
			CASE_WRAP_GEOMPARAM_NON_INDEXED( EDataTraits_Float32, Alembic::Abc::Float32TPTraits );
			CASE_WRAP_GEOMPARAM_NON_INDEXED( EDataTraits_Float64, Alembic::Abc::Float64TPTraits );

			CASE_WRAP_GEOMPARAM_NON_INDEXED( EDataTraits_StringPtr, Alembic::Abc::StringTPTraits );
			CASE_WRAP_GEOMPARAM_NON_INDEXED( EDataTraits_WStringPtr, Alembic::Abc::WstringTPTraits );

			CASE_WRAP_GEOMPARAM_NON_INDEXED( EDataTraits_V2s , Alembic::Abc::V2sTPTraits );
			CASE_WRAP_GEOMPARAM_NON_INDEXED( EDataTraits_V2i , Alembic::Abc::V2iTPTraits );
			CASE_WRAP_GEOMPARAM_NON_INDEXED( EDataTraits_V2f , Alembic::Abc::V2fTPTraits );
			CASE_WRAP_GEOMPARAM_NON_INDEXED( EDataTraits_V2d , Alembic::Abc::V2dTPTraits );

			CASE_WRAP_GEOMPARAM_NON_INDEXED( EDataTraits_V3s , Alembic::Abc::V3sTPTraits );
			CASE_WRAP_GEOMPARAM_NON_INDEXED( EDataTraits_V3i , Alembic::Abc::V3iTPTraits );
			CASE_WRAP_GEOMPARAM_NON_INDEXED( EDataTraits_V3f , Alembic::Abc::V3fTPTraits );
			CASE_WRAP_GEOMPARAM_NON_INDEXED( EDataTraits_V3d , Alembic::Abc::V3dTPTraits );

			CASE_WRAP_GEOMPARAM_NON_INDEXED( EDataTraits_P2s, Alembic::Abc::P2sTPTraits );
			CASE_WRAP_GEOMPARAM_NON_INDEXED( EDataTraits_P2i, Alembic::Abc::P2iTPTraits );  
			CASE_WRAP_GEOMPARAM_NON_INDEXED( EDataTraits_P2f, Alembic::Abc::P2fTPTraits );  
			CASE_WRAP_GEOMPARAM_NON_INDEXED( EDataTraits_P2d, Alembic::Abc::P2dTPTraits );  

			CASE_WRAP_GEOMPARAM_NON_INDEXED( EDataTraits_P3s, Alembic::Abc::P3sTPTraits );  
			CASE_WRAP_GEOMPARAM_NON_INDEXED( EDataTraits_P3i, Alembic::Abc::P3iTPTraits );  
			CASE_WRAP_GEOMPARAM_NON_INDEXED( EDataTraits_P3f, Alembic::Abc::P3fTPTraits );  
			CASE_WRAP_GEOMPARAM_NON_INDEXED( EDataTraits_P3d, Alembic::Abc::P3dTPTraits );  

			CASE_WRAP_GEOMPARAM_NON_INDEXED( EDataTraits_Box2s, Alembic::Abc::Box2sTPTraits );
			CASE_WRAP_GEOMPARAM_NON_INDEXED( EDataTraits_Box2i, Alembic::Abc::Box2iTPTraits );
			CASE_WRAP_GEOMPARAM_NON_INDEXED( EDataTraits_Box2f, Alembic::Abc::Box2fTPTraits );
			CASE_WRAP_GEOMPARAM_NON_INDEXED( EDataTraits_Box2d, Alembic::Abc::Box2dTPTraits );

			CASE_WRAP_GEOMPARAM_NON_INDEXED( EDataTraits_Box3s, Alembic::Abc::Box3sTPTraits );
			CASE_WRAP_GEOMPARAM_NON_INDEXED( EDataTraits_Box3i, Alembic::Abc::Box3iTPTraits );
			CASE_WRAP_GEOMPARAM_NON_INDEXED( EDataTraits_Box3f, Alembic::Abc::Box3fTPTraits );
			CASE_WRAP_GEOMPARAM_NON_INDEXED( EDataTraits_Box3d, Alembic::Abc::Box3dTPTraits );

			CASE_WRAP_GEOMPARAM_NON_INDEXED( EDataTraits_M33f, Alembic::Abc::M33fTPTraits );
			CASE_WRAP_GEOMPARAM_NON_INDEXED( EDataTraits_M33d, Alembic::Abc::M33dTPTraits );
			CASE_WRAP_GEOMPARAM_NON_INDEXED( EDataTraits_M44f, Alembic::Abc::M44fTPTraits );
			CASE_WRAP_GEOMPARAM_NON_INDEXED( EDataTraits_M44d, Alembic::Abc::M44dTPTraits );

			CASE_WRAP_GEOMPARAM_NON_INDEXED( EDataTraits_Quatf, Alembic::Abc::QuatfTPTraits );
			CASE_WRAP_GEOMPARAM_NON_INDEXED( EDataTraits_Quatd, Alembic::Abc::QuatdTPTraits );

			CASE_WRAP_GEOMPARAM_NON_INDEXED( EDataTraits_C3h , Alembic::Abc::C3hTPTraits );
			CASE_WRAP_GEOMPARAM_NON_INDEXED( EDataTraits_C3f , Alembic::Abc::C3fTPTraits );
			CASE_WRAP_GEOMPARAM_NON_INDEXED( EDataTraits_C3c , Alembic::Abc::C3cTPTraits );

			CASE_WRAP_GEOMPARAM_NON_INDEXED( EDataTraits_C4h , Alembic::Abc::C4hTPTraits );
			CASE_WRAP_GEOMPARAM_NON_INDEXED( EDataTraits_C4f , Alembic::Abc::C4fTPTraits );
			CASE_WRAP_GEOMPARAM_NON_INDEXED( EDataTraits_C4c , Alembic::Abc::C4cTPTraits );

			CASE_WRAP_GEOMPARAM_NON_INDEXED( EDataTraits_N2f, Alembic::Abc::N2fTPTraits );
			CASE_WRAP_GEOMPARAM_NON_INDEXED( EDataTraits_N2d , Alembic::Abc::N2dTPTraits );

			CASE_WRAP_GEOMPARAM_NON_INDEXED( EDataTraits_N3f, Alembic::Abc::N3fTPTraits );
			CASE_WRAP_GEOMPARAM_NON_INDEXED( EDataTraits_N3d , Alembic::Abc::N3dTPTraits );

			CASE_WRAP_GEOMPARAM_NON_INDEXED( EDataTraits_V4f , AbcFramework::V4fTPTraits );
			CASE_WRAP_GEOMPARAM_NON_INDEXED( EDataTraits_Rotf, AbcFramework::VRotationfTraits );

		}
	}
	
#undef CASE_WRAP_GEOMPARAM_INDEXED
#undef CASE_WRAP_GEOMPARAM_NON_INDEXED

	if ( l_pGeomParam )
	{
		l_pGeomParam->AddRef();
		*out_ppGeomParam = l_pGeomParam;

		return EResult_Success;
	}

	return EResult_Fail;
}
