//*****************************************************************************
/*!
	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*

#include "AbcFramework.h"
#include "AbcFrameworkMem.h"

#include "CAbcFramework.h"

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
#	ifdef TRACK_ALLOCATIONS
			BeginMemTracking();
#	endif
			CAbcFramework::Init();
		}
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		{
			CAbcFramework::Destroy();
#	ifdef TRACK_ALLOCATIONS
			EndMemTracking();
#	endif // _DEBUG
		}
		break;
	}
	return TRUE;
}
#else // WIN32

void __attribute__ ((constructor))	open_lib(void)
{
#	ifdef TRACK_ALLOCATIONS
	BeginMemTracking();
#	endif // _DEBUG
	CAbcFramework::Init();
}

void __attribute__ ((destructor))	close_lib(void)
{
	CAbcFramework::Destroy();
#	ifdef TRACK_ALLOCATIONS
	EndMemTracking();
#endif
}
#endif // WIN32