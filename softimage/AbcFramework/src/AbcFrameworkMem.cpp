//*****************************************************************************
/*!
	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*

#include "AbcFrameworkMem.h"

#ifdef TRACK_ALLOCATIONS
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#ifndef linux
#	include <windows.h>
#	define MEM_LOG(x) OutputDebugStringA((x))
#else
#	define MEM_LOG(x) printf((x));
#endif
size_t	g_TotalSize = 0;
size_t	g_PeakSize = 0;


size_t	g_NumBlocks = 0;
size_t	g_BlockCounter = 0;
char	g_szMsgBuf[512];

#ifdef MEM_TRACK_DEBUG
bool	g_bCheckMem = false;
#endif

#define MEM_MAGIC 0xABCD1234
struct SAllocBlock
{
#ifdef MEM_TRACK_DEBUG
	unsigned long m_ulMagic;
	bool	m_Checked;
	void*	m_pBlock;
	size_t	m_Count;
#endif
	size_t	m_Size;
	SAllocBlock* m_pPrev;
	SAllocBlock* m_pNext;
	SAllocBlock( void* in_pBlock, size_t in_Size ) : 
#ifdef MEM_TRACK_DEBUG
		m_ulMagic( MEM_MAGIC ), 
		m_Checked( g_bCheckMem ), 
		m_pBlock( in_pBlock ), 
		m_Count( g_BlockCounter++ ), 
#endif
		m_Size( in_Size ), 
		m_pPrev( 0 ), 
		m_pNext( 0 ) 
		{}

	bool Verify() const
	{
#ifdef MEM_TRACK_DEBUG
		return m_ulMagic == MEM_MAGIC;
#else
		return true;
#endif
	}
};

SAllocBlock* g_pFirstBlock = 0;
SAllocBlock* g_pLastBlock = 0;

void BeginMemTracking()
{
#ifdef MEM_TRACK_DEBUG
	g_bCheckMem = true;
#endif
}

void EndMemTracking()
{
	sprintf( g_szMsgBuf, "Alembic Peak Memory Usage: %f MB\n", (g_PeakSize / 1048576.0f) );
	MEM_LOG( g_szMsgBuf );

#ifdef MEM_TRACK_DEBUG
	MEM_LOG( "Checking for AbcFramework Leaks:\n" );
	if ( g_NumBlocks )
	{
		size_t l_CurCount = 1;
		for ( SAllocBlock* l_pCurBlock = g_pFirstBlock; l_pCurBlock != 0; l_pCurBlock = l_pCurBlock->m_pNext )
		{
			if ( l_pCurBlock->m_Checked )
			{
				sprintf( g_szMsgBuf, "\t%d) Block ID = %d, Size = %d, Address = %x\n", l_CurCount, l_pCurBlock->m_Count, l_pCurBlock->m_Size, l_pCurBlock->m_pBlock );
				MEM_LOG( g_szMsgBuf );
				++l_CurCount;
			}
		};
	}
#endif
}

void PushBlock( SAllocBlock* in_pBlock )
{
	assert( in_pBlock->Verify() );

	if ( !g_pFirstBlock )
	{
		g_pFirstBlock = in_pBlock;
	}

	if ( g_pLastBlock )
	{
		g_pLastBlock->m_pNext = in_pBlock;
		in_pBlock->m_pPrev = g_pLastBlock;
	}

	g_pLastBlock = in_pBlock;
	++g_NumBlocks;
	g_TotalSize += in_pBlock->m_Size;
	if ( g_TotalSize > g_PeakSize )
		g_PeakSize = g_TotalSize;
}

void RemoveBlock( SAllocBlock* in_pBlock )
{
	assert( in_pBlock->Verify() );

	SAllocBlock* l_pPrevBlock = in_pBlock->m_pPrev;
	SAllocBlock* l_pNextBlock = in_pBlock->m_pNext;

	if ( in_pBlock == g_pFirstBlock )
	{
		g_pFirstBlock = l_pNextBlock;
	}
	else if ( in_pBlock == g_pLastBlock )
	{
		g_pLastBlock = l_pPrevBlock;
	}

	if ( l_pPrevBlock)
	{
		assert( l_pPrevBlock->Verify() );
		l_pPrevBlock->m_pNext = l_pNextBlock;
	}

	if ( l_pNextBlock )
	{
		assert( l_pNextBlock->Verify() );
		l_pNextBlock->m_pPrev = l_pPrevBlock;
	}
	

	--g_NumBlocks;
	g_TotalSize -= in_pBlock->m_Size;
}

void* AllocateBlock( size_t in_Size )
{
	void* l_pBlock = malloc( sizeof(SAllocBlock) + in_Size );

	if ( l_pBlock )
	{
		SAllocBlock* l_pInfo = new(l_pBlock) SAllocBlock( (unsigned char*)l_pBlock + sizeof(SAllocBlock), in_Size );
		PushBlock( l_pInfo );
		return (unsigned char*)l_pBlock + sizeof(SAllocBlock);
	}
	return 0;
}

void FreeBlock( void* in_pPtr )
{
	SAllocBlock* l_pBlock = (SAllocBlock*)((unsigned char*)in_pPtr - sizeof(SAllocBlock));
	RemoveBlock( l_pBlock );
	free( l_pBlock );
}

void* operator new		( size_t in_Size )
{
	return AllocateBlock(in_Size);
}

void* operator new[]	( size_t in_Size )
{
	return AllocateBlock(in_Size);
}

void  operator delete	( void* in_pPtr )
{
	FreeBlock( in_pPtr );
}

void  operator delete[]	( void* in_pPtr )
{
	FreeBlock( in_pPtr );
}

#endif // TRACK_ALLOCATIONS