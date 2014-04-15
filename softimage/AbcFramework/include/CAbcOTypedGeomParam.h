//*****************************************************************************
/*!
	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*

#ifndef CABCOTYPEDGEOMPARAM_H
#define CABCOTYPEDGEOMPARAM_H

#include <Alembic/Abc/OObject.h>
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
class CAbcOTypedGeomParam : public IAbcOTypedGeomParam<TRAITS>, protected CRefCount
{
	IMPL_REFCOUNT;
public:
	typedef IAbcOTypedGeomParam<TRAITS> TheInterface;
	typedef Alembic::AbcGeom::OTypedGeomParam<TRAITS> TheAbcClass;

	// constructor to create new property
	template <class COMPOUND_PTR>
	CAbcOTypedGeomParam( COMPOUND_PTR in_Parent, const char* in_pszName, bool in_bIsIndexed, EAbcGeomScope in_geomScope, size_t in_iExtent )
		: m_GeomParam( in_Parent, in_pszName, in_bIsIndexed, (Alembic::AbcGeom::GeometryScope)in_geomScope, in_iExtent )
	{
		if ( in_bIsIndexed )
		{
			CAbcOPropertyFactory::WrapOProperty( &m_spProperty, m_GeomParam.getIndexProperty().getPtr()->getParent() );
			CAbcPtr<IAbcOCompoundProperty> l_spCompound = static_cast<IAbcOCompoundProperty*>(m_spProperty.GetPtr());
			l_spCompound->GetProperty( (IAbcOProperty**) &m_spValueProperty, ".vals" );
			l_spCompound->GetProperty( (IAbcOProperty**) &m_spValueProperty, ".indices" );
		}
		else
		{
			CAbcOPropertyFactory::WrapOProperty( &m_spProperty, m_GeomParam.getValueProperty().getPtr() );
			m_spValueProperty = static_cast<typename TheInterface::TheValuePropertyClass*>( m_spProperty.GetPtr() );
		}

		m_bIsIndexed = in_bIsIndexed;	
	}

	CAbcOTypedGeomParam( Alembic::Abc::BasePropertyWriterPtr in_PropWriter, IAbcOProperty* in_pProp )
		: m_spProperty( in_pProp )
	{
		switch ( in_pProp->GetType() )
		{
		case EPropertyType_Compound:
			{
				m_spProperty = in_pProp;

				CAbcPtr<IAbcOCompoundProperty> l_spCompound = static_cast<IAbcOCompoundProperty*>(in_pProp);
				l_spCompound->GetProperty( (IAbcOProperty**) &m_spValueProperty, ".vals" );
				l_spCompound->GetProperty( (IAbcOProperty**) &m_spIndexProperty, ".indices" );

				m_bIsIndexed = true;
			}
			break;

		case  EPropertyType_Array:
			{
				if ( in_pProp->GetDataTraits() == GetDataTraits() )
				{
					m_spValueProperty = static_cast<typename TheInterface::TheValuePropertyClass*>(in_pProp);
					m_spProperty = in_pProp;

					m_bIsIndexed = false;
				}
			}
			break;
		}
	}

	EAbcPodType GetPOD() const { return GetPodType(m_GeomParam.getDataType());}

	EAbcDataTraits GetDataTraits() const { return AbcFramework::GetEDataTraits<TRAITS>();}

	EAbcResult GetValueProperty( typename TheInterface::TheValuePropertyClass** out_ppProperty )
	{
		if ( m_spValueProperty != NULL )
		{
			m_spValueProperty->AddRef();
			*out_ppProperty = m_spValueProperty.GetPtr();
			return EResult_Success;
		}
		else
		{
			return EResult_Fail;
		}
	}

	EAbcResult GetIndexProperty( typename TheInterface::TheIndexPropertyClass** out_ppProperty )
	{
		if ( m_spIndexProperty != NULL )
		{
			m_spIndexProperty->AddRef();
			*out_ppProperty = m_spIndexProperty.GetPtr();
			return EResult_Success;
		}
		else
		{
			return EResult_Fail;
		}
	}

	EAbcResult GetAsProperty( IAbcOProperty** out_ppProperty )
	{
		m_spProperty->AddRef();
		*out_ppProperty = m_spProperty.GetPtr();
		return EResult_Success;
	}

	void AddSample(	const typename TRAITS::value_type* in_pVals, int in_iNumVals,
		const Alembic::Util::uint32_t* in_pIndices, int in_iNumIndices )
	{
		typename TheAbcClass::sample_type l_sample;
		if ( in_pVals != NULL && in_iNumVals > 0 )
		{
			l_sample.setVals( Alembic::Abc::TypedArraySample<TRAITS>( in_pVals, in_iNumVals ) );

			if ( in_pIndices != NULL && in_iNumIndices > 0 )
			{
				l_sample.setIndices( Alembic::Abc::TypedArraySample<Alembic::Abc::Uint32TPTraits>( in_pIndices, in_iNumIndices ) );
			}

			m_GeomParam.set( l_sample );
		}
	}

protected:
	Alembic::AbcGeom::OTypedGeomParam<TRAITS> m_GeomParam;
	CAbcPtr<IAbcOProperty> m_spProperty;
	CAbcPtr< typename TheInterface::TheValuePropertyClass > m_spValueProperty;
	CAbcPtr< typename TheInterface::TheIndexPropertyClass > m_spIndexProperty;
	bool m_bIsIndexed;
};

#endif