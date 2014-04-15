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
#include <float.h>

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
// CAbcTimeSampling
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CAbcTimeSampling::CAbcTimeSampling( const Alembic::AbcCoreAbstract::TimeSampling& in_TimeSampling ) : m_TimeSampling( in_TimeSampling )
{
}

size_t	CAbcTimeSampling::GetNumTimeSamples() const
{
	return m_TimeSampling.getNumStoredTimes();
}

double	CAbcTimeSampling::GetTimeAt( long long in_Index ) const
{
	return m_TimeSampling.getSampleTime( (index_t)in_Index );
}

void	CAbcTimeSampling::GetFloorIndex( double in_dTime, long long in_llNumSamples, long long& out_llIndex, double& out_dTime ) const
{
	std::pair< index_t, Alembic::Abc::chrono_t > l_TimeSample = m_TimeSampling.getFloorIndex( (Alembic::Abc::chrono_t)in_dTime, (index_t)in_llNumSamples );
	out_llIndex = (index_t)l_TimeSample.first;
	out_dTime = (Alembic::Abc::chrono_t)l_TimeSample.second;
}

void	CAbcTimeSampling::GetCeilIndex( double in_dTime, long long in_llNumSamples, long long& out_llIndex, double& out_dTime ) const
{
	std::pair< index_t, Alembic::Abc::chrono_t > l_TimeSample = m_TimeSampling.getCeilIndex( (Alembic::Abc::chrono_t)in_dTime, (index_t)in_llNumSamples );
	out_llIndex = (index_t)l_TimeSample.first;
	out_dTime = (Alembic::Abc::chrono_t)l_TimeSample.second;
}

void	CAbcTimeSampling::GetNearIndex( double in_dTime, long long in_llNumSamples, long long& out_llIndex, double& out_dTime ) const
{
	std::pair< index_t, Alembic::Abc::chrono_t > l_TimeSample = m_TimeSampling.getNearIndex( (Alembic::Abc::chrono_t)in_dTime, (index_t)in_llNumSamples );
	out_llIndex = (index_t)l_TimeSample.first;
	out_dTime = (Alembic::Abc::chrono_t)l_TimeSample.second;
}

EAbcResult CAbcTimeSampling::CreateTimeSampling( const Alembic::AbcCoreAbstract::TimeSampling& in_TimeSampling, IAbcTimeSampling** out_ppSampling )
{
	CAbcTimeSampling* l_pNewSampling = new CAbcTimeSampling( in_TimeSampling );
	if ( l_pNewSampling )
	{
		l_pNewSampling->AddRef();
		*out_ppSampling = l_pNewSampling;
		return EResult_Success;
	}
	return EResult_OutOfMemory;
}
