//*****************************************************************************
/*!
	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*

#include "AbcXsiUtils.h"

#include <xsi_application.h>
#include <xsi_string.h>
#include <xsi_uitoolkit.h>

#include <stdarg.h>

#include "AbcFrameworkUtil.h"

using namespace XSI;

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Utility Functions
//////////////////////////////////////////////////////////////////////////////////////////////////////////
struct SRawFloat2
{
	float x;
	float y;
};

struct SRawFloat3
{
	float x;
	float y;
	float z;
};

template< typename T >
void ReverseFaceWinding( T* in_pDest, const LONG* in_plFaceCounts, size_t in_NumPolys, const T* in_pSrc )
{
	size_t l_CurIndex = 0;
	for ( size_t l_CurPoly = 0; l_CurPoly < in_NumPolys; l_CurPoly++ )
	{
		LONG l_lFaceCount = in_plFaceCounts[ l_CurPoly ];
		for ( size_t l_CurCount = 0; l_CurCount < l_lFaceCount; l_CurCount++ )
		{
			in_pDest[l_CurIndex + l_CurCount] = in_pSrc[l_CurIndex + (l_lFaceCount - l_CurCount - 1)];
		}
		l_CurIndex += l_lFaceCount;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// CLog
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void CLog::Indent()
{
	m_iLogLevel++;
}

void CLog::UnIndent()
{
	m_iLogLevel--;
}

void CLog::Log( bool in_bIndent, XSI::siSeverityType in_SeverityType, const char * in_szFormat, ... )
{
	char l_szBuffer[1024];
	va_list l_vaArgs;
	va_start (l_vaArgs, in_szFormat);
	vsnprintf ( l_szBuffer,1024,in_szFormat, l_vaArgs );
	va_end (l_vaArgs);
	l_szBuffer[1023] = 0;
	XSI::CString l_csTab;
	if ( in_bIndent )
	{
		for ( int i = 0; i < m_iLogLevel; i++ )
			l_csTab += "\t";
	}
	CString l_csMsg = l_csTab;
	l_csMsg += l_szBuffer;

	Application().LogMessage( l_csMsg, in_SeverityType );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// CXsiProgressBar
//////////////////////////////////////////////////////////////////////////////////////////////////////////
std::stack<CXsiProgressBar::SProgressBarState> CXsiProgressBar::m_States;
XSI::ProgressBar CXsiProgressBar::m_ProgressBar;
bool CXsiProgressBar::m_bInit = false;

void CXsiProgressBar::Push()
{
	m_States.push( SProgressBarState() );
	
	SProgressBarState& l_State = m_States.top();
	l_State.m_lMin = m_ProgressBar.GetMinimum();
	l_State.m_lMax = m_ProgressBar.GetMaximum();
	l_State.m_lCurrent = m_ProgressBar.GetValue();
	l_State.m_csText = m_ProgressBar.GetStatusText();
	l_State.m_csCaption = m_ProgressBar.GetCaption();
}

void CXsiProgressBar::Pop()
{
	m_States.pop();
	if ( m_States.size() != 0 )
	{
		const SProgressBarState& l_State = m_States.top();
		m_ProgressBar.PutMinimum( l_State.m_lMin );
		m_ProgressBar.PutMaximum( l_State.m_lMax );
		m_ProgressBar.PutValue( l_State.m_lCurrent );
		m_ProgressBar.PutStatusText( l_State.m_csText );
	}
}

void CXsiProgressBar::Increment()
{
	m_ProgressBar.Increment();

	SProgressBarState& l_State = m_States.top();
	l_State.m_lCurrent = m_ProgressBar.GetValue();
}

void CXsiProgressBar::SetMin( long in_lMin )
{
	m_ProgressBar.PutMinimum( in_lMin );

	SProgressBarState& l_State = m_States.top();
	l_State.m_lMin = m_ProgressBar.GetMinimum();
}

void CXsiProgressBar::SetMax( long in_lMax )
{
	m_ProgressBar.PutMaximum( in_lMax );

	SProgressBarState& l_State = m_States.top();
	l_State.m_lMax = m_ProgressBar.GetMaximum();
}

void CXsiProgressBar::SetValue( long in_lVal )
{
	m_ProgressBar.PutValue( in_lVal );

	SProgressBarState& l_State = m_States.top();
	l_State.m_lCurrent = m_ProgressBar.GetValue();
}

void CXsiProgressBar::SetStatusText( const XSI::CString& in_csText )
{
	m_ProgressBar.PutStatusText( in_csText );

	SProgressBarState& l_State = m_States.top();
	l_State.m_csText = m_ProgressBar.GetStatusText();
}

void CXsiProgressBar::SetCaption( const XSI::CString& in_csText )
{
	m_ProgressBar.PutCaption( in_csText );

	SProgressBarState& l_State = m_States.top();
	l_State.m_csCaption = m_ProgressBar.GetCaption();
}

CXsiProgressBar::CXsiProgressBar()
{
	if ( !m_bInit )
	{
		m_ProgressBar = XSI::Application().GetUIToolkit().GetProgressBar();
		m_bInit = true;
	}
	Push();
}

CXsiProgressBar::~CXsiProgressBar()
{
	Pop();
}

void CXsiProgressBar::SetVisible( bool in_bVisible )
{
	m_ProgressBar.PutVisible( in_bVisible );
}

long CXsiProgressBar::GetMin() const
{
	return m_ProgressBar.GetMinimum();
}

long CXsiProgressBar::GetMax() const
{
	return m_ProgressBar.GetMaximum();
}

long CXsiProgressBar::GetValue() const
{
	return m_ProgressBar.GetValue();
}

float CXsiProgressBar::GetPercent() const
{
	long l_lNum = m_ProgressBar.GetMaximum() - m_ProgressBar.GetMinimum();
	if ( l_lNum > 0 )
	{
		long l_lVal = m_ProgressBar.GetValue() - m_ProgressBar.GetMinimum();
		return 100.0f * ( l_lVal / (float)l_lNum );
	}
	return 0.0f;
}

bool CXsiProgressBar::IsCancelPressed() const
{
	return m_ProgressBar.IsCancelPressed();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// CPolyMeshDefinition
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPolyMeshDefinition::SetPositions( const float* in_pfVals, size_t in_NumPos )
{
	m_vecPositions.resize( in_NumPos * 3 );
	CastedCopy<double, float>( m_vecPositions.data(), in_pfVals, in_NumPos * 3 );
}

void CPolyMeshDefinition::SetPositions( const double* in_pdVals, size_t in_NumPos )
{
	m_vecPositions.resize( in_NumPos * 3 );
	std::copy( in_pdVals, in_pdVals + (in_NumPos * 3), m_vecPositions.begin() );
}

void CPolyMeshDefinition::SetIndicesAndFaceCounts( const LONG* in_plIndices, size_t in_IndexCount, const LONG* in_plFaceCounts, size_t in_FaceCount )
{
	m_vecFaceCounts.resize( in_FaceCount );
	std::copy( in_plFaceCounts, in_plFaceCounts + in_FaceCount, m_vecFaceCounts.begin() );

	m_vecIndices.resize( in_IndexCount );
	ReverseFaceWinding( m_vecIndices.data(), m_vecFaceCounts.data(), m_vecFaceCounts.size(), in_plIndices );
}

void CPolyMeshDefinition::SetNormals( const float* in_pfNormals, size_t in_NumNormals, bool in_bInvertWinding )
{
	m_vecNormals.resize( in_NumNormals * 3 );
	if ( in_bInvertWinding )
	{
		const SRawFloat3* l_pfSrcNormals = (const SRawFloat3*)in_pfNormals;
		SRawFloat3* l_pfReverseNormals = new SRawFloat3[in_NumNormals * 3];
		const std::vector<LONG>& l_vecFaceCounts = GetFaceCountsContainer();
		ReverseFaceWinding( l_pfReverseNormals, l_vecFaceCounts.data(), l_vecFaceCounts.size(), l_pfSrcNormals );
		std::copy( (const float*)l_pfReverseNormals, (const float*)( l_pfReverseNormals + in_NumNormals ), m_vecNormals.begin());
		delete[] l_pfReverseNormals;
	}
	else
	{
		std::copy( (const float*)in_pfNormals, (const float*)( in_pfNormals + in_NumNormals ), m_vecNormals.begin() );
	}
}

void CPolyMeshDefinition::SetUVs( const float* in_pfUVs, size_t in_NumUVs, bool in_bInvertWinding )
{
	SRawFloat2* l_pfReverseUVs = 0;
	bool l_bAlloc = false;
	if ( in_bInvertWinding )
	{
		const SRawFloat2* l_pfSrcUVs = (SRawFloat2*)in_pfUVs;	
		l_pfReverseUVs = new SRawFloat2[in_NumUVs];
		const std::vector<LONG>& l_vecFaceCounts = GetFaceCountsContainer();
		ReverseFaceWinding( l_pfReverseUVs, l_vecFaceCounts.data(), l_vecFaceCounts.size(), l_pfSrcUVs );
		l_bAlloc = true;
	}
	else
	{
		l_pfReverseUVs = (SRawFloat2*)in_pfUVs;
	}

	m_vecUVs.resize( in_NumUVs * 3 );
	// Soft needs UVWs so we add a W component
	for ( size_t i = 0; i < in_NumUVs; i++ )
	{
		m_vecUVs[i * 3 + 0] = l_pfReverseUVs[i].x; 
		m_vecUVs[i * 3 + 1] = l_pfReverseUVs[i].y;
		m_vecUVs[i * 3 + 2] = 0.0f;
	}

	if ( l_bAlloc )
		delete[] l_pfReverseUVs;
}

void CPolyMeshDefinition::SetIndexedNormals( const float* in_pfNormals, size_t in_NumNormals )
{
	const std::vector<LONG>& l_vecIndices = GetIndicesContainer();
	if ( l_vecIndices.size() == 0 )
		return;

	m_vecNormals.resize( l_vecIndices.size() * 3 );

	// Expand the normals array
	for ( size_t i = 0; i < l_vecIndices.size(); i++ )
	{
		size_t l_Index = l_vecIndices[i];
		m_vecNormals[ i * 3 + 0 ] = in_pfNormals[ l_Index * 3 + 0 ];
		m_vecNormals[ i * 3 + 1 ] = in_pfNormals[ l_Index * 3 + 1 ];
		m_vecNormals[ i * 3 + 2 ] = in_pfNormals[ l_Index * 3 + 2 ];
	}
}

void CPolyMeshDefinition::SetIndexedUVs( const float* in_pfUVs, size_t in_NumUVs )
{
	const std::vector<LONG>& l_vecIndices = GetIndicesContainer();
	if ( l_vecIndices.size() == 0 )
		return;

	m_vecUVs.resize( l_vecIndices.size() * 3 );

	// Expand the UV array
	for ( size_t i = 0; i < l_vecIndices.size(); i++ )
	{
		size_t l_Index = l_vecIndices[i];
		m_vecUVs[ i * 3 + 0 ] = in_pfUVs[ l_Index * 2 + 0 ];
		m_vecUVs[ i * 3 + 1 ] = in_pfUVs[ l_Index * 2 + 1 ];
		m_vecUVs[ i * 3 + 2 ] = 0.0f;
	}
}

double* CPolyMeshDefinition::GetPositions()
{
	std::vector<double>& l_vec = GetPositionsContainer();
	return !l_vec.empty() ? (double*)&l_vec[0] : 0;
}

float* CPolyMeshDefinition::GetUVs()
{
	std::vector<float>& l_vec = GetUVsContainer();
	return !l_vec.empty() ? (float*)&l_vec[0] : 0;
}

float* CPolyMeshDefinition::GetNormals()
{
	std::vector<float>& l_vec = GetNormalsContainer();
	return !l_vec.empty() ? (float*)&l_vec[0] : 0;
}

LONG* CPolyMeshDefinition::GetIndices()
{
	std::vector<LONG>& l_vec = GetIndicesContainer();
	return !l_vec.empty() ? (LONG*)&l_vec[0] : 0;
}

LONG* CPolyMeshDefinition::GetFaceCounts()
{
	std::vector<LONG>& l_vec = GetFaceCountsContainer();
	return !l_vec.empty() ? (LONG*)&l_vec[0] : 0;
}

size_t CPolyMeshDefinition::GetPositionsCount()
{
	return GetPositionsContainer().size() / 3;
}

size_t CPolyMeshDefinition::GetUVsCount()
{
	return GetUVsContainer().size() / 3;
}

size_t CPolyMeshDefinition::GetNormalsCount()
{
	return GetNormalsContainer().size() / 3;
}

size_t CPolyMeshDefinition::GetIndicesCount()
{
	return GetIndicesContainer().size();
}

size_t CPolyMeshDefinition::GetFaceCount()
{
	return GetFaceCountsContainer().size();
}

void CPolyMeshDefinition::SetParent( CPolyMeshDefinition* in_pParent )
{
	m_pParentDef = in_pParent;
}

CPolyMeshDefinition* CPolyMeshDefinition::GetParent()
{
	return m_pParentDef;
}

std::vector<double>& CPolyMeshDefinition::GetPositionsContainer()
{
	if ( !m_vecPositions.empty() )
		return m_vecPositions;
	else
		return m_pParentDef ? m_pParentDef->GetPositionsContainer() : m_vecPositions;
}

std::vector<float>& CPolyMeshDefinition::GetUVsContainer()
{
	if ( !m_vecUVs.empty() )
		return m_vecUVs;
	else
		return m_pParentDef ? m_pParentDef->GetUVsContainer() : m_vecUVs;
}

std::vector<float>& CPolyMeshDefinition::GetNormalsContainer()
{
	if ( !m_vecNormals.empty() )
		return m_vecNormals;
	else
		return m_pParentDef ? m_pParentDef->GetNormalsContainer() : m_vecNormals;
}

std::vector<LONG>& CPolyMeshDefinition::GetIndicesContainer()
{
	if ( !m_vecIndices.empty() )
		return m_vecIndices;
	else
		return m_pParentDef ? m_pParentDef->GetIndicesContainer() : m_vecIndices;
}

std::vector<LONG>& CPolyMeshDefinition::GetFaceCountsContainer()
{
	if ( !m_vecFaceCounts.empty() )
		return m_vecFaceCounts;
	else
		return m_pParentDef ? m_pParentDef->GetFaceCountsContainer() : m_vecFaceCounts;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// CPolyMeshDefinitionCache
//////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CPolyMeshDefinitionCache::GetEntryBySample( long long in_llSample, CPolyMeshDefinition** out_ppDefinition ) const
{
	TCacheMap::const_iterator it = m_Entries.find( in_llSample );
	if ( it != m_Entries.end() )
	{
		if ( out_ppDefinition )
			*out_ppDefinition = it->second;
		return true;
	}

	if ( out_ppDefinition )
		*out_ppDefinition = 0;
	return false;
}

bool CPolyMeshDefinitionCache::GetRootEntry( CPolyMeshDefinition** out_ppDefinition ) const
{
	if ( m_Entries.empty() )
	{
		if ( out_ppDefinition )
			*out_ppDefinition = 0;
		return false;
	}

	if ( out_ppDefinition )
		*out_ppDefinition = m_Entries.begin()->second;
	return true;
}

bool CPolyMeshDefinitionCache::CreateEntry( long long in_llSample, CPolyMeshDefinition** out_ppDefinition )
{
	if ( GetEntryBySample( in_llSample, 0 ) )
		return false;
	
	CPolyMeshDefinition* l_pNewDef = new CPolyMeshDefinition();
	if ( l_pNewDef )
	{
		if ( out_ppDefinition )
			*out_ppDefinition = l_pNewDef;
		m_Entries[in_llSample] = l_pNewDef;
		return true;
	}
	return false;
}

size_t CPolyMeshDefinitionCache::GetNumEntries() const
{
	return m_Entries.size();
}

CPolyMeshDefinitionCache::CPolyMeshDefinitionCache()
{

}

CPolyMeshDefinitionCache::~CPolyMeshDefinitionCache()
{
	for ( TCacheMap::iterator it = m_Entries.begin(); it != m_Entries.end(); ++it )
		delete it->second;
}

bool HasShearing( IAbcIXformSample* in_pSample )
{
	size_t l_NumOps = in_pSample->GetNumOps();

	for ( size_t i = 0; i < l_NumOps; i++ )
	{
		CAbcPtr<IAbcIXformOp> l_spOp;
		if ( in_pSample->GetOp( i, &l_spOp ) == EResult_Success )
		{
			if ( l_spOp->GetType() == EXformOp_Matrix &&
				l_spOp->GetHint() == EMatrixHint_MayaShear )
				 return true;
		}
	}
	return false;
}

void CTransformHelper::AddTranslation( double in_dX, double in_dY, double in_dZ )
{
	MATH::CTransformation l_Transfo;
	l_Transfo.SetTranslationFromValues( in_dX, in_dY, in_dZ );
	m_Transforms.push_back( l_Transfo.GetMatrix4() );
}

void CTransformHelper::AddRotationAxisAngle( double in_dX, double in_dY, double in_dZ, double in_dAngle )
{
	MATH::CTransformation l_Transfo;
	l_Transfo.SetRotationFromAxisAngle( MATH::CVector3( in_dX, in_dY, in_dZ ), MATH::DegreesToRadians( in_dAngle ) );
	m_Transforms.push_back( l_Transfo.GetMatrix4() );
}

void CTransformHelper::AddScaling( double in_dX, double in_dY, double in_dZ )
{
	MATH::CTransformation l_Transfo;
	l_Transfo.SetScalingFromValues( in_dX, in_dY, in_dZ );
	m_Transforms.push_back( l_Transfo.GetMatrix4() );
}

void CTransformHelper::AddRotX( double in_dAngle )
{
	MATH::CTransformation l_Transfo;
	l_Transfo.SetRotationFromAxisAngle( MATH::CVector3( 1.0, 0.0, 0.0 ), MATH::DegreesToRadians( in_dAngle ) );
	m_Transforms.push_back( l_Transfo.GetMatrix4() );
}

void CTransformHelper::AddRotZ( double in_dAngle )
{
	MATH::CTransformation l_Transfo;
	l_Transfo.SetRotationFromAxisAngle( MATH::CVector3( 0.0, 0.0, 1.0 ), MATH::DegreesToRadians( in_dAngle ) );
	m_Transforms.push_back( l_Transfo.GetMatrix4() );
}

void CTransformHelper::AddMatrix( const double* in_dMatrix44 )
{
	MATH::CMatrix4 l_Mat44( in_dMatrix44[0], in_dMatrix44[1], in_dMatrix44[2], in_dMatrix44[3],
		in_dMatrix44[4], in_dMatrix44[5], in_dMatrix44[6], in_dMatrix44[7],
		in_dMatrix44[8], in_dMatrix44[9], in_dMatrix44[10], in_dMatrix44[11],
		in_dMatrix44[12], in_dMatrix44[13], in_dMatrix44[14], in_dMatrix44[15] );
	m_Transforms.push_back( l_Mat44 );
}

MATH::CTransformation CTransformHelper::GetTransform() const
{
	MATH::CTransformation l_Transform;
	MATH::CMatrix4 l_Matrix;
	l_Matrix.SetIdentity();
	for ( std::vector<MATH::CMatrix4>::const_iterator it = m_Transforms.begin(); it != m_Transforms.end(); ++it )
	{
		l_Matrix = l_Matrix.Mul( *it, l_Matrix );
	}
	l_Transform.SetMatrix4( l_Matrix );
	return l_Transform;
}

void CTransformHelper::AddRotY( double in_dAngle )
{
	MATH::CTransformation l_Transfo;
	l_Transfo.SetRotationFromAxisAngle( MATH::CVector3( 0.0, 1.0, 0.0 ), MATH::DegreesToRadians( in_dAngle ) );
	m_Transforms.push_back( l_Transfo.GetMatrix4() );
}

CTransformHelper::CTransformHelper( IAbcIXformSample* in_pSample )
{
	size_t l_NumOps = in_pSample->GetNumOps();
	CTransformHelper l_TransformHelper;

	for ( size_t i = 0; i < l_NumOps; i++ )
	{
		CAbcPtr<IAbcIXformOp> l_spOp;
		if ( in_pSample->GetOp( i, &l_spOp ) == EResult_Success )
		{
			EAbcXformOpType l_XformType = l_spOp->GetType();

			if ( l_XformType == EXformOp_Translate )
			{
				const double* l_pdVal = l_spOp->GetTranslate();
				l_TransformHelper.AddTranslation( l_pdVal[0], l_pdVal[1], l_pdVal[2] );
			}
			else if ( l_XformType == EXformOp_Rotate )
			{
				const double* l_pdVal = l_spOp->GetAxis();
				l_TransformHelper.AddRotationAxisAngle( l_pdVal[0], l_pdVal[1], l_pdVal[2], l_spOp->GetAngle() );
			}
			else if ( l_XformType == EXformOp_RotateX )
			{
				l_TransformHelper.AddRotX( l_spOp->GetXRotation() );
			}
			else if ( l_XformType == EXformOp_RotateY )
			{
				l_TransformHelper.AddRotY( l_spOp->GetYRotation() );
			}
			else if ( l_XformType == EXformOp_RotateZ )
			{
				l_TransformHelper.AddRotZ( l_spOp->GetZRotation() );
			}
			else if ( l_XformType == EXformOp_Scale ) 
			{
				const double* l_pdVal = l_spOp->GetScale();
				l_TransformHelper.AddScaling( l_pdVal[0], l_pdVal[1], l_pdVal[2] );
			}
			else if ( l_XformType == EXformOp_Matrix )
			{
				double l_dMat44[16];
				for ( int i = 0; i < 16; i++ )
					l_dMat44[i] = l_spOp->GetChannelValue( i );
				l_TransformHelper.AddMatrix( l_dMat44 );
				if ( l_spOp->GetHint() == EMatrixHint_MayaShear )
				{
					double l_dXY = l_spOp->GetChannelValue(4);
					double l_dXZ = l_spOp->GetChannelValue(8);
					double l_dYZ = l_spOp->GetChannelValue(9);
				}
			}
		}
	}

}

// Type Info
bool isTypeOf( const GenericTypeInfo* a, const GenericTypeInfo* b )
{
	if ( a == NULL )
		return false;

	if ( a == b )
		return true;
	return isTypeOf( a->m_BaseType, b );
}