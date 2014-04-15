//*****************************************************************************
/*!
	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*

#ifndef CABCIPROPERTYACCESSOR_H
#define CABCIPROPERTYACCESSOR_H

#include "IAbcFramework.h"
#include "IAbcInput.h"
#include "CRefCount.h"
#include <Alembic/Abc/All.h>
#include <Alembic/AbcGeom/All.h>
#include <Alembic/AbcCoreAbstract/All.h>

void UpdateDataType( const Alembic::AbcCoreAbstract::PropertyHeader& in_AbcHeader, SAbcDataType& out_AbcFrameworkDataType );
EAbcResult CreatePropFromHeader( const Alembic::Abc::ICompoundProperty& in_Parent, const Alembic::Abc::AbcA::PropertyHeader &in_Header, IAbcIPropertyAccessor** out_ppProp );
EAbcGeomScope GetGeomScopeFromHeader( const Alembic::Abc::AbcA::PropertyHeader& in_Header );

template<typename TInternalProp, typename TReaderPtrType, typename TParent>
class CAbcPropertyAccessorImpl : public TParent
{
public:
	CAbcPropertyAccessorImpl( TReaderPtrType in_Reader );
	virtual const char*	GetName() const;
	virtual EAbcPodType	GetPodType() const;
	virtual EAbcPropertyType	GetPropertyType() const;
	//virtual EAbcResult	GetContainingObject( IAbcIObject** out_ppObject ) const;
	virtual const SAbcDataType& GetDataType() const;
	virtual EAbcResult GetParentProperty( IAbcIPropertyAccessor** out_ppProp ) const;
	virtual EAbcGeomScope GetGeomScope() const;
protected:
	std::string m_strName;
	SAbcDataType m_DataType;
	TInternalProp m_Prop;
};

typedef CAbcPropertyAccessorImpl<Alembic::Abc::IScalarProperty, Alembic::Abc::AbcA::ScalarPropertyReaderPtr, IAbcIScalarPropertyAccessor> TAbcIScalarPropertyAccessorBase;

class CAbcIScalarPropertyAccessor : 
	public TAbcIScalarPropertyAccessorBase, protected CRefCount
{
	IMPL_REFCOUNT;
public:
	CAbcIScalarPropertyAccessor( Alembic::Abc::AbcA::ScalarPropertyReaderPtr in_Reader ) : TAbcIScalarPropertyAccessorBase( in_Reader ) {}
	EAbcResult	GetSample( IAbcISampleSelector* in_pSampleSelector, void* out_pDest ) const;
	size_t		GetNumSamples() const;
	bool		IsConstant() const;
	EAbcResult	GetTimeSampling( IAbcTimeSampling** out_ppSampling );
private:

};

typedef CAbcPropertyAccessorImpl<Alembic::Abc::IArrayProperty, Alembic::Abc::AbcA::ArrayPropertyReaderPtr, IAbcIArrayPropertyAccessor> TAbcIArrayPropertyAccessorBase;
class CAbcIArrayPropertyAccessor : 
	public TAbcIArrayPropertyAccessorBase, protected CRefCount
{
	IMPL_REFCOUNT;
public:
	CAbcIArrayPropertyAccessor( Alembic::Abc::AbcA::ArrayPropertyReaderPtr in_Reader ) : TAbcIArrayPropertyAccessorBase( in_Reader ) {}

	EAbcResult	GetSample( IAbcISampleSelector* in_pSampleSelector, IAbcSampleBuffer** out_ppDest ) const;
	size_t		GetNumSamples() const;
	bool		IsConstant() const;
	EAbcResult	GetTimeSampling( IAbcTimeSampling** out_ppSampling );
private:
};

typedef CAbcPropertyAccessorImpl<Alembic::Abc::ICompoundProperty, Alembic::Abc::AbcA::CompoundPropertyReaderPtr, IAbcICompoundPropertyAccessor> TAbcICompoundPropertyAccessorBase;
class CAbcICompoundPropertyAccessor : 
	public TAbcICompoundPropertyAccessorBase, protected CRefCount
{
	IMPL_REFCOUNT;
public:
	CAbcICompoundPropertyAccessor( Alembic::Abc::AbcA::CompoundPropertyReaderPtr in_Reader ) : TAbcICompoundPropertyAccessorBase( in_Reader ) {}

	size_t		GetNumChildrenProperties() const;
	EAbcResult	GetChildProperty( size_t in_index, IAbcIPropertyAccessor** out_ppProp ) const;
	EAbcResult	GetChildProperty( const char* in_szName, IAbcIPropertyAccessor** out_ppProp ) const;
	EAbcResult	GetObject( IAbcIObject** out_ppObject ) const { return EResult_Success; }
private:
	
};

template<class TRAITS>
class CAbcIGeomParamImpl : public IAbcIGeomParam, protected CRefCount
{
	IMPL_REFCOUNT;
public:
	template<class CPROP>
	CAbcIGeomParamImpl( CPROP in_Parent, const std::string &in_Name )
		: m_GeomParam( in_Parent, in_Name )
	{
		m_strName = m_GeomParam.getName();

		// indexed or not, Value property should always be valid
		UpdateDataType( m_GeomParam.getValueProperty().getHeader(), m_DataType );
	}

	virtual const char*	GetName() const			{ return m_strName.c_str();}
	virtual EAbcPodType	GetPodType() const		{ return m_DataType.m_eType;}
	virtual bool IsIndexed() const				{ return m_GeomParam.isIndexed();}
	
	virtual const SAbcDataType& GetDataType() const { return m_DataType;}

	virtual EAbcResult GetParentProperty( IAbcIPropertyAccessor** out_ppProp ) const;
	virtual EAbcResult GetValueProperty( IAbcIPropertyAccessor** out_ppProp );
	virtual EAbcResult GetIndexProperty( IAbcIPropertyAccessor** out_ppProp );
	virtual EAbcResult GetSample( bool in_bExpand, IAbcISampleSelector* in_pSampleSelector, IAbcSampleBuffer** out_ppBuffer );

protected:
	typedef Alembic::AbcGeom::ITypedGeomParam<TRAITS> InternalT;
	std::string m_strName;
	SAbcDataType m_DataType;
	InternalT m_GeomParam;
};

#endif