//*****************************************************************************
/*!
   \file AbcFrameworkMem.h
   \brief Utility functions for tracking memory allocations within the ABC Framework

	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*****************************************************************************
#ifndef ABCFRAMEWORKMEM_H
#define ABCFRAMEWORKMEM_H
#ifdef _DEBUG
#	define 	TRACK_ALLOCATIONS
#	define	MEM_TRACK_DEBUG
#endif // DEBUG

#ifdef TRACK_ALLOCATIONS
#include <new>

// Global allocation overloads
void* operator new		( size_t in_Size );
void* operator new[]	( size_t in_Size );
void  operator delete	( void* in_pPtr );
void  operator delete[]	( void* in_pPtr );

/*! Begins memory tracking */
void BeginMemTracking();

/*! Ends memory tracking and shows a list of unfreed blocks since BeginMemTracking was called */
void EndMemTracking();

#endif // TRACK_ALLOCATIONS

#endif // ABCFRAMEWORKMEM_H