//*****************************************************************************
/*!
	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*

#include "AbcFramework.h"
#include "CAbcFramework.h"
#include "CAbcInput.h"
#include "CAbcOArchive.h"

// Alembic Includes
#include <Alembic/AbcGeom/All.h>
// This is required to tell Alembic which implementation to use.  In this case,
// the HDF5 implementation, currently the only one available.
#include <Alembic/AbcCoreHDF5/All.h>
#include <boost/filesystem/operations.hpp>
#include <exception>

CAbcPtr<CAbcFramework> CAbcFramework::ms_pInstance = NULL;

IAbcOObject::~IAbcOObject()
{

}

IAbcOArchive::~IAbcOArchive()
{

}


ABCFRAMEWORK_API IAbcFramework* GetFrameworkInterface()
{
	IAbcFramework* l_pInterface = CAbcFramework::GetInstance();
	return l_pInterface;
}

CAbcFramework::CAbcFramework()
{
}

CAbcFramework::CAbcFramework( const CAbcFramework& in_other )
{

}

const char* CAbcFramework::GetFrameworkVersionString() const
{
    return "1.0";
}

const char* CAbcFramework::GetAlembicVersionString() const
{
	static std::string l_strLibVer;
	if ( l_strLibVer.length() == 0 )
		l_strLibVer = Alembic::AbcCoreAbstract::GetLibraryVersion();
    return l_strLibVer.c_str();
}

EAbcResult CAbcFramework::OpenOArchive( const char* in_pszFileName, IAbcOArchive** out_ppArchive, EAbcArchiveType in_eArchiveType )
{
	if (out_ppArchive == NULL)
		return EResult_InvalidPtr;

	try
	{
		// Use canonical to "normalize" the file path, e.g. resolving double backslash or symbolic link
		boost::filesystem::path l_path( in_pszFileName );

		OArchiveMap::iterator it = m_OArchiveMap.end();

		if ( boost::filesystem::exists( l_path ) )
		{
			l_path = boost::filesystem::canonical( l_path );
			it = m_OArchiveMap.find( l_path );
		}

		if ( it == m_OArchiveMap.end() )
		{
			if ( in_eArchiveType == EArchiveType_Any )
				return EResult_Fail;

			std::string l_strFileName( l_path.string() );
			CAbcPtr<CAbcOArchive> l_spOArchive = new CAbcOArchive( l_strFileName.c_str(), in_eArchiveType );

			if ( l_spOArchive )
			{
				*out_ppArchive = l_spOArchive;
				l_spOArchive->AddRef();

				// normalize the file name before using it as map key
				l_path = boost::filesystem::canonical( l_path );

				m_OArchiveMap[ l_path ] = l_spOArchive;
				return EResult_Success;
			}

			return EResult_Fail;
		}
		else
		{
			it->second->AddRef();
			*out_ppArchive = it->second;
			return EResult_Success;
		}
	}
	catch (std::exception&)
	{
		return EResult_Fail;
	}
}


EAbcResult CAbcFramework::OpenIArchive( const char* in_pszFileName, IAbcIArchive** out_ppArchive )
{
	if (out_ppArchive == NULL)
		return EResult_InvalidPtr;

	boost::filesystem::path l_path( in_pszFileName );
	IArchiveMap::iterator it = m_IArchiveMap.end();
	bool l_bExists = boost::filesystem::exists( l_path );
	if ( l_bExists )
	{
		it = m_IArchiveMap.find( l_path );
	}
	
	CAbcIArchive* l_pIArchive = NULL;
	
	std::time_t l_timeLastWrite;
	if ( l_bExists )
		l_timeLastWrite = boost::filesystem::last_write_time( l_path );

	if ( it != m_IArchiveMap.end() )
	{
		// Has the cached file changed?
		bool l_bChanged = !l_bExists || l_timeLastWrite != it->second.m_timeLastWrite;

		// Update our map if the file has changed
		if ( l_bChanged )
		{
			m_IArchiveMap.erase(it);
			it = m_IArchiveMap.end();
			if ( !l_bExists )
				return EResult_Fail;
		}
		else
		{
			l_pIArchive = (CAbcIArchive*)it->second.m_pArchive;
		}
	}

	if ( it == m_IArchiveMap.end() )
	{
		try
		{
			std::string l_strFileName = l_path.string();
			l_pIArchive = new CAbcIArchive( l_strFileName.c_str() );
		}
		catch ( std::exception& )
		{
			return EResult_Fail;
		}

		// Add this new entry to our map
		m_IArchiveMap[l_path] = SAbcIArchiveInfo( l_pIArchive, l_timeLastWrite );
	}

	if ( l_pIArchive )
	{
		*out_ppArchive = l_pIArchive;
		l_pIArchive->AddRef();
		return EResult_Success;
	}
	return EResult_Fail;
}

EAbcResult CAbcFramework::RemoveOArchiveFromMap( IAbcOArchive* in_pArchive )
{
	OArchiveMap::iterator it = m_OArchiveMap.find( in_pArchive->GetName() );

	if ( it == m_OArchiveMap.end() )
	{
		return EResult_Fail;
	}
	else
	{
		m_OArchiveMap.erase( it );
		return EResult_Success;
	}
}

EAbcResult CAbcFramework::RemoveIArchiveFromMap( IAbcIArchive* in_pArchive )
{
	for ( IArchiveMap::iterator it = m_IArchiveMap.begin(); it != m_IArchiveMap.end(); ++it )
	{
		if ( it->second.m_pArchive == in_pArchive )
		{
			m_IArchiveMap.erase( it );
			return EResult_Success;
		}
	}
	return EResult_Fail;
}

EAbcResult CAbcFramework::Init()
{
	if ( ms_pInstance == NULL )
	{
		ms_pInstance = new CAbcFramework();
	}

	return EResult_Success;
}

EAbcResult CAbcFramework::Destroy()
{
	if ( ms_pInstance != NULL )
	{
		ms_pInstance = NULL;
	}

	return EResult_Success;
}

CAbcFramework::~CAbcFramework()
{
}

const IAbcUtils& CAbcFramework::GetUtils() const
{
	return m_Utils;
}

CAbcFramework::SAbcIArchiveInfo::SAbcIArchiveInfo( class IAbcIArchive* in_pArchive, const std::time_t& in_time ) : m_pArchive( in_pArchive ), m_timeLastWrite( in_time )
{
}

CAbcFramework::SAbcIArchiveInfo::~SAbcIArchiveInfo()
{
}
