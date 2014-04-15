//*****************************************************************************
/*!
	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*

#ifndef CABCOARCHIVE_H
#define CABCOARCHIVE_H

#include "IAbcFramework.h"
#include "CAbcFramework.h"
#include "CAbcOObject.h"
#include "CRefCount.h"
#include <Alembic/Abc/OArchive.h>

class CAbcOArchive : public IAbcOArchive, protected CRefCount
{
	IMPL_REFCOUNT
public:							
	CAbcOArchive( const char* in_pszFileName, EAbcArchiveType in_eArchiveType );
	~CAbcOArchive();

	const char* GetName( ) const;

	EAbcResult AddTimeSampling( double in_dTimePerCycle, double in_dStartTime );

	EAbcResult FindObject( const char* in_pszName, IAbcOObject** out_ppOObject );

	Alembic::Abc::OArchive& GetAbcOArchive() { return m_OArchive;}

	EAbcResult GetTop( IAbcOObject** out_ppOObject )
	{ 
		m_spTopObject->AddRef();
		*out_ppOObject = m_spTopObject;
		return EResult_Success;
	}

private:
	Alembic::Abc::OArchive m_OArchive;
	std::string m_strName;
	CAbcPtr<CAbcOObject> m_spTopObject;
};

#endif