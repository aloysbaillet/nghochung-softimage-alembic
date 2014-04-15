//*****************************************************************************
/*!
	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*

#ifndef CABCOUTPUT_HELPERS_H
#define CABCOUTPUT_HELPERS_H

#include <Alembic/Abc/OObject.h>

#include "AbcFrameworkUtil.h"
#include "CAbcOObjectFactory.h"
#include "CAbcOPropertyFactory.h"
#include "CRefCount.h"
#include "IAbcFramework.h"
#include "IAbcOutput.h"
#include <string>

template< class T >
class CAbcNameMap
{
public:
	~CAbcNameMap()
	{
#ifdef _DEBUG
		for ( typename NameMap::const_iterator it = m_NameMap.begin(); it != m_NameMap.end(); ++it )
		{
			assert( it->second->GetRefCount() == 1 );
		}
#endif
		m_NameMap.clear();
	}

	EAbcResult GetChild( T** out_ppOObject, const char* in_pszChildName )
	{ 
		typename NameMap::iterator it = m_NameMap.find( in_pszChildName );
		if ( it != m_NameMap.end() )
		{
			it->second->AddRef();
			*out_ppOObject = ((CAbcPtr<T>&)(it->second)).GetPtr();
			return EResult_Success;
		}
		else
		{
			return EResult_Fail;
		}
	}

	bool ResolveUniqueName( const std::string& in_name, std::string& out_uniqueName ) const
	{
		out_uniqueName = in_name;
		typename NameMap::const_iterator it = m_NameMap.find( in_name );

		if ( it == m_NameMap.end() )
			return true;

		// just append "1" at the end
		out_uniqueName = in_name;
		do 
		{
			out_uniqueName += '1';
		} while ( m_NameMap.find(out_uniqueName) != m_NameMap.end() );

		return false;
	}

protected:
	typedef std::map< std::string, CAbcAdapt< CAbcPtr<T> > > NameMap;
	NameMap m_NameMap;
};


class CAbcOObjectList : public CAbcNameMap<IAbcOObject>
{
public:
	EAbcResult	CreateChild( IAbcOObject** out_ppOObject, IAbcOObject* in_pParent,  EAbcOObjectType in_type, const char* in_pszName, Alembic::Abc::OObject& in_parentOObj )
	{
		CAbcPtr<IAbcOObject> l_spObj = NULL;
		std::string l_uniqueName;

		ResolveUniqueName( in_pszName, l_uniqueName );

		EAbcResult l_result = CAbcOObjectFactory::CreateOObject( &l_spObj, in_pParent, in_type, in_parentOObj, l_uniqueName.c_str() );

		if (l_result == EResult_Success)
		{
			m_NameMap[ l_uniqueName ] = l_spObj;

			l_spObj->AddRef();
			*out_ppOObject = l_spObj;
			return EResult_Success;
		}

		return EResult_Fail;
	}
};

template< class TInternal, class TInterface >
class CAbcOObjectImpl : public TInterface
{
public:
	// constructor to create new object
	CAbcOObjectImpl( IAbcOObject* in_pParent, Alembic::Abc::OObject& in_Parent, const char* in_pszName )
		: m_pParent( in_pParent )
		, m_OObject( in_Parent, in_pszName )
	{	
		CAbcOPropertyFactory::WrapOProperty( (IAbcOProperty**)&m_spProperties, Alembic::Abc::GetCompoundPropertyWriterPtr(GetInternalObject().getProperties()) );
	}

	virtual ~CAbcOObjectImpl()
	{	
#ifdef _DEBUG
		if ( m_spProperties )
		{
			assert( m_spProperties->GetRefCount() == 1 );
		}
#endif		
	}

	virtual const char* GetName( ) const
	{
		return m_OObject.getName().c_str();
	}

	virtual const char* GetFullName( ) const
	{
		return m_OObject.getHeader().getFullName().c_str();
	}

	virtual EAbcResult GetChild( IAbcOObject** out_ppOObject, const char* in_pszChildName )
	{
		return m_Children.GetChild( out_ppOObject, in_pszChildName );
	}

	virtual EAbcResult	CreateChild( IAbcOObject** out_ppOObject, EAbcOObjectType in_type, const char* in_pszName )
	{
		return m_Children.CreateChild( out_ppOObject, this, in_type, in_pszName, m_OObject );
	}

	virtual EAbcResult	GetProperties( IAbcOCompoundProperty** out_ppProp )
	{
		if ( out_ppProp == NULL )
			return EResult_InvalidPtr;

		if ( !m_spProperties )
			return EResult_Fail;

		m_spProperties->AddRef();
		*out_ppProp = m_spProperties.GetPtr();
		return EResult_Success;
	}

	virtual EAbcResult GetParent( IAbcOObject** out_ppParent )
	{
		if ( out_ppParent == NULL )
			return EResult_InvalidPtr;

		if ( m_pParent )
		{
			m_pParent->AddRef();
			*out_ppParent = m_pParent;
			return EResult_Success;
		}
		else
		{
			return EResult_Fail;
		}
	}


	virtual IAbcOObject* GetParent() const
	{
		return m_pParent;
	}

	TInternal& GetInternalObject() 
	{
		return m_OObject;
	}

	const TInternal& GetInternalObject() const
	{
		return m_OObject;
	}

protected:
	explicit CAbcOObjectImpl( Alembic::Abc::OObject in_Copy )
		: m_OObject( in_Copy )
	{	}

	TInternal		m_OObject;
	CAbcOObjectList m_Children;
	IAbcOObject*	m_pParent;
	CAbcPtr<IAbcOCompoundProperty> m_spProperties;
};

template< class TInternal, class TInterface >
class CAbcOSchemaObjectImpl : public CAbcOObjectImpl< TInternal, TInterface >
{
public:
	typedef typename TInternal::schema_type SCHEMA;
	typedef CAbcOObjectImpl< TInternal, TInterface > TParentClass;

	// constructor to create new Schema object
	CAbcOSchemaObjectImpl( IAbcOObject* in_pParent, Alembic::Abc::OObject& in_obj, const char* in_pszName ) 
		: TParentClass( in_pParent, in_obj, in_pszName )
	{
		CAbcOPropertyFactory::WrapOProperty( (IAbcOProperty**)&m_spSchemaProperties, Alembic::Abc::GetCompoundPropertyWriterPtr(GetSchema()) );
		CAbcOPropertyFactory::WrapOProperty( (IAbcOProperty**)&m_spArbGeomParams, Alembic::Abc::GetCompoundPropertyWriterPtr(GetSchema().getArbGeomParams()) );
		CAbcOPropertyFactory::WrapOProperty( (IAbcOProperty**)&m_spUserProperties, Alembic::Abc::GetCompoundPropertyWriterPtr(GetSchema().getUserProperties()) );
	}

	~CAbcOSchemaObjectImpl()
	{
#ifdef _DEBUG		
		if ( m_spArbGeomParams )
		{
			assert( m_spArbGeomParams->GetRefCount() == 1 );
		}
		
		if ( m_spUserProperties )
		{
			assert( m_spUserProperties->GetRefCount() == 1 );
		}
		
		if ( m_spSchemaProperties )
		{
			assert( m_spSchemaProperties->GetRefCount() == 1 );
		}
#endif
	}

	SCHEMA& GetSchema()					{ return TParentClass::m_OObject.getSchema();}
	const SCHEMA& GetSchema() const		{ return TParentClass::m_OObject.getSchema();}

	EAbcResult GetSchemaProperties( IAbcOCompoundProperty** out_ppProp )
	{
		if ( out_ppProp == NULL )
			return EResult_InvalidPtr;

		if ( !m_spSchemaProperties )
			return EResult_Fail;

		m_spSchemaProperties->AddRef();
		*out_ppProp = m_spSchemaProperties.GetPtr();
		return EResult_Success;
	}

	EAbcResult GetArbGeomParams( IAbcOCompoundProperty** out_ppProp )
	{
		if ( out_ppProp == NULL )
			return EResult_InvalidPtr;

		if ( !m_spArbGeomParams )
			return EResult_Fail;

		m_spArbGeomParams->AddRef();
		*out_ppProp = m_spArbGeomParams.GetPtr();
		return EResult_Success;
	}

	EAbcResult GetUserProperties( IAbcOCompoundProperty** out_ppProp )
	{
		if ( out_ppProp == NULL )
			return EResult_InvalidPtr;

		if ( !m_spUserProperties )
			return EResult_Fail;

		m_spUserProperties->AddRef();
		*out_ppProp = m_spUserProperties.GetPtr();
		return EResult_Success;
	}

	virtual EAbcResult SetTimeSampling( double in_dTimePerCycle, double in_dStartTime )
	{
		uint32_t l_tsIdx = GetInternalObject().getArchive().addTimeSampling( Alembic::AbcCoreAbstract::TimeSampling(in_dTimePerCycle, in_dStartTime) );
		GetSchema().setTimeSampling( l_tsIdx );
		return EResult_Success;
	}
protected:
	CAbcPtr<IAbcOCompoundProperty> m_spSchemaProperties;
	CAbcPtr<IAbcOCompoundProperty> m_spArbGeomParams;
	CAbcPtr<IAbcOCompoundProperty> m_spUserProperties;
};

#endif