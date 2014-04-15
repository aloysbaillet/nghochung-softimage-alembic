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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CAbcIXformOp
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CAbcIXformOp::CAbcIXformOp( const Alembic::AbcGeom::XformOp& in_Xform ) : 
	m_XformOp( in_Xform )
{
	XformOperationType l_XformType = in_Xform.getType();

	if ( l_XformType == kMatrixOperation )
		m_Matrix4x4 = m_XformOp.getMatrix();
	if ( l_XformType == kTranslateOperation )
		m_Translate = m_XformOp.getTranslate();
	if ( l_XformType == kScaleOperation )
		m_Scale = m_XformOp.getScale();
	if ( l_XformType == kRotateOperation ||
		 l_XformType == kRotateXOperation ||
		 l_XformType == kRotateYOperation ||
		 l_XformType == kRotateZOperation )
		m_Axis = m_XformOp.getAxis();
}

EAbcXformOpType CAbcIXformOp::GetType() const
{
	return (EAbcXformOpType)m_XformOp.getType();
}

unsigned char CAbcIXformOp::GetHint() const
{
	return m_XformOp.getHint();
}

bool CAbcIXformOp::IsXAnimated() const
{
	return m_XformOp.isXAnimated();
}

bool CAbcIXformOp::IsYAnimated() const
{
	return m_XformOp.isYAnimated();
}

bool CAbcIXformOp::IsZAnimated() const
{
	return m_XformOp.isZAnimated();
}

bool CAbcIXformOp::IsAngleAnimated() const
{
	return m_XformOp.isAngleAnimated();
}

bool CAbcIXformOp::IsChannelAnimated( size_t in_Index ) const
{
	return m_XformOp.isChannelAnimated( in_Index );
}

size_t CAbcIXformOp::GetNumChannels() const
{
	return m_XformOp.getNumChannels();
}

double CAbcIXformOp::GetDefaultChannelValue( size_t in_Index ) const
{
	return m_XformOp.getDefaultChannelValue( in_Index );
}

double CAbcIXformOp::GetChannelValue( size_t in_Index ) const
{
	return m_XformOp.getChannelValue( in_Index );
}

const double* CAbcIXformOp::GetTranslate() const
{
	return &m_Translate.x;
}

const double* CAbcIXformOp::GetScale() const
{
	return &m_Scale.x;
}

const double* CAbcIXformOp::GetAxis() const
{
	return &m_Axis.x;
}

const double* CAbcIXformOp::GetMatrix4x4() const
{
	return (const double*)m_Matrix4x4.x;
}

double CAbcIXformOp::GetXRotation() const
{
	return m_XformOp.getXRotation();
}

double CAbcIXformOp::GetYRotation() const
{
	return m_XformOp.getYRotation();
}

double CAbcIXformOp::GetZRotation() const
{
	return m_XformOp.getZRotation();
}

double CAbcIXformOp::GetAngle() const
{
	return m_XformOp.getAngle();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CAbcIXformSample
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CAbcIXformSample::CAbcIXformSample( const Alembic::AbcGeom::XformSample& in_Sample ) : 
	m_Sample( in_Sample ),
	m_Translate( in_Sample.getTranslation() ),
	m_Scale( in_Sample.getScale() ),
	m_Axis( in_Sample.getAxis() ),
	m_Matrix4x4( in_Sample.getMatrix() )
{

}

size_t CAbcIXformSample::GetNumOps() const
{
	return m_Sample.getNumOps();
}

size_t CAbcIXformSample::GetNumOpChannels() const
{
	return m_Sample.getNumOpChannels();
}

EAbcResult CAbcIXformSample::GetOp( size_t in_index, IAbcIXformOp** out_ppXformOp ) const
{
	if ( !out_ppXformOp )
		return EResult_InvalidPtr;
	if ( in_index >= m_Sample.getNumOps() )
		return EResult_OutOfRange;

	XformOp l_XformOp = m_Sample.getOp( in_index );
	IAbcIXformOp* l_pNewOp = new CAbcIXformOp( l_XformOp );
	if ( l_pNewOp )
	{
		l_pNewOp->AddRef();
		*out_ppXformOp = l_pNewOp;
		return EResult_Success;
	}
	return EResult_OutOfMemory;
}

bool CAbcIXformSample::GetInheritsXforms() const
{
	return m_Sample.getInheritsXforms();
}

const double* CAbcIXformSample::GetTranslation() const
{
	return &m_Translate.x;
}

const double* CAbcIXformSample::GetAxis() const
{
	return &m_Axis.x;
}

const double* CAbcIXformSample::GetScale() const
{
	return &m_Scale.x;
}

const double* CAbcIXformSample::GetMatrix4x4() const
{
	return (const double*)m_Matrix4x4.x;
}

double CAbcIXformSample::GetAngle() const
{
	return m_Sample.getAngle();
}

double CAbcIXformSample::GetXRotation() const
{
	return m_Sample.getXRotation();
}

double CAbcIXformSample::GetYRotation() const
{
	return m_Sample.getYRotation();
}

double CAbcIXformSample::GetZRotation() const
{
	return m_Sample.getZRotation();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CAbcIXform
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CAbcIXform::CAbcIXform( Alembic::Abc::IObject in_object ) : CAbcISchemaObjectImpl< IXform, IAbcIXform, EIObject_Xform >( in_object, kWrapExisting )
{

}

bool CAbcIXform::IsConstant() const
{
	return m_Schema.isConstant();
}

bool CAbcIXform::IsConstantIdentity() const
{
	return m_Schema.isConstantIdentity();
}

size_t CAbcIXform::GetNumOps() const
{
	return m_Schema.getNumOps();
}

EAbcResult CAbcIXform::GetSample( IAbcISampleSelector* in_pSampleSelector, IAbcIXformSample** out_ppXformSample ) const
{
	if ( !out_ppXformSample )
		return EResult_InvalidPtr;
	XformSample l_sample;
	
	ISampleSelector l_selector;

	if ( in_pSampleSelector )
		( (CAbcISampleSelector*)in_pSampleSelector )->GetSampleSelector( l_selector );

	m_Schema.get( l_sample, l_selector );

	IAbcIXformSample* l_pNewSample = new CAbcIXformSample( l_sample );
	if ( l_pNewSample )
	{
		l_pNewSample->AddRef();
		*out_ppXformSample = l_pNewSample;
		return EResult_Success;
	}
	return EResult_OutOfMemory;
}