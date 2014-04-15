//*****************************************************************************
/*!
	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*

#ifndef CABCOCOMPOUNDPROPERTY_H
#define CABCOCOMPOUNDPROPERTY_H

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

class CAbcOPropertyMap : public CAbcNameMap<IAbcOProperty>
{
public:
	EAbcResult	CreateChild(  IAbcOProperty** out_ppProp, const char* in_pszName, Alembic::Abc::OCompoundProperty& in_parent, EAbcPropertyType in_propType, EAbcDataTraits in_DataType )
	{
		CAbcPtr<IAbcOProperty> l_spOProp;
		std::string l_uniqueName;

		ResolveUniqueName( in_pszName, l_uniqueName );

		EAbcResult l_result = CAbcOPropertyFactory::CreateOProperty( &l_spOProp, in_parent, l_uniqueName.c_str(), in_propType, in_DataType );

		if (l_result == EResult_Success)
		{
			m_NameMap[ l_uniqueName ] = l_spOProp;
			l_spOProp->AddRef();
			*out_ppProp = l_spOProp;
			return EResult_Success;
		}

		return EResult_Fail;
	}

	EAbcResult	WrapChild(  IAbcOProperty** out_ppProp, size_t in_szIndex, Alembic::Abc::OCompoundProperty& in_parent )
	{
		CAbcPtr<IAbcOProperty> l_spOProp;
		Alembic::Abc::OBaseProperty l_child = in_parent.getProperty( in_szIndex );
		EAbcResult l_result = CAbcOPropertyFactory::WrapOProperty( &l_spOProp, l_child.getPtr() );

		if ( l_result == EResult_Success )
		{
			m_NameMap[ l_child.getName() ] = l_spOProp;
			l_spOProp->AddRef();
			*out_ppProp = l_spOProp;
			return EResult_Success;
		}

		return EResult_Fail;
	}

	EAbcResult	CreateChild( IAbcOGeomParamBase** out_ppGeomParam, const char* in_pszName, Alembic::Abc::OCompoundProperty& in_parent, bool in_bIsIndexed, EAbcDataTraits in_DataType, EAbcGeomScope in_geomScope, size_t in_iExtent )
	{
		CAbcPtr<IAbcOGeomParamBase> l_spOGeomParam = NULL;
		std::string l_uniqueName;

		ResolveUniqueName( in_pszName, l_uniqueName );

		EAbcResult l_result = CAbcOPropertyFactory::CreateOGeomParam( &l_spOGeomParam, in_parent, l_uniqueName.c_str(), in_bIsIndexed, in_DataType, in_geomScope, in_iExtent );

		if (l_result == EResult_Success)
		{
			CAbcPtr<IAbcOProperty> l_spProp;
			l_spOGeomParam->GetAsProperty( &l_spProp );

			m_NameMap[ l_uniqueName ] = l_spProp.GetPtr();

			l_spOGeomParam->AddRef();
			*out_ppGeomParam = l_spOGeomParam;
			return EResult_Success;
		}

		return EResult_Fail;
	}

	EAbcResult	GetChildAsGeomParam(  IAbcOGeomParamBase** out_ppGeomParam, const char* in_pszName )
	{
		CAbcPtr<IAbcOProperty> l_spOChildProp;
		if ( CAbcNameMap<IAbcOProperty>::GetChild( &l_spOChildProp, in_pszName ) == EResult_Success )
		{
			return CAbcOPropertyFactory::WrapOGeomParam( out_ppGeomParam, l_spOChildProp );
		}

		return EResult_Fail;
	}

};

class CAbcOCompoundPropertyBase
{
public:
	typedef Alembic::Abc::OCompoundProperty TheAbcPropertyClass;
	typedef IAbcOCompoundProperty TheFrameworkInterface;

	// constructor to create new property
	template <class COMPOUND_PTR>
	CAbcOCompoundPropertyBase( COMPOUND_PTR in_Parent, const char* in_pszName )
		: m_CompoundProperty( in_Parent, in_pszName )
	{ }

	// constructor to wrap existing property
	CAbcOCompoundPropertyBase( Alembic::AbcCoreAbstract::CompoundPropertyWriterPtr in_Ptr, Alembic::Abc::WrapExistingFlag )
		: m_CompoundProperty( in_Ptr, Alembic::Abc::kWrapExisting )
	{
		for (size_t i=0; i<in_Ptr->getNumProperties(); ++i)
		{
			CAbcPtr<IAbcOProperty> l_spChild;
			m_Children.WrapChild( &l_spChild, i, m_CompoundProperty );
		}
	}

	TheAbcPropertyClass& GetInternal() { return m_CompoundProperty;}

	EAbcPropertyType	GetType() const { return EPropertyType_Compound;}
	EAbcPodType			GetPOD() const { return EPodType_Unknown;}
	EAbcDataTraits		GetDataTraits() const { return EDataTraits_Unknown;}

	EAbcResult			Reset() { m_CompoundProperty.reset(); return EResult_Success;}

	EAbcResult CreateProperty(  IAbcOProperty** out_ppProp, const char* in_pszName, EAbcPropertyType in_propType, EAbcDataTraits in_DataType )
	{
		CAbcPtr<IAbcOProperty> l_spChild;
		if (m_Children.CreateChild( &l_spChild, in_pszName, m_CompoundProperty, in_propType, in_DataType ) == EResult_Success)
		{
			l_spChild->AddRef();
			*out_ppProp = l_spChild.GetPtr();
			return EResult_Success;
		}

		return EResult_Fail;
	}

	EAbcResult CreateParticleShapeProperty(  IAbcOParticleShapeProperty** out_ppProp, const char* in_pszName )
	{
		CAbcPtr<IAbcOParticleShapeProperty> l_spChild;
		if (m_Children.CreateChild( (IAbcOProperty**)&l_spChild, in_pszName, m_CompoundProperty, EPropertyType_Compound, EDataTraits_Unknown ) == EResult_Success)
		{
			CAbcPtr<IAbcOProperty> l_spShapeTypeProp;
			l_spChild->CreateProperty( &l_spShapeTypeProp, PROPNAME_SHAPETYPE, EPropertyType_Array, EDataTraits_UInt8 );

			CAbcPtr<IAbcOProperty> l_spShapeInstanceIdProp;
			l_spChild->CreateProperty( &l_spShapeInstanceIdProp, PROPNAME_SHAPEINSTANCEID, EPropertyType_Array, EDataTraits_UInt32 );

			CAbcPtr<IAbcOProperty> l_spShapeInstanceHierarchyProp;
			l_spChild->CreateProperty( &l_spShapeInstanceHierarchyProp, PROPNAME_SHAPEINSTANCEHIERARCHY, EPropertyType_Array, EDataTraits_Bool );

			l_spChild->AddRef();
			*out_ppProp = l_spChild.GetPtr();
			return EResult_Success;
		}

		return EResult_Fail;
	}

	EAbcResult CreateArray2DProperty( IAbcOProperty** out_ppProp, const char* in_pszName, EAbcDataTraits in_EDataTraits )
	{
		CAbcPtr<IAbcOCompoundProperty> l_spCompound;
		if (m_Children.CreateChild( (IAbcOProperty**)&l_spCompound, in_pszName, m_CompoundProperty, EPropertyType_Compound, EDataTraits_Unknown ) == EResult_Success)
		{
			CAbcPtr<IAbcOProperty> l_spValProp;
			l_spCompound->CreateProperty( &l_spValProp, PROPNAME_VALS, EPropertyType_Array, in_EDataTraits );

			CAbcPtr<IAbcOProperty> l_spSubArrayIndicesProp;
			l_spCompound->CreateProperty( &l_spSubArrayIndicesProp, PROPNAME_SUBARRAYINDICES, EPropertyType_Array, EDataTraits_UInt32 );

			l_spCompound->AddRef();
			*out_ppProp = l_spCompound.GetPtr();
			return EResult_Success;
		}

		return EResult_Fail;
	}

	EAbcResult CreateGeomParam( IAbcOGeomParamBase** out_ppGeomParam, const char* in_pszName, bool in_bIsIndexed, EAbcDataTraits in_DataType, EAbcGeomScope in_geomScope, size_t in_iExtent )
	{
		return m_Children.CreateChild( out_ppGeomParam, in_pszName, m_CompoundProperty, in_bIsIndexed, in_DataType, in_geomScope, in_iExtent );
	}

	EAbcResult Create2DGeomParam( IAbcOProperty** out_ppProp, const char* in_pszName, EAbcDataTraits in_EDataTraits, EAbcGeomScope in_geomScope )
	{
		CAbcPtr<IAbcOCompoundProperty> l_spCompound;
		if (m_Children.CreateChild( (IAbcOProperty**)&l_spCompound, in_pszName, m_CompoundProperty, EPropertyType_Compound, EDataTraits_Unknown ) == EResult_Success)
		{
			CAbcPtr<IAbcOGeomParamBase> l_spValGeomParam;
			l_spCompound->CreateGeomParam( &l_spValGeomParam, PROPNAME_VALS, false, in_EDataTraits, in_geomScope, 1 );

			CAbcPtr<IAbcOProperty> l_spSubArrayIndicesProp;
			l_spCompound->CreateProperty( &l_spSubArrayIndicesProp, PROPNAME_SUBARRAYINDICES, EPropertyType_Array, EDataTraits_UInt32 );

			l_spCompound->AddRef();
			*out_ppProp = l_spCompound.GetPtr();
			return EResult_Success;
		}

		return EResult_Fail;
	}

	EAbcResult GetProperty( IAbcOProperty** out_ppProp, const char* in_pszName )
	{
		return m_Children.GetChild( out_ppProp, in_pszName );
	}

	EAbcResult GetGeomParam( IAbcOGeomParamBase** out_ppGeomParam, const char* in_pszName )
	{
		return m_Children.GetChildAsGeomParam( out_ppGeomParam, in_pszName );
	}

protected:
	TheAbcPropertyClass m_CompoundProperty;
	CAbcOPropertyMap m_Children;
};


class CAbcOCompoundProperty : public IAbcOCompoundProperty, protected CAbcOCompoundPropertyBase, protected CRefCount
{
	IMPL_REFCOUNT
public:
	// constructor to create new property
	template <class COMPOUND_PTR>
	CAbcOCompoundProperty( COMPOUND_PTR in_Parent, const char* in_pszName )
		: CAbcOCompoundPropertyBase( in_Parent, in_pszName )
	{ }

	// constructor to wrap existing property
	CAbcOCompoundProperty( Alembic::AbcCoreAbstract::CompoundPropertyWriterPtr in_Ptr )
		: CAbcOCompoundPropertyBase( in_Ptr, Alembic::Abc::kWrapExisting )
	{ }

	TheAbcPropertyClass& GetInternal() { return CAbcOCompoundPropertyBase::GetInternal();}

	virtual EAbcPropertyType	GetType() const { return CAbcOCompoundPropertyBase::GetType();}
	virtual EAbcPodType			GetPOD() const { return CAbcOCompoundPropertyBase::GetPOD();}
	virtual EAbcDataTraits		GetDataTraits() const { return CAbcOCompoundPropertyBase::GetDataTraits();}

	virtual EAbcResult			Reset() { return CAbcOCompoundPropertyBase::Reset();}

	virtual EAbcResult			GetAsGeomParam( IAbcOGeomParamBase** out_ppGeomParam )
	{
		return CAbcOPropertyFactory::WrapOGeomParam( out_ppGeomParam, this );
	}

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

};

#endif