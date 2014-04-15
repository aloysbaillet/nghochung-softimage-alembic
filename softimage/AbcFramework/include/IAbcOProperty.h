//*****************************************************************************
/*!
	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*

#ifndef IABCOPROPERTY_H
#define IABCOPROPERTY_H

#include "AbcFrameworkUtil.h"
#include "IAbcFramework.h"
#include "AbcFrameworkDataTypes.h"

class IAbcOGeomParamBase;

class IAbcOProperty : public IBase
{
public:
	virtual EAbcPropertyType	GetType() const = 0;
	virtual EAbcPodType			GetPOD() const = 0;
	virtual EAbcDataTraits		GetDataTraits() const = 0;

	virtual EAbcResult			Reset() = 0;
	virtual EAbcResult			GetAsGeomParam( IAbcOGeomParamBase** out_ppGeomParam ) = 0;
};

template< class TRAITS >
class IAbcOTypedScalarProperty : public IAbcOProperty
{
public:
	virtual void			AddSample( const typename TRAITS::value_type& in_val ) = 0;
};

template< class TRAITS >
class IAbcOTypedArrayProperty : public IAbcOProperty
{
public:
	virtual void			AddSample( const typename TRAITS::value_type* in_pVals, int in_iNumVals ) = 0;
};

class IAbcOParticleShapeProperty;

class IAbcOCompoundProperty : public IAbcOProperty
{
public:
	virtual EAbcResult CreateProperty( IAbcOProperty** out_ppProp, const char* in_pszName, EAbcPropertyType in_propType, EAbcDataTraits in_EDataTraits ) = 0;
	virtual EAbcResult CreateGeomParam( IAbcOGeomParamBase** out_ppGeomParam, const char* in_pszName, bool in_bIsIndexed, EAbcDataTraits in_DataType, EAbcGeomScope in_geomScope, size_t in_iExtent ) = 0;
	
	virtual EAbcResult CreateParticleShapeProperty( IAbcOParticleShapeProperty** out_ppProperty, const char* in_pszName ) = 0;
	virtual EAbcResult CreateArray2DProperty( IAbcOProperty** out_ppProp, const char* in_pszName, EAbcDataTraits in_EDataTraits ) = 0;
	virtual EAbcResult Create2DGeomParam( IAbcOProperty** out_ppProp, const char* in_pszName, EAbcDataTraits in_EDataTraits, EAbcGeomScope in_geomScope ) = 0;

	virtual EAbcResult GetProperty( IAbcOProperty** out_ppProp, const char* in_pszName ) = 0;
	virtual EAbcResult GetGeomParam( IAbcOGeomParamBase** out_ppGeomParam, const char* in_pszName ) = 0;
};

class IAbcOParticleShapeProperty : public IAbcOCompoundProperty
{
public:
	EAbcResult GetShapeTypeProperty( IAbcOProperty** out_ppProp )				{ return GetProperty( out_ppProp, PROPNAME_SHAPETYPE );}
	EAbcResult GetShapeInstanceIdProperty( IAbcOProperty** out_ppProp )			{ return GetProperty( out_ppProp, PROPNAME_SHAPEINSTANCEID );}
	EAbcResult GetShapeInstanceHierarchyProperty( IAbcOProperty** out_ppProp )	{ return GetProperty( out_ppProp, PROPNAME_SHAPEINSTANCEHIERARCHY );}
};

// IAbcOTypedArray2DProperty is a convenient interface to write ICE attribute data array 2D
// Basically it's a Compound Property with 2 child Properties:
// - ".vals": the flattened values of all the subarrays (in order)
// - ".subArrayIndices": the start and end index of each subarray
template< class TRAITS >
class IAbcOTypedArray2DProperty : public IAbcOCompoundProperty
{
public:
	typedef IAbcOTypedArrayProperty<TRAITS> TheValuePropertyClass;
	typedef IAbcOTypedArrayProperty<Alembic::Util::Uint32PODTraits> TheSubArrayIndexPropertyClass;

	EAbcResult	GetValueProperty( TheValuePropertyClass** out_ppProperty )
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

	EAbcResult	GetSubArrayIndicesProperty( TheSubArrayIndexPropertyClass** out_ppProperty )
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

// OTypedGeomParam is a convenient class to encapsulate a geometry-dependent property
// Basically it's a Compound Property with 2 child Properties:
// - ".vals": the property values
// - ".indices": the indices to the geometry subcomponents (vertices or faces)
class IAbcOGeomParamBase : public IBase
{
public:
	virtual EAbcPodType			GetPOD() const = 0;
	virtual EAbcDataTraits		GetDataTraits() const = 0;
	
	virtual EAbcResult			GetAsProperty( IAbcOProperty** out_ppProperty ) = 0;
};

template< class TRAITS >
class IAbcOTypedGeomParam : public IAbcOGeomParamBase
{
public:
	typedef IAbcOTypedArrayProperty<Alembic::Abc::Uint32TPTraits> TheIndexPropertyClass;
	typedef IAbcOTypedArrayProperty<TRAITS> TheValuePropertyClass;

	virtual EAbcResult GetValueProperty( TheValuePropertyClass** out_ppProperty ) = 0;
	virtual EAbcResult GetIndexProperty( TheIndexPropertyClass** out_ppProperty ) = 0;

	virtual void			AddSample(	const typename TRAITS::value_type* in_pVals, int in_iNumVals,
										const Alembic::Util::uint32_t* in_pIndices, int in_iNumIndices ) = 0;
};

#endif