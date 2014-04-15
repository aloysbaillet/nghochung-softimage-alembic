//*****************************************************************************
/*!
	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*

#include "CAbcInput.h"
#include "CAbcFramework.h"
#include <assert.h>

// Alembic Includes
#include <Alembic/Abc/ArchiveInfo.h>
#include <Alembic/AbcGeom/All.h>
#include <Alembic/AbcMaterial/All.h>
// This is required to tell Alembic which implementation to use.  In this case,
// the HDF5 implementation, currently the only one available.
#include <Alembic/AbcCoreHDF5/All.h>
#include <Alembic/AbcCoreFactory/IFactory.h>
#include <boost/xpressive/xpressive.hpp>

using namespace Alembic;
using namespace Alembic::Abc;
using namespace Alembic::AbcGeom;
using namespace Alembic::AbcMaterial;
using namespace Alembic::AbcCoreAbstract;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CAbcIArchve
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CAbcIArchive::CAbcIArchive( const char* in_szFilename )
{
	try
	{
		Alembic::AbcCoreFactory::IFactory l_factory;
		m_Archive = l_factory.getArchive( in_szFilename );
		GetArchiveInfo( m_Archive, m_strApplicationWriter, m_strAbcVersion, m_uiAbcApiVersion, m_strDateWritten, m_strUserDescription );
		if ( !m_Archive )
		{
			throw Alembic::Util::Exception( "Error: IArchive not constructed");
		}
		Alembic::Abc::GetArchiveStartAndEndTime( m_Archive, m_dStartTime, m_dEndTime );

		// Alembic files saved using versions prior to 1.1.3 does not support GetArchiveStartAndEndTime, for this case we will do it manually by walking the graph
		if ( m_dStartTime == DBL_MAX || m_dEndTime == -DBL_MAX )
		{
			IObject l_Top = m_Archive.getTop();
			GetArchiveStartAndEndTimeManually( l_Top );
		}
	}
	catch (std::exception* e)
	{
		throw e;
	}
}

CAbcIArchive::~CAbcIArchive()
{
	for ( TObjectMap::iterator it = m_mapObjects.begin(); it != m_mapObjects.end(); ++it )
	{
		it->second->Release();
	}
	CAbcFramework::GetInstance()->RemoveIArchiveFromMap( this );
}

EAbcResult CAbcIArchive::GetTop( IAbcIObject** out_ppObject ) const
{
	if ( !out_ppObject ) 
		return EResult_InvalidPtr;
	*out_ppObject = NULL;
	IObject l_obj( m_Archive, kTop );
	if ( l_obj.valid() )
		return CreateFromInternal( l_obj, out_ppObject );
	else
		return EResult_Fail;
}

const char* CAbcIArchive::GetName() const
{
	return m_Archive.getName().c_str();
}

unsigned int CAbcIArchive::GetNumTimeSampling()
{
	return m_Archive.getNumTimeSamplings();
}

EAbcResult CAbcIArchive::GetTimeSampling( unsigned int in_uiIndex, IAbcTimeSampling** out_ppSampling )
{
	return CAbcTimeSampling::CreateTimeSampling( *m_Archive.getTimeSampling( in_uiIndex ), out_ppSampling );
}

Alembic::Abc::IArchive* CAbcIArchive::GetInternalArchive()
{
	return &m_Archive;
}

EAbcResult CAbcIArchive::CreateSampleSelector( IAbcISampleSelector** out_ppSelector ) const
{
	CAbcISampleSelector* l_pNewSelector = new CAbcISampleSelector();
	if ( l_pNewSelector ) 
	{
		l_pNewSelector->AddRef();
		*out_ppSelector = l_pNewSelector;
		return EResult_Success;
	}
	return EResult_OutOfMemory;
}

EAbcResult CAbcIArchive::FindObject( const char* in_pszName, IAbcIObject** out_ppObject )
{
	using namespace boost::xpressive;

	TObjectMap::iterator it = m_mapObjects.find( in_pszName );

	if ( it != m_mapObjects.end() )
	{
		*out_ppObject = it->second;
		it->second->AddRef();
		return EResult_Success;
	}
	else
	{
		const sregex reName = (alpha | as_xpr('_') | as_xpr('-') | as_xpr(':')) >> (*( alnum | as_xpr('_') | as_xpr('-') | as_xpr(':')));
	
		const std::string l_sFullName( in_pszName );
		sregex_token_iterator l_curNameIt( l_sFullName.begin(), l_sFullName.end(), reName );
		sregex_token_iterator l_end;

		IObject l_curObj( m_Archive, kTop );

		for ( ; l_curNameIt != l_end; ++l_curNameIt )
		{
			const std::string l_curName( *l_curNameIt );

			IObject l_child = l_curObj.getChild( l_curName );
			if ( l_child.valid() )
				l_curObj = l_child;
			else
				return EResult_Fail;
		}

		if ( l_curObj.valid() )
		{
			EAbcResult l_result = CreateFromInternal( l_curObj, out_ppObject );
			if ( l_result == EResult_Success )
			{
				// Add this to our map
				m_mapObjects[in_pszName] = *out_ppObject;
				// AddRef again for our object map
				(*out_ppObject)->AddRef();
			}
			return l_result;
		}
	}
	return EResult_Fail;
}

void CAbcIArchive::GetArchiveStartAndEndTime( double* out_ppdStartTime, double* out_ppdEndTime ) const
{
	if ( out_ppdStartTime )
		*out_ppdStartTime = m_dStartTime;
	if ( out_ppdEndTime )
		*out_ppdEndTime = m_dEndTime;
}

unsigned int CAbcIArchive::GetAbcApiVersion() const
{
	return m_uiAbcApiVersion;
}
const char* CAbcIArchive::GetApplicationWriter() const
{
	return m_strApplicationWriter.c_str();
}

const char*	CAbcIArchive::GetAbcVersion() const
{
	return m_strAbcVersion.c_str();
}

const char*	CAbcIArchive::GetDateWritten() const
{
	return m_strDateWritten.c_str();
}

const char* CAbcIArchive::GetUserDescription() const
{
	return m_strUserDescription.c_str();
}

template<typename T>
void GetMinMaxTime( T& in_Schema, double& in_dMin, double& in_dMax )
{
	size_t l_NumSamples = in_Schema.getNumSamples();
	TimeSamplingPtr l_pTimeSampling = in_Schema.getTimeSampling();
		
	std::pair<index_t, chrono_t> l_Floor = l_pTimeSampling->getFloorIndex( 0, l_NumSamples );
	std::pair<index_t, chrono_t> l_Ceil = l_pTimeSampling->getCeilIndex( DBL_MAX - 1.0, l_NumSamples );
	in_dMin = std::min<double>( in_dMin, l_Floor.second );
	in_dMax = std::max<double>( in_dMax, l_Ceil.second );
}

void CAbcIArchive::GetArchiveStartAndEndTimeManually( Alembic::Abc::IObject in_Object )
{
	AbcA::MetaData l_md = in_Object.getMetaData();
	
	if ( IPolyMesh::matches( l_md ) )
	{
		IPolyMesh l_SchemaObj( in_Object, kWrapExisting );
		GetMinMaxTime( l_SchemaObj.getSchema(), m_dStartTime, m_dEndTime );		
	}
	else if ( ISubD::matches( l_md ) )
	{
		ISubD l_SchemaObj( in_Object, kWrapExisting );
		GetMinMaxTime( l_SchemaObj.getSchema(), m_dStartTime, m_dEndTime );		
	}
	else if ( IPoints::matches( l_md ) )
	{
		IPoints l_SchemaObj( in_Object, kWrapExisting );
		GetMinMaxTime( l_SchemaObj.getSchema(), m_dStartTime, m_dEndTime );		
	}
	else if ( ICamera::matches( l_md ) )
	{
		ICamera l_SchemaObj( in_Object, kWrapExisting );
		GetMinMaxTime( l_SchemaObj.getSchema(), m_dStartTime, m_dEndTime );		
	}
	else if ( IXform::matches( l_md ) )
	{
		IXform l_SchemaObj( in_Object, kWrapExisting );
		GetMinMaxTime( l_SchemaObj.getSchema(), m_dStartTime, m_dEndTime );		
	}

	for ( size_t i = 0; i < in_Object.getNumChildren(); i++ )
		GetArchiveStartAndEndTimeManually( in_Object.getChild( i ) );
}