//*****************************************************************************
/*!
	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*

#include "CAbcInput.h"
#include <assert.h>

// Alembic Includes
#include <Alembic/AbcGeom/All.h>
#include <Alembic/AbcMaterial/All.h>
// This is required to tell Alembic which implementation to use.  In this case,
// the HDF5 implementation, currently the only one available.
#include <Alembic/AbcCoreHDF5/All.h>

using namespace Alembic;
using namespace Alembic::Abc;
using namespace Alembic::AbcGeom;
using namespace Alembic::AbcMaterial;
using namespace Alembic::AbcCoreAbstract;
class CAbcICameraSample : public IAbcICameraSample, protected CRefCount
{
public:
	IMPL_REFCOUNT;

	CAbcICameraSample( const CameraSample& in_sample ) : m_sample( in_sample ) {}
	double	GetFocalLength( ) const
	{
		return m_sample.getFocalLength();
	}
	void	GetAperture( double& out_dHorizAperture, double& out_dVertAperture ) const
	{
		out_dHorizAperture = m_sample.getHorizontalAperture();
		out_dVertAperture = m_sample.getVerticalAperture();
	}
	void	GetFilmOffset( double& out_dHorizOffset, double& out_dVertOffset ) const
	{
		out_dHorizOffset = m_sample.getHorizontalFilmOffset();
		out_dVertOffset = m_sample.getVerticalFilmOffset();
	}
	double	GetLensSqueezeRatio( ) const
	{
		return m_sample.getLensSqueezeRatio();
	}
	void	GetOverscan( double& out_dLeft, double& out_dRight, double& out_dTop, double& out_dBottom ) const
	{
		// Bug in alembic library, getOverScanLeft is non-const while the other get functions are const
		out_dLeft = const_cast<CAbcICameraSample*>(this)->m_sample.getOverScanLeft();
		out_dRight = m_sample.getOverScanRight();
		out_dTop = m_sample.getOverScanTop();
		out_dBottom = m_sample.getOverScanBottom();
	}
	double	GetFStop( ) const
	{
		return m_sample.getFStop();
	}
	double	GetFocusDistance( ) const
	{
		return m_sample.getFocusDistance();
	}
	double	GetShutterOpen( ) const
	{
		return m_sample.getShutterOpen();
	}
	double	GetShutterClose( ) const
	{
		return m_sample.getShutterClose();
	}
	void	GetClippingPlanes( double& out_dNear, double& out_dFar ) const 
	{
		out_dNear = m_sample.getNearClippingPlane();
		out_dFar = m_sample.getFarClippingPlane();
	}
	void	GetChildBounds( Alembic::Abc::Box3d& out_BBox ) const
	{
		out_BBox = m_sample.getChildBounds();
	}
	double	GetEffectiveFov( ) const
	{
		return m_sample.getFieldOfView();
	}
private:
	CameraSample m_sample;
};
\
CAbcICamera::CAbcICamera( Alembic::Abc::IObject in_object ) : CAbcISchemaObjectImpl< ICamera, IAbcICamera, EIObject_Camera >( in_object, kWrapExisting )
{
}

// ICameraSchema
bool	CAbcICamera::IsConstant() const
{
	return m_Schema.isConstant();
}

// Standard Accessors
EAbcResult CAbcICamera::GetCameraSample( IAbcISampleSelector* in_pSampleSelector, IAbcICameraSample** out_ppSample )
{
	if ( out_ppSample == NULL )
		return EResult_InvalidPtr;

	ISampleSelector l_ss;
	if ( in_pSampleSelector != NULL )
		((CAbcISampleSelector*)in_pSampleSelector)->GetSampleSelector( l_ss );
	CameraSample l_sample;
	m_Schema.get( l_sample, l_ss );

	CAbcICameraSample* l_pNewSample = new CAbcICameraSample( l_sample );
	if ( l_pNewSample )
	{
		l_pNewSample->AddRef();
		*out_ppSample = l_pNewSample;
		return EResult_Success;
	}
	else
	{
		return EResult_OutOfMemory;
	}
}

