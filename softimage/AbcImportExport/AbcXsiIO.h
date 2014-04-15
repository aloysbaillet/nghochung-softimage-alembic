//*****************************************************************************
/*!
	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*

#ifndef _AbcXsiIO_h_
#define _AbcXsiIO_h_

#include <xsi_ref.h>
#include <xsi_siobject.h>
#include <xsi_string.h>
#include <IAbcFramework.h>
#include <AbcFrameworkLoader.h>
#include <AbcFrameworkUtil.h>

//*****************************************************************************
/*! \class AbcXsiIO
	\brief Base class for importer and exporter class
*/
class AbcXsiIO
{
public:
	/*! Constructor
	*/
	AbcXsiIO();

	/*! Destructor
	*/
	virtual ~AbcXsiIO();

	/*! Return the IAbcFramework pointer
		\return The IAbcFramework pointer
	*/
	IAbcFramework* GetFrameworkPtr() { return m_spIFramework; }

protected:
	MODULE_HANDLE  m_pAbcDll;
	CAbcPtr<IAbcFramework> m_spIFramework;
};

#endif