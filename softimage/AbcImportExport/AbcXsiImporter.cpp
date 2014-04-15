//*****************************************************************************
/*!
	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*

#include "AbcXsiImporter.h"

#include <xsi_application.h>
#include <xsi_clip.h>
#include <xsi_clusterproperty.h>
#include <xsi_clusterpropertybuilder.h>
#include <xsi_comapihandler.h>
#include <xsi_fcurve.h>
#include <xsi_griddata.h>
#include <xsi_icenode.h>
#include <xsi_kinematics.h>
#include <xsi_meshbuilder.h>
#include <xsi_mixer.h>
#include <xsi_model.h>
#include <xsi_null.h>
#include <xsi_parameter.h>
#include <xsi_polygonmesh.h>
#include <xsi_primitive.h>
#include <xsi_project.h>
#include <xsi_shapekey.h>
#include <xsi_shapeclip.h>
#include <xsi_source.h>
#include <xsi_status.h>
#include <xsi_utils.h>

#include <fstream>
#include <algorithm>
#include <sstream>

using namespace XSI;

CLog g_ImportLog;

enum EGridColumnIds
{
	EItemsGridColumn_SourceFullName = 0,
	EItemsGridColumn_TargetFullName,
	EItemsGridColumn_TargetKineCnx,
	EItemsGridColumn_NbColumns
};

bool isNodeRenderable( AbcImportTreeNode* pNode )
{
	return isNodeA<AbcImportRenderable>( pNode );
}

const char* AbcImportTreeNode::GetName() const
{
	return m_strName.c_str();
}

IAbcIObject* AbcImportTreeNode::GetObject()
{
	return m_spObject.GetPtr();
}

const IAbcIObject* AbcImportTreeNode::GetObject() const
{
	return m_spObject.GetPtr();
}

void AbcImportTreeNode::AddChild( AbcImportTreeNode* pNode )
{
	assert( pNode->m_pParent == NULL );
	m_children.push_back( pNode );
	pNode->m_pParent = this;
}

template<typename T>
T* AbcImportTreeNode::GetParent() const
{
	if ( m_pParent )
		return node_cast<T>( m_pParent );
	else
		return NULL;
}

void AbcImportTreeNode::ReparentChildren( AbcImportTreeNode* pNewParent )
{
	for ( TChildList::iterator it = m_children.begin(); it != m_children.end(); it++ )
	{
		AbcImportTreeNode* l_pChild = *it;
		l_pChild->m_pParent = NULL;
		pNewParent->AddChild( l_pChild );
	}
	m_children.clear();
}

void AbcImportTreeNode::RemoveChild( AbcImportTreeNode* pNode, bool bDelete )
{
	TChildList::iterator it = std::find( m_children.begin(), m_children.end(), pNode );
	if ( it != m_children.end() )
	{
		AbcImportTreeNode* l_pNode = *it;
		l_pNode->m_pParent = NULL;
		m_children.erase( it );
		if ( bDelete )
		{
			delete l_pNode;
		}
	}
}

template<typename T>
T* AbcImportTreeNode::GetChild( size_t index )
{
	return node_cast<T>( m_children[index] );
}

size_t AbcImportTreeNode::GetNumChildren() const
{
	return m_children.size();
}

AbcImportTreeNode::~AbcImportTreeNode()
{
	for ( size_t i = 0; i < m_children.size(); i++ )
		delete m_children[i];
	m_children.clear();
}

void AbcImportTreeNode::SetRef( const XSI::CRef& in_Ref )
{
	m_Ref = in_Ref;
}

XSI::CRef AbcImportTreeNode::GetRef() const
{
	return m_Ref;
}

void AbcImportXform::ResetTransform( AbcImportXform* pNewXform, bool bResetName )
{
	m_spObject = pNewXform->m_spObject;
	if ( bResetName )
		m_strName = pNewXform->m_strName;
}

bool AbcImportXform::IsConstantIdentity() const
{
	return ((IAbcIXform*)m_spObject.GetPtr())->IsConstantIdentity();
}

XSI::MATH::CMatrix4 AbcImportXform::GetGlobalTransform() const
{
	MATH::CMatrix4 l_CurMatrix = m_LocalMat;
	CAbcPtr<IAbcIXformSample> l_spSample;
	if ( ((IAbcIXform*)m_spObject.GetPtr())->GetSample( NULL, &l_spSample ) )
	{
		if ( l_spSample->GetInheritsXforms() == false )
			return l_CurMatrix;
	}

	// Find a transform parent
	AbcImportTreeNode* l_pParent = GetParent<AbcImportTreeNode>();
	while ( l_pParent && !isNodeA<AbcImportXform>( l_pParent ) )
		l_pParent = GetParent<AbcImportTreeNode>();

	if ( l_pParent )
	{
		AbcImportXform* l_pXform = node_cast<AbcImportXform>( l_pParent );
		MATH::CMatrix4 l_Concat;
		l_Concat = l_CurMatrix.Mul( l_CurMatrix, l_pXform->GetGlobalTransform() );
		return l_Concat;
	}
	else
		return l_CurMatrix;
}

void CountImportNodes( AbcImportTreeNode* pRootNode, size_t& out_nChildren )
{
	out_nChildren += pRootNode->GetNumChildren();

	for (size_t i = 0; i < pRootNode->GetNumChildren(); i++)
	{
		CountImportNodes( pRootNode->GetChild<AbcImportTreeNode>(i), out_nChildren );
	}
}

void BuildImportTree( IAbcIObject* pObject, AbcImportTreeNode* pImportNode )
{
	size_t numChildren = pObject->GetNumChildren();

	for ( size_t i = 0; i < numChildren; i++ )
	{
		CAbcPtr<IAbcIObject> spObj;
		pObject->GetChild( i, &spObj );
		AbcImportTreeNode* pNewNode = NULL;
		if ( spObj->GetType() == EIObject_Xform )
		{
			CAbcPtr<IAbcIXform> spXfo;
			spObj->TransformInto( EIObject_Xform, (IAbcIObject**)&spXfo );

			CAbcPtr<IAbcICompoundPropertyAccessor> l_spPropAccessor;
			spXfo->GetPropertyAccessor( &l_spPropAccessor );

			CAbcPtr<IAbcIPropertyAccessor> l_spLocatorProp;
			if ( l_spPropAccessor->GetChildProperty( "locator", &l_spLocatorProp ) == EResult_Success )
			{
				pNewNode = new AbcImportLocator( spXfo );
			}
			else
			{
				pNewNode = new AbcImportXform( spXfo );
			}
		}
		else if ( spObj->GetType() == EIObject_Polymesh )
		{
			CAbcPtr<IAbcIPolyMesh> spPoly;
			spObj->TransformInto( EIObject_Polymesh, (IAbcIObject**)&spPoly );
			pNewNode = new AbcImportPolymesh( spPoly );
		}
		else if ( spObj->GetType() == EIObject_FaceSet )
		{
			CAbcPtr<IAbcIFaceSet> spFaceSet;
			spObj->TransformInto( EIObject_FaceSet, (IAbcIObject**)&spFaceSet );
			pNewNode = new AbcImportFaceSet( spFaceSet );
		}
		else if ( spObj->GetType() == EIObject_Points )
		{
			CAbcPtr<IAbcIPoints> spPoints;
			spObj->TransformInto( EIObject_Points, (IAbcIObject**)&spPoints );
			pNewNode = new AbcImportPointCloud( spPoints );
		}
		else if( spObj->GetType() == EIObject_Camera )
		{
			CAbcPtr<IAbcICamera> spCamera;
			spObj->TransformInto( EIObject_Camera, (IAbcIObject**)&spCamera );
			pNewNode = new AbcImportCamera( spCamera );
		}
		if ( pNewNode != NULL )
		{
			pImportNode->AddChild( pNewNode );
			BuildImportTree( spObj, pNewNode );
		}
	}
}

void SimplifyImportTree( AbcImportTreeNode* pRoot )
{
	std::vector<AbcImportTreeNode*> l_vecNodeList;
	l_vecNodeList.push_back( pRoot );

	while ( !l_vecNodeList.empty() )
	{
		AbcImportTreeNode* pCurrentNode = l_vecNodeList.back();
		l_vecNodeList.pop_back();

		for ( size_t i = 0; i < pCurrentNode->GetNumChildren(); i++ )
			l_vecNodeList.push_back( pCurrentNode->GetChild<AbcImportTreeNode>( i ) );

		if ( isNodeA<AbcImportXform>(pCurrentNode) )
		{
			AbcImportXform* l_pXform = node_cast<AbcImportXform>( pCurrentNode );
			size_t numChildren = l_pXform->GetNumChildren();

			if ( numChildren == 0 )
			{
				// Remove this node
				if ( pCurrentNode->GetParent<AbcImportTreeNode>() )
				{
					pCurrentNode->GetParent<AbcImportTreeNode>()->RemoveChild( pCurrentNode, true );					
				}
			}
			else
			{
				AbcImportRenderable* l_pRenderable = NULL;
				size_t numRenderables = 0;
				size_t numTransforms = 0;

				for ( size_t i = 0; i < numChildren; i++ )
				{
					AbcImportTreeNode* l_pNode = l_pXform->GetChild<AbcImportTreeNode>( i );
					if ( isNodeRenderable( l_pNode ) )
					{
						l_pRenderable = node_cast<AbcImportRenderable>( l_pNode );
						numRenderables++;
					}
					else if ( isNodeA<AbcImportXform>( l_pNode ) )
					{
						numTransforms++;
					}
				}

				AbcImportTreeNode* l_pParent = l_pXform->GetParent<AbcImportTreeNode>();
				assert( l_pParent );

				if ( l_pParent == pRoot && numChildren == 1 && numTransforms == 1 )
				{
					AbcImportXform* l_pChildXform = l_pXform->GetChild<AbcImportXform>( 0 );
					if ( l_pChildXform->IsConstantIdentity() )
					{
						l_pChildXform->ResetTransform( l_pXform, false );
						// Take over the previous transform
						l_pXform->RemoveChild( l_pChildXform, false );
						l_pParent->AddChild( l_pChildXform );
						l_pXform->ReparentChildren( l_pChildXform );
						l_pParent->RemoveChild( l_pXform, true );
					}
					else if ( l_pXform->IsConstantIdentity() )
					{
						l_pXform->ResetTransform( l_pChildXform, true );
						l_pChildXform->ReparentChildren( l_pXform );
						l_vecNodeList.erase( std::find( l_vecNodeList.begin(), l_vecNodeList.end(), l_pChildXform ) );						
						l_pXform->RemoveChild( l_pChildXform, true );

						// We now have a different hierarchy so we should reprocess this node
						l_vecNodeList.push_back( l_pXform );
					}
				}
				else if ( numRenderables == 1 && l_pRenderable->m_pXform == NULL )
				{
					// We take over the transform
					// Remove the the renderable from the transform's children
					l_pXform->RemoveChild( l_pRenderable, false );
					// Add the renderable to the parent node
					l_pParent->AddChild( l_pRenderable );
					// Reparent the Xform's children to the renderable
					l_pXform->ReparentChildren( l_pRenderable );
					// Remove the Xform from the parent and keep it within the renderable instead
					l_pParent->RemoveChild( l_pXform, false );
					l_pRenderable->m_pXform = l_pXform;						
				}
			}
		}

	}
}

SScenePlaybackSettings::SScenePlaybackSettings()
{
	// PlayControl.In, GlobalIn, LoopIn
	// PlayControl.Out, GlobalOut, LoopOut
	// PlayControl.Loop
	// PlayControl.Current
	// PlayControl.Key
	// PlayControl.Step
	// PlayControl.Rate
	Property l_PlayControl = Application( ).GetActiveProject().GetProperties().GetItem( L"Play Control" );
	if ( l_PlayControl.IsValid() )
	{
		m_dFrameIn		= (double)l_PlayControl.GetParameterValue("In");
		m_dFrameOut		= (double)l_PlayControl.GetParameterValue("Out");
		m_dGlobalIn		= (double)l_PlayControl.GetParameterValue("GlobalIn");
		m_dGlobalOut	= (double)l_PlayControl.GetParameterValue("GlobalOut");
		m_dFrameRate	= (double)l_PlayControl.GetParameterValue("Rate");
	}
	else
	{
		m_dFrameIn		= 0;
		m_dFrameOut		= 0;
		m_dGlobalIn		= 0;
		m_dGlobalOut	= 0;
		m_dFrameRate	= 0.0;
	}
}

void SScenePlaybackSettings::Apply()
{
	Property l_PlayControl = Application( ).GetActiveProject().GetProperties().GetItem( L"Play Control" );
	if ( l_PlayControl.IsValid() )
	{
		l_PlayControl.PutParameterValue( "In", (double)m_dFrameIn );
		l_PlayControl.PutParameterValue( "Out", (double)m_dFrameOut );
		l_PlayControl.PutParameterValue( "GlobalIn", (double)m_dGlobalIn );
		l_PlayControl.PutParameterValue( "GlobalOut", (double)m_dGlobalOut );
		//l_PlayControl.PutParameterValue( "Rate", (LONG)m_dFrameRate );
	}
}

double SScenePlaybackSettings::TimeToFrame( double in_dTime )
{
	return in_dTime * m_dFrameRate;
}

double SScenePlaybackSettings::FrameToTime( double in_dFrame )
{
	return m_dFrameRate > 0.0 ? in_dFrame / m_dFrameRate : 0.0;
}


void GetTimeRange( IAbcTimeSampling* in_pTimeSampling, size_t in_NumSamples, double in_dTimeStart, double in_dTimeEnd, STimeRange& out_Range )
{
	double l_dMinTime = 0.0;
	double l_dMaxTime = 0.0;

	long long l_llTempSample;

	if ( in_dTimeStart == -1.0 )
		in_dTimeStart = l_dMinTime = in_pTimeSampling->GetTimeAt( 0 );
	else
		in_pTimeSampling->GetFloorIndex( in_dTimeStart, (long long)in_NumSamples, l_llTempSample, l_dMinTime );

	if ( in_dTimeEnd == -1.0 )
		in_dTimeEnd = l_dMaxTime = in_pTimeSampling->GetTimeAt( (long long)in_NumSamples - 1 );
	else
		in_pTimeSampling->GetCeilIndex( in_dTimeEnd, (long long)in_NumSamples, l_llTempSample, l_dMaxTime );

	// Clamp our start and end times to the time range in the IPolyMesh
 	out_Range.m_dStart = std::min<double>( std::max<double>( l_dMinTime, in_dTimeStart ), l_dMaxTime );
	out_Range.m_dEnd = std::max<double>( std::min<double>( l_dMaxTime, in_dTimeEnd ), l_dMinTime );
}

AbcXsiImporter::AbcXsiImporter()
	: AbcXsiIO(), 
	m_spIArchive(0), 
	m_bAnimated( false ), 
	m_dStartTime( -1.0 ), 
	m_dEndTime( -1.0 ), 
	m_dMinTime( DBL_MAX ), 
	m_dMaxTime( DBL_MIN ), 
	m_bPadShapeClips( false ), 
	m_eTimeRangeFitting( ETimeRange_Expand ), 
	m_eAnimationImportMode( EImport_CacheOnFile ),
	m_bCreateRootModel( false ),
	m_bCreateICETreeMode( EImport_CreateICETreePerObj )
{
}

XSI::CStatus AbcXsiImporter::SetFilename( const XSI::CString& in_csFileName )
{
	if ( GetFrameworkPtr() == 0 )
	{
		return CStatus::Fail;
	}
	// Make sure the path exists
	{
		std::ifstream l_Stream( in_csFileName.GetAsciiString() );
		if ( !l_Stream )
		{
			g_ImportLog.Log( true, siErrorMsg, "Cannot open '%s'", in_csFileName.GetAsciiString() );
			return CStatus::Fail;
		}
	}

	ULONG l_ulStartPos =  in_csFileName.ReverseFindString( CUtils::Slash() );
	if ( l_ulStartPos == UINT_MAX )
		l_ulStartPos = 0;
	else
	{
		l_ulStartPos++;
		if ( l_ulStartPos == in_csFileName.Length() )
			return CStatus::Fail;
	}
	ULONG l_ulTrimPos =  in_csFileName.ReverseFindString( L"." );
	if ( l_ulTrimPos == UINT_MAX || l_ulStartPos > l_ulTrimPos )
		l_ulTrimPos = in_csFileName.Length();

	if ( l_ulStartPos < l_ulTrimPos )
	{
		m_csModelName = in_csFileName.GetSubString( l_ulStartPos, l_ulTrimPos - l_ulStartPos );
		if ( GetFrameworkPtr()->OpenIArchive( in_csFileName.GetAsciiString(), &m_spIArchive ) == EResult_Success )
		{
			m_csFileName = in_csFileName;
			return CStatus::OK;
		}
	}
	return CStatus::Fail;
}

CStatus AbcXsiImporter::ImportObjects( )
{
	if ( GetFrameworkPtr() == 0 || m_spIArchive == 0)
	{
		return CStatus::Fail;
	}

	CAbcPtr<IAbcIObject> l_spObject;
	if ( m_spIArchive->GetTop( &l_spObject ) == EResult_Success )
	{
		// Get the time range
		{
			double l_dStartTime, l_dEndTime;
			m_spIArchive->GetArchiveStartAndEndTime( &l_dStartTime, &l_dEndTime );
			// For scenes with no samples
			if ( l_dStartTime != DBL_MAX && l_dEndTime != -DBL_MAX )
			{
				long l_lMinFrame = (long)m_PlaybackSettings.TimeToFrame( l_dStartTime );
				long l_lMaxFrame = (long)( m_PlaybackSettings.TimeToFrame( l_dEndTime ) + 0.5 );

				// Playback settings shouldn't have the same min and max frames
				if ( l_lMaxFrame == l_lMinFrame )
					l_lMaxFrame++;

				g_ImportLog.Log( false, siVerboseMsg, "Alembic File Time Range: %f to %f", l_dStartTime, l_dEndTime );
				if ( m_eTimeRangeFitting == ETimeRange_Fit )
				{
					m_PlaybackSettings.m_dFrameIn = m_PlaybackSettings.m_dGlobalIn = l_lMinFrame;
					m_PlaybackSettings.m_dFrameOut = m_PlaybackSettings.m_dGlobalOut = l_lMaxFrame;
					m_PlaybackSettings.Apply();
				}
				else if ( m_eTimeRangeFitting == ETimeRange_Expand )
				{
					m_PlaybackSettings.m_dGlobalIn = m_PlaybackSettings.m_dFrameIn = std::min<double>( m_PlaybackSettings.m_dGlobalIn, (double)l_lMinFrame );
					m_PlaybackSettings.m_dGlobalOut = m_PlaybackSettings.m_dFrameOut = std::max<double>( m_PlaybackSettings.m_dGlobalOut, (double)l_lMaxFrame );
					m_PlaybackSettings.Apply();
				}
			}
		}

		std::auto_ptr<AbcImportTreeNode> pTop( new AbcImportTreeNode( l_spObject ) );
		// Build the initial tree
		BuildImportTree( l_spObject, pTop.get() );
		// Simplify for Softimage
		SimplifyImportTree( pTop.get() );

		size_t l_nImportNodes = 0;
		CountImportNodes( pTop.get(), l_nImportNodes );

		// Setup our progress bar
		CXsiProgressBar l_ProgressBar;
		m_ProgressBar.SetMin( 0 );
		m_ProgressBar.SetMax( (long)l_nImportNodes );
		m_ProgressBar.SetValue( 0 );
		m_ProgressBar.SetVisible( true );
		// Process the tree 
		AbcImportTreeTraverser<AbcXsiImporter> l_Traverser;
		l_Traverser.Traverse( *this, pTop.get() );
		CreateRootCachedICETree( pTop.get(), m_csFileName );
		m_ProgressBar.SetVisible( false );
	}
	return CStatus::OK;
}

AbcXsiImporter::~AbcXsiImporter()
{
}

void AbcXsiImporter::OnPreImportNode( AbcImportTreeNode* in_pNode )
{
	g_ImportLog.Log( true, siVerboseMsg, "Reading object '%s', Schema = '%s'", in_pNode->GetObject()->GetName(), GetSchemaString( in_pNode->GetObject() ).GetAsciiString() );
	g_ImportLog.Indent();
	
	CString l_csCaption = L"Importing ";
	l_csCaption += in_pNode->GetName();

	std::stringstream l_ss;
	l_ss << (int)(m_ProgressBar.GetPercent()) << "%";
	m_ProgressBar.SetStatusText( l_ss.str().c_str() );
	m_ProgressBar.SetCaption( l_csCaption );
}

void AbcXsiImporter::OnPostImportNode( AbcImportTreeNode* in_pNode )
{
	g_ImportLog.UnIndent();
	m_ProgressBar.Increment();
}

XSI::CStatus AbcXsiImporter::OnImportNode( AbcImportTreeNode* in_pNode )
{
	if ( in_pNode->GetParent<AbcImportTreeNode>() == NULL )
	{
		return ProcessRoot( in_pNode );
	}
	else if ( isNodeA<AbcImportPolymesh>( in_pNode ) )
	{
		return ProcessPolymesh( node_cast<AbcImportPolymesh>( in_pNode ) );
	}
	else if ( isNodeA<AbcImportFaceSet>( in_pNode ) )
	{
		return ProcessFaceSet( node_cast<AbcImportFaceSet>( in_pNode ) );
	}
	else if ( isNodeA<AbcImportPointCloud>( in_pNode ) )
	{
		return ProcessPointCloud( node_cast<AbcImportPointCloud>( in_pNode ) );
	}
	else if ( isNodeA<AbcImportLocator>( in_pNode ) )
	{
		return ProcessLocator( node_cast<AbcImportLocator>( in_pNode ) );
	}
	else if ( isNodeA<AbcImportXform>( in_pNode ) )
	{
		return ProcessTransform( node_cast<AbcImportXform>( in_pNode ), true );
	}
	else if ( isNodeA<AbcImportCamera>( in_pNode ) )
	{
		return ProcessCamera( node_cast<AbcImportCamera>( in_pNode ) );
	}
	else
	{
		g_ImportLog.Log( true, siVerboseMsg, "Object %s is an unhandled type", in_pNode->GetObject()->GetName() );
	}
	return CStatus::OK;
}

XSI::CStatus AbcXsiImporter::CreatePolyMesh( const XSI::CString& in_csPolyMeshName, CPolyMeshDefinition& in_rPolymeshDef, XSI::X3DObject& in_x3dParentObj, XSI::X3DObject& in_x3dObj, AbcImportPolymesh* in_pNode )
{
	CMeshBuilder l_MeshBuilder;
	
	in_x3dParentObj.AddPolygonMesh( in_csPolyMeshName, in_x3dObj, l_MeshBuilder );
	
	// Only apply transform if we are holding the transform
	if ( in_pNode->m_pXform != NULL )
	{
		ProcessTransform( in_pNode->m_pXform, false );
		ApplyTransform( in_x3dObj, in_pNode->m_pXform );
	}
	else
	{
		MATH::CTransformation l_Transform;
		l_Transform.SetIdentity();
		in_x3dObj.GetKinematics().GetLocal().PutTransform( l_Transform );
	}

	CAbcPtr<IAbcIPolyMesh> l_spIAbcPolyMesh;
	in_pNode->GetObject()->TransformInto( EIObject_Polymesh, (IAbcIObject**)&l_spIAbcPolyMesh );
	if ( l_spIAbcPolyMesh->GetTopologyVariance() == ETopoVariance_Heterogenous )
	{
		// Leave the mesh empty. The topology will be created by AlembicCacheNode
		return CStatus::OK;
	}

	l_MeshBuilder.AddVertices( (LONG)in_rPolymeshDef.GetPositionsCount(), in_rPolymeshDef.GetPositions() );
	l_MeshBuilder.AddPolygons( (LONG)in_rPolymeshDef.GetFaceCount(), in_rPolymeshDef.GetFaceCounts(), in_rPolymeshDef.GetIndices() );
	l_MeshBuilder.Build( false );
	
	PolygonMesh l_PolyMesh = in_x3dObj.GetActivePrimitive().GetGeometry();

	if ( l_PolyMesh.IsValid() )
	{
		CClusterPropertyBuilder l_ClusterPropBuilder = l_PolyMesh.GetClusterPropertyBuilder();

		if ( in_rPolymeshDef.GetUVsCount() > 0 )
		{
			ClusterProperty l_UvProp = l_ClusterPropBuilder.AddUV();
			l_UvProp.SetValues( in_rPolymeshDef.GetUVs(), (LONG)in_rPolymeshDef.GetUVsCount() );
		}
		if ( in_rPolymeshDef.GetNormalsCount() > 0 )
		{
			ClusterProperty l_NormalProp = l_ClusterPropBuilder.AddUserNormal();
			l_NormalProp.SetValues( in_rPolymeshDef.GetNormals(), (LONG)in_rPolymeshDef.GetNormalsCount() );
		}
		return CStatus::OK;
	}
	else
		return CStatus::Fail;
}

XSI::CRef AbcXsiImporter::CreatePolyMeshShapeKey( const XSI::CString& in_csShapeKeyName, double in_dFrame, double in_dFrequency, long in_lNumKeys, CPolyMeshDefinition& in_rPolymeshDef, X3DObject& in_x3dObj )
{
	PolygonMesh l_PolyMesh = in_x3dObj.GetActivePrimitive().GetGeometry();
	if ( l_PolyMesh.IsValid() )
	{
		CClusterPropertyBuilder l_ClusterPropBuilder = l_PolyMesh.GetClusterPropertyBuilder();
		ShapeKey l_ShapeKey = l_ClusterPropBuilder.AddShapeKey( siShapeAbsoluteReferenceMode, true );
		if ( l_ShapeKey.IsValid() )
		{
			l_ShapeKey.PutName( in_csShapeKeyName );
			l_ShapeKey.ReserveNumKeys( in_lNumKeys );
			l_ShapeKey.PutFrequency( 1.0 / in_dFrequency );
			CRef l_refKey( l_ShapeKey );
			if ( UpdatePolyMeshShapeKey( l_refKey, 0, in_rPolymeshDef ) == CStatus::OK )
			{
				return l_refKey; 
			}
		}
	}
	return CRef();
}

XSI::CStatus AbcXsiImporter::UpdatePolyMeshShapeKey( CRef& in_refKey, long in_lKeyIndex, CPolyMeshDefinition& in_rPolymeshDef )
{
	ShapeKey l_ShapeKey( in_refKey );

	if ( l_ShapeKey.IsValid() )
	{
		std::vector<LONG> l_vecIndices;
		l_vecIndices.resize( in_rPolymeshDef.GetPositionsCount() );
		for ( size_t i = 0; i < in_rPolymeshDef.GetPositionsCount(); i++ )
			l_vecIndices[i] = (LONG)i;

		std::vector<float> l_vecPositions;
		l_vecPositions.resize( in_rPolymeshDef.GetPositionsCount() * 3 );
		CastedCopy( &l_vecPositions[0], in_rPolymeshDef.GetPositions(), l_vecPositions.size() );
		return l_ShapeKey.SetValuesByKeyIndex( &l_vecIndices[0], &l_vecPositions[0], (LONG)l_vecIndices.size(), in_lKeyIndex );
	}
	return CStatus::Fail;
}

XSI::CStatus AbcXsiImporter::CachePolyMesh( IAbcISampleSelector* in_pSampleSelector, IAbcIPolyMesh* in_pPolyMesh, bool in_bRoot, CPolyMeshDefinitionCache& in_Cache, CPolyMeshDefinition* io_pDefinition )
{
	
	CAbcPtr<IAbcSampleBuffer> l_spPositions;
	CAbcPtr<IAbcSampleBuffer> l_spNormals;
	CAbcPtr<IAbcSampleBuffer> l_spUVs;

	if ( in_bRoot )
	{
		CAbcPtr<IAbcSampleBuffer> l_spFaceCounts;
		CAbcPtr<IAbcSampleBuffer> l_spFaceIndices;
		in_pPolyMesh->GetFaceCounts( in_pSampleSelector, &l_spFaceCounts );
		in_pPolyMesh->GetFaceIndices( in_pSampleSelector, &l_spFaceIndices );
		io_pDefinition->SetIndicesAndFaceCounts( (LONG*)l_spFaceIndices->GetBuffer(), l_spFaceIndices->GetNumElements(), (LONG*)l_spFaceCounts->GetBuffer(), l_spFaceCounts->GetNumElements() );
	}
	else
	{
		CPolyMeshDefinition* l_pParentDef = 0;
		if ( in_Cache.GetRootEntry( &l_pParentDef ) )
		{
			// We should use the face counts and indices of the root entry since topo doesn't change
			io_pDefinition->SetParent( l_pParentDef );
		}
		else
		{
			assert( false && "Invalid cache state" );
		}
	}

	in_pPolyMesh->GetPositions( in_pSampleSelector, &l_spPositions );
	in_pPolyMesh->GetNormals( true, in_pSampleSelector, &l_spNormals );
	in_pPolyMesh->GetUVs( true, in_pSampleSelector, &l_spUVs );

	io_pDefinition->SetPositions( (float*)l_spPositions->GetBuffer(), l_spPositions->GetNumElements() );
					
	if ( l_spNormals )
	{
		EAbcGeomScope l_eGeomScope = l_spNormals->GetGeomScope();
		if ( l_eGeomScope == EGeomScope_FaceVarying )
			io_pDefinition->SetNormals( (float*)l_spNormals->GetBuffer(), l_spNormals->GetNumElements() );
		else if ( l_eGeomScope == EGeomScope_Vertex )
			io_pDefinition->SetIndexedNormals( (float*)l_spNormals->GetBuffer(), l_spNormals->GetNumElements() );
	}				
	if ( l_spUVs )
	{
		EAbcGeomScope l_eGeomScope = l_spUVs->GetGeomScope();
		if ( l_eGeomScope == EGeomScope_FaceVarying )
			io_pDefinition->SetUVs( (float*)l_spUVs->GetBuffer(), l_spUVs->GetNumElements() );
		else
			io_pDefinition->SetIndexedUVs( (float*)l_spUVs->GetBuffer(), l_spUVs->GetNumElements() );
	}

	return CStatus::OK;
}

XSI::CStatus AbcXsiImporter::ProcessRoot( AbcImportTreeNode* in_pNode )
{
	Model l_SceneRoot = Application().GetActiveSceneRoot();	
	if( m_bCreateRootModel )
	{
		Model l_AbcRoot;
		CRefArray l_EmptyRefArray;
		if( m_objRootParent.IsValid() )
		{
			m_objRootParent.AddModel( l_EmptyRefArray, m_csModelName, l_AbcRoot );
		}
		else
			l_SceneRoot.AddModel( l_EmptyRefArray, m_csModelName, l_AbcRoot );
		in_pNode->SetRef( l_AbcRoot );
		m_refNewRootModel = l_AbcRoot.GetRef();
	}
	else
	{
		if( m_objRootParent.IsValid() )
		{
			// Import the objects underneath the specified parent.
			in_pNode->SetRef( m_objRootParent.GetRef() );
		}
		else
		{
			// By default, parent is scene root.
			in_pNode->SetRef( l_SceneRoot.GetRef() );
		}
	}
	return CStatus::OK;
}

XSI::CStatus AbcXsiImporter::ProcessFaceSet( AbcImportFaceSet* in_pNode )
{
	AbcImportPolymesh* l_pParent = in_pNode->GetParent<AbcImportPolymesh>();
	if ( l_pParent )
	{
		CRef l_refMesh = l_pParent->GetRef();
		X3DObject l_obj( l_refMesh );
	
		if ( l_obj.IsValid() )
		{

			PolygonMesh l_PolyMesh = l_obj.GetActivePrimitive().GetGeometry();

			if ( l_PolyMesh.IsValid() )
			{
				CAbcPtr<IAbcISampleSelector> l_spSampleSelector;
				if ( m_spIArchive->CreateSampleSelector( &l_spSampleSelector ) == EResult_Success )
				{
					IAbcIFaceSet* l_pFaceSet = (IAbcIFaceSet*)in_pNode->GetObject();
					CAbcPtr<IAbcSampleBuffer> l_spBuffer;
					if ( l_pFaceSet->GetSample( l_spSampleSelector, &l_spBuffer ) == EResult_Success )
					{
						CString l_strClusterName = in_pNode->GetName();

						if ( l_spBuffer->GetDataType().m_eType == EPodType_Int32 && l_spBuffer->GetDataType().m_ucExtent == 1 )
						{
							CLongArray l_clusterIndices;
							LONG l_numElems = (LONG)l_spBuffer->GetNumElements();
							l_clusterIndices.Resize( l_numElems );
							const int* l_pIndices = (const int*)l_spBuffer->GetBuffer();
							for ( LONG i = 0; i < l_numElems; i++ )
							{
								l_clusterIndices[i] = (LONG)l_pIndices[i];
							}
							XSI::Cluster l_newCluster;
							if ( l_PolyMesh.AddCluster( siPolygonCluster, l_strClusterName, l_clusterIndices, l_newCluster ) == CStatus::OK )
							{
								in_pNode->SetRef( l_newCluster );
								if ( !l_pFaceSet->IsConstant() )
								{
									// TODO: Faceset animation?
								}
								return CStatus::OK;
							}
						}
					}
				}
			}
		}
	}
	return CStatus::Fail;
}

XSI::CStatus AbcXsiImporter::ProcessPolymesh( AbcImportPolymesh* in_pNode )
{
	CStatus l_Result = CStatus::Fail;

	IAbcIPolyMesh* l_pPolyMesh = (IAbcIPolyMesh*)in_pNode->GetObject();
	const char* l_szName = l_pPolyMesh->GetName();
	CAbcPtr<IAbcISampleSelector> l_spSampleSelector;

	if ( m_spIArchive->CreateSampleSelector( &l_spSampleSelector ) == EResult_Success )
	{
		bool l_bImportAnimation = m_bAnimated;

		CAbcPtr<IAbcTimeSampling> l_spTimeSampling;
		if ( l_pPolyMesh->GetTimeSampling( &l_spTimeSampling ) != EResult_Success )
			return CStatus::Fail;
		

		// Calculate time bounds
		double l_dStart = 0.0;
		double l_dEnd = 0.0;

		if ( !l_bImportAnimation )
		{
			// Start from the specified time even if not importing animation
			l_dStart = l_dEnd = std::max<double>( m_dStartTime, 0.0 );
		}
		else
		{
			l_dStart = m_dStartTime;
			l_dEnd = m_dEndTime;
		}

		// Check what is the start/end time in the source IPolyMesh
		size_t l_NumSamples = l_pPolyMesh->GetNumSamples();
		
		STimeRange l_TimeRange;
		GetTimeRange( l_spTimeSampling, l_NumSamples, l_dStart, l_dEnd, l_TimeRange );
		l_dStart = l_TimeRange.m_dStart;
		l_dEnd = l_TimeRange.m_dEnd;

		// Get Softimage Frame Range based from these times
		long l_lMinFrame = ( long )( m_PlaybackSettings.TimeToFrame( l_dStart ) );
		long l_lMaxFrame = l_dStart != l_dEnd ? ( long )( m_PlaybackSettings.TimeToFrame( l_dEnd ) + 0.5 ) : l_lMinFrame;
		double l_dSampling = m_PlaybackSettings.m_dFrameRate > 0.0 ? 1.0 / m_PlaybackSettings.m_dFrameRate : 0.33;

		// Setup Source objects
		CRef l_refKey;

		CRef l_refRoot = in_pNode->GetParent<AbcImportTreeNode>()->GetRef();
		X3DObject l_CurrentRoot( l_refRoot );

		CString l_csName = in_pNode->m_pXform ? in_pNode->m_pXform->GetName() : l_szName;

		X3DObject l_NewObj;

		// These are constant since we are not supporting changes in topo
		CPolyMeshDefinitionCache l_PolyMeshCache;

		for ( long i = l_lMinFrame; i <= l_lMaxFrame; i++ ) 
		{
			double l_dSampleTime = m_PlaybackSettings.FrameToTime( i );
			double l_dSoftimageTime = l_dSampleTime;
			double l_dLerp = 0.0;

 			bool l_bFirstSample = ( i == l_lMinFrame );
			long long l_llSourceSample = 0;
			long long l_llDestSample = 0;
			double l_dSourceTime = 0.0;
			double l_dDestTime = 0.0;
			l_spTimeSampling->GetFloorIndex( l_dSampleTime, (long long)l_NumSamples, l_llSourceSample, l_dSourceTime );
			l_spTimeSampling->GetCeilIndex( l_dSampleTime, (long long)l_NumSamples, l_llDestSample, l_dDestTime );

			CPolyMeshDefinition* l_pSourceDefinition = 0;
			
			// Check if we have the source sample in the cache
			bool l_bSourceCached = l_PolyMeshCache.GetEntryBySample( l_llSourceSample, &l_pSourceDefinition );
			
			if ( !l_bSourceCached  )
			{
				l_PolyMeshCache.CreateEntry( l_llSourceSample, &l_pSourceDefinition );
			}
			
			if ( l_llSourceSample != l_llDestSample )
			{
				if ( fabs( l_dDestTime - l_dSourceTime ) <= 0.0001 )
					l_dLerp = 0.0;
				else
					l_dLerp = ( l_dSampleTime - l_dSourceTime ) / ( l_dDestTime - l_dSourceTime );
			}

			// Cache the source sample if it isn't yet
			if ( !l_bSourceCached )
			{
				l_spSampleSelector->SetRequestedIndex( l_llSourceSample );
				CachePolyMesh( l_spSampleSelector, l_pPolyMesh, l_bFirstSample, l_PolyMeshCache, l_pSourceDefinition );
			}

			// Make sure we don't work on an empty mesh
			if ( (l_pSourceDefinition->GetPositions() && l_pSourceDefinition->GetFaceCounts() && l_pSourceDefinition->GetIndices()) || 
				l_pPolyMesh->GetTopologyVariance() == ETopoVariance_Heterogenous )
			{
				size_t l_NumVerticesToCopy = l_pSourceDefinition->GetPositionsCount() * 3;

				double* l_pdPositions = 0;

				// If we need to interpolate, get the positions of the destination sample and interpolate
				if ( l_dLerp != 0.0 )
				{
					CPolyMeshDefinition* l_pDestDefinition = 0;
					bool l_bDestCached = l_PolyMeshCache.GetEntryBySample( l_llDestSample, &l_pDestDefinition );

					if ( !l_bDestCached )
					{
						l_spSampleSelector->SetRequestedIndex( l_llDestSample );
						l_PolyMeshCache.CreateEntry( l_llDestSample, &l_pDestDefinition );
						CachePolyMesh( l_spSampleSelector, l_pPolyMesh, false, l_PolyMeshCache, l_pDestDefinition );
					}

					l_pdPositions = new double[l_NumVerticesToCopy];
					LerpArray( l_pdPositions, l_pSourceDefinition->GetPositions(), l_pDestDefinition->GetPositions(), l_dLerp, l_NumVerticesToCopy );
				}
	
				CPolyMeshDefinition l_MeshDef;
				l_MeshDef.SetParent( l_pSourceDefinition );

				// Here we only set the positions if we actually interpolated, otherwise, the parent's definition will be used instead.
				if ( l_pdPositions != 0 )
				{
					l_MeshDef.SetPositions( l_pdPositions, l_pSourceDefinition->GetPositionsCount() );
					delete[] l_pdPositions;
				}

				if ( l_bFirstSample )
				{
					l_Result = CreatePolyMesh( l_csName, l_MeshDef, l_CurrentRoot, l_NewObj, in_pNode );
					in_pNode->SetRef( l_NewObj );
					if ( l_bImportAnimation )
					{
						if ( m_eAnimationImportMode == EImport_CacheOnFile )
						{
							CAbcPtr<IAbcIPolyMesh> l_spIAbcPolyMesh;
							in_pNode->GetObject()->TransformInto( EIObject_Polymesh, (IAbcIObject**)&l_spIAbcPolyMesh );
							
							bool l_bCreateKineCnx = false;
							if ( in_pNode->m_pXform )
							{
								CAbcPtr<IAbcIXform> l_spIAbcXfm;
								if ( in_pNode->m_pXform->GetObject()->TransformInto( EIObject_Xform, (IAbcIObject**)&l_spIAbcXfm ) == EResult_Success )
									l_bCreateKineCnx = !l_spIAbcXfm->IsConstant();
							}

							if( m_bCreateICETreeMode == EImport_CreateICETreePerObj )
								l_Result = CreateCachedICETree( l_NewObj, m_csFileName, l_pPolyMesh, l_bCreateKineCnx );
							else
								AddObjectMapping( l_NewObj.GetRef(), l_pPolyMesh, l_bCreateKineCnx );	
							break;
						}
						else
						{
							// Also create our shape key if we are animated
							if ( l_Result == CStatus::OK && l_lMinFrame != l_lMaxFrame )
							{
								CString l_csShapeName = l_csName + CString("_ShapeKey");
					
								l_refKey = CreatePolyMeshShapeKey( l_csShapeName, i, l_dSampling, (LONG)( l_lMaxFrame - l_lMinFrame + 1 ), l_MeshDef, l_NewObj );
								l_Result = l_refKey.IsValid() ? CStatus::OK : CStatus::Fail;
							}
						}
					}
				}
				else
				{
					assert( l_refKey.IsValid() );
					l_Result = UpdatePolyMeshShapeKey( l_refKey, (LONG)( i - l_lMinFrame ), l_MeshDef );
				}
			}
			// Exit if something went wrong
			if ( l_Result != CStatus::OK )
				break;
		}
		
		if ( l_Result == CStatus::OK && m_eAnimationImportMode == EImport_Recreate && l_lMinFrame != l_lMaxFrame )
		{
			CString l_csShapeClipName = l_csName + CString("_ShapeClip");
			CreateShapeClip( l_NewObj, l_refKey, l_lMinFrame, l_lMaxFrame - l_lMinFrame + 1, l_csShapeClipName );
		}
	}
	return l_Result;
}

XSI::CStatus AbcXsiImporter::ProcessPointCloud( AbcImportPointCloud* in_pNode )
{
	CStatus l_Status = CStatus::Fail;
	CString l_csFullName = in_pNode->GetObject()->GetFullName();
	CString l_csName = in_pNode->m_pXform != NULL ? in_pNode->m_pXform->GetName() : in_pNode->GetName();

	IAbcIPoints* l_pPoints = (IAbcIPoints*)in_pNode->GetObject();
	CAbcPtr<IAbcTimeSampling> l_spTimeSampling;
	if ( l_pPoints->GetTimeSampling( &l_spTimeSampling ) == EResult_Success )
	{
		// Create a point cloud
		CRef l_refRoot = in_pNode->GetParent<AbcImportTreeNode>()->GetRef();
		CComAPIHandler l_COMObj( l_refRoot );
	
		CValueArray l_arrArgs;
		l_arrArgs.Resize(2);
		l_arrArgs[0] = CString( L"PointCloud" );
		l_arrArgs[1] = l_csName;
	
		CValue l_retVal;
		if ( l_COMObj.Invoke( "AddPrimitive", 0, l_retVal, l_arrArgs ).Succeeded() )
		{
			X3DObject l_PointCloud( l_retVal );
		
			if ( l_PointCloud.IsValid() ) 
			{
				bool l_bCreateKineCnx = false;
				if ( in_pNode->m_pXform != NULL )
				{
					ProcessTransform( in_pNode->m_pXform, false );
					ApplyTransform( l_PointCloud, in_pNode->m_pXform );

					CAbcPtr<IAbcIXform> l_spIAbcXfm;
					if ( in_pNode->m_pXform->GetObject()->TransformInto( EIObject_Xform, (IAbcIObject**)&l_spIAbcXfm ) == EResult_Success )
						l_bCreateKineCnx = !l_spIAbcXfm->IsConstant();
				}

				if ( m_eAnimationImportMode == EImport_Recreate )
				{
					g_ImportLog.Log( false, siWarningMsg, "Alembic Points Import only supports cached import, using CacheOnFile instead." );
				}
				if( m_bCreateICETreeMode == EImport_CreateICETreePerObj )
					l_Status = CreateCachedICETree( l_PointCloud, m_csFileName, l_pPoints, l_bCreateKineCnx );
				else
				{
					AddObjectMapping( l_PointCloud.GetRef(), l_pPoints, l_bCreateKineCnx );
					l_Status = CStatus::OK;
				}

				in_pNode->SetRef( l_PointCloud );
			}
		}
	}
	return l_Status;
}

XSI::CStatus AbcXsiImporter::ProcessCamera( AbcImportCamera* in_pNode )
{
	CStatus l_Status = CStatus::Fail;
	CString l_csFullName = in_pNode->GetObject()->GetFullName();
	CString l_csName = in_pNode->m_pXform != NULL ? in_pNode->m_pXform->GetName() : in_pNode->GetName();

	IAbcICamera* l_pCamera = (IAbcICamera*)in_pNode->GetObject();
	CAbcPtr<IAbcTimeSampling> l_spTimeSampling;
	if ( l_pCamera->GetTimeSampling( &l_spTimeSampling ) == EResult_Success )
	{
		// Create a camera
		CRef l_refRoot = in_pNode->GetParent<AbcImportTreeNode>()->GetRef();
		CComAPIHandler l_COMObj( l_refRoot );
	
		CValueArray l_arrArgs;
		l_arrArgs.Resize(2);
		l_arrArgs[0] = CString( L"Camera" );
		l_arrArgs[1] = l_csName;
	
		CValue l_retVal;
		if ( l_COMObj.Invoke( "AddPrimitive", 0, l_retVal, l_arrArgs ).Succeeded() )
		{
			X3DObject l_Camera( l_retVal );
		
			if ( l_Camera.IsValid() ) 
			{
				if ( in_pNode->m_pXform != NULL )
				{
					ProcessTransform( in_pNode->m_pXform, false );
					ApplyTransform( l_Camera, in_pNode->m_pXform );
				}
				if ( m_eAnimationImportMode != EImport_CacheOnFile )
				{
					g_ImportLog.Log( false, siWarningMsg, "Alembic Camera Import only supports AlembicCacheNode import, using AlembicCacheNode instead." );
				}
				if( m_bCreateICETreeMode == EImport_CreateICETreePerObj )
				{
					l_Status = CreateCachedICETree( l_Camera, m_csFileName, l_pCamera, true );
				}
				else
				{
					AddObjectMapping( l_Camera.GetRef(), l_pCamera, true );
					l_Status = CStatus::OK;
				}
				in_pNode->SetRef( l_Camera );
			}
		}
	}
	return l_Status;
}

XSI::CStatus AbcXsiImporter::ProcessTransform( AbcImportXform* in_pNode, bool in_bCreateNull )
{
	
	CStatus l_Status = CStatus::Fail;

	size_t l_NumChildTransforms = 0;
	size_t l_NumChildRenderables = 0;
	
	IAbcIXform* l_pXform = (IAbcIXform*)in_pNode->GetObject();
	const char* l_szName = l_pXform->GetName();

	CAbcPtr<IAbcISampleSelector> l_spSampleSelector;
	if ( m_spIArchive->CreateSampleSelector( &l_spSampleSelector ) == EResult_Success )
	{
		CAbcPtr<IAbcTimeSampling> l_spTimeSampling;
		if ( l_pXform->GetTimeSampling( &l_spTimeSampling ) != EResult_Success )
			return CStatus::Fail;

		size_t l_NumSamples = l_pXform->GetNumSamples();
		// Calculate time bounds
		double l_dStart = 0.0;
		double l_dEnd = 0.0;

		if ( !m_bAnimated || m_eAnimationImportMode == EImport_CacheOnFile )
		{
			// Start from the specified time even if not importing animation
			l_dStart = l_dEnd = std::max<double>( m_dStartTime, 0.0 );
		}
		else
		{
			l_dStart = m_dStartTime;
			l_dEnd = m_dEndTime;
		}

		STimeRange l_TimeRange;
		GetTimeRange( l_spTimeSampling, l_NumSamples, l_dStart, l_dEnd, l_TimeRange );

		long l_lMinFrame = ( long )( m_PlaybackSettings.TimeToFrame( l_TimeRange.m_dStart ) );
		long l_lMaxFrame = l_TimeRange.m_dStart != l_TimeRange.m_dEnd ? ( long )( m_PlaybackSettings.TimeToFrame( l_TimeRange.m_dEnd ) + 0.5 ) : l_lMinFrame;
		double l_dSampling = m_PlaybackSettings.m_dFrameRate > 0.0 ? 1.0 / m_PlaybackSettings.m_dFrameRate : 0.33;
		
		in_pNode->m_bInheritParent = true;
		in_pNode->m_lStartFrame = l_lMinFrame;

		TKeyList l_TempKeys;
		l_TempKeys.reserve( l_lMaxFrame - l_lMinFrame + 1 );

		// Add transform keys
		for ( long i = l_lMinFrame; i <= l_lMaxFrame; i++ )
		{
			double l_dSampleTime = m_PlaybackSettings.FrameToTime( i );
			double l_dLerp = 0.0;

 			bool l_bFirstSample = ( i == l_lMinFrame );
			long long l_llSourceSample = 0;
			long long l_llDestSample = 0;
			double l_dSourceTime = 0.0;
			double l_dDestTime = 0.0;
			l_spTimeSampling->GetFloorIndex( l_dSampleTime, (long long)l_NumSamples, l_llSourceSample, l_dSourceTime );
			l_spTimeSampling->GetCeilIndex( l_dSampleTime, (long long)l_NumSamples, l_llDestSample, l_dDestTime );
			
			if ( l_llSourceSample != l_llDestSample )
			{
				if ( fabs( l_dDestTime - l_dSourceTime ) <= 0.0001 )
					l_dLerp = 0;
				else
					l_dLerp = ( l_dSampleTime - l_dSourceTime ) / ( l_dDestTime - l_dSourceTime );
			}

			CAbcPtr<IAbcIXformSample> l_spSample;

			l_spSampleSelector->SetRequestedIndex( l_llSourceSample );
		
			if ( l_pXform->GetSample( l_spSampleSelector, &l_spSample ) != EResult_Success )
				break;

			const double* l_pdMat44 = l_spSample->GetMatrix4x4();

			MATH::CMatrix4 l_mat4x4( l_pdMat44[0], l_pdMat44[1], l_pdMat44[2], l_pdMat44[3],
									 l_pdMat44[4], l_pdMat44[5], l_pdMat44[6], l_pdMat44[7],
									 l_pdMat44[8], l_pdMat44[9], l_pdMat44[10], l_pdMat44[11],
									 l_pdMat44[12], l_pdMat44[13], l_pdMat44[14], l_pdMat44[15] );

			double l_pdTranslation[3];
			double l_pdScale[3];
			double l_dRotationX;
			double l_dRotationY; 
			double l_dRotationZ;

			// Test whether this is a simple transform or a complex one
			in_pNode->m_LocalMat = l_mat4x4;
			in_pNode->m_bHasShearing = HasShearing( l_spSample );
			
			MATH::CVector3 l_vecTrans, l_vecScale, l_vecOri;
			// For now use alembic's implementation if we have shearing (until we implement the op)
			if ( in_pNode->m_bHasShearing )
			{				
				memcpy( l_pdTranslation, l_spSample->GetTranslation(), sizeof(double) * 3 );
 				memcpy( l_pdScale, l_spSample->GetScale(), sizeof(double) * 3 );
				l_dRotationX = l_spSample->GetXRotation();
				l_dRotationY = l_spSample->GetYRotation();
				l_dRotationZ = l_spSample->GetZRotation();
			}
			else
			{
				MATH::CTransformation l_transform;
				l_transform.SetMatrix4( l_mat4x4 );

				l_pdTranslation[0] = l_transform.GetPosX();
				l_pdTranslation[1] = l_transform.GetPosY();
				l_pdTranslation[2] = l_transform.GetPosZ();
			
				l_pdScale[0] = l_transform.GetSclX();
				l_pdScale[1] = l_transform.GetSclY();
				l_pdScale[2] = l_transform.GetSclZ();
			
				l_dRotationX = l_transform.GetRotX();
				l_dRotationY = l_transform.GetRotY(); 
				l_dRotationZ = l_transform.GetRotZ();
			}

			STransformKey l_CurrentKey;
			if ( l_dLerp == 0.0 )
			{
				l_CurrentKey.m_Position.Set( l_pdTranslation[0], l_pdTranslation[1], l_pdTranslation[2] );
				l_CurrentKey.m_Scale.Set( l_pdScale[0], l_pdScale[1], l_pdScale[2] );
				l_CurrentKey.m_Rotation.Set( l_dRotationX, l_dRotationY, l_dRotationZ );
			}
			else
			{
				CAbcPtr<IAbcIXformSample> l_spDestSample;
				l_spSampleSelector->SetRequestedIndex( l_llDestSample );
				l_pXform->GetSample( l_spSampleSelector, &l_spDestSample );
				const double* l_pdDestTranslation = l_spDestSample->GetTranslation();
				const double* l_pdDestScale = l_spDestSample->GetScale();
				double l_dDestRotationX = l_spDestSample->GetXRotation();
				double l_dDestRotationY = l_spDestSample->GetYRotation();
				double l_dDestRotationZ = l_spDestSample->GetZRotation();

				l_CurrentKey.m_Position.Set( 
					LerpSingleValue( l_pdTranslation[0], l_pdDestTranslation[0], l_dLerp ),
					LerpSingleValue( l_pdTranslation[1], l_pdDestTranslation[1], l_dLerp ),
					LerpSingleValue( l_pdTranslation[2], l_pdDestTranslation[2], l_dLerp ) 
					);

				l_CurrentKey.m_Scale.Set( 
					LerpSingleValue( l_pdScale[0], l_pdDestScale[0], l_dLerp ),
					LerpSingleValue( l_pdScale[1], l_pdDestScale[1], l_dLerp ),
					LerpSingleValue( l_pdScale[2], l_pdDestScale[2], l_dLerp ) 
					);

				l_CurrentKey.m_Rotation.Set( 
					LerpAngle( l_dRotationX, l_dDestRotationX, l_dLerp ),
					LerpAngle( l_dRotationY, l_dDestRotationY, l_dLerp ),
					LerpAngle( l_dRotationZ, l_dDestRotationZ, l_dLerp ) 
					);
			}

			if ( l_bFirstSample )
			{
				in_pNode->m_bInheritParent = l_spSample->GetInheritsXforms();				
			}
			l_TempKeys.push_back( l_CurrentKey );
		}

		// Filter out unneeded keys
		if ( l_TempKeys.size() > 0 )
		{
			// First key
			STransformKey& l_FirstKey = l_TempKeys[0];
			l_FirstKey.m_KeyFlags = ETransformKey_Position | ETransformKey_Rotation | ETransformKey_Scale;
			in_pNode->m_TransformKeys[0] = l_FirstKey;

			for ( size_t i = 1; i < l_TempKeys.size() - 1; i++ )
			{
				STransformKey& l_PrevKey = l_TempKeys[i - 1];
				STransformKey& l_CurKey = l_TempKeys[i];
				STransformKey& l_NextKey = l_TempKeys[i + 1];
				
				l_CurKey.m_KeyFlags = ETransformKey_None;
				
				if ( !l_PrevKey.m_Position.EpsilonEquals( l_CurKey.m_Position, POSITION_EPSILON ) || !l_NextKey.m_Position.EpsilonEquals( l_CurKey.m_Position, POSITION_EPSILON ) )
					l_CurKey.m_KeyFlags |= ETransformKey_Position;

				if ( !l_PrevKey.m_Rotation.EpsilonEquals( l_CurKey.m_Rotation, ROTATION_EPSILON ) || !l_NextKey.m_Rotation.EpsilonEquals( l_CurKey.m_Rotation, ROTATION_EPSILON ) )
					l_CurKey.m_KeyFlags |= ETransformKey_Rotation;

				if ( !l_PrevKey.m_Scale.EpsilonEquals( l_CurKey.m_Scale, SCALE_EPSILON ) || !l_NextKey.m_Scale.EpsilonEquals( l_CurKey.m_Scale, SCALE_EPSILON ) )
					l_CurKey.m_KeyFlags |= ETransformKey_Scale;

				if ( l_CurKey.m_KeyFlags != ETransformKey_None )
				{
					in_pNode->m_TransformKeys[(long)i] = l_CurKey;
				}
			}

			// Last key
			if ( l_TempKeys.size() > 1 )
			{
				STransformKey& l_CurKey = l_TempKeys.back();
				STransformKey& l_PrevKey = l_TempKeys[l_TempKeys.size() - 2];

				if ( !l_PrevKey.m_Position.EpsilonEquals( l_CurKey.m_Position, POSITION_EPSILON ) )
					l_CurKey.m_KeyFlags |= ETransformKey_Position;

				if ( !l_PrevKey.m_Rotation.EpsilonEquals( l_CurKey.m_Rotation, ROTATION_EPSILON ) )
					l_CurKey.m_KeyFlags |= ETransformKey_Rotation;

				if ( !l_PrevKey.m_Scale.EpsilonEquals( l_CurKey.m_Scale, SCALE_EPSILON ) )
					l_CurKey.m_KeyFlags |= ETransformKey_Scale;

				if ( l_CurKey.m_KeyFlags != ETransformKey_None )
				{
					in_pNode->m_TransformKeys[ (long)l_TempKeys.size() - 1 ] = l_CurKey;
				}
			}
		}
		// Create a null transform if needed
		if ( in_bCreateNull )
		{
			X3DObject l_CurrentRoot = in_pNode->GetParent<AbcImportTreeNode>()->GetRef();

			CString l_csName = l_szName;
			Null l_Null;

			// Naming rules for NULL transforms, if we have a child that is already named the l_csName + "Shape" then we should not add "_transform" to
			// our name
			bool l_bHasSimilarNaming = false;
			for ( size_t i = 0; i < in_pNode->GetNumChildren(); i++ )
			{
				CString l_csChildName = in_pNode->GetChild<AbcImportTreeNode>(0)->GetName();
				if ( l_csChildName == l_csName || l_csChildName == l_csName + CString( L"Shape" ) )
				{
					l_bHasSimilarNaming = true;
					break;
				}
			}
			if  ( l_bHasSimilarNaming )
			{
				l_csName += "_transform";
			}

			l_Status = l_CurrentRoot.AddNull( l_csName, l_Null );
			if ( l_Status == CStatus::OK )
			{
				X3DObject l_NullObj( l_Null );
				in_pNode->SetRef( l_NullObj );
				ApplyTransform( l_NullObj, in_pNode );
			}

		}
		else
		{
			l_Status = CStatus::OK;
		}
	}
	return l_Status;

}

XSI::CStatus AbcXsiImporter::ProcessLocator( AbcImportLocator* in_pNode )
{
	CStatus l_Status = CStatus::Fail;
	
	X3DObject l_CurrentRoot = in_pNode->GetParent<AbcImportTreeNode>()->GetRef();
	IAbcIXform* l_pXform = (IAbcIXform*)in_pNode->GetObject();
	const char* l_szName = l_pXform->GetName();
	CString l_csName = in_pNode->m_pXform != NULL ? in_pNode->m_pXform->GetName() : in_pNode->GetName();
	
	Null l_Null;

	l_Status = l_CurrentRoot.AddNull( l_csName, l_Null );
	if ( l_Status == CStatus::OK )
	{
		if ( in_pNode->m_pXform != NULL )
		{
			ProcessTransform( in_pNode->m_pXform, false );
			ApplyTransform( l_Null, in_pNode->m_pXform );

			CAbcPtr<IAbcIObject> l_spIObj = in_pNode->m_pXform->GetObject();
			CAbcPtr<IAbcIXform> l_spIXfm;
			l_spIObj->TransformInto( EIObject_Xform, (IAbcIObject**)&l_spIXfm );

			if ( m_eAnimationImportMode == EImport_CacheOnFile )
			{
				if( m_bCreateICETreeMode == EImport_CreateICETreePerObj )
					l_Status = CreateCachedICETree( l_Null, m_csFileName, l_pXform, true );
				else
					AddObjectMapping( l_Null.GetRef(), l_pXform, true );
			}

		}

		
		in_pNode->SetRef( l_Null );

		return XSI::CStatus::OK;
	}

	return l_Status;
}

void AbcXsiImporter::ApplyTransform( XSI::X3DObject& in_Object, AbcImportXform* in_pNode )
{
	MATH::CTransformation l_Transform;

	KinematicState l_KineState = in_pNode->m_bInheritParent ? in_Object.GetKinematics().GetLocal() : in_Object.GetKinematics().GetGlobal();

	// Turn off Softimage Scaling, it doesn't make sense in Alembic
	if ( in_pNode->m_bInheritParent )
		l_KineState.GetParameter("siscaling").PutValue( false );

	// We don't have animation
	if ( in_pNode->m_TransformKeys.size() == 1 )
	{
		STransformKey& l_TransformKey = in_pNode->m_TransformKeys[0];

		l_Transform.SetScaling( l_TransformKey.m_Scale );
		
		l_Transform.SetRotX( l_TransformKey.m_Rotation.GetX() );
		l_Transform.SetRotY( l_TransformKey.m_Rotation.GetY() );
		l_Transform.SetRotZ( l_TransformKey.m_Rotation.GetZ() );

		l_Transform.SetPosX( l_TransformKey.m_Position.GetX() );
		l_Transform.SetPosY( l_TransformKey.m_Position.GetY() );
		l_Transform.SetPosZ( l_TransformKey.m_Position.GetZ() );
		l_KineState.PutTransform( l_Transform );
	}
	// We have animation!
	else if ( in_pNode->m_TransformKeys.size() > 1 )
	{
		// Animate rotation
		Parameter l_RotX = l_KineState.GetParameter( L"rotx" );
		Parameter l_RotY = l_KineState.GetParameter( L"roty" );
		Parameter l_RotZ = l_KineState.GetParameter( L"rotz" );
		Parameter l_SclX = l_KineState.GetParameter( L"sclx" );
		Parameter l_SclY = l_KineState.GetParameter( L"scly" );
		Parameter l_SclZ = l_KineState.GetParameter( L"sclz" );
		Parameter l_PosX = l_KineState.GetParameter( L"posx" );
		Parameter l_PosY = l_KineState.GetParameter( L"posy" );
		Parameter l_PosZ = l_KineState.GetParameter( L"posz" );

		assert( l_RotX.IsValid() && l_RotY.IsValid() && l_RotZ.IsValid() );
		assert( l_SclX.IsValid() && l_SclY.IsValid() && l_SclZ.IsValid() );
		assert( l_PosX.IsValid() && l_PosY.IsValid() && l_PosZ.IsValid() );

		FCurve l_RotKeyX, l_RotKeyY, l_RotKeyZ;
		l_RotX.AddFCurve( siStandardFCurve, l_RotKeyX );
		l_RotY.AddFCurve( siStandardFCurve, l_RotKeyY );
		l_RotZ.AddFCurve( siStandardFCurve, l_RotKeyZ );

		FCurve l_SclKeyX, l_SclKeyY, l_SclKeyZ;
		l_SclX.AddFCurve( siStandardFCurve, l_SclKeyX );
		l_SclY.AddFCurve( siStandardFCurve, l_SclKeyY );
		l_SclZ.AddFCurve( siStandardFCurve, l_SclKeyZ );

		FCurve l_PosKeyX, l_PosKeyY, l_PosKeyZ;
		l_PosX.AddFCurve( siStandardFCurve, l_PosKeyX );
		l_PosY.AddFCurve( siStandardFCurve, l_PosKeyY );
		l_PosZ.AddFCurve( siStandardFCurve, l_PosKeyZ );


		CDoubleArray l_arrRotX, l_arrRotY, l_arrRotZ;
		CDoubleArray l_arrSclX, l_arrSclY, l_arrSclZ;
		CDoubleArray l_arrPosX, l_arrPosY, l_arrPosZ;
		CLongArray l_arrInterpolations[3];
		CTimeArray l_arrTimes[3];

		for ( TKeyMap::iterator it = in_pNode->m_TransformKeys.begin(); it != in_pNode->m_TransformKeys.end(); ++it )
		{
			long l_lCurFrame = it->first + in_pNode->m_lStartFrame;
			STransformKey& l_TransformKey = it->second;

			CTime l_CurTime( (double)l_lCurFrame, CTime::Frames, CTime::CUSTOM, m_PlaybackSettings.m_dFrameRate );

			if ( l_TransformKey.m_KeyFlags & ETransformKey_Rotation )
			{
				l_arrRotX.Add( l_TransformKey.m_Rotation.GetX() );
				l_arrRotY.Add( l_TransformKey.m_Rotation.GetY() );
				l_arrRotZ.Add( l_TransformKey.m_Rotation.GetZ() );
				l_arrInterpolations[0].Add( siLinearKeyInterpolation );
				l_arrTimes[0].Add( l_CurTime );
			}
			if ( l_TransformKey.m_KeyFlags & ETransformKey_Scale )
			{
				l_arrSclX.Add( l_TransformKey.m_Scale.GetX() );
				l_arrSclY.Add( l_TransformKey.m_Scale.GetY() );
				l_arrSclZ.Add( l_TransformKey.m_Scale.GetZ() );
				l_arrInterpolations[1].Add( siLinearKeyInterpolation );
				l_arrTimes[1].Add( l_CurTime );
			}
			if ( l_TransformKey.m_KeyFlags & ETransformKey_Position )
			{
				l_arrPosX.Add( l_TransformKey.m_Position.GetX() );
				l_arrPosY.Add( l_TransformKey.m_Position.GetY() );
				l_arrPosZ.Add( l_TransformKey.m_Position.GetZ() );
				l_arrInterpolations[2].Add( siLinearKeyInterpolation );
				l_arrTimes[2].Add( l_CurTime );
			}
		}
		CStatus l_Result = CStatus::OK;

		l_Result = l_RotKeyX.SetKeys( l_arrTimes[0], l_arrRotX, l_arrInterpolations[0] );
		l_Result = l_RotKeyY.SetKeys( l_arrTimes[0], l_arrRotY, l_arrInterpolations[0] );
		l_Result = l_RotKeyZ.SetKeys( l_arrTimes[0], l_arrRotZ, l_arrInterpolations[0] );

		l_Result = l_SclKeyX.SetKeys( l_arrTimes[1], l_arrSclX, l_arrInterpolations[1] );
		l_Result = l_SclKeyY.SetKeys( l_arrTimes[1], l_arrSclY, l_arrInterpolations[1] );
		l_Result = l_SclKeyZ.SetKeys( l_arrTimes[1], l_arrSclZ, l_arrInterpolations[1] );

		l_Result = l_PosKeyX.SetKeys( l_arrTimes[2], l_arrPosX, l_arrInterpolations[2] );
		l_Result = l_PosKeyY.SetKeys( l_arrTimes[2], l_arrPosY, l_arrInterpolations[2] );
		l_Result = l_PosKeyZ.SetKeys( l_arrTimes[2], l_arrPosZ, l_arrInterpolations[2] );
	}
}

XSI::CString AbcXsiImporter::GetSchemaString( IAbcIObject* in_pObject )
{
	CString l_csRet = "None";
	size_t l_ValCount = 0;
	in_pObject->GetMetaDataValue( "schema", 0, &l_ValCount );
	if ( l_ValCount )
	{
		char* l_szVal = new char[l_ValCount + 1];
		in_pObject->GetMetaDataValue( "schema", l_szVal, NULL );
		l_csRet = l_szVal;
		delete[] l_szVal;
	}
	return l_csRet;
}

bool IsTimeValid( double in_dTime )
{
	return in_dTime == -1.0 || in_dTime >= 0.0;
}

XSI::CStatus AbcXsiImporter::SetTimeParams( bool in_bAnimated, double in_dStart, double in_dEnd, ETimeRangeFitting in_eTimeFitting )
{
	if ( IsTimeValid( in_dStart ) && IsTimeValid( in_dEnd ) )
	{
		m_bAnimated = in_bAnimated;
		m_eTimeRangeFitting = in_eTimeFitting;
		if ( in_bAnimated )
		{
			if ( in_dEnd == -1.0 || in_dEnd >= in_dStart )
			{
				m_dStartTime = in_dStart != -1.0 ? m_PlaybackSettings.FrameToTime( in_dStart ) : -1.0;
				m_dEndTime = in_dEnd != -1.0 ? m_PlaybackSettings.FrameToTime( in_dEnd ) : -1.0;
				return CStatus::OK;
			}
		}
		else
		{
			m_dStartTime = m_dEndTime = in_dStart != -1.0 ? m_PlaybackSettings.FrameToTime( in_dStart ) : -1.0;
			return CStatus::OK;
		}
	}
	g_ImportLog.Log( false, siErrorMsg, "Invalid frame arguments (%f to %f)", in_dStart, in_dEnd );

	return CStatus::InvalidArgument;
}

void AbcXsiImporter::SetPadShapeClips( bool in_bPad )
{
	m_bPadShapeClips = in_bPad;
}

//
// Set the option to determine if the new model is required to create as the root parent.
//
void AbcXsiImporter::SetCreateNewRootModel( bool in_bCreate )
{
	m_bCreateRootModel = in_bCreate;
}

//
// Specified the parent for the imported objects
//
void AbcXsiImporter::SetRootParent( XSI::X3DObject& in_objParent )
{
	m_objRootParent = in_objParent;
}

//
// Set the option to determine if one ICETree is created to drive
// all objects, or if one ICETree per object is created.
//
void AbcXsiImporter::SetCreateICETreeMode( EImportICETreeMode in_eCreateMode )
{
	m_bCreateICETreeMode = in_eCreateMode;
}

CStatus AbcXsiImporter::CreateShapeClip( XSI::X3DObject& in_Object, CRef& in_refShapeKey, double in_dFrame, long in_lNumFrames, const XSI::CString& in_csClipName )
{
	// pull the geom at frame first to make sure the data is up-to-date
	XSI::ShapeKey l_upToDateKey( in_refShapeKey );
	l_upToDateKey.EvaluateAt( in_dFrame );

	// Create our clip
	CRef l_refParentModel = in_Object.GetModel();
	CValueArray l_arrArgs(5);
	l_arrArgs[0] = l_refParentModel.GetAsText();
	l_arrArgs[1] = in_refShapeKey.GetAsText();
	l_arrArgs[2] = CValue();
	l_arrArgs[3] = CValue();
	l_arrArgs[4] = in_dFrame;
	l_arrArgs[5] = in_csClipName;

	CValue l_cvalReturn;
	CStatus l_Ret = Application().ExecuteCommand( L"AddClip", l_arrArgs, l_cvalReturn );
	
	// Pad our shape clip if needed
	if ( l_Ret == CStatus::OK && m_bPadShapeClips )
	{
		CRef l_refClip( l_cvalReturn );
		Clip l_Clip( l_refClip );
		if ( l_Clip.IsValid() )
		{
			CRef l_refClipTimeControl;
			l_refClipTimeControl.Set( l_refClip.GetAsText() + CString(".actionclip.timectrl" ) );

			Property l_Prop = l_refClipTimeControl;
			if ( l_Prop.IsValid() )
			{
				l_Prop.PutParameterValue( L"extrapaft_type", (LONG)1L );
				l_Prop.PutParameterValue( L"extrapaft_timehold", (LONG)( m_PlaybackSettings.m_dGlobalOut - ( in_dFrame + in_lNumFrames  ) + 1 ) );
			}		
		}
	}
	return l_Ret;
}
CRefArray ApplyOp( const CString& in_presetobj, CString & io_connectionset, const siConstructionMode& in_constructionmode )
{
	CValueArray args(6); 
	CValue retval;

	args[0]=in_presetobj;
	args[1]=io_connectionset;
	args[2]=(LONG)siUnspecified;
	args[3]=(LONG)siPersistentOperation;
	if ( in_constructionmode != siConstructionModeDefault ) 
	{
		args[5]=(LONG)in_constructionmode;
	}

	Application app;
	CStatus st = app.ExecuteCommand( L"ApplyOp", args, retval );
	io_connectionset=args[1];
	return retval; 
}

CValue AddICENode( const CString&  in_presetobj, const CString&  in_container )
{
	CValueArray args(2);
	CValue retval;

	args[0]= in_presetobj;
	args[1]= in_container;

	Application app;
	CStatus st = app.ExecuteCommand( L"AddICENode", args, retval );

	return retval;
}

void ConnectICENodes( const CString&  in_inputport, const CString&  in_outputport )
{
	CValueArray args(2);
	CValue retval;

	args[0]= in_inputport;
	args[1]= in_outputport;

	Application app;
	CStatus st = app.ExecuteCommand( L"ConnectICENodes", args, retval );

	return;
}

XSI::CStatus AbcXsiImporter::CreateCachedICETree( XSI::X3DObject& in_TargetObject, const XSI::CString& in_csAlembicFile, IAbcIObject* in_pAbcObject, bool in_bCreateKineCnx )
{
	CString l_csObjectText = in_TargetObject.GetFullName();
	XSI::siConstructionMode l_constructionMode = siConstructionModeAnimation;

	if ( in_pAbcObject->GetType() == EIObject_Polymesh )
	{
		CAbcPtr<IAbcIPolyMesh> l_spIAbcPolyMesh;
		in_pAbcObject->TransformInto( EIObject_Polymesh, (IAbcIObject**)&l_spIAbcPolyMesh );
		if ( l_spIAbcPolyMesh->GetTopologyVariance() == ETopoVariance_Heterogenous )
			l_constructionMode = siConstructionModeModeling;
	}

	ApplyOp( L"ICETree", l_csObjectText, l_constructionMode);

	CString l_csICETreeText = in_TargetObject.GetActivePrimitive().GetFullName() + L".ICETree";
	XSI::ICENode l_IceNode( AddICENode( L"$XSI_DSPRESETS\\ICENodes\\AlembicCacheNode.Preset", l_csICETreeText ) );
	if ( l_IceNode.IsValid() )
	{
		CString l_csSlash = CUtils::Slash();
		ULONG l_ulSlash = in_csAlembicFile.ReverseFindString( CUtils::Slash() );
		
		CString l_csPath = in_csAlembicFile.GetSubString( 0, l_ulSlash );
		CString l_csFile = l_ulSlash == (ULONG)-1 ? in_csAlembicFile : in_csAlembicFile.GetSubString( l_ulSlash + 1 );

		ULONG l_ulDot = l_csFile.ReverseFindString( L"." );
		l_csFile = l_csFile.GetSubString( 0, l_ulDot );

		XSI::GridData l_gridData( l_IceNode.GetParameter("Items").GetValue() );
		l_gridData.BeginEdit();
		
		l_IceNode.PutParameterValue( L"FileType", (LONG)2 );
		l_IceNode.PutParameterValue( L"FileTypeString", CString( L".abc" ) );
		l_IceNode.PutParameterValue( L"FilePath", l_csPath );
		l_IceNode.PutParameterValue( L"FileName", l_csFile );
		l_IceNode.PutParameterValue( L"FileMode", (LONG)2 );

		l_gridData.PutRowCount( 1 );
		l_gridData.PutCell( EItemsGridColumn_TargetFullName, 0, L"self" );
		l_gridData.PutCell( EItemsGridColumn_SourceFullName, 0, in_pAbcObject->GetFullName() );
		l_gridData.PutCell( EItemsGridColumn_TargetKineCnx, 0, in_bCreateKineCnx );

		CString l_csExecutePort = l_IceNode.GetFullName() + L".execute";
		ConnectICENodes( l_csICETreeText + L".port1", l_csExecutePort );

		XSI::ICENode l_IceNodeCurFrame( AddICENode( L"$XSI_DSPRESETS\\ICENodes\\CurrentFrameNode.Preset", l_csICETreeText ) );
		ConnectICENodes( l_IceNode.GetFullName() + L".frame", l_IceNodeCurFrame.GetFullName() + L".frame" );
		l_gridData.EndEdit();
	}
	return CStatus::OK;
}

XSI::CStatus AbcXsiImporter::CreateRootCachedICETree( AbcImportTreeNode* in_pRootNode, const XSI::CString& in_csAlembicFile )
{
	if ( m_ObjectMapping.empty() )
		return XSI::CStatus::False;

	XSI::X3DObject l_root3DObj;
	bool l_bHasRootModel = false;
	// If new model was created, create the single ICETree underneath it.
	if( m_refNewRootModel.IsValid() )
	{
		l_root3DObj = (XSI::X3DObject)m_refNewRootModel;
		l_bHasRootModel = true;
	}
	else
	{
		// Otherwise, look for a imported object as the owner of the ICETree
		if( !in_pRootNode || in_pRootNode->GetNumChildren() == 0 )
		{
			g_ImportLog.Log( false, siErrorMsg, "Cannot find a valid imported object to apply ICETree" );
			return XSI::CStatus::False;
		}
		for ( ULONG i = 0; i < in_pRootNode->GetNumChildren(); ++i )
		{
			CRef l_crefChild( in_pRootNode->GetChild<AbcImportTreeNode>( i )->GetRef() );
			if( l_crefChild.IsValid() && l_crefChild.IsA( siX3DObjectID ) )
			{
				l_root3DObj = (XSI::X3DObject)l_crefChild;
				break;
			}
		}
	}
	XSI::Application app;
	if( !l_root3DObj.IsValid() )
	{
		g_ImportLog.Log( false, siErrorMsg, "Cannot find a valid imported object to apply ICETree" );
		return XSI::CStatus::False;
	}

	XSI::siConstructionMode l_constructionMode = siConstructionModeAnimation;
	for ( CacheObjectInfoVector::iterator it=m_ObjectMapping.begin(); it!=m_ObjectMapping.end(); ++it )
	{
		if ( it->m_spAbcIObj->GetType() == EIObject_Polymesh )
		{
			CAbcPtr<IAbcIPolyMesh> l_spIAbcPolyMesh;
			it->m_spAbcIObj->TransformInto( EIObject_Polymesh, (IAbcIObject**)&l_spIAbcPolyMesh );
			if ( l_spIAbcPolyMesh->GetTopologyVariance() == ETopoVariance_Heterogenous )
			{
				l_constructionMode = siConstructionModeModeling;
				break;
			}
		}
	}

	CString l_csRootObjFullName = l_root3DObj.GetFullName();
	ApplyOp( L"ICETree", l_csRootObjFullName, l_constructionMode );

	CString l_csICETreeText = l_root3DObj.GetActivePrimitive().GetFullName() + L".ICETree";
	XSI::ICENode l_IceNode( AddICENode( L"$XSI_DSPRESETS\\ICENodes\\AlembicCacheNode.Preset", l_csICETreeText ) );
	if ( l_IceNode.IsValid() )
	{
		CString l_csSlash = CUtils::Slash();
		ULONG l_ulSlash = in_csAlembicFile.ReverseFindString( CUtils::Slash() );

		CString l_csPath = in_csAlembicFile.GetSubString( 0, l_ulSlash );
		CString l_csFile = l_ulSlash == (ULONG)-1 ? in_csAlembicFile : in_csAlembicFile.GetSubString( l_ulSlash + 1 );

		ULONG l_ulDot = l_csFile.ReverseFindString( L"." );
		l_csFile = l_csFile.GetSubString( 0, l_ulDot );

		XSI::GridData l_gridData( l_IceNode.GetParameter("Items").GetValue() );
		l_gridData.BeginEdit();

		l_IceNode.PutParameterValue( L"FileType", (LONG)2 );
		l_IceNode.PutParameterValue( L"FileTypeString", CString( L".abc" ) );
		l_IceNode.PutParameterValue( L"FilePath", l_csPath );
		l_IceNode.PutParameterValue( L"FileName", l_csFile );
		l_IceNode.PutParameterValue( L"FileMode", (LONG)2 );

		l_gridData.PutRowCount( (LONG)m_ObjectMapping.size() );
		LONG l_ulLastIdx = 0;
		for ( CacheObjectInfoVector::const_iterator it=m_ObjectMapping.begin(); it!=m_ObjectMapping.end(); ++it, ++l_ulLastIdx )
		{
			XSI::X3DObject l_obj( it->m_Ref );
			XSI::CString l_csObjFullName = l_obj.GetFullName();
			if( l_bHasRootModel )
			{
				ULONG l_ulRootNamePos = l_csObjFullName.FindString( l_csRootObjFullName );
				if ( l_ulRootNamePos != UINT_MAX )
				{
					l_csObjFullName = l_csObjFullName.GetSubString( l_csRootObjFullName.Length() );
					l_csObjFullName = "this_model" + l_csObjFullName;
				}

			}
			
			l_gridData.PutCell( EItemsGridColumn_TargetFullName, l_ulLastIdx, l_csObjFullName );
			l_gridData.PutCell( EItemsGridColumn_SourceFullName, l_ulLastIdx, it->m_spAbcIObj->GetFullName() );
			l_gridData.PutCell( EItemsGridColumn_TargetKineCnx, l_ulLastIdx, it->m_bCreateKineCnx );
		}

		CString l_csExecutePort = l_IceNode.GetFullName() + L".execute";
		ConnectICENodes( l_csICETreeText + L".port1", l_csExecutePort );

		XSI::ICENode l_IceNodeCurFrame( AddICENode( L"$XSI_DSPRESETS\\ICENodes\\CurrentFrameNode.Preset", l_csICETreeText ) );
		ConnectICENodes( l_IceNode.GetFullName() + L".frame", l_IceNodeCurFrame.GetFullName() + L".frame" );

		l_gridData.EndEdit();
	}

	return CStatus::OK;
}

XSI::CStatus AbcXsiImporter::SetAnimationImportMode( EAnimationImportMode in_eImportMode )
{
	m_eAnimationImportMode = in_eImportMode;
	return CStatus::OK;
}

void AbcXsiImporter::AddObjectMapping( XSI::CRef in_ref, IAbcIObject* in_pAbcIObj, bool in_bCreateKineCnx )
{
	m_ObjectMapping.push_back( SCacheObjectInfo() );
	SCacheObjectInfo& l_info = m_ObjectMapping.back();
	l_info.m_Ref = in_ref;
	l_info.m_spAbcIObj = in_pAbcIObj;
	l_info.m_bCreateKineCnx = in_bCreateKineCnx;
}
