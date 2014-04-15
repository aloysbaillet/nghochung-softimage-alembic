//*****************************************************************************
/*!
	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*

#ifndef CABCOTYPEDARRAY2DPROPERTY_H
#define CABCOTYPEDARRAY2DPROPERTY_H

#include <Alembic/Abc/OObject.h>
#include <Alembic/Abc/OTypedArrayProperty.h>
#include <Alembic/Abc/OTypedScalarProperty.h>
#include <Alembic/AbcCoreAbstract/BasePropertyWriter.h>
#include <Alembic/AbcGeom/OGeomParam.h>

#include "AbcFrameworkUtil.h"
#include "CAbcOObjectFactory.h"
#include "CAbcOPropertyFactory.h"
#include "CAbcOutput_Helpers.h"
#include "CRefCount.h"
#include "IAbcFramework.h"
#include "IAbcOutput.h"
#include <string>

template< class TRAITS >
class CAbcOTypedArray2DProperty : public IAbcOTypedArray2DProperty<TRAITS>, protected CAbcOCompoundPropertyBase, protected CRefCount
{
	IMPL_REFCOUNT

public:
	// constructor to create new property
	template <class COMPOUND_PTR>
	CAbcOTypedArray2DProperty( COMPOUND_PTR in_Parent, const char* in_pszName )
		: CAbcOCompoundPropertyBase( in_Parent, in_pszName )
	{ }

	// constructor to wrap existing property
	CAbcOTypedArray2DProperty( Alembic::AbcCoreAbstract::CompoundPropertyWriterPtr in_Ptr )
		: CAbcOCompoundPropertyBase( in_Ptr, Alembic::Abc::kWrapExisting )
	{ }

	TheAbcPropertyClass& GetInternal() { return CAbcOCompoundPropertyBase::GetInternal();}

	EAbcPropertyType	GetType() const { return CAbcOCompoundPropertyBase::GetType();}
	EAbcPodType			GetPOD() const { return CAbcOCompoundPropertyBase::GetPOD();}
	EAbcDataTraits		GetDataTraits() const { return CAbcOCompoundPropertyBase::GetDataTraits();}

	virtual EAbcResult CreateProperty( IAbcOProperty** out_ppProp, const char* in_pszName, EAbcPropertyType in_propType, EAbcDataTraits in_EDataTraits )
	{
		return CAbcOCompoundPropertyBase::CreateProperty( out_ppProp, in_pszName, in_propType, in_EDataTraits );
	}

	virtual EAbcResult CreateGeomParam( IAbcOGeomParamBase** out_ppGeomParam, const char* in_pszName, bool in_bIsIndexed, EAbcDataTraits in_DataType, EAbcGeomScope in_geomScope, size_t in_iExtent )
	{
		return CAbcOCompoundPropertyBase::CreateGeomParam( out_ppGeomParam, in_pszName, in_bIsIndexed, in_DataType, in_geomScope, in_iExtent );
	}

	virtual EAbcResult CreateParticleShapeProperty( IAbcOParticleShapeProperty** out_ppProperty, const char* in_pszName )
	{
		return CAbcOCompoundPropertyBase::CreateParticleShapeProperty( out_ppProperty, in_pszName );
	}

	virtual EAbcResult CreateArray2DProperty( IAbcOProperty** out_ppProp, const char* in_pszName, EAbcDataTraits in_EDataTraits )
	{
		return CAbcOCompoundPropertyBase::CreateArray2DProperty( out_ppProp, in_pszName, in_EDataTraits );
	}

	virtual EAbcResult Create2DGeomParam( IAbcOProperty** out_ppProp, const char* in_pszName, EAbcDataTraits in_EDataTraits, EAbcGeomScope in_geomScope )
	{
		return CAbcOCompoundPropertyBase::Create2DGeomParam( out_ppProp, in_pszName, in_EDataTraits, in_geomScope );
	}

	virtual EAbcResult GetProperty( IAbcOProperty** out_ppProp, const char* in_pszName )
	{
		return CAbcOCompoundPropertyBase::GetProperty( out_ppProp, in_pszName );
	}

	virtual EAbcResult GetGeomParam( IAbcOGeomParamBase** out_ppGeomParam, const char* in_pszName )
	{
		return CAbcOCompoundPropertyBase::GetGeomParam( out_ppGeomParam, in_pszName );
	}

	virtual EAbcResult	GetValueProperty( TheValuePropertyClass** out_ppProperty )
	{
		if ( out_ppProperty == NULL )
			return EResult_InvalidPtr;

		CAbcPtr<IAbcOProperty> l_spChild;
		if ( GetProperty( &l_spChild, PROPNAME_VALS ) == EResult_Success )
		{
			if ( l_spChild->GetType() == EPropertyType_Array && l_spChild->GetDataTraits() == AbcFramework::GetEDataTraits<TRAITS>() )
			{
				l_spChild->AddRef();
				*out_ppProperty = (TheValuePropertyClass*)l_spChild.GetPtr();
				return EResult_Success;
			}
		}

		return EResult_Fail;
	}

	virtual EAbcResult	GetSubArrayIndicesProperty( TheSubArrayIndexPropertyClass** out_ppProperty )
	{
		if ( out_ppProperty == NULL )
			return EResult_InvalidPtr;

		CAbcPtr<IAbcOProperty> l_spChild;
		if ( GetProperty( &l_spChild, PROPNAME_SUBARRAYINDICES ) == EResult_Success )
		{
			if ( l_spChild->GetType() == EPropertyType_Array && l_spChild->GetDataTraits() == EDataTraits_UInt32 )
			{
				l_spChild->AddRef();
				*out_ppProperty = (TheSubArrayIndexPropertyClass*)l_spChild.GetPtr();
				return EResult_Success;
			}
		}

		return EResult_Fail;
	}
};

#endif