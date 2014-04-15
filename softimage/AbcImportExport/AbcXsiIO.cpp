//*****************************************************************************
/*!
	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*

#include "AbcXsiIO.h"
#include <xsi_application.h>
using namespace XSI;

AbcXsiIO::AbcXsiIO() : m_pAbcDll(0)
{
	m_pAbcDll = AbcFrameworkLoader::InitFramework();
	if ( m_pAbcDll == 0 )
	{
		CString l_csError = L"Unable to load the Alembic Framwork library";
		Application().LogMessage( l_csError, siErrorMsg );
#ifdef linux
		l_csError = L"Reason: ";
		l_csError += dlerror();
		Application().LogMessage( l_csError, siErrorMsg );
#endif
	}
	else
	{
		m_spIFramework = AbcFrameworkLoader::GetFramework( m_pAbcDll );
		if ( !m_spIFramework )
			Application().LogMessage( L"Unable to get an Alembic Framework Interface", siErrorMsg );
	}
}

AbcXsiIO::~AbcXsiIO()
{
	// Be sure to release first before unloading the DLL
	if ( m_spIFramework )
		m_spIFramework = NULL;
	if ( m_pAbcDll )
		AbcFrameworkLoader::CloseFramework( m_pAbcDll );
}
