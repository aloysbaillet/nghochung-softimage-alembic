//*****************************************************************************
/*!
   \file AbcFrameworkDll.h
   \brief Defines the calling standard for library functions

	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*****************************************************************************


#ifndef ABCFRAMEWORKDLL_H
#define ABCFRAMEWORKDLL_H
#ifdef WIN32
#	ifdef ABCFRAMEWORK_EXPORTS
#		define ABCFRAMEWORK_API __declspec( dllexport )
#	else
#		define ABCFRAMEWORK_API __declspec( dllimport )
#	endif // ALEMBICFWK_EXPORTS
#else
#		define ABCFRAMEWORK_API extern "C"
#endif // WIN32

#endif // ABCFRAMEWORK_H
