//*****************************************************************************
/*!
	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*

#include "CAbcOArchive.h"
#include "CAbcOPolyMesh.h"
#include "CAbcOObjectFactory.h"
#include <Alembic/AbcCoreHDF5/ReadWrite.h>
#include <Alembic/AbcCoreOgawa/ReadWrite.h>
#include <boost/xpressive/xpressive.hpp>

CAbcOArchive::CAbcOArchive( const char* in_pszFileName, EAbcArchiveType in_eArchiveType )
{
	if ( in_eArchiveType == EArchiveType_Ogawa  )
	{
		m_OArchive = Alembic::Abc::OArchive( Alembic::AbcCoreOgawa::WriteArchive(), in_pszFileName );
	}
	else
	{
		m_OArchive = Alembic::Abc::OArchive( Alembic::AbcCoreHDF5::WriteArchive(), in_pszFileName );
	}

	m_spTopObject = new CAbcOObject( m_OArchive );
	m_strName = m_OArchive.getName();
}

CAbcOArchive::~CAbcOArchive()
{
	CAbcFramework::GetInstance()->RemoveOArchiveFromMap( this );
	m_spTopObject = NULL;
}

EAbcResult CAbcOArchive::FindObject( const char* in_pszName, IAbcOObject** out_ppOObject )
{
	using namespace boost::xpressive;
	const sregex reName = (alpha | as_xpr('_') | as_xpr('-')) >> (*( alnum | as_xpr('_') | as_xpr('-') ));
	//const sregex reFullName = reName >> (*(as_xpr('/') >> reName));
	
	const std::string l_sFullName( in_pszName );
	sregex_token_iterator l_curNameIt( l_sFullName.begin(), l_sFullName.end(), reName );
	sregex_token_iterator l_end;

	CAbcPtr<IAbcOObject> l_curObj = static_cast<IAbcOObject*>( m_spTopObject.GetPtr() );
	CAbcPtr<IAbcOObject> l_curChild;

	for ( ; l_curNameIt != l_end; ++l_curNameIt )
	{
		const std::string l_curName( *l_curNameIt );

		// important: curChild must be released
		// when we go one level deeper
		l_curChild = NULL;

		if ( l_curObj->GetChild( &l_curChild, l_curName.c_str() ) == EResult_Success )
		{
			l_curObj = l_curChild;

		}
		else
		{
			return EResult_Fail;
		}
	}

	if ( l_curChild != NULL)
	{
		l_curChild->AddRef();
		*out_ppOObject = l_curChild.GetPtr();
		return EResult_Success;
	}

	return EResult_Fail;
}

const char* CAbcOArchive::GetName( ) const
{
	return m_strName.c_str();
}

EAbcResult CAbcOArchive::AddTimeSampling( double in_dTimePerCycle, double in_dStartTime )
{
	m_OArchive.addTimeSampling( Alembic::AbcCoreAbstract::TimeSampling( in_dTimePerCycle, in_dStartTime ) );
	return EResult_Success;
}
