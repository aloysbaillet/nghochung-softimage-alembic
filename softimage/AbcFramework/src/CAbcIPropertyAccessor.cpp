//*****************************************************************************
/*!
	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*

#include "CAbcIPropertyAccessor.h"
#include "CAbcInput.h"
#include "CAbcDataTypeHelper.h"
using namespace Alembic;
using namespace Alembic::Abc;
using namespace Alembic::AbcGeom;
using namespace Alembic::AbcCoreAbstract;

void UpdateDataType( const Alembic::AbcCoreAbstract::PropertyHeader& in_AbcHeader, SAbcDataType& out_AbcFrameworkDataType )
{
	Alembic::Abc::AbcA::DataType in_AbcADataType = in_AbcHeader.getDataType();
	out_AbcFrameworkDataType.m_eType = (EAbcPodType)in_AbcADataType.getPod();
	out_AbcFrameworkDataType.m_numBytes = in_AbcADataType.getPod() == kUnknownPOD ? 0 : in_AbcADataType.getNumBytes();
	out_AbcFrameworkDataType.m_ucExtent = in_AbcADataType.getExtent();
	out_AbcFrameworkDataType.m_eTraits = GetDataTraits( in_AbcHeader );
}

EAbcResult CreatePropFromHeader( const Alembic::Abc::ICompoundProperty& in_Parent, const AbcA::PropertyHeader &l_header, IAbcIPropertyAccessor** out_ppProp )
{
	*out_ppProp = NULL;
	if ( l_header.isArray() )
	{
		IArrayProperty l_arrProp( in_Parent, l_header.getName() );
		*out_ppProp = new CAbcIArrayPropertyAccessor( l_arrProp.getPtr() );
	}
	else if ( l_header.isScalar() )
	{
		IScalarProperty l_scProp( in_Parent, l_header.getName() );
		*out_ppProp = new CAbcIScalarPropertyAccessor( l_scProp.getPtr() );
	}
	else if ( l_header.isCompound() )
	{
		ICompoundProperty l_cmpProp( in_Parent, l_header.getName() );
		*out_ppProp = new CAbcICompoundPropertyAccessor( l_cmpProp.getPtr() );
	}

	if (*out_ppProp)
	{
		(*out_ppProp)->AddRef();
		return EResult_Success;
	}
	else
		return EResult_OutOfMemory;
}

EAbcGeomScope GetGeomScopeFromHeader( const Alembic::Abc::AbcA::PropertyHeader& in_rHeader )
{
	const std::string l_geoScope = in_rHeader.getMetaData().get( "geoScope" );
	if ( l_geoScope == "" )
		return EGeomScope_Unknown;
	else
		return (EAbcGeomScope)Alembic::AbcGeom::GetGeometryScope( in_rHeader.getMetaData() );
}

template<typename TInternalProp, typename TReaderPtrType, typename TParent>
EAbcResult CAbcPropertyAccessorImpl<TInternalProp, TReaderPtrType, TParent>::GetParentProperty( IAbcIPropertyAccessor** out_ppProp ) const
{
	if ( out_ppProp == NULL )
		return EResult_InvalidPtr;

	Alembic::Abc::ICompoundProperty l_compoundProp = m_Prop.getParent();
	if ( l_compoundProp.valid() )
	{
		CAbcICompoundPropertyAccessor* l_pNewAccessor = new CAbcICompoundPropertyAccessor( l_compoundProp.getPtr() );
		if ( l_pNewAccessor )
		{
			l_pNewAccessor->AddRef();
			*out_ppProp = l_pNewAccessor;
			return EResult_Success;
		}
		else
			return EResult_OutOfMemory;
	}
	return EResult_Fail;
}

/*
template<typename TInternalProp, typename TReaderPtrType, typename TParent>
EAbcResult CAbcPropertyAccessorImpl<TInternalProp, TReaderPtrType, TParent>::GetContainingObject( IAbcIObject** out_ppObject ) const
{
	if ( out_ppObject == NULL )
		return EResult_InvalidPtr;
	Alembic::Abc::IObject l_Object = m_Prop.getObject();
	if ( l_Object.valid() )
	{
		CAbcIObject* l_pNewObject = new CAbcIObject( l_Object );
		if ( l_pNewObject )
		{
			l_pNewObject->AddRef();
			*out_ppObject = l_pNewObject;
			return EResult_Success;
		}
		else
			return EResult_OutOfMemory;
	}
	return EResult_Fail;
}
*/

template<typename TInternalProp, typename TReaderPtrType, typename TParent>
EAbcPropertyType CAbcPropertyAccessorImpl<TInternalProp, TReaderPtrType, TParent>::GetPropertyType() const
{
	switch( m_Prop.getPropertyType() )
	{
	case Alembic::Abc::kCompoundProperty:
		return EPropertyType_Compound;
	case Alembic::Abc::kScalarProperty:
		return EPropertyType_Scalar;
	case Alembic::Abc::kArrayProperty:
		return EPropertyType_Array;
	default:
		return EPropertyType_Unknown;
	}
}

