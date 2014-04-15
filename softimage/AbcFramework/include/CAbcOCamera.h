//*****************************************************************************
/*!
	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*

#ifndef CABCOCAMERA_H
#define CABCOCAMERA_H

#include <Alembic/Abc/OObject.h>
#include <Alembic/AbcGeom/OCamera.h>

#include "AbcFrameworkUtil.h"
#include "CAbcOObjectFactory.h"
#include "CAbcOutput_Helpers.h"
#include "CRefCount.h"
#include "IAbcFramework.h"
#include "IAbcOutput.h"
#include <string>

class CAbcOCameraSample : public IAbcOCameraSample, protected CRefCount
{
	IMPL_REFCOUNT;
public:
	CAbcOCameraSample();

	void SetFocalLength( double in_dFocalLength );
	void SetAperture( double in_dHorizAperture, double in_dVertAperture );
	void SetFilmOffset( double in_dHorizOffset, double in_dVertOffset );
	void SetLensSqueezeRatio( double in_dRatio );
	void SetOverscan( double in_dLeft, double in_dRight, double in_dTop, double in_dBottom );
	void SetFStop( double in_dFStop );
	void SetFocusDistance( double in_dFocusDistance );
	void SetShutterOpen( double in_dTime );
	void SetShutterClose( double in_dTime );
	void SetClippingPlanes( double in_dNear, double in_dFar );
	void SetChildBounds( const Alembic::Abc::Box3d& in_BBox );

	const Alembic::AbcGeom::CameraSample& GetSample() const;
private:
	Alembic::AbcGeom::CameraSample m_CameraSample;
};

class CAbcOCamera : public CAbcOSchemaObjectImpl< Alembic::AbcGeom::OCamera, IAbcOCamera >, protected CRefCount
{
	IMPL_REFCOUNT;
public:
	typedef CAbcOSchemaObjectImpl< Alembic::AbcGeom::OCamera, IAbcOCamera > TheBaseClass;

	CAbcOCamera( IAbcOObject* in_pParent, Alembic::Abc::OObject& in_ParentOObj, const char* in_pszName )
		: TheBaseClass( in_pParent, in_ParentOObj, in_pszName )
	{

	}
	EAbcOObjectType GetType() const { return EOObject_Camera;}

	EAbcResult CreateCameraSample( IAbcOCameraSample** out_ppSample );
	void AddCameraSample( const IAbcOCameraSample* in_pSample );
};

#endif // CABCOCAMERA_H