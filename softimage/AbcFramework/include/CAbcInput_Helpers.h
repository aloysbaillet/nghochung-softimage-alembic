//*****************************************************************************
/*!
	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*

#ifndef CABCINPUT_HELPERS_H
#define CABCINPUT_HELPERS_H

#include <Alembic/Abc/All.h>
#include <Alembic/AbcGeom/All.h>
#include <Alembic/AbcCoreAbstract/All.h>
#include "CAbcIPropertyAccessor.h"

EAbcResult CreateFromInternal( Alembic::Abc::IObject in_object, IAbcIObject** out_ppObject );

template < typename TInternal, typename TInterface >
class CAbcIObjectImpl : public TInterface
{
protected:
	CAbcIObjectImpl( const TInternal& in_obj ) : m_Object( in_obj ) {}

	template< typename U >
	CAbcIObjectImpl( const U& in_obj, Alembic::Abc::WrapExistingFlag in_flag ) : m_Object( in_obj, in_flag ) {}

	const char*	GetName() const
	{
		return m_Object.getName().c_str();
	}
	const char*	GetFullName() const
	{
		return m_Object.getFullName().c_str();
	}
	size_t GetNumChildren() const
	{
		return m_Object.getNumChildren();
	}

	EAbcResult GetChild( size_t in_Index, IAbcIObject** out_ppObject ) const
	{
		if ( !out_ppObject )
			return EResult_InvalidPtr;

		if ( in_Index < m_Object.getNumChildren() )
		{
			Alembic::Abc::IObject l_obj = m_Object.getChild( in_Index );
			if ( l_obj.valid() )
				return CreateFromInternal( l_obj, out_ppObject );
			else
				return EResult_Fail;
		}
		else
			return EResult_OutOfRange;	
	}

	EAbcResult GetNamedChild( const char* in_szName, IAbcIObject** out_ppObject ) const
	{
		if ( !out_ppObject )
			return EResult_InvalidPtr;

		Alembic::Abc::IObject l_obj = m_Object.getChild( in_szName );
	
		if ( l_obj.valid() )
			return CreateFromInternal( l_obj, out_ppObject );
		else
			return EResult_OutOfRange;
	}
	void GetMetaDataValue( const char* in_szKey, char* out_szValue, size_t* out_pCount ) const
	{
		std::string l_strVal = m_Object.getMetaData().get( in_szKey );
		if ( out_szValue )
		{
			strcpy( out_szValue, l_strVal.c_str() );
		}
		if ( out_pCount )
			*out_pCount = l_strVal.length();
	}
	bool IsEqualObject( IAbcIObject* in_pOther )
	{
		return !strcmp( in_pOther->GetFullName(), GetFullName() );
	}
	
	bool IsEqualObjectAndType( IAbcIObject* in_pOther )
	{
		return  ( GetType() == in_pOther->GetType() ) && IsEqualObject( in_pOther );
	}

	EAbcResult GetParent( IAbcIObject** out_ppObject )
	{
		return CreateFromInternal( m_Object.getParent(), out_ppObject );
	}

	virtual EAbcResult GetPropertyAccessor( IAbcICompoundPropertyAccessor** out_ppPropertyAccessor )
	{
		if ( out_ppPropertyAccessor == NULL )
			return EResult_InvalidPtr;

		CAbcICompoundPropertyAccessor* l_pAccessor = new CAbcICompoundPropertyAccessor( m_Object.getProperties().getPtr() );
		if ( l_pAccessor )
		{
			l_pAccessor->AddRef();
			*out_ppPropertyAccessor = l_pAccessor;
			return EResult_Success;
		}
		else
			return EResult_OutOfMemory;
	}

	

	TInternal* GetInternalObject() { return &m_Object; }												\
	const TInternal* GetInternalObject() const { return &m_Object; }

protected:
	TInternal	m_Object;
};

template < typename TInternal, typename TInterface, int ObjectType >
class CAbcISchemaObjectImpl : public CAbcIObjectImpl< TInternal, TInterface >
{
protected:
	typedef typename TInternal::schema_type SCHEMA;
	typedef CAbcIObjectImpl< TInternal, TInterface > TParentClass;
	CAbcISchemaObjectImpl( const Alembic::Abc::IObject& in_obj, Alembic::Abc::WrapExistingFlag in_flag ) : TParentClass( in_obj, in_flag ), m_Schema( TParentClass::m_Object.getSchema() ) {}
public:
	virtual size_t GetNumSamples() const
	{
		return m_Schema.getNumSamples();
	}

	virtual EAbcResult GetTimeSampling( IAbcTimeSampling** out_ppSampling )
	{
		return CAbcTimeSampling::CreateTimeSampling( *m_Schema.getTimeSampling(), out_ppSampling );
	}

	virtual EAbcResult GetSchemaObject( IAbcISchemaObject** out_ppObject ) const
	{
		if ( out_ppObject == NULL )
			return EResult_InvalidPtr;

		*out_ppObject = const_cast<CAbcISchemaObjectImpl*>( this );
		const_cast<CAbcISchemaObjectImpl*>( this )->AddRef();

		return EResult_Success;
	}

	virtual EAbcResult TransformInto( EAbcIObjectType in_objType, IAbcIObject** out_ppObject )
	{
		if ( out_ppObject == NULL )
			return EResult_InvalidPtr;

		if ( (int)in_objType == ObjectType )
		{
			*out_ppObject = this;
			this->AddRef();
			return EResult_Success;
		}
		return EResult_InvalidCast;
	}

	virtual EAbcIObjectType GetType() const
	{
		return (EAbcIObjectType)ObjectType;
	}
	
	virtual EAbcResult GetSchemaPropertyAccessor( IAbcICompoundPropertyAccessor** out_ppPropertyAccessor )
	{
		if ( out_ppPropertyAccessor == NULL )
			return EResult_InvalidPtr;

		CAbcICompoundPropertyAccessor* l_pAccessor = new CAbcICompoundPropertyAccessor( m_Schema.getPtr() );
		if ( l_pAccessor )
		{
			l_pAccessor->AddRef();
			*out_ppPropertyAccessor = l_pAccessor;
			return EResult_Success;
		}
		else
			return EResult_OutOfMemory;
	}

protected:
	SCHEMA&		m_Schema;
};

#endif // CABCINPUT_HELPERS