template<typename TInternalProp, typename TReaderPtrType, typename TParent>
const char* CAbcPropertyAccessorImpl<TInternalProp, TReaderPtrType, TParent>::GetName() const
{
	return m_strName.c_str();
}

template<typename TInternalProp, typename TReaderPtrType, typename TParent>
CAbcPropertyAccessorImpl<TInternalProp, TReaderPtrType, TParent>::CAbcPropertyAccessorImpl( TReaderPtrType in_Reader ) : m_Prop( in_Reader, Alembic::Abc::kWrapExisting )
{
	m_strName = m_Prop.getName();
	UpdateDataType( m_Prop.getHeader(), m_DataType );
}

template<typename TInternalProp, typename TReaderPtrType, typename TParent>
const SAbcDataType& CAbcPropertyAccessorImpl<TInternalProp, TReaderPtrType, TParent>::GetDataType() const
{
	return m_DataType;
}

template<typename TInternalProp, typename TReaderPtrType, typename TParent>
EAbcPodType CAbcPropertyAccessorImpl<TInternalProp, TReaderPtrType, TParent>::GetPodType() const
{
	return m_DataType.m_eType;
}

template<typename TInternalProp, typename TReaderPtrType, typename TParent>
EAbcGeomScope CAbcPropertyAccessorImpl<TInternalProp, TReaderPtrType, TParent>::GetGeomScope() const
{
	return GetGeomScopeFromHeader( m_Prop.getHeader() );
}

class CAbcRawArraySampleBuffer : public IAbcSampleBuffer, public CRefCount
{
	IMPL_REFCOUNT;
public:
	CAbcRawArraySampleBuffer( const PropertyHeader& in_rHeader, ArraySamplePtr& in_arrPtr ) : m_pArray( in_arrPtr )
	{
		Alembic::Abc::DataType l_srcType = m_pArray->getDataType();
		m_DataType.m_eType = (EAbcPodType)l_srcType.getPod();
		m_DataType.m_ucExtent = l_srcType.getExtent();
		m_DataType.m_eTraits = GetDataTraits( in_rHeader );
		m_DataType.m_numBytes = l_srcType.getNumBytes();
		m_GeomScope = GetGeomScopeFromHeader(in_rHeader);
	}
	const void*			GetBuffer() const
	{
		return m_pArray->getData();
	}
	size_t				GetNumElements() const
	{
		return m_pArray->size();
	}
	const SAbcDataType& GetDataType() const
	{
		return m_DataType;
	}
	EAbcGeomScope GetGeomScope() const
	{
		return m_GeomScope;
	}
	bool IsIndexed() const
	{
		return false;
	}
private:
	ArraySamplePtr m_pArray;
	SAbcDataType m_DataType;
	EAbcGeomScope m_GeomScope;
};

EAbcResult CAbcIScalarPropertyAccessor::GetSample( IAbcISampleSelector* in_pSampleSelector, void* out_pDest ) const
{
	if ( out_pDest == NULL )
		return EResult_InvalidPtr;

	ISampleSelector l_ss;
	CAbcISampleSelector* l_pSelector = (CAbcISampleSelector*)in_pSampleSelector;
	if ( l_pSelector )
		l_pSelector->GetSampleSelector( l_ss );

	try
	{
		m_Prop.get( out_pDest, l_ss );
	}
	catch(...)
	{
		return EResult_Fail;
	}
	return EResult_Success;
}

size_t CAbcIScalarPropertyAccessor::GetNumSamples() const
{
	return m_Prop.getNumSamples();
}

bool CAbcIScalarPropertyAccessor::IsConstant() const
{
	return m_Prop.isConstant();
}

EAbcResult CAbcIScalarPropertyAccessor::GetTimeSampling( IAbcTimeSampling** out_ppSampling )
{
	return CAbcTimeSampling::CreateTimeSampling( *m_Prop.getTimeSampling().get(), out_ppSampling );
}

EAbcResult CAbcIArrayPropertyAccessor::GetSample( IAbcISampleSelector* in_pSampleSelector, IAbcSampleBuffer** out_ppDest ) const
{
	if ( out_ppDest == NULL )
		return EResult_InvalidPtr;

	ISampleSelector l_ss;
	CAbcISampleSelector* l_pSelector = (CAbcISampleSelector*)in_pSampleSelector;
	if ( l_pSelector )
		l_pSelector->GetSampleSelector( l_ss );

	ArraySamplePtr l_pArrSample;

	try
	{
		m_Prop.get( l_pArrSample, l_ss );
	}
	catch(...)
	{
		return EResult_OutOfRange;
	}

	CAbcRawArraySampleBuffer* l_pSampleBuffer = new CAbcRawArraySampleBuffer( m_Prop.getHeader(), l_pArrSample );
	if ( l_pSampleBuffer )
	{
		l_pSampleBuffer->AddRef();
		*out_ppDest = l_pSampleBuffer;
		return EResult_Success;
	}
	else
		return EResult_OutOfMemory;
}

size_t CAbcIArrayPropertyAccessor::GetNumSamples() const
{
	return m_Prop.getNumSamples();
}

bool CAbcIArrayPropertyAccessor::IsConstant() const
{
	return m_Prop.isConstant();
}

EAbcResult CAbcIArrayPropertyAccessor::GetTimeSampling( IAbcTimeSampling** out_ppSampling )
{
	return CAbcTimeSampling::CreateTimeSampling( *m_Prop.getTimeSampling().get(), out_ppSampling );
}

size_t CAbcICompoundPropertyAccessor::GetNumChildrenProperties() const
{
	return m_Prop.getNumProperties();
}

EAbcResult CAbcICompoundPropertyAccessor::GetChildProperty( size_t in_index, IAbcIPropertyAccessor** out_ppProp ) const
{
	if ( out_ppProp == NULL )
		return EResult_InvalidPtr;

	if ( in_index >= m_Prop.getNumProperties() )
		return EResult_OutOfRange;

	const AbcA::PropertyHeader& l_header = m_Prop.getPropertyHeader( in_index );
	
	return CreatePropFromHeader( m_Prop, l_header, out_ppProp );
}

EAbcResult CAbcICompoundPropertyAccessor::GetChildProperty( const char* in_szName, IAbcIPropertyAccessor** out_ppProp ) const
{
	if ( out_ppProp == NULL )
		return EResult_InvalidPtr;

	const AbcA::PropertyHeader* l_pHeader = m_Prop.getPropertyHeader( in_szName );
	if ( l_pHeader )
		return CreatePropFromHeader( m_Prop, *l_pHeader, out_ppProp );
	return EResult_Fail;
}

template<class TRAITS>
EAbcResult CAbcIGeomParamImpl<TRAITS>::GetSample( bool in_bExpand, IAbcISampleSelector* in_pSampleSelector, IAbcSampleBuffer** out_ppBuffer )
{
	if ( !out_ppBuffer ) 
		return EResult_InvalidPtr;

	if ( m_GeomParam.valid() )
	{
		ISampleSelector l_Selector;
		if ( in_pSampleSelector )
			( ( CAbcISampleSelector* )in_pSampleSelector )->GetSampleSelector( l_Selector );

		typename InternalT::Sample l_samp = in_bExpand ? m_GeomParam.getExpandedValue( l_Selector ) : m_GeomParam.getIndexedValue( l_Selector );
		typename InternalT::Sample::samp_ptr_type l_values = l_samp.getVals();

		return CreateBuffer< typename InternalT::Sample::samp_ptr_type, Alembic::Abc::TypedArraySample<TRAITS> >( l_values, (EAbcGeomScope)m_GeomParam.getScope(), m_GeomParam.isIndexed(), out_ppBuffer );
	}

	return EResult_InvalidProp;
}

template<class TRAITS>
EAbcResult CAbcIGeomParamImpl<TRAITS>::GetParentProperty( IAbcIPropertyAccessor** out_ppProp ) const
{
	if ( out_ppProp == NULL )
		return EResult_InvalidPtr;

	Alembic::Abc::ICompoundProperty l_compoundProp = m_GeomParam.getParent();
	if ( l_compoundProp.valid() )
	{
		CAbcICompoundPropertyAccessor* l_pNewAccessor = new CAbcICompoundPropertyAccessor( l_compoundProp.getPtr() );
		if ( l_pNewAccessor )
		{
			l_pNewAccessor->AddRef();
			*out_ppProp = l_pNewAccessor;
			return EResult_Success;
		}
		else
			return EResult_OutOfMemory;
	}
	return EResult_Fail;
}

template<class TRAITS>
EAbcResult CAbcIGeomParamImpl<TRAITS>::GetIndexProperty( IAbcIPropertyAccessor** out_ppProp )
{
	if ( out_ppProp == NULL )
		return EResult_InvalidPtr;

	if ( m_GeomParam.isIndexed() )
	{
		Alembic::Abc::IUInt32ArrayProperty l_indicesProp = m_GeomParam.getIndexProperty();
		return CreatePropFromHeader( l_indicesProp.getParent(), l_indicesProp.getHeader(), out_ppProp );
	}

	return EResult_InvalidProp;
}

template<class TRAITS>
EAbcResult CAbcIGeomParamImpl<TRAITS>::GetValueProperty( IAbcIPropertyAccessor** out_ppProp )
{
	if ( out_ppProp == NULL )
		return EResult_InvalidPtr;

	typename InternalT::prop_type l_valsProp = m_GeomParam.getValueProperty();
	return CreatePropFromHeader( l_valsProp.getParent(), l_valsProp.getHeader(), out_ppProp );
}

template class CAbcIGeomParamImpl<Alembic::Abc::N3fTPTraits>;
template class CAbcIGeomParamImpl<Alembic::Abc::V2fTPTraits>;