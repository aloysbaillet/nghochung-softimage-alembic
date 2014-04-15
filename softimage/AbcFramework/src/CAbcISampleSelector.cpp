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
// CAbcSampleSelector
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CAbcISampleSelector::CAbcISampleSelector() : m_llIndex( 0 ), m_dTime( 0.0 ), m_eTimeIndexType( ETimeIndex_Near ), m_bIndexRequested( true )
{

}

void CAbcISampleSelector::SetRequestedIndex( long long in_llIndex )
{
	m_bIndexRequested = true;
	m_llIndex = in_llIndex;
}

void CAbcISampleSelector::SetRequestedTime( EAbcTimeIndex in_eTimeIndexType, double in_dTime )
{
	m_bIndexRequested = false;
	m_eTimeIndexType = in_eTimeIndexType;
	m_dTime = in_dTime;
}

void CAbcISampleSelector::GetRequestedIndex( long long& out_llIndex ) const
{
	out_llIndex = m_llIndex;
}

void CAbcISampleSelector::GetRequestedTime( EAbcTimeIndex& out_eTimeIndexType, double& out_dTime ) const
{
	out_eTimeIndexType = m_eTimeIndexType;
	out_dTime = m_dTime;
}

void CAbcISampleSelector::GetSampleSelector( Alembic::Abc::ISampleSelector& out_Selector ) const
{
	if ( m_bIndexRequested )
	{
		out_Selector = ISampleSelector( (index_t)m_llIndex );
	}
	else
	{
		out_Selector = ISampleSelector( (chrono_t)m_dTime, (ISampleSelector::TimeIndexType)m_eTimeIndexType );
	}
}
