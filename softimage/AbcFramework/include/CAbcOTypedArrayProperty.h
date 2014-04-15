//*****************************************************************************
/*!
	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*

#ifndef CABCOTYPEDARRAYPROPERTY_H
#define CABCOTYPEDARRAYPROPERTY_H

#include <Alembic/Abc/OObject.h>
#include <Alembic/Abc/OTypedArrayProperty.h>
#include <Alembic/AbcCoreAbstract/BasePropertyWriter.h>

#include "AbcFrameworkUtil.h"
#include "CRefCount.h"
#include "IAbcFramework.h"
#include "IAbcOutput.h"

template< class TRAITS >
class CAbcOTypedArrayProperty : public IAbcOTypedArrayProperty<TRAITS>, protected CRefCount
{
	IMPL_REFCOUNT;
public:
	typedef Alembic::Abc::OTypedArrayProperty<TRAITS> TheAbcPropertyClass;
	typedef IAbcOTypedArrayProperty<TRAITS> TheFrameworkInterface;

	// constructor to create new property
	template <class COMPOUND_PTR>
	CAbcOTypedArrayProperty( COMPOUND_PTR in_Parent, const char* in_pszName )
		: m_Property( in_Parent, in_pszName )
	{ }

	// constructor to wrap existing property
	CAbcOTypedArrayProperty( Alembic::AbcCoreAbstract::ArrayPropertyWriterPtr in_Ptr )
		: m_Property( in_Ptr, Alembic::Abc::kWrapExisting )
	{

	}

	TheAbcPropertyClass& GetInternal() { return m_Property;}

	EAbcPodType			GetPOD() const { return GetPodType(m_Property.getDataType());}
	EAbcPropertyType	GetType() const { return EPropertyType_Array;}
	EAbcDataTraits		GetDataTraits() const { return AbcFramework::GetEDataTraits<TRAITS>();}

	EAbcResult			GetAsGeomParam( IAbcOGeomParamBase** out_ppGeomParam )
	{
		return CAbcOPropertyFactory::WrapOGeomParam( out_ppGeomParam, this );
	}

	EAbcResult			Reset() { m_Property.reset(); return EResult_Success;}

	void AddSample( const typename TRAITS::value_type* in_pArray, int in_iSize )
	{
		m_Property.set( typename TheAbcPropertyClass::sample_type( in_pArray, in_iSize ) );
	}

protected:
	TheAbcPropertyClass m_Property;
};

#endif