//*****************************************************************************
/*!
   \file AbcFrameworkLoader.h
   \brief Helper classes to load the ABC Framework Library on Windows and Linux

	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*****************************************************************************

#ifndef ABCFRAMEWORK_LOADER_H
#define ABCFRAMEWORK_LOADER_H

#include "AbcFramework.h"
#if !defined( linux )
// Windows
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#	define MODULE_HANDLE	HMODULE
#	define LOAD_MODULE		LoadLibraryA
#	define LINK_FUNCTION	GetProcAddress
#	define CLOSE_MODULE		FreeLibrary
#	define LIB_NAME			"AbcFramework.dll"
#else
// Linux
#	include <dlfcn.h>
#	define MODULE_HANDLE	void*
#	define LOAD_MODULE(x)	dlopen((x), RTLD_LAZY)
#	define LINK_FUNCTION	dlsym
#	define CLOSE_MODULE		dlclose
#	define LIB_NAME			"libAbcFramework.so"
#endif // WIN32

//*****************************************************************************
/*! \class AbcFrameworkLoader
	\brief Helper class to load the framework dynamic library and get the proper interface

	\eg Using AbcFrameworkLoader
	\code
		// Load the dynamic library
		MODULE handle = AbcFrameworkLoader::InitFramework();

		// Get the interface pointer
		IAbcFramework* pFramework = AbcFrameworkLoader::GetFramework( handle );

		// Do stuff with the framework
		...
		
		// Cleanup
		pFramework->Release();
		AbcFrameworkLoader::CloseFramework( handle );
	\endcode

 */
//*****************************************************************************
class AbcFrameworkLoader
{
public:
	/*! Loads the ABC Framework library 
	\return A non-zero platform specific handle to the loeaded library on success, zero otherwise
	*/
	static MODULE_HANDLE InitFramework()
	{
		MODULE_HANDLE l_hHandle = LOAD_MODULE( LIB_NAME );
		return l_hHandle;
	}
	
	/*! Unloads the ABC Framework Library
	\param in_hHandle Handle returned by InitFramework()
	*/
	static void CloseFramework( MODULE_HANDLE in_hHandle )
	{
		if ( in_hHandle )
		{
			CLOSE_MODULE( in_hHandle );
		}
	}

	/*! Gets the ABC Framework interface, must be called after InitFramework()
	\param in_hHandle Handle returned by InitFramework()
	\return A pointer to the ABC Framework interface
	*/
	static IAbcFramework* GetFramework( MODULE_HANDLE in_hHandle )
	{
		if ( in_hHandle )
		{
			TPFNGETFRAMEWORKINTERFACE l_pfnGetFrameworkInterface = (TPFNGETFRAMEWORKINTERFACE)LINK_FUNCTION( in_hHandle, "GetFrameworkInterface" );
			if ( l_pfnGetFrameworkInterface )
			{
				IAbcFramework* l_pFwk = (IAbcFramework*)l_pfnGetFrameworkInterface();
				return l_pFwk;
			}
		}
		return 0;
	}
private:
	typedef IAbcFramework* (*TPFNGETFRAMEWORKINTERFACE)();
};

#endif // ABCFRAMEWORKLOADER_H
