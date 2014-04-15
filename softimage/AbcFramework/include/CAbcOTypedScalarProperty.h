//*****************************************************************************
/*!
	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*

#ifndef CABCOTYPEDSCALARPROPERTY_H
#define CABCOTYPEDSCALARPROPERTY_H

#include <Alembic/Abc/OObject.h>
#include <Alembic/Abc/OTypedScalarProperty.h>
#include <Alembic/AbcCoreAbstract/BasePropertyWriter.h>

#include "AbcFrameworkUtil.h"
#include "CRefCount.h"
#include "IAbcFramework.h"
#include "IAbcOutput.h"

template< class TRAITS >
class CAbcOTypedScalarProperty : public IAbcOTypedScalarProperty<TRAITS>, protected CRefCount
{
	IMPL_REFCOUNT;
public:
	typedef Alembic::Abc::OTypedScalarProperty<TRAITS> TheAbcPropertyClass;
	typedef IAbcOTypedScalarProperty<TRAITS> TheFrameworkInterface;

	// constructor to create new property
	template <class COMPOUND_PTR>
	CAbcOTypedScalarProperty( COMPOUND_PTR in_Parent, const char* in_pszName )
		: m_Property( in_Parent, in_pszName )
	{ }

	// constructor to wrap existing property
	CAbcOTypedScalarProperty( Alembic::AbcCoreAbstract::ScalarPropertyWriterPtr in_Ptr )
		: m_Property( in_Ptr, Alembic::Abc::kWrapExisting )
	{ }

	TheAbcPropertyClass& GetInternal() { return m_Property;}

	EAbcPodType			GetPOD() const { return GetPodType(m_Property.getDataType());}
	EAbcPropertyType	GetType() const { return EPropertyType_Scalar;}
	EAbcDataTraits		GetDataTraits() const { return AbcFramework::GetEDataTraits<TRAITS>();}

	EAbcResult			Reset() { m_Property.reset(); return EResult_Success;}

	EAbcResult			GetAsGeomParam( IAbcOGeomParamBase** out_ppGeomParam ) { return EResult_Fail;}

	void AddSample( const typename TRAITS::value_type& inVal )
	{
		m_Property.set( inVal );
	}

protected:
	TheAbcPropertyClass m_Property;
};

#endif