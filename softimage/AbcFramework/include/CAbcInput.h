//*****************************************************************************
/*!
	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*

#ifndef CABCINPUT_H
#define CABCINPUT_H
#include "IAbcFramework.h"
#include "IAbcInput.h"
#include "CRefCount.h"
#include "AbcFrameworkDataTypes.h"
#include <Alembic/Abc/All.h>
#include <Alembic/AbcGeom/All.h>
#include <Alembic/AbcCoreAbstract/All.h>

class CAbcTimeSampling : public IAbcTimeSampling, protected CRefCount
{
	IMPL_REFCOUNT;
public:
	CAbcTimeSampling( const Alembic::AbcCoreAbstract::TimeSampling& in_TimeSampling );

	size_t	GetNumTimeSamples() const;
	double	GetTimeAt( long long in_Index ) const;
	void	GetFloorIndex( double in_dTime, long long in_llNumSamples, long long& out_llIndex, double& out_dTime ) const;
	void	GetCeilIndex( double in_dTime, long long in_llNumSamples, long long& out_llIndex, double& out_dTime ) const;
	void	GetNearIndex( double in_dTime, long long in_llNumSamples, long long& out_llIndex, double& out_dTime ) const;

	static EAbcResult CreateTimeSampling( const Alembic::AbcCoreAbstract::TimeSampling& in_TimeSampling, IAbcTimeSampling** out_ppSampling );
private:
	// We store a copy, which could be expensive but safe
	Alembic::AbcCoreAbstract::TimeSampling m_TimeSampling;
};

#include "CAbcInput_Helpers.h"

class CAbcIObject : public CAbcIObjectImpl<Alembic::Abc::IObject, IAbcIObject>, protected CRefCount
{
	IMPL_REFCOUNT;
public:
	CAbcIObject( Alembic::Abc::IObject in_object );

	// Type Info
	EAbcIObjectType	GetType() const;
	EAbcResult		TransformInto( EAbcIObjectType in_objType, IAbcIObject** out_ppObject );
	EAbcResult		GetSchemaObject( IAbcISchemaObject** out_ppObject ) const;

};

class CAbcIArchive : public IAbcIArchive, protected CRefCount
{
	IMPL_REFCOUNT;
public:
	CAbcIArchive( const char* in_szFilename );
	~CAbcIArchive();
	EAbcResult		GetTop(IAbcIObject** out_ppObject) const;
	const char*		GetName() const;
	unsigned int	GetNumTimeSampling();
	EAbcResult		GetTimeSampling( unsigned int in_uiIndex, IAbcTimeSampling** out_ppSampling );
	EAbcResult		CreateSampleSelector( IAbcISampleSelector** out_ppSelector ) const;

	EAbcResult		FindObject( const char* in_pszName, IAbcIObject** out_ppObject );

	// Archive Info
	void			GetArchiveStartAndEndTime( double* out_ppdStartTime, double* out_ppdEndTime ) const;
	unsigned int	GetAbcApiVersion() const;
	const char*		GetApplicationWriter() const;
	const char*		GetAbcVersion() const;
	const char*		GetDateWritten() const;
	const char*		GetUserDescription() const;

	Alembic::Abc::IArchive*	GetInternalArchive();
protected:
	Alembic::Abc::IArchive m_Archive;
	std::string m_strApplicationWriter;
	std::string m_strAbcVersion;
	std::string m_strDateWritten;
	std::string m_strUserDescription;
	double		m_dStartTime;
	double		m_dEndTime;
	unsigned int m_uiAbcApiVersion;

	typedef std::map<std::string, IAbcIObject*> TObjectMap;
	typedef std::pair<std::string, IAbcIObject*> TStringObjectPair;
	TObjectMap m_mapObjects;
	void GetArchiveStartAndEndTimeManually( Alembic::Abc::IObject in_Object );
};

template<typename TSamplePtrType, typename TSampleType>
class CSampleBuffer : public IAbcSampleBuffer, protected CRefCount
{
	IMPL_REFCOUNT;
	typedef typename TSampleType::value_type TSampleValueType;
	typedef typename TSampleType::traits_type TSampleTraitsType;
public:
	CSampleBuffer( TSamplePtrType& in_ptr, EAbcGeomScope in_eGeomScope, bool in_bIndexed ) : m_SharedPtr( in_ptr ), m_eGeomScope( in_eGeomScope ), m_bIndexed( in_bIndexed )
	{
		Alembic::Abc::DataType l_srcType = in_ptr->getDataType();
	
		m_DataType.m_eType = (EAbcPodType)l_srcType.getPod();
		m_DataType.m_ucExtent = l_srcType.getExtent();
		m_DataType.m_eTraits = AbcFramework::GetEDataTraits<TSampleTraitsType>();
		m_DataType.m_numBytes =  l_srcType.getNumBytes();
	}
	
	const void*			GetBuffer() const
	{
		return (void*)m_SharedPtr->get();
	}

	size_t				GetNumElements() const
	{
		return m_SharedPtr->size();
	}

	const SAbcDataType& GetDataType() const
	{
		return m_DataType;
	}

	EAbcGeomScope GetGeomScope() const
	{
		return m_eGeomScope;
	}

	bool IsIndexed() const
	{
		return m_bIndexed;
	}


private:
	TSamplePtrType		m_SharedPtr;
	SAbcDataType		m_DataType;
	EAbcGeomScope		m_eGeomScope;
	bool				m_bIndexed;

};

template<typename TSamplePtrType, typename TSampleType >
static EAbcResult CreateBuffer( TSamplePtrType& in_ptr, EAbcGeomScope in_eGeomScope, bool in_bIndexed, IAbcSampleBuffer** out_ppBuffer )
{
	*out_ppBuffer = new CSampleBuffer<TSamplePtrType, TSampleType>( in_ptr, in_eGeomScope, in_bIndexed );
	if ( !(*out_ppBuffer) )
	{
		return EResult_OutOfMemory;
	}
	(*out_ppBuffer)->AddRef();
	return EResult_Success;
}

class CAbcISampleSelector : public IAbcISampleSelector, protected CRefCount
{
	IMPL_REFCOUNT;
public:
	CAbcISampleSelector();
	
	// IAbcSampleSelector
	void SetRequestedIndex( long long in_llIndex );
	void SetRequestedTime( EAbcTimeIndex in_eTimeIndexType, double in_dTime );
	void GetRequestedIndex( long long& out_llIndex ) const;
	void GetRequestedTime( EAbcTimeIndex& out_eTimeIndexType, double& out_dTime ) const;

	// Internal functions
	void GetSampleSelector( Alembic::Abc::ISampleSelector& out_Selector ) const;
private:
	long long m_llIndex;
	double m_dTime;
	EAbcTimeIndex m_eTimeIndexType;
	bool m_bIndexRequested;
};

class CAbcIPolyMesh : public CAbcISchemaObjectImpl< Alembic::AbcGeom::IPolyMesh, IAbcIPolyMesh, EIObject_Polymesh >, protected CRefCount
{
	IMPL_REFCOUNT;

public:
	CAbcIPolyMesh( Alembic::Abc::IObject in_object );
	// IPolyMeshSchema
	EAbcTopoVariance	GetTopologyVariance() const;
	bool				IsConstant() const;

	// Standard Accessors
	EAbcResult			GetUVs( bool in_bExpand, IAbcISampleSelector* in_pSampleSelector, IAbcSampleBuffer** out_ppBuffer );
	EAbcResult			GetNormals( bool in_bExpand, IAbcISampleSelector* in_pSampleSelector, IAbcSampleBuffer** out_ppBuffer );
	EAbcResult			GetFaceCounts( IAbcISampleSelector* in_pSampleSelector, IAbcSampleBuffer** out_ppBuffer );
	EAbcResult			GetFaceIndices( IAbcISampleSelector* in_pSampleSelector, IAbcSampleBuffer** out_ppBuffer );
	EAbcResult			GetPositions( IAbcISampleSelector* in_pSampleSelector, IAbcSampleBuffer** out_ppBuffer );
	EAbcResult			GetVelocities( IAbcISampleSelector* in_pSampleSelector, IAbcSampleBuffer** out_ppBuffer );
	EAbcResult			GetUVsParam( IAbcIGeomParam** out_ppGeomParam );
	EAbcResult			GetNormalsParam( IAbcIGeomParam** out_ppGeomParam );

protected:
	
};

class CAbcIPoints : public CAbcISchemaObjectImpl< Alembic::AbcGeom::IPoints, IAbcIPoints, EIObject_Points >, protected CRefCount
{
	IMPL_REFCOUNT;

public:
	CAbcIPoints( Alembic::Abc::IObject in_object );

	// IPointsSchema
	bool	IsConstant() const;
	
	// Standard Accessors
	EAbcResult	GetPositions( IAbcISampleSelector* in_pSampleSelector, IAbcSampleBuffer** out_ppBuffer );
	EAbcResult	GetVelocities( IAbcISampleSelector* in_pSampleSelector, IAbcSampleBuffer** out_ppBuffer );
	EAbcResult	GetIds( IAbcISampleSelector* in_pSampleSelector, IAbcSampleBuffer** out_ppBuffer );

protected:
	template<typename TSamplePtrType, typename TSampleType >
	static EAbcResult CreateBuffer( TSamplePtrType& in_ptr, IAbcSampleBuffer** out_ppBuffer );
};

class CAbcICamera : public CAbcISchemaObjectImpl< Alembic::AbcGeom::ICamera, IAbcICamera, EIObject_Camera >, protected CRefCount
{
	IMPL_REFCOUNT;

public:
	CAbcICamera( Alembic::Abc::IObject in_object );
	// ICameraSchema
	bool	IsConstant() const;

	// Standard Accessors
	EAbcResult GetCameraSample( IAbcISampleSelector* in_pSampleSelector, IAbcICameraSample** out_ppSample );
};

class CAbcIXformOp : public IAbcIXformOp, protected CRefCount
{
	IMPL_REFCOUNT;
public:
	CAbcIXformOp( const Alembic::AbcGeom::XformOp& in_Xform );

	EAbcXformOpType	GetType() const;
	unsigned char	GetHint() const;
	bool			IsXAnimated() const;
	bool			IsYAnimated() const;
	bool			IsZAnimated() const;
	bool			IsAngleAnimated() const;
	bool			IsChannelAnimated( size_t in_Index ) const;
	size_t			GetNumChannels() const;
	double			GetDefaultChannelValue( size_t in_Index ) const;
	double			GetChannelValue( size_t in_Index ) const;
	const double*	GetTranslate() const;
	const double*	GetScale() const;
	const double*	GetAxis() const;
	const double*	GetMatrix4x4() const;
	double			GetXRotation() const;
	double			GetYRotation() const;
	double			GetZRotation() const;
	double			GetAngle() const;
protected:
	Alembic::AbcGeom::XformOp m_XformOp;

	Alembic::Abc::V3d	m_Translate;
	Alembic::Abc::V3d	m_Scale;
	Alembic::Abc::V3d	m_Axis;
	Alembic::Abc::M44d	m_Matrix4x4;
};

class CAbcIXformSample : public IAbcIXformSample, protected CRefCount
{
	IMPL_REFCOUNT;
public:
	CAbcIXformSample( const Alembic::AbcGeom::XformSample& in_Sample );
	size_t			GetNumOps() const;
	size_t			GetNumOpChannels() const;
	EAbcResult		GetOp( size_t in_index, IAbcIXformOp** out_ppXformOp ) const;
	bool			GetInheritsXforms() const;

	const double*	GetTranslation() const;
	const double*	GetAxis() const;
	const double*	GetScale() const;
	const double*	GetMatrix4x4() const;
	double			GetAngle() const;
	double			GetXRotation() const;
	double			GetYRotation() const;
	double			GetZRotation() const;
protected:
	Alembic::AbcGeom::XformSample m_Sample;

	Alembic::Abc::V3d	m_Translate;
	Alembic::Abc::V3d	m_Scale;
	Alembic::Abc::V3d	m_Axis;
	Alembic::Abc::M44d	m_Matrix4x4;
};

class CAbcIXform : public CAbcISchemaObjectImpl< Alembic::AbcGeom::IXform, IAbcIXform, EIObject_Xform >, protected CRefCount
{
	IMPL_REFCOUNT;
public:
	CAbcIXform( Alembic::Abc::IObject in_object );

	// IXformSchema
	bool			IsConstant() const;
	bool			IsConstantIdentity() const;

	// Xform Ops Accessors
	size_t			GetNumOps() const;
	EAbcResult		GetSample( IAbcISampleSelector* in_pSampleSelector, IAbcIXformSample** out_ppXformSample ) const;

};

class CAbcIFaceSet : public CAbcISchemaObjectImpl< Alembic::AbcGeom::IFaceSet, IAbcIFaceSet, EIObject_FaceSet >, protected CRefCount
{
	IMPL_REFCOUNT;
public:
	CAbcIFaceSet( Alembic::Abc::IObject in_object );

	// IFaceSetSchema
	bool					IsConstant() const;
	EAbcIFaceSetExclusivity	GetExclusivity() const;
	EAbcResult				GetSample( IAbcISampleSelector* in_pSampleSelector, IAbcSampleBuffer** out_ppBuffer );
};

#endif
