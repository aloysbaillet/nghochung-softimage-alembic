//*****************************************************************************
/*!
	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*

#ifndef CABCFRAMEWORK_H
#define CABCFRAMEWORK_H

#include "AbcFrameworkUtil.h"
#include "CRefCount.h"
#include "IAbcFramework.h"
#include "CAbcUtils.h"
#include <map>
#include <boost/filesystem/path.hpp>
#include <boost/interprocess/smart_ptr/unique_ptr.hpp>

class CAbcFramework : public IAbcFramework, private CRefCount
{
	IMPL_REFCOUNT
public:

	const char*		GetFrameworkVersionString() const;
	const char*		GetAlembicVersionString() const;
	EAbcResult		OpenIArchive( const char* in_pszFileName, IAbcIArchive** out_ppArchive );

	EAbcResult		OpenOArchive( const char* in_pszFileName, IAbcOArchive** out_ppArchive, EAbcArchiveType in_eArchiveType );
	EAbcResult		RemoveOArchiveFromMap( IAbcOArchive* in_pArchive );
	EAbcResult		RemoveIArchiveFromMap( IAbcIArchive* in_pArchive );
	const IAbcUtils& GetUtils() const;
	static CAbcFramework*	GetInstance() { return ms_pInstance;}
	static EAbcResult		Init();
	static EAbcResult		Destroy();

private:
	static CAbcPtr<CAbcFramework> ms_pInstance;

	CAbcFramework();
	CAbcFramework( const CAbcFramework& in_other );
	~CAbcFramework();

	// OArchive map keeps a raw pointer to IAbcOArchive because it doesn't own the archive
	// IAbcOArchive is owned by the caller of OpenOArchive()
	typedef std::map< boost::filesystem::path, IAbcOArchive* > OArchiveMap;
	OArchiveMap m_OArchiveMap;

	struct SAbcIArchiveInfo
	{
		class IAbcIArchive* m_pArchive;
		std::time_t m_timeLastWrite;
		SAbcIArchiveInfo() : m_pArchive(0) {}
		SAbcIArchiveInfo( class IAbcIArchive* in_pArchive, const std::time_t& in_time );
		~SAbcIArchiveInfo();
	};

	typedef std::map< boost::filesystem::path, SAbcIArchiveInfo > IArchiveMap;
	IArchiveMap m_IArchiveMap;
	CAbcUtils m_Utils;
};


#endif // ABCFRAMEWORKINTERFACE_H
