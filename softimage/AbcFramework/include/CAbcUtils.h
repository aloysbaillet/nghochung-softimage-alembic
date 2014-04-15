//*****************************************************************************
/*!
	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*

#ifndef CABCUTILS_H
#define CABCUTILS_H

#include "IAbcFramework.h"

class CAbcUtils : public IAbcUtils
{
public:
	double RadiansToDegrees( double in_dAngle ) const;
	double DegreesToRadians( double in_dAngle ) const;
	double FocalLengthFromFov( double in_dFov, double in_dAperture ) const;
	const char* GetStringFromSample( size_t in_Index, const class IAbcSampleBuffer* in_pBuffer ) const;
	const wchar_t* GetWideStringFromSample( size_t in_Index, const class IAbcSampleBuffer* in_pBuffer ) const;
	
	void GetTotalStringSize( const class IAbcSampleBuffer* in_pBuffer, 
		Alembic::Util::uint32_t in_startIndex, Alembic::Util::uint32_t in_endIndex, 
		Alembic::Util::uint32_t& out_TotalSize, Alembic::Util::uint32_t* out_pPerStringSize ) const;

	EAbcResult GetInterpolatedBuffer( const class IAbcSampleBuffer* in_pBufferFrom, const class IAbcSampleBuffer* in_pBufferTo, class IAbcSampleBuffer** out_ppBufferDest, float in_fAlpha ) const;
	virtual EAbcResult GetInterpolatedTransformMat44( const class IAbcIXformSample* in_pBufferFrom, const class IAbcIXformSample* in_pBufferTo, double* out_pdMat, float in_fAlpha ) const;

	bool IsValidArray2DProp( IAbcOProperty* in_pOProp, IAbcOProperty** out_ppValProp, IAbcOProperty** out_ppSubArrayIndicesProp ) const;
	bool IsValidArray2DProp( IAbcIPropertyAccessor* in_pOProp, IAbcIPropertyAccessor** out_ppValProp, IAbcIPropertyAccessor** out_ppSubArrayIndicesProp ) const;
	EAbcResult ReverseFaceWinding( IAbcSampleBuffer* io_pBuffer, const Alembic::Util::int32_t* in_pFaceCounts, size_t in_szNbFaces ) const;
};

#endif // CABCUTILS_H
