//*****************************************************************************
/*!
	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*

#include "AbcFrameworkUtil.h"
#include "CAbcUtils.h"
#include <math.h>
#include <string>
#include "IAbcInput.h"
#include "IAbcOProperty.h"
#include "CAbcIPropertyAccessor.h"

using namespace Alembic::Util;

const double g_dPi = 3.14159265358979323846;

template< class DataElemT >
void ReverseFaceWindingImpl( DataElemT* io_pData, const Alembic::Abc::int32_t* in_faceVertexCount, size_t in_szNbFaces )
{
	const Alembic::Abc::int32_t* l_pVertexCount = in_faceVertexCount;
	DataElemT* l_pElem = (DataElemT*)io_pData;

	for (Alembic::Abc::int32_t i = (Alembic::Abc::int32_t)in_szNbFaces; i>0; --i, ++l_pVertexCount)
	{
		const Alembic::Abc::int32_t l_lCount = *l_pVertexCount;

		if ( l_lCount == 4 )
		{
			std::swap( *l_pElem, *(l_pElem + 3) );
			std::swap( *(l_pElem + 1), *(l_pElem + 2) );
			l_pElem += 4;
		}
		else if ( l_lCount == 3 )
		{
			std::swap( *l_pElem, *(l_pElem + 2) );
			l_pElem += 3;
		}
		else
		{
			DataElemT* l_lLeft = l_pElem;
			DataElemT* l_lRight = l_pElem + l_lCount - 1;

			for (Alembic::Abc::int32_t j=l_lCount/2; j>0; --j, ++l_lLeft, --l_lRight)
			{
				std::swap( *l_lLeft, *l_lRight );
			}

			l_pElem += l_lCount;
		}
	}
}

double CAbcUtils::RadiansToDegrees( double in_dAngle ) const
{
	return in_dAngle * 180.0 / g_dPi;
}

double CAbcUtils::DegreesToRadians( double in_dAngle ) const
{
	return in_dAngle * g_dPi / 180.0;
}

double CAbcUtils::FocalLengthFromFov( double in_dFov, double in_dAperture ) const
{
	return in_dAperture / ( 2.0 * tan( DegreesToRadians( in_dFov ) / 2.0 ) ) * 10.0;
}

const char* CAbcUtils::GetStringFromSample( size_t in_Index, const IAbcSampleBuffer* in_pBuffer ) const
{
	if ( in_pBuffer->GetDataType().m_eTraits != EDataTraits_StringPtr || in_Index >= in_pBuffer->GetNumElements() )
		return NULL;
	return ((std::string*)in_pBuffer->GetBuffer())[in_Index].c_str();
}

const wchar_t* CAbcUtils::GetWideStringFromSample( size_t in_Index, const IAbcSampleBuffer* in_pBuffer ) const
{
	if ( in_pBuffer->GetDataType().m_eTraits != EDataTraits_WStringPtr || in_Index >= in_pBuffer->GetNumElements() )
		return NULL;
	return ((std::wstring*)in_pBuffer->GetBuffer())[in_Index].c_str();
}

void CAbcUtils::GetTotalStringSize( const class IAbcSampleBuffer* in_pBuffer, 
								   Alembic::Util::uint32_t in_startIndex, Alembic::Util::uint32_t in_endIndex, 
								   Alembic::Util::uint32_t& out_TotalSize, Alembic::Util::uint32_t* out_pPerStringSize ) const
{
	if ( in_pBuffer->GetDataType().m_eTraits == EDataTraits_StringPtr )
	{
		for ( size_t i = in_startIndex; i < in_endIndex; ++ i )
		{
			size_t len = ((std::string*)in_pBuffer->GetBuffer())[i].length();
			out_pPerStringSize[ i - in_startIndex ] = (Alembic::Util::uint32_t)len;
			out_TotalSize += (Alembic::Util::uint32_t)len;
		}
	}
	else if ( in_pBuffer->GetDataType().m_eTraits == EDataTraits_WStringPtr )
	{
		for ( size_t i = in_startIndex; i < in_endIndex; ++ i )
		{
			size_t len = ((std::string*)in_pBuffer->GetBuffer())[i].length();
			out_pPerStringSize[ i - in_startIndex ] = (Alembic::Util::uint32_t)len;
			out_TotalSize += (Alembic::Util::uint32_t)len;
		}
	}
}

class CAbcRawBufferSampleBuffer : public IAbcSampleBuffer, public CRefCount
{
	IMPL_REFCOUNT;
public:
	CAbcRawBufferSampleBuffer( const IAbcSampleBuffer* in_pSrc, void* in_pBuffer, size_t in_NumElements ) 
	{
		m_DataType = in_pSrc->GetDataType();
		m_GeomScope = in_pSrc->GetGeomScope();
		m_bIndexed = in_pSrc->IsIndexed();
		m_Size = in_NumElements;
		m_pBuffer = in_pBuffer;
	}
	~CAbcRawBufferSampleBuffer()
	{
		free( m_pBuffer );
	}

	const void*			GetBuffer() const
	{
		return m_pBuffer;
	}
	size_t				GetNumElements() const
	{
		return m_Size;
	}
	const SAbcDataType& GetDataType() const
	{
		return m_DataType;
	}
	EAbcGeomScope GetGeomScope() const
	{
		return m_GeomScope;
	}
	bool IsIndexed() const
	{
		return m_bIndexed;
	}

	// Interface for writing
	void* GetBuffer()
	{
		return m_pBuffer;
	}
private:
	CAbcRawBufferSampleBuffer( const CAbcRawBufferSampleBuffer& other ) {}
	size_t m_Size;
	void* m_pBuffer;
	bool m_bIndexed;
	SAbcDataType m_DataType;
	EAbcGeomScope m_GeomScope;
};

EAbcResult CAbcUtils::GetInterpolatedBuffer( const IAbcSampleBuffer* in_pBufferFrom, const IAbcSampleBuffer* in_pBufferTo, IAbcSampleBuffer** out_ppBufferDest, float in_fAlpha ) const
{
	if ( in_pBufferFrom == 0 || in_pBufferTo == 0 || out_ppBufferDest == 0 )
		return EResult_InvalidPtr;

	if ( in_pBufferFrom->GetDataType() == in_pBufferTo->GetDataType() &&
		 in_pBufferFrom->GetGeomScope() == in_pBufferTo->GetGeomScope() &&
		 in_pBufferFrom->IsIndexed() == in_pBufferTo->IsIndexed() )
	{
		size_t l_numElems = std::min<size_t>( in_pBufferFrom->GetNumElements(), in_pBufferTo->GetNumElements() );
		void* l_pArray = malloc( in_pBufferFrom->GetDataType().m_numBytes * l_numElems );
		
		if( in_pBufferFrom->GetDataType().m_eType == EPodType_Float32 )
		{
			Alembic::Util::float32_t* l_pDest = (Alembic::Util::float32_t *)l_pArray;
			AbcLerp::InterpolateArray<Alembic::Util::float32_t>( l_pDest, (const Alembic::Util::float32_t*)in_pBufferFrom->GetBuffer(), (const Alembic::Util::float32_t*)in_pBufferTo->GetBuffer(), in_fAlpha, l_numElems * in_pBufferFrom->GetDataType().m_ucExtent );
		}
		else if ( in_pBufferFrom->GetDataType().m_eType == EPodType_Float64 )
		{
			Alembic::Util::float64_t* l_pDest = (Alembic::Util::float64_t *)l_pArray;
			AbcLerp::InterpolateArray<Alembic::Util::float64_t>( l_pDest, (const Alembic::Util::float64_t*)in_pBufferFrom->GetBuffer(), (const Alembic::Util::float64_t*)in_pBufferTo->GetBuffer(), in_fAlpha, l_numElems * in_pBufferFrom->GetDataType().m_ucExtent );
		}
		else if ( in_pBufferFrom->GetDataType().m_eType == EPodType_UInt8 )
		{
			Alembic::Util::uint8_t* l_pDest = (Alembic::Util::uint8_t *)l_pArray;
			AbcLerp::InterpolateArray<Alembic::Util::uint8_t>( l_pDest, (const Alembic::Util::uint8_t*)in_pBufferFrom->GetBuffer(), (const Alembic::Util::uint8_t*)in_pBufferTo->GetBuffer(), in_fAlpha, l_numElems * in_pBufferFrom->GetDataType().m_ucExtent );
		}
		else if ( in_pBufferFrom->GetDataType().m_eType == EPodType_UInt16 )
		{
			Alembic::Util::uint16_t* l_pDest = (Alembic::Util::uint16_t *)l_pArray;
			AbcLerp::InterpolateArray<Alembic::Util::uint16_t>( l_pDest, (const Alembic::Util::uint16_t*)in_pBufferFrom->GetBuffer(), (const Alembic::Util::uint16_t*)in_pBufferTo->GetBuffer(), in_fAlpha, l_numElems * in_pBufferFrom->GetDataType().m_ucExtent );
		}
		else if ( in_pBufferFrom->GetDataType().m_eType == EPodType_UInt32 )
		{
			Alembic::Util::uint32_t* l_pDest = (Alembic::Util::uint32_t *)l_pArray;
			AbcLerp::InterpolateArray<Alembic::Util::uint32_t>( l_pDest, (const Alembic::Util::uint32_t*)in_pBufferFrom->GetBuffer(), (const Alembic::Util::uint32_t*)in_pBufferTo->GetBuffer(), in_fAlpha, l_numElems * in_pBufferFrom->GetDataType().m_ucExtent );
		}
		else if ( in_pBufferFrom->GetDataType().m_eType == EPodType_UInt64 )
		{
			Alembic::Util::uint64_t* l_pDest = (Alembic::Util::uint64_t *)l_pArray;
			AbcLerp::InterpolateArray<Alembic::Util::uint64_t>( l_pDest, (const Alembic::Util::uint64_t*)in_pBufferFrom->GetBuffer(), (const Alembic::Util::uint64_t*)in_pBufferTo->GetBuffer(), in_fAlpha, l_numElems * in_pBufferFrom->GetDataType().m_ucExtent );
		}
		else if ( in_pBufferFrom->GetDataType().m_eType == EPodType_Int8 )
		{
			Alembic::Util::int8_t* l_pDest = (Alembic::Util::int8_t *)l_pArray;
			AbcLerp::InterpolateArray<Alembic::Util::int8_t>( l_pDest, (const Alembic::Util::int8_t*)in_pBufferFrom->GetBuffer(), (const Alembic::Util::int8_t*)in_pBufferTo->GetBuffer(), in_fAlpha, l_numElems * in_pBufferFrom->GetDataType().m_ucExtent );
		}
		else if ( in_pBufferFrom->GetDataType().m_eType == EPodType_Int16 )
		{
			Alembic::Util::int16_t* l_pDest = (short *)l_pArray;
			AbcLerp::InterpolateArray<Alembic::Util::int16_t>( l_pDest, (const Alembic::Util::int16_t*)in_pBufferFrom->GetBuffer(), (const Alembic::Util::int16_t*)in_pBufferTo->GetBuffer(), in_fAlpha, l_numElems * in_pBufferFrom->GetDataType().m_ucExtent );
		}
		else if ( in_pBufferFrom->GetDataType().m_eType == EPodType_Int32 )
		{
			Alembic::Util::int32_t* l_pDest = (Alembic::Util::int32_t *)l_pArray;
			AbcLerp::InterpolateArray<Alembic::Util::int32_t>( l_pDest, (const Alembic::Util::int32_t*)in_pBufferFrom->GetBuffer(), (const Alembic::Util::int32_t*)in_pBufferTo->GetBuffer(), in_fAlpha, l_numElems * in_pBufferFrom->GetDataType().m_ucExtent );
		}
		else if ( in_pBufferFrom->GetDataType().m_eType == EPodType_Int64 )
		{
			Alembic::Util::int64_t* l_pDest = (Alembic::Util::int64_t *)l_pArray;
			AbcLerp::InterpolateArray<Alembic::Util::int64_t>( l_pDest, (const Alembic::Util::int64_t*)in_pBufferFrom->GetBuffer(), (const Alembic::Util::int64_t*)in_pBufferTo->GetBuffer(), in_fAlpha, l_numElems * in_pBufferFrom->GetDataType().m_ucExtent );
		}
		else
		{
			return EResult_NotApplicable;
		}
		CAbcRawBufferSampleBuffer* l_pNewBuffer = new CAbcRawBufferSampleBuffer( in_pBufferFrom, l_pArray, l_numElems );
		if ( l_pNewBuffer )
		{
			l_pNewBuffer->AddRef();
			*out_ppBufferDest = l_pNewBuffer;
			return EResult_Success;
		}
		else
		{
			return EResult_OutOfMemory;
		}
	}
	return EResult_Fail;
}

EAbcResult CAbcUtils::GetInterpolatedTransformMat44( const class IAbcIXformSample* in_pBufferFrom, const class IAbcIXformSample* in_pBufferTo, double* out_pdMat, float in_fAlpha ) const
{
	if ( in_pBufferFrom == NULL || in_pBufferTo == NULL || out_pdMat == NULL )
		return EResult_InvalidPtr;

	const double* l_pdSrc = in_pBufferFrom->GetMatrix4x4();
	const double* l_pdDst = in_pBufferTo->GetMatrix4x4();

	AbcLerp::InterpolateArray< double >( out_pdMat, l_pdSrc, l_pdDst, in_fAlpha, 16 );

	return EResult_Success;
}

bool CAbcUtils::IsValidArray2DProp( IAbcOProperty* in_pOProp, IAbcOProperty** out_ppValProp, IAbcOProperty** out_ppSubArrayIndicesProp ) const
{
	if ( in_pOProp->GetType() == EPropertyType_Compound )
	{
		CAbcPtr<IAbcOCompoundProperty> l_spCompoundProp = static_cast<IAbcOCompoundProperty*>(in_pOProp);
		CAbcPtr<IAbcOProperty> l_spValProp;
		CAbcPtr<IAbcOProperty> l_spSubArrayIndicesProp;

		if ( l_spCompoundProp->GetProperty( &l_spValProp, PROPNAME_VALS ) == EResult_Success &&
			l_spCompoundProp->GetProperty( &l_spSubArrayIndicesProp, PROPNAME_SUBARRAYINDICES ) == EResult_Success )
		{
			if ( l_spValProp->GetType() == EPropertyType_Array &&
				l_spSubArrayIndicesProp->GetType() == EPropertyType_Array && l_spSubArrayIndicesProp->GetDataTraits() == EDataTraits_UInt32 )
			{
				if ( out_ppValProp )
				{
					l_spValProp->AddRef();
					*out_ppValProp = l_spValProp.GetPtr();
				}

				if ( out_ppSubArrayIndicesProp )
				{
					l_spSubArrayIndicesProp->AddRef();
					*out_ppSubArrayIndicesProp = l_spSubArrayIndicesProp.GetPtr();
				}

				return true;
			}
		}
	}

	return false;
}


bool CAbcUtils::IsValidArray2DProp( IAbcIPropertyAccessor* in_pOProp, IAbcIPropertyAccessor** out_ppValProp, IAbcIPropertyAccessor** out_ppSubArrayIndicesProp ) const
{
	if ( in_pOProp->GetPropertyType() == EPropertyType_Compound )
	{
		CAbcPtr<IAbcICompoundPropertyAccessor> l_spCompoundProp = static_cast<IAbcICompoundPropertyAccessor*>(in_pOProp);
		CAbcPtr<IAbcIPropertyAccessor> l_spValProp;
		CAbcPtr<IAbcIPropertyAccessor> l_spSubArrayIndicesProp;

		if ( l_spCompoundProp->GetChildProperty( PROPNAME_VALS, &l_spValProp ) == EResult_Success &&
			l_spCompoundProp->GetChildProperty( PROPNAME_SUBARRAYINDICES, &l_spSubArrayIndicesProp ) == EResult_Success )
		{
			if ( l_spValProp->GetPropertyType() == EPropertyType_Array &&
				l_spSubArrayIndicesProp->GetPropertyType() == EPropertyType_Array && l_spSubArrayIndicesProp->GetDataType().m_eTraits == EDataTraits_UInt32 )
			{
				if ( out_ppValProp )
				{
					l_spValProp->AddRef();
					*out_ppValProp = l_spValProp.GetPtr();
				}

				if ( out_ppSubArrayIndicesProp )
				{
					l_spSubArrayIndicesProp->AddRef();
					*out_ppSubArrayIndicesProp = l_spSubArrayIndicesProp.GetPtr();
				}

				return true;
			}
		}
	}

	return false;
}

EAbcResult CAbcUtils::ReverseFaceWinding( IAbcSampleBuffer* io_pBuffer, const Alembic::Util::int32_t* in_pFaceCounts, size_t in_szNbFaces ) const
{
	if ( io_pBuffer->GetNumElements() <= 1 )
		return EResult_Fail;

	switch ( io_pBuffer->GetDataType().m_eTraits )
	{
#define CASE_TRAITS(TRAITS) \
	case EDataTraits_##TRAITS: \
		ReverseFaceWindingImpl<Alembic::Abc::TRAITS>( (Alembic::Abc::TRAITS*)io_pBuffer->GetBuffer(), in_pFaceCounts, in_szNbFaces ); \
		return EResult_Success

#define CASE_FRAMEWORK_ABCTRAITS(FRAMEWORKTRAITS, ABCTRAITS) \
	case EDataTraits_##FRAMEWORKTRAITS: \
		ReverseFaceWindingImpl<Alembic::Abc::ABCTRAITS>( (Alembic::Abc::ABCTRAITS*)io_pBuffer->GetBuffer(), in_pFaceCounts, in_szNbFaces ); \
		return EResult_Success

#define CASE_EXT_TRAITS(TRAITS, TRAITSDATATYPE) \
	case EDataTraits_##TRAITS: \
		ReverseFaceWindingImpl<TRAITSDATATYPE>( (TRAITSDATATYPE*)io_pBuffer->GetBuffer(), in_pFaceCounts, in_szNbFaces ); \
		return EResult_Success

		CASE_FRAMEWORK_ABCTRAITS(UInt8, uint8_t);
		CASE_FRAMEWORK_ABCTRAITS(Int8, int8_t);
		CASE_FRAMEWORK_ABCTRAITS(UInt16, uint16_t);
		CASE_FRAMEWORK_ABCTRAITS(Int16, int16_t);
		CASE_FRAMEWORK_ABCTRAITS(UInt32, uint32_t);
		CASE_FRAMEWORK_ABCTRAITS(Int32, int32_t);
		CASE_FRAMEWORK_ABCTRAITS(UInt64, uint64_t);
		CASE_FRAMEWORK_ABCTRAITS(Int64, int64_t);
		CASE_FRAMEWORK_ABCTRAITS(Float16, float16_t);
		CASE_FRAMEWORK_ABCTRAITS(Float32, float32_t);
		CASE_FRAMEWORK_ABCTRAITS(Float64, float64_t);
		CASE_FRAMEWORK_ABCTRAITS(StringPtr, string);
		CASE_FRAMEWORK_ABCTRAITS(WStringPtr, wstring);

		CASE_TRAITS(V2s);
		CASE_TRAITS(V2i);
		CASE_TRAITS(V2f);
		CASE_TRAITS(V2d);
		CASE_TRAITS(V3s);
		CASE_TRAITS(V3i);
		CASE_TRAITS(V3f);
		CASE_TRAITS(V3d);
		CASE_EXT_TRAITS(V4f, AbcFramework::V4f);
		CASE_EXT_TRAITS(Rotf, AbcFramework::VRotationf);
		CASE_FRAMEWORK_ABCTRAITS(P2s, V2s);
		CASE_FRAMEWORK_ABCTRAITS(P2i, V2i);
		CASE_FRAMEWORK_ABCTRAITS(P2f, V2f);
		CASE_FRAMEWORK_ABCTRAITS(P2d, V2d);
		CASE_FRAMEWORK_ABCTRAITS(P3s, V3s);
		CASE_FRAMEWORK_ABCTRAITS(P3i, V3i);
		CASE_FRAMEWORK_ABCTRAITS(P3f, V3f);
		CASE_FRAMEWORK_ABCTRAITS(P3d, V3d);
		CASE_TRAITS(Box2s);
		CASE_TRAITS(Box2i);
		CASE_TRAITS(Box2f);
		CASE_TRAITS(Box2d);
		CASE_TRAITS(Box3s);
		CASE_TRAITS(Box3i);
		CASE_TRAITS(Box3f);
		CASE_TRAITS(Box3d);
		CASE_TRAITS(M33f); 
		CASE_TRAITS(M33d); 
		CASE_TRAITS(M44f); 
		CASE_TRAITS(M44d); 
		CASE_TRAITS(Quatf);
		CASE_TRAITS(Quatd);
		CASE_TRAITS(C3h);  
		CASE_TRAITS(C3f);  
		CASE_TRAITS(C3c);  
		CASE_TRAITS(C4h);  
		CASE_TRAITS(C4f);  
		CASE_TRAITS(C4c);  
		CASE_FRAMEWORK_ABCTRAITS(N2f, V2f);  
		CASE_FRAMEWORK_ABCTRAITS(N2d, V2d);  
		CASE_TRAITS(N3f);  
		CASE_TRAITS(N3d);  
	}

	
	return EResult_NotImpl;
}
