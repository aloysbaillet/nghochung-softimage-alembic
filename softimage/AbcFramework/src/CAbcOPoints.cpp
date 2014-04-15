//*****************************************************************************
/*!
	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*

#include "CAbcOPoints.h"


void CAbcOPoints::AddSample( const float* in_pfPointPositions, int in_iPointCount, const Alembic::Abc::Box3d& in_BBox )
{
	Alembic::AbcGeom::OPointsSchema& l_pointsSchema = GetSchema();

	Alembic::AbcGeom::OPointsSchema::Sample l_pointsSample(
		Alembic::Abc::P3fArraySample( (const Alembic::Abc::V3f*)in_pfPointPositions, in_iPointCount )
		);

	l_pointsSample.setSelfBounds( in_BBox );

	l_pointsSchema.set( l_pointsSample );

}

void CAbcOPoints::AddSample( 
	const float* in_pfPointPositions, int in_iPointCount, 
	const float* in_pVelocities, int in_iVelocityCount, 
	const float* in_pWidths, int in_iWidthCount,
	const Alembic::Abc::Box3d& in_BBox
)
{
	Alembic::AbcGeom::OPointsSchema& l_pointsSchema = GetSchema();

	Alembic::AbcGeom::OPointsSchema::Sample l_pointsSample(
		Alembic::Abc::P3fArraySample( (const Alembic::Abc::V3f*)in_pfPointPositions, in_iPointCount )
	);

	if ( in_iVelocityCount > 0 && in_pVelocities != NULL )
	{
		l_pointsSample.setVelocities( Alembic::Abc::V3fArraySample( (const Alembic::Abc::V3f*)in_pVelocities, in_iVelocityCount ) );
	}

	if ( in_iWidthCount > 0 && in_pWidths != NULL )
	{
		Alembic::Abc::FloatArraySample l_widthArraySample( in_pWidths, in_iWidthCount );
		Alembic::AbcGeom::OFloatGeomParam::Sample  l_widthParamSample( l_widthArraySample, Alembic::AbcGeom::kVaryingScope );
		l_pointsSample.setWidths( l_widthParamSample );
	}

	l_pointsSample.setSelfBounds( in_BBox );

	l_pointsSchema.set( l_pointsSample );

}

void CAbcOPoints::AddSample( 
	const float* in_pfPointPositions, int in_iPointCount, 
	const unsigned long long* in_pIds, int in_iIdCount, 
	const Alembic::Abc::Box3d& in_BBox
	)
{

	Alembic::AbcGeom::OPointsSchema& l_pointsSchema = GetInternalObject().getSchema();

	Alembic::AbcGeom::OPointsSchema::Sample l_pointsSample(
		Alembic::Abc::P3fArraySample( (const Alembic::Abc::V3f*)in_pfPointPositions, in_iPointCount ),
		Alembic::Abc::UInt64ArraySample( in_pIds, in_iIdCount )
		);

	l_pointsSample.setSelfBounds( in_BBox );

	l_pointsSchema.set( l_pointsSample );

}

void CAbcOPoints::AddSample( 
	const float* in_pfPointPositions, int in_iPointCount, 
	const unsigned long long* in_pIds, int in_iIdCount, 
	const float* in_pVelocities, int in_iVelocityCount, 
	const float* in_pWidths, int in_iWidthCount,
	const Alembic::Abc::Box3d& in_BBox
)
{

	Alembic::AbcGeom::OPointsSchema& l_pointsSchema = GetInternalObject().getSchema();

	Alembic::AbcGeom::OPointsSchema::Sample l_pointsSample(
		Alembic::Abc::P3fArraySample( (const Alembic::Abc::V3f*)in_pfPointPositions, in_iPointCount ),
		Alembic::Abc::UInt64ArraySample( in_pIds, in_iIdCount )
	);

	if ( in_iVelocityCount > 0 && in_pVelocities != NULL )
	{
		l_pointsSample.setVelocities( Alembic::Abc::V3fArraySample( (const Alembic::Abc::V3f*)in_pVelocities, in_iVelocityCount ) );
	}

	if ( in_iWidthCount > 0 && in_pWidths != NULL )
	{
		Alembic::Abc::FloatArraySample l_widthArraySample( in_pWidths, in_iWidthCount );
		Alembic::AbcGeom::OFloatGeomParam::Sample  l_widthParamSample( l_widthArraySample, Alembic::AbcGeom::kVaryingScope );
		l_pointsSample.setWidths( l_widthParamSample );
	}
	
	l_pointsSample.setSelfBounds( in_BBox );

	l_pointsSchema.set( l_pointsSample );

}
