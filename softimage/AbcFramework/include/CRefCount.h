//*****************************************************************************
/*!
	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*

#ifndef REFCOUNT_H
#define REFCOUNT_H

#include <assert.h>
class CRefCount
{
public:
	CRefCount() : m_iRefCount(0) {}
	virtual ~CRefCount()
	{
		assert( m_iRefCount == 0 );
	}
	void AddRef()
	{
		m_iRefCount++;
	}
	void Release()
	{
		if (--m_iRefCount == 0)
		{
			delete this;			
		}
	}
	int GetRefCount() const
	{
		return m_iRefCount;
	}
protected:
	int m_iRefCount;
};

#define IMPL_REFCOUNT				\
	public:							\
		void AddRef()				\
		{							\
			CRefCount::AddRef();	\
		}							\
		void Release()				\
		{							\
			CRefCount::Release();	\
		}							\
		int GetRefCount() const		\
		{							\
			return CRefCount::GetRefCount();\
		}

#endif // REFCOUNT_H