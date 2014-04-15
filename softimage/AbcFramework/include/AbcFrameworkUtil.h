//*****************************************************************************
/*!
   \file AbcFrameworkUtil.h
   \brief Utility classes and functions used throughout the ABC Framework

	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*****************************************************************************

#ifndef ABCFRAMEWORK_UTIL_H
#define ABCFRAMEWORK_UTIL_H

#include "IAbcInput.h"

//*****************************************************************************
/*! \class CAbcPtr
	\brief A smart pointer class for ABC Framework objects
 */
//*****************************************************************************
template< typename T >
class CAbcPtr
{
public:
	CAbcPtr() : m_pPtr(0) {}
	
	CAbcPtr( T* in_pPtr ) : m_pPtr( in_pPtr )
	{
		AddRefPtr();
	}
	
	CAbcPtr( const CAbcPtr<T>& in_spPtr ) : m_pPtr( in_spPtr.m_pPtr )
	{
		AddRefPtr();
	}

	~CAbcPtr()
	{
		ReleasePtr();
	}

	CAbcPtr& operator = ( const CAbcPtr<T>& in_spOther )
	{
		ReleasePtr();
		m_pPtr = in_spOther.m_pPtr;
		AddRefPtr();
		return *this;
	}

	CAbcPtr& operator = ( T* in_pOther )
	{
		ReleasePtr();
		m_pPtr = in_pOther;
		AddRefPtr();
		return *this;
	}

	void SetPtr( T* in_pPtr )
	{
		ReleasePtr();
		m_pPtr = in_pPtr;
	}

	bool operator == ( const T* in_pOther )
	{
		return m_pPtr == in_pOther;
	}

	bool operator == ( const CAbcPtr<T>& in_spOther )
	{
		return m_pPtr == in_spOther.m_pPtr;
	}

	bool operator != ( const T* in_pOther )
	{
		return m_pPtr != in_pOther;
	}

	bool operator != ( const CAbcPtr<T>& in_spOther )
	{
		return m_pPtr != in_spOther.m_pPtr;
	}

	T* operator -> ()
	{
		return m_pPtr;
	}
	const T* operator -> () const
	{
		return m_pPtr;
	}

	T** operator& ()
	{
		return &m_pPtr;
	}

	T* GetPtr()
	{
		return m_pPtr;
	}

	const T* GetPtr() const
	{
		return m_pPtr;
	}

	operator const T*() const
	{
		return m_pPtr;
	}

	operator T*()
	{
		return m_pPtr;
	}

private:
	inline void AddRefPtr()
	{
		if ( m_pPtr )
			m_pPtr->AddRef();
	}
	inline void ReleasePtr()
	{
		if ( m_pPtr )
			m_pPtr->Release();
	}
	T* m_pPtr;
};


//*****************************************************************************
/*! \class CAbcAdapt
	\brief Class to adapt CAbcPtr for use with STL containers because CAbcPtr overrides the address-of operator

	\eg Syntax
	\code
		std::vector< CAbcAdapt < CAbcPtr<T> > > > vect;
	\endcode

 */
//*****************************************************************************
template <class T>
class CAbcAdapt
{
public:
    CAbcAdapt()
    {
    }
    CAbcAdapt(const T& rSrc) :
        m_T( rSrc )
    {
    }	
    CAbcAdapt(const CAbcAdapt& rSrCA) :
        m_T( rSrCA.m_T )
    {	
    }	
    CAbcAdapt& operator=(const T& rSrc)
    {		
        m_T = rSrc;

        return *this;
    }	
    CAbcAdapt& operator=(const CAbcAdapt<T>& rSrc)
    {		
        if (this != &rSrc)
        {
            m_T = rSrc.m_T;			
        }		
        return *this;
    }	
    bool operator<(const T& rSrc) const
    {
        return m_T < rSrc;
    }
    bool operator==(const T& rSrc) const
    {
        return m_T == rSrc;
    }
    operator T&()
    {
        return m_T;
    }

    operator const T&() const
    {
        return m_T;
    }

    T& operator->()
    {
        return m_T;
    }

    const T& operator->() const
    {
        return m_T;
    }

    T m_T;
};

template <typename T, int TRAITS>
class CAbcTypedSampleReader
{
public:
	CAbcTypedSampleReader() : m_bValid( false )
	{
	}

	CAbcTypedSampleReader( IAbcISampleSelector* in_pSampleSelector, IAbcIArrayPropertyAccessor* in_spAccessor )
	{
		if ( in_spAccessor->GetSample( in_pSampleSelector, &m_spSample ) == EResult_Success )
			m_bValid = Init();
		else
			m_bValid = false;
	}

	CAbcTypedSampleReader( IAbcSampleBuffer* in_pSampleBuffer ) : m_spSample( in_pSampleBuffer )
	{
		m_bValid = Init();
	}

	inline const T& operator [] ( size_t index ) const
	{
		return m_pBuffer[index];
	}

	inline size_t GetNumElements() const 
	{
		return m_spSample->GetNumElements();
	}

	inline bool IsValid() const
	{
		return m_bValid;
	}
private:
	bool Init()
	{
		if ( ( (int)m_spSample->GetDataType().m_eTraits == TRAITS ) && ( sizeof(T) == m_spSample->GetDataType().m_numBytes ) )
		{
			m_pBuffer = (const T*)m_spSample->GetBuffer();
			return true;
		}
		return false;
	}
	CAbcPtr<IAbcSampleBuffer> m_spSample;
	const T* m_pBuffer;
	bool m_bValid;
};

// Helper class to handle indexed compound props
template <typename T, int TRAITS>
class CAbcIndexedPropReader
{
public:
	typedef CAbcTypedSampleReader<T, TRAITS> TValueReader;
	typedef CAbcTypedSampleReader<unsigned int, EDataTraits_UInt32> TIndexReader;
	CAbcIndexedPropReader( IAbcISampleSelector* in_pSampleSelector, IAbcICompoundPropertyAccessor* in_pAccessor ) : m_spCompoundProp( in_pAccessor ), m_spSampleSelector( in_pSampleSelector )
	{
		m_bValid = InitializeProps();
	}
	~CAbcIndexedPropReader()
	{
	}

	inline const T& operator [] ( size_t index ) const
	{
		size_t valIndex = (size_t)m_IndexBuffer[index];
		return m_ValueBuffer[valIndex];
	}

	inline size_t GetNumElements() const
	{
		return m_IndexBuffer.GetNumElements();
	}

	inline void GetFlatenned( size_t in_OutputSize, T* out_pBuffer ) const
	{
		size_t l_SizeToCopy = in_OutputSize < GetNumElements() ? in_OutputSize : GetNumElements();
		for ( size_t i = 0; i < l_SizeToCopy; i++ )
			out_pBuffer[i] = operator[](i);
	}

	inline bool IsValid() const
	{
		return m_bValid;
	}
	
	inline const TIndexReader& GetIndices() const
	{
		return m_IndexBuffer;
	}
	
	inline const TValueReader& GetValues() const
	{
		return m_ValueBuffer;
	}
private:
	bool InitializeProps()
	{
		CAbcPtr<IAbcIPropertyAccessor> l_spIndices;
		if ( m_spCompoundProp->GetChildProperty( ".indices", &l_spIndices ) != EResult_Success ||
			 l_spIndices->GetPropertyType() != EPropertyType_Array || 
			 l_spIndices->GetDataType().m_eTraits != EDataTraits_UInt32 )
			 return false;

		CAbcPtr<IAbcIPropertyAccessor> l_spVals;
		if ( m_spCompoundProp->GetChildProperty( ".vals", &l_spVals ) != EResult_Success ||
			 l_spVals->GetPropertyType() != EPropertyType_Array ||
			 l_spVals->GetDataType().m_eTraits != (EAbcDataTraits)TRAITS )
			 return false;

		CAbcPtr<IAbcSampleBuffer> l_spIndicesSample;
		if ( ((IAbcIArrayPropertyAccessor*)l_spIndices.GetPtr())->GetSample( m_spSampleSelector, &l_spIndicesSample ) != EResult_Success )
			return false;

		CAbcPtr<IAbcSampleBuffer> l_spValsSample;
		if ( ((IAbcIArrayPropertyAccessor*)l_spVals.GetPtr())->GetSample( m_spSampleSelector, &l_spValsSample ) != EResult_Success )
			return false;

		m_IndexBuffer = TIndexReader( l_spIndicesSample );
		m_ValueBuffer = TValueReader( l_spValsSample );

		return m_IndexBuffer.IsValid() && m_ValueBuffer.IsValid();
	}

	CAbcPtr<IAbcICompoundPropertyAccessor> m_spCompoundProp;
	CAbcPtr<IAbcISampleSelector> m_spSampleSelector;
	TIndexReader m_IndexBuffer;	
	TValueReader m_ValueBuffer;
	bool m_bValid;
};

namespace AbcLerp
{
	/*! Does a linear interpolation between two values
	\param from Value to interpolate from
	\param to Value to interpolate to
	\param f The interpolation value
	\return The value between from and to at time f
	*/
	template<typename T>
	T InterpolateSingle(T from, T to, float f)
	{
		return (T)( from + ( to - from ) * f );
	}

	/*! Does a linear interpolation between values of two arrays
	\param dest Pre-allocated destination array
	\param from Array to interpolate from
	\param to Array to interpolate to
	\param f The interpolation value
	\param num The number of elements in the array
	*/
	template<typename T>
	void InterpolateArray(T* dest, const T* from, const T* to, float f, size_t num)
	{
		for (size_t i = 0; i < num; i++)
			dest[i] = InterpolateSingle<T>( from[i], to[i], f );
	}
}

#endif