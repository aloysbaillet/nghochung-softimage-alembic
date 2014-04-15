//*****************************************************************************
/*!
	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*

#include "AbcXsiExporter.h"
#include "AbcXsiUtils.h"
#include "AbcXsiExportDataTypeConverter.h"
#include "ExportHelper.h"
#include <IAbcOProperty.h>
#include <IAbcOutput.h>

#include <xsi_application.h>
#include <xsi_geometryaccessor.h>
#include <xsi_group.h>
#include <xsi_iceattributedataarray.h>
#include <xsi_iceattributedataarray2D.h>
#include <xsi_kinematics.h>
#include <xsi_kinematicstate.h>
#include <xsi_model.h>
#include <xsi_particlecloudprimitive.h>
#include <xsi_pass.h>
#include <xsi_polygonmesh.h>
#include <xsi_primitive.h>
#include <xsi_project.h>
#include <xsi_scene.h>
#include <xsi_status.h>
#include <xsi_x3dobject.h>
#include <xsi_camera.h>
#include <xsi_selection.h>
#include <xsi_utils.h>
#include <memory>
#include <map>
#include <set>

const wchar_t* AbcXsiExporter::DEFAULT_ATTRIBUTES_POINTCLOUD = L"Color, Scale, Size, PointVelocity, Orientation, AngularVelocity, Shape, StrandPosition, StrandVelocity, StrandDeform, StrandOrientation, StrandUpVector, StrandColor, StrandSize, ColorAlongStrands";
const wchar_t* AbcXsiExporter::DEFAULT_ATTRIBUTES_POLYMESH = L"Materials, MaterialID, PointUserMotions";

/* Excluded attributes:
PointPosition:	exported as "P" in PolyMesh and Points sample
PointVelocity:	exported as "velocities"
Size:			exported as "widths"
ID:				exported as "pointIds"
NodeUserNormal: exported as "N" in PolyMesh sample
LocalTime:		excluded because it interferes with GetCurrentFrame node
Topology:		excluded because data type is Topology
PolygonalDescription: 
EmitLocation:	excluded because the data type is PointLocator
*/

const wchar_t* AbcXsiExporter::EXCLUDED_ATTRIBUTES_POINTCLOUD = L"PointPosition, Size, PointVelocity, ID, EmitLocation, LocalTime";
const wchar_t* AbcXsiExporter::EXCLUDED_ATTRIBUTES_POLYMESH	=	L"PointPosition, NodeUserNormal, LocalTime, Topology, PolygonalDescription";

using namespace XSI;

XSI::CStatus WriteMeshAtTime( XSI::CRef in_ref, IAbcOObject* in_pOPolyMesh, double in_dTime );

void FindAbcGeomParamType( EAbcGeomScope& out_scope, EAbcDataTraits& out_traits, ICEAttribute& in_attr )
{
	switch (in_attr.GetContextType())
	{
	case XSI::siICENodeContextSingleton:		out_scope = EGeomScope_Constant; break;	
	case XSI::siICENodeContextComponent0D:		out_scope = EGeomScope_Vertex; break;
	case XSI::siICENodeContextComponent0D2D:	out_scope = EGeomScope_FaceVarying; break;
	case XSI::siICENodeContextComponent2D:		out_scope = EGeomScope_Uniform; break;

	default: out_scope = EGeomScope_Constant; break;// Abc doesn't support per-Edge scope
	}

	switch (in_attr.GetDataType())
	{
	case XSI::siICENodeDataBool:		out_traits = EDataTraits_Bool; break;
	case XSI::siICENodeDataLong:		out_traits = EDataTraits_Int32; break;
	case XSI::siICENodeDataFloat:		out_traits = EDataTraits_Float32; break;
	case XSI::siICENodeDataVector2:		out_traits = EDataTraits_V2f; break;
	case XSI::siICENodeDataVector3:		out_traits = EDataTraits_V3f; break;
	case XSI::siICENodeDataVector4:		out_traits = EDataTraits_V3f; break;	// Alembic doesn't include V4f, extended by AbcFramework
	case XSI::siICENodeDataQuaternion:	out_traits = EDataTraits_Quatf; break;
	case XSI::siICENodeDataMatrix33: 	out_traits = EDataTraits_M33f; break;
	case XSI::siICENodeDataMatrix44: 	out_traits = EDataTraits_M44f; break;
	case XSI::siICENodeDataColor4: 		out_traits = EDataTraits_C4f; break;
	case XSI::siICENodeDataRotation:	out_traits = EDataTraits_Quatf; break;
	case XSI::siICENodeDataShape:		out_traits = EDataTraits_UInt8; break;
	case XSI::siICENodeDataString:		out_traits = EDataTraits_WStringPtr; break;
	
	default: out_traits = EDataTraits_Unknown;
	}
}

void SplitAndTrim( XSI::CStringArray& out_csArray, const XSI::CString& in_cs )
{
	CStringArray curArray = in_cs.Split(L",");
	for ( LONG i=0; i<curArray.GetCount(); ++i )
	{
		XSI::CString csName = curArray[i];
		csName.TrimLeft(L" ");
		csName.TrimRight(L" ");
		if ( csName.Length() > 0 )
			out_csArray.Add( csName );
	}
}

void NextFrame( void )
{
	CValueArray args(0);
	CValue retval;LONG i;i=0;

	CStatus st = XSI::Application().ExecuteCommand( L"NextFrame", args, retval );

	return;
}
// Refresh
void Refresh( const CValue& /*undefined*/ in_time )
{
	CValueArray args(1);
	CValue retval;LONG i;i=0;


	args[i++]= in_time;

	CStatus st = XSI::Application().ExecuteCommand( L"Refresh", args, retval );

	return;
}

// SetValue
CValue SetValue( const CString& in_target,const CValue& /*value*/ in_value, const CValue& /*undefined*/ in_time )
{
	CValueArray args(3);
	CValue retval;LONG i;i=0;


	args[i++]= in_target;
	args[i++]= in_value;
	args[i++]= in_time;

	CStatus st = XSI::Application().ExecuteCommand( L"SetValue", args, retval );

	return retval;
}

// GetValue
CValue GetValue( const CValue& /*undefined*/ in_target, const CValue& /*undefined*/ in_time )
{
	CValueArray args(2);
	CValue retval;LONG i;i=0;


	args[i++]= in_target;
	args[i++]= in_time;

	CStatus st = XSI::Application().ExecuteCommand( L"GetValue", args, retval );

	return retval;
}

// FirstFrame
void FirstFrame( void )
{
	CValueArray args(0);
	CValue retval;LONG i;i=0;



	CStatus st = XSI::Application().ExecuteCommand( L"FirstFrame", args, retval );

	return;
}

/*! \class ExportTreeNode
	\brief The ExportTreeNode class represents an object to be exported into Alembic.
	
	The hierarchy of ExportTreeNode is created out of the selected XSI objects to maintain the parent-child
	hierarchy and the global transforms. At the root of the hierarchy is an ExportTreeRootNode.
	If the user chooses not to include the XSI hierarchy, all ExportTreeNode are laid out flat under the root node.
	If an XSI object is not selected but must be included in the ExportTreeNode hierarchy, only its transform is exported
	and not its primitive.

	\sa ExportTreeRootNode
*/
class ExportTreeNode
{
public:
	/*! Constructor
	\param in_pExporter The exporter object
	\param in_ref The CRef reference to the XSI object
	\param in_bExportPrim True if the primitive should be exported
	\param in_bExportTransform True if the transform should be exported
	*/
	ExportTreeNode( AbcXsiExporter* in_pExporter, XSI::CRef in_ref, bool in_bExportPrim, bool in_bExportTransform );

	/*! Destructor
	*/
	virtual ~ExportTreeNode()
	{
		for ( ChildrenList::iterator it = m_Children.begin(); it != m_Children.end(); ++it )
		{
			delete (*it);
		}

		m_Children.clear();
	}

	/*! Export the XSI object into Alembic
		\param in_pOObjParent The parent Alembic object
		\return CStatus::OK Success
	*/
	virtual XSI::CStatus CreateOObject( IAbcOObject* in_pOOjbParent );

	/*! Set the flag to export the primitive
		\param in_bool True if the primitive should be exported
	*/
	void SetPrimitiveExport( bool in_bool ) { m_ToExportPrimitive = in_bool;}

	/*! Set the flag to export the transform
		\param in_bool True if the transform should be exported
	*/
	void SetTransformExport( bool in_bool ) { m_ToExportTransform = in_bool;}

	/*! Insert a child node
		\param in_pChild The child node
	*/
	void InsertChild( ExportTreeNode* in_pChild ) 
	{ 
		m_Children.insert( in_pChild );
	}

	XSI::CStatus WriteSample( double in_dFrame, double in_dTime );

	ULONG GetNbSubframes() const { return m_ulNbSubframes;}
	
	XSI::CRef GetCRef() const { return m_Obj;}

	IAbcOObject* GetOObject() { return m_spAbcOPrim.GetPtr();}

	double GetObjectFPS( ) const;

protected:
	typedef std::set< ExportTreeNode* > ChildrenList;
	typedef std::map< XSI::CString, IAbcOProperty*, CaseInsensitiveLess > AttrToPropMap;
	
	enum EExportObjectType
	{
		EExportObject_Polymesh,
		EExportObject_Pointcloud,
		EExportObject_Camera,
		EExportObject_Locator
	};

	// Constructor for the sub nodes
	ExportTreeNode()
		: m_pExporter( NULL )
		, m_ToExportPrimitive( false ) 
		, m_ToExportTransform( false)
		, m_ulNbSubframes( 1 )
	{}

	XSI::CStatus CreatePolymesh	 ( IAbcOPolyMesh** out_ppPolyMesh, XSI::SIObject in_obj, IAbcOObject* in_pParent, const XSI::CString& in_csName );
	XSI::CStatus CreatePointCloud( IAbcOPoints** out_ppPoints, XSI::SIObject in_obj, IAbcOObject* in_pParent, const XSI::CString& in_csName );
	XSI::CStatus CreateLocator	 ( IAbcOXform** out_ppXform, XSI::SIObject in_obj, IAbcOObject* in_pParent, const XSI::CString& in_csName );
	XSI::CStatus CreateXform	 ( IAbcOXform** out_ppXform, XSI::SIObject in_obj, IAbcOObject* in_pParent, const XSI::CString& in_csName );
	XSI::CStatus CreateCamera	 ( IAbcOCamera** out_ppCamera, XSI::SIObject in_obj, IAbcOObject* in_pParent, const XSI::CString& in_csName );
	XSI::CStatus CreateProperties( IAbcOGeom* in_pAbcGeom, XSI::SIObject& in_obj, XSI::CRefArray& in_arefAttrs, const AttributeNameSet* in_pUserAttrs = NULL, const AttributeNameSet* in_pExcludedAttrs = NULL );
	
	XSI::CStatus WriteXformSample( double in_dFrame, double in_dTime );
	XSI::CStatus WritePolymeshSample( double in_dFrame, double in_dTime );
	XSI::CStatus WritePointcloudSample( double in_dFrame, double in_dTime );
	XSI::CStatus WriteCameraSample( double in_dFrame, double in_dTime );
	XSI::CStatus WriteLocatorSample( double in_dFrame, double in_dTime );
	XSI::CStatus WriteAttributeSample( XSI::Primitive& in_prim, XSI::Geometry& in_geom );

	EExportObjectType		m_eExportObjType;
	AbcXsiExporter*			m_pExporter;
	XSI::CRef				m_Obj;
	CAbcPtr<IAbcOXform>		m_spAbcOXform;
	CAbcPtr<IAbcOObject>	m_spAbcOPrim;
	ULONG					m_ulNbSubframes;
	AttributeNameSet		m_AttrNameSet;
	AttrToPropMap			m_AttrToPropMap;
	ExportHelper::LongVector m_FaceCounts;

	ChildrenList m_Children;
	bool m_ToExportPrimitive;
	bool m_ToExportTransform;
};

/*! \class ExportTreeRootNode
	\brief The ExportTreeRootNode is the root of the ExportTreeNode hierarchy

	The hierarchy is automatically created by calling the constructor ExportTreeRootNode with an array of XSI objects as argument.
*/
class ExportTreeRootNode : public ExportTreeNode
{
public:
	/*! Constructor for the root node, which will create the whole tree 
		\param in_pExporter The exporter object
		\param in_objs The CRefArray of XSI objects
		\param in_bIncludeParentHier True if the parents of the selected objects should be included in the hierarchy
	*/
	ExportTreeRootNode( AbcXsiExporter* in_pExporter, const CRefArray& in_objs, bool in_bIncludeParentHier );

	/*! Create the Alembic OObject from the XSI object
		\param in_pOObjParent The Alembic OObject
		\return CStatus::OK Success
	*/
	virtual XSI::CStatus CreateOObject( IAbcOObject* in_pOOjbParent );

	/*! Create or update the hierarchy from an array of XSI objects
		\param in_objs The CRefArray of XSI objects
		\param in_bIncludeParentHier True if the parents of the selected objects should be included in the hierarchy
	*/
	void CreateHierarchy( const CRefArray& in_objs, bool in_bIncludeParentHier );

private:
	typedef std::map<XSI::CRef, ExportTreeNode*> ExportTreeNodeMap;
	typedef std::set<XSI::CRef> CRefSet;

	void Filter( CRefSet& out_crefSet, const XSI::CRefArray& in_objs );
	ExportTreeNodeMap m_nodeMap;
};

ExportTreeNode::ExportTreeNode( AbcXsiExporter* in_pExporter, XSI::CRef in_ref, bool in_bExportPrim, bool in_bExportTransform )
	: m_pExporter( in_pExporter )
	, m_Obj( in_ref )
	, m_ToExportPrimitive( in_bExportPrim ) 
	, m_ToExportTransform( in_bExportTransform )
	, m_ulNbSubframes( 1 )
{
	XSI::X3DObject x3dObj( m_Obj );

	XSI::CString csType( x3dObj.GetType() );

	if ( csType.IsEqualNoCase( XSI::siPolyMeshType ) )
	{
		m_eExportObjType = EExportObject_Polymesh;
	}
	else if ( csType.IsEqualNoCase( XSI::siPointCloudPrimType ) )
	{
		m_eExportObjType = EExportObject_Pointcloud;
	}
	else if ( csType.IsEqualNoCase( XSI::siCameraPrimType ) )
	{
		m_eExportObjType = EExportObject_Camera;
	}
	else
	{
		m_eExportObjType = EExportObject_Locator;
	}

	XSI::Property propSimSettings;
	if ( x3dObj.GetPropertyFromName( L"simulationsettings", propSimSettings ) == XSI::CStatus::OK )
	{
		LONG lNbSubframes = propSimSettings.GetParameterValue( L"subframesampling" );
		bool bSaveAllSubframes = propSimSettings.GetParameterValue( L"saveallsamplesonfile" );

		if ( bSaveAllSubframes )
			m_ulNbSubframes = lNbSubframes;
	}

}

double ExportTreeNode::GetObjectFPS( ) const
{
	return 1.0 / (  m_pExporter->GetFrameRate() * m_ulNbSubframes );
}

void ExportTreeRootNode::Filter( CRefSet& out_crefSet, const XSI::CRefArray& in_objs )
{
	XSI::Application XSIApp;
	XSI::CRef crefSceneRoot( XSIApp.GetActiveSceneRoot().GetRef() );

	// push the cref to a set to filter out duplicates SceneRoot
	for (LONG i=0; i<in_objs.GetCount(); ++i)
	{
		XSI::CRef crefObj( in_objs[i] );
		
		if ( crefObj != crefSceneRoot )
		{
			X3DObject x3dObj( crefObj );
			
			if ( x3dObj.IsValid() )
				out_crefSet.insert( crefObj );
		}
	}
}

ExportTreeRootNode::ExportTreeRootNode( AbcXsiExporter* in_pExporter, const CRefArray& in_objs, bool in_bIncludeParentHier )
{
	Application XSIApp;
	CRef crefSceneRoot( XSIApp.GetActiveSceneRoot().GetRef() );

	m_pExporter = in_pExporter;

	// Add the exportNode for SceneRoot
	m_Obj = crefSceneRoot;
	m_ToExportPrimitive = false;
	m_ToExportTransform = false;

	m_nodeMap[ crefSceneRoot ] = this;

	CreateHierarchy( in_objs, in_bIncludeParentHier );
}

void ExportTreeRootNode::CreateHierarchy( const CRefArray& in_objs, bool in_bIncludeParentHier)
{
	Application XSIApp;
	CRef crefSceneRoot( XSIApp.GetActiveSceneRoot().GetRef() );
	CRefSet setObjs;

	Filter( setObjs, in_objs );

	// For each object in the list, traverse up the parents to create the export hierarchy
	for ( CRefSet::iterator it = setObjs.begin(); it != setObjs.end(); ++it )
	{
		CRef crefObj( *it );
		ExportTreeNodeMap::const_iterator itNode = m_nodeMap.find( crefObj );
		ExportTreeNode* pNode = NULL;

		if ( itNode == m_nodeMap.end() )
		{
			pNode = new ExportTreeNode( m_pExporter, crefObj, true, true );
			m_nodeMap[crefObj] = pNode;
		}
		else
		{
			pNode = itNode->second;
			pNode->SetPrimitiveExport( true );
			pNode->SetTransformExport( true );
		}
		
		if( in_bIncludeParentHier )
		{
			// Check the option to determine if the parent hierarchy should be exported.
			// If yes, traverse the up hierarchy to build the export tree.
			while ( crefObj != crefSceneRoot )
			{
				SIObject siObj( crefObj );
				CRef crefParent = siObj.GetParent();

				ExportTreeNodeMap::const_iterator itParent = m_nodeMap.find( crefParent );
				ExportTreeNode* pNodeParent = NULL;

				if ( itParent == m_nodeMap.end() )
				{
					pNodeParent = new ExportTreeNode( m_pExporter, crefParent, false, true );
					m_nodeMap[ crefParent ] = pNodeParent;
				}
				else
				{
					pNodeParent = itParent->second;
				}

				pNodeParent->InsertChild( pNode );

				pNode = pNodeParent;
				crefObj = crefParent;
			}
		}
	}
	// If it's not requested to export the parent hierarchy in the scene,  try to check if its parent would
	// be exported. Link it as a child if exists. Otherwise, export it into root level.
	// The tree node hierarchy here is no longer same to the hierarchy in the scene.
	if( !in_bIncludeParentHier )
	{
		for (ExportTreeNodeMap::const_iterator itNode = m_nodeMap.begin(); itNode != m_nodeMap.end(); ++itNode )
		{			
			if( itNode->first == crefSceneRoot )
				continue;
			X3DObject x3dobj( itNode->first );
			bool bParent = false;
			CRef crefParent = x3dobj.GetParent();
			if( crefParent.IsValid() )
			{
				ExportTreeNodeMap::const_iterator itParent = m_nodeMap.find( crefParent );
				if( itParent != m_nodeMap.end() )
				{
					itParent->second->InsertChild( itNode->second );
					bParent = true;
				}			
			}
			if( !bParent )
				InsertChild( itNode->second ); // Reparent to the root
		}
	}	
}

XSI::CStatus ExportTreeRootNode::CreateOObject( IAbcOObject* in_pOOjbParent )
{
	for ( ChildrenList::iterator it=m_Children.begin(); it!=m_Children.end(); ++it )
	{
		(*it)->CreateOObject( in_pOOjbParent );
	}

	return XSI::CStatus::OK;
}

// Export() can be visited twice
// First time: to export the user's selection
// Second time: to export geometries instanced by a pointcloud in the first export
XSI::CStatus ExportTreeNode::CreateOObject( IAbcOObject* in_pOOjbParent )
{
	CAbcPtr<IAbcOObject> spXformOObj;

	if ( m_spAbcOXform == NULL )
	{
		// The XSI object hasn't been exported yet
		XSI::SIObject siObj( m_Obj );

		XSI::CString csType( siObj.GetType() );

		const XSI::CString csName = siObj.GetName();

		// Export transform cause we have filtered out non-x3dobjects
		CreateXform( &m_spAbcOXform, siObj, in_pOOjbParent, csName );

		spXformOObj = static_cast<IAbcOObject*>( m_spAbcOXform.GetPtr() ) ;

		if ( m_ToExportPrimitive )
		{
			if ( m_eExportObjType == EExportObject_Polymesh )
			{
				CAbcPtr<IAbcOPolyMesh> spOPolyMesh;
				CreatePolymesh( &spOPolyMesh, siObj, spXformOObj, siObj.GetName() + "Shape" );
				m_spAbcOPrim = spOPolyMesh;
			}
			else if ( m_eExportObjType == EExportObject_Pointcloud )
			{
				CAbcPtr<IAbcOPoints> spOPoints;
				CreatePointCloud( &spOPoints, siObj, spXformOObj, siObj.GetName() + "Shape" );
				m_spAbcOPrim = spOPoints;
			}
			else if ( m_eExportObjType == EExportObject_Camera )
			{
				CAbcPtr<IAbcOCamera> spOCamera;
				CreateCamera( &spOCamera, siObj, spXformOObj, siObj.GetName() + "Shape" );
				m_spAbcOPrim = spOCamera;
			}
			else
			{
				CAbcPtr<IAbcOXform> spPrim;
				CreateLocator( &spPrim, siObj, spXformOObj, siObj.GetName() + "Shape" );
				m_spAbcOPrim = spPrim;
			}

		}
		else
		{
			CAbcPtr<IAbcOXform> spPrim;
			CreateLocator( &spPrim, siObj, spXformOObj, siObj.GetName() + "Shape" );
			m_spAbcOPrim = spPrim;
		}
	}
	else
	{
		spXformOObj = static_cast<IAbcOObject*>( m_spAbcOXform.GetPtr() ) ;
	}

	for ( ChildrenList::iterator it=m_Children.begin(); it!=m_Children.end(); ++it )
	{
		(*it)->CreateOObject( spXformOObj );
	}

	return CStatus::OK;
}

XSI::CStatus ExportTreeNode::WriteSample( double in_dFrame, double in_dTime )
{
	if ( m_ToExportPrimitive )
	{
		if ( m_eExportObjType == EExportObject_Polymesh )
		{
			WritePolymeshSample( in_dFrame, in_dTime );
		}
		else if ( m_eExportObjType == EExportObject_Pointcloud )
		{
			WritePointcloudSample( in_dFrame, in_dTime );
		}
		else if ( m_eExportObjType == EExportObject_Camera )
		{
			WriteCameraSample( in_dFrame, in_dTime );
		}
		else
		{
			WriteLocatorSample( in_dFrame, in_dTime );
		}
	}
	else
	{
		WriteLocatorSample( in_dFrame, in_dTime );
	}
	
	WriteXformSample( in_dFrame, in_dTime );

	return XSI::CStatus::OK;
}

//===========================================================================================================================================================================
// POLYMESH
//===========================================================================================================================================================================
XSI::CStatus ExportTreeNode::CreatePolymesh( IAbcOPolyMesh** out_ppPolyMesh, XSI::SIObject in_obj, IAbcOObject* in_pParent, const XSI::CString& in_csName )
{
	XSI::X3DObject x3dObj( in_obj.GetRef() );
	XSI::PolygonMesh polymesh( x3dObj.GetActivePrimitive( m_pExporter->GetStartFrame() ).GetGeometry() );

	CAbcPtr<IAbcOPolyMesh> spAbcPolyMesh;
	in_pParent->CreateChild( (IAbcOObject**)&spAbcPolyMesh, EOObject_Polymesh, in_csName.GetAsciiString() );

	double dFPS = GetObjectFPS( );
	spAbcPolyMesh->SetTimeSampling( dFPS, m_pExporter->GetStartTime() );

	XSI::CRefArray arefAttrs = polymesh.GetICEAttributes();
	CreateProperties( (IAbcOGeom*)spAbcPolyMesh.GetPtr(), in_obj, arefAttrs, &(m_pExporter->GetUserSpecifiedAttributesPolymesh()), &(m_pExporter->GetExcludedAttributesPolymesh()) );

	// Add the object to the list of animated items
	m_pExporter->AddAnimatedItem( this );

	spAbcPolyMesh->AddRef();
	*out_ppPolyMesh = spAbcPolyMesh;

	return XSI::CStatus::OK;
}

XSI::CStatus ExportTreeNode::WritePolymeshSample( double in_dFrame, double in_dTime )
{
	XSI::X3DObject x3dObj( m_Obj );
	XSI::Primitive prim = x3dObj.GetActivePrimitive( in_dFrame );
	XSI::PolygonMesh polymeshGeom = prim.GetGeometry( in_dFrame, siConstructionModeSecondaryShape );

	XSI::CGeometryAccessor ga = polymeshGeom.GetGeometryAccessor2( siConstructionModeSecondaryShape );
	
	// get the number of vertices for each polygons
    ExportHelper::LongVector cwVertIndices;
	ExportHelper::FloatVector fvPos;
	ExportHelper::PrepareGeomData( fvPos, cwVertIndices, m_FaceCounts, ga );

	ExportHelper::LongVector nodeIndices;
	ExportHelper::FloatVector UVs;
	ExportHelper::ClusterInfoVector clusterInfoVec;

	ExportHelper::PrepareUVData( UVs, nodeIndices, clusterInfoVec, m_FaceCounts, ga, polymeshGeom );

	ExportHelper::FloatVector normals;
	ExportHelper::LongVector normalNodeIndices;
	ExportHelper::PrepareNormalData( normals, normalNodeIndices, m_FaceCounts, ga, polymeshGeom );

	// Get the bounding box
	double centerx;
	double centery;
	double centerz;
	double extentx;
	double extenty;
	double extentz;
	MATH::CTransformation XfoObjectToBBoxSpace;
	polymeshGeom.GetBoundingBox( centerx, centery, centerz, extentx, extenty, extentz, XfoObjectToBBoxSpace );

	Alembic::Abc::Box3d AbcBBox(
		Alembic::Abc::V3d( centerx - extentx, centery - extenty, centerz - extentz ),
		Alembic::Abc::V3d( centerx + extentx, centery + extenty, centerz + extentz )
		);

	assert( m_spAbcOPrim->GetType() == EOObject_Polymesh );
	CAbcPtr<IAbcOPolyMesh> spAbcPolyMesh = static_cast<IAbcOPolyMesh*>( m_spAbcOPrim.GetPtr() );

	spAbcPolyMesh->AddSample(
		fvPos.GetArray(), (int)fvPos.GetCount()/3,
		(const int*)cwVertIndices.GetArray(), (int)cwVertIndices.GetCount(),
		(const int*)m_FaceCounts.GetArray(), (int)m_FaceCounts.GetCount(),
		UVs.GetArray(), (int)UVs.GetCount()/2,
		(const unsigned int*)nodeIndices.GetArray(), (int)nodeIndices.GetCount(),
		(const unsigned int*)normalNodeIndices.GetArray(), (int)normalNodeIndices.GetCount(),
		normals.GetArray(), (int)normals.GetCount()/3,
		AbcBBox
		);

	for ( ExportHelper::ClusterInfoVector::const_iterator it=clusterInfoVec.begin(); it!=clusterInfoVec.end(); ++it)
	{
		ExportHelper::ClusterInfo* pClusterInfo = *it;

		CAbcPtr<IAbcOFaceSet> spOFaceSet;
		spAbcPolyMesh->GetFaceSet( &spOFaceSet, pClusterInfo->m_csName.GetAsciiString() );
		
		if ( spOFaceSet == NULL )
			spAbcPolyMesh->CreateFaceSet( &spOFaceSet, pClusterInfo->m_csName.GetAsciiString() );
		
		if ( spOFaceSet)
			spOFaceSet->AddSample( (const Alembic::Util::int32_t*) pClusterInfo->m_Elements.GetArray(), pClusterInfo->m_Elements.GetCount() );
	}

	for ( ExportHelper::ClusterInfoVector::iterator it=clusterInfoVec.begin(); it!=clusterInfoVec.end(); ++it)
	{
		delete *it;
	}

	WriteAttributeSample( prim, polymeshGeom );

	return CStatus::OK;
}

//===========================================================================================================================================================================
// LOCATOR
//===========================================================================================================================================================================
XSI::CStatus ExportTreeNode::CreateLocator( IAbcOXform** out_ppXfo, XSI::SIObject in_obj, IAbcOObject* in_pParent, const XSI::CString& in_csName )
{
	// Null and bone primitives are exported as a transform
	CAbcPtr<IAbcOXform> spAbcPrim;
	in_pParent->CreateChild( (IAbcOObject**)&spAbcPrim, EOObject_Xform, in_csName.GetAsciiString() );
	
	CAbcPtr<IAbcOCompoundProperty> spPrimProps;
	spAbcPrim->GetProperties( &spPrimProps );
	
	CAbcPtr< IAbcOTypedScalarProperty<Alembic::Abc::Box3dTPTraits> > spLocatorProp;
	spPrimProps->CreateProperty( (IAbcOProperty**)&spLocatorProp, "locator", EPropertyType_Scalar, EDataTraits_Box3d );

	Alembic::Abc::Box3d vals;
	// locator local position
	vals.min.x = vals.min.y = vals.min.z = 0.0;
	// locator local scale
	vals.max.x = vals.max.y = vals.max.z = 1.0;
	spLocatorProp->AddSample( vals );

	XSI::X3DObject x3dObj( in_obj.GetRef() );
	XSI::Primitive prim = x3dObj.GetActivePrimitive();

	XSI::CRefArray arefAttrs = prim.GetICEAttributes();
	CreateProperties( (IAbcOGeom*)spAbcPrim.GetPtr(), in_obj, arefAttrs, &(m_pExporter->GetUserSpecifiedAttributesOtherPrim()) );

	m_pExporter->AddAnimatedItem( this );

	spAbcPrim->AddRef();
	*out_ppXfo = spAbcPrim;

	return XSI::CStatus::OK;
}

XSI::CStatus ExportTreeNode::WriteLocatorSample( double in_dFrame, double in_dTime )
{
	XSI::X3DObject x3DObj( m_Obj );

	XSI::Primitive prim = x3DObj.GetActivePrimitive( in_dFrame );
	XSI::Geometry geom = prim.GetGeometry( in_dFrame, siConstructionModeSecondaryShape );	
	
	WriteAttributeSample( prim, geom );

	return XSI::CStatus::OK;
}

//===========================================================================================================================================================================
// XFORM
//===========================================================================================================================================================================
XSI::CStatus ExportTreeNode::CreateXform( IAbcOXform** out_ppXfo, XSI::SIObject in_obj, IAbcOObject* in_pParent, const XSI::CString& in_csName )
{
	CAbcPtr<IAbcOXform> spAbcXfo;
	in_pParent->CreateChild( (IAbcOObject**)&spAbcXfo, EOObject_Xform, in_csName.GetAsciiString() );

	double dFPS = GetObjectFPS( );
	spAbcXfo->SetTimeSampling( dFPS, m_pExporter->GetStartTime() );

	spAbcXfo->AddRef();
	*out_ppXfo = spAbcXfo;

	return CStatus::OK;
}

XSI::CStatus ExportTreeNode::WriteXformSample( double in_dFrame, double in_dTime )
{
	XSI::X3DObject x3dObj( m_Obj );

	XSI::Kinematics kine = x3dObj.GetKinematics();

	XSI::X3DObject x3dObjParent( x3dObj.GetParent() );

	MATH::CTransformation thisGlobal		= kine.GetGlobal().GetTransform( in_dFrame );
	MATH::CTransformation parentGlobal;

	if ( x3dObjParent.IsValid() )
	{
		parentGlobal	= x3dObjParent.GetKinematics().GetGlobal().GetTransform( in_dFrame );
	}

	MATH::CMatrix4 thisGlobalMat = thisGlobal.GetMatrix4();
	MATH::CMatrix4 parentGlobalMat = parentGlobal.GetMatrix4();

	Alembic::Abc::M44d thisM44d;
	Alembic::Abc::M44d parentM44d;

	AbcFramework::DataTypeConverter<XSI::MATH::CMatrix4, Alembic::Abc::M44d>::Convert( thisM44d, thisGlobalMat );
	AbcFramework::DataTypeConverter<XSI::MATH::CMatrix4, Alembic::Abc::M44d>::Convert( parentM44d, parentGlobalMat );

	m_spAbcOXform->AddSampleFromMatrix( thisM44d, parentM44d );

	return XSI::CStatus::OK;
}

//===========================================================================================================================================================================
// CAMERA
//===========================================================================================================================================================================
XSI::CStatus ExportTreeNode::CreateCamera( IAbcOCamera** out_ppCamera, XSI::SIObject in_obj, IAbcOObject* in_pParent, const XSI::CString& in_csName )
{
	CAbcPtr<IAbcOCamera> spAbcCamera;
	if ( in_pParent->CreateChild( (IAbcOObject**)&spAbcCamera, EOObject_Camera, in_csName.GetAsciiString() ) == EResult_Success )
	{
		LONG startFrame = m_pExporter->GetStartFrame();
		LONG endFrame = m_pExporter->GetEndFrame();

		double dFPS = GetObjectFPS( );
		spAbcCamera->SetTimeSampling( dFPS, m_pExporter->GetStartTime() );

		XSI::Camera cameraObj( in_obj.GetRef() );
		XSI::Camera currentCamera( cameraObj.EvaluateAt( startFrame ) );
		bool bOrtho = ( (LONG)currentCamera.GetParameterValue( L"proj" ) == 0 );
		if ( !bOrtho )
		{
			XSI::Geometry geom = cameraObj.GetActivePrimitive().GetGeometry();

			XSI::CRefArray arefAttrs = geom.GetICEAttributes();
			CreateProperties( (IAbcOGeom*)spAbcCamera.GetPtr(), in_obj, arefAttrs, &(m_pExporter->GetUserSpecifiedAttributesOtherPrim()) );

			m_pExporter->AddAnimatedItem( this );
		}
		else
		{
			CString csMsg;
			csMsg = CString("The Camera \"") + in_csName + CString("\" is orthographic, properties won't be exported");

			Application().LogMessage( csMsg, siWarningMsg );
		}

		spAbcCamera->AddRef();
		*out_ppCamera = spAbcCamera;
	}
	return CStatus::Fail;
}

XSI::CStatus ExportTreeNode::WriteCameraSample( double in_dFrame, double in_dTime )
{
	XSI::Camera cameraObj( m_Obj );
	assert( m_spAbcOPrim->GetType() == EOObject_Camera );
	CAbcPtr<IAbcOCamera> spAbcCamera = static_cast<IAbcOCamera*>( m_spAbcOPrim.GetPtr() );

	if ( cameraObj.IsValid() )
	{
		XSI::Camera currentCamera( cameraObj.EvaluateAt( in_dFrame ) );

		CAbcPtr<IAbcOCameraSample> spSample;
		if ( spAbcCamera->CreateCameraSample( &spSample ) == EResult_Success )
		{
			bool bOrtho = ( (LONG)currentCamera.GetParameterValue( L"proj" ) == 0 );
			if( !bOrtho )
			{
				// Get the scene rendering options for shutter open/close
				double dShutterOpen = 0.0;
				double dShutterClose = 0.0;
				bool bMotionBlur = currentCamera.GetParameterValue( L"motionblurenable" );
				if ( bMotionBlur )
				{
					double dShutterSpeed = currentCamera.GetParameterValue( L"motionblurspeed" );
					double dShutterOffset = currentCamera.GetParameterValue( L"motionbluroffset" );
					dShutterOpen = dShutterOffset;
					dShutterClose = dShutterOpen + dShutterSpeed;
				}
				else
				{
					XSI::Pass curPass = Application().GetActiveProject().GetActiveScene().GetActivePass();
					if ( curPass.IsValid() && (bool)curPass.GetParameterValue(L"MotionBlur") == true )
					{

						double dShutterSpeed = curPass.GetParameterValue( L"MotionBlurShutterSpeed" );
						double dShutterOffset = curPass.GetParameterValue( L"MotionBlurShutterOffset" );

						dShutterOpen = dShutterOffset;
						dShutterClose = dShutterOpen + dShutterSpeed;
					}
				}
				// Get the camera PPG values
				double dNear = currentCamera.GetParameterValue( L"near" );
				double dFar = currentCamera.GetParameterValue( L"far" );
				double dAspect = currentCamera.GetParameterValue( L"aspect" );
				double dFocal = currentCamera.GetParameterValue( L"projplanedist" ); // Focal Length in mm
				double dHorizAperture = (double)currentCamera.GetParameterValue( L"projplanewidth" ) * 2.54; // inches to cm
				double dVertAperture = (double)currentCamera.GetParameterValue( L"projplaneheight" ) * 2.54; // inches to cm
				double dProjPlaneOffsetX = (double)currentCamera.GetParameterValue( L"projplaneoffx" ) * 2.54; // inches to cm
				double dProjPlaneOffsetY = (double)currentCamera.GetParameterValue( L"projplaneoffy" ) * 2.54; // inches to cm						

				Alembic::Abc::Box3d bounds;
				currentCamera.GetBoundingBox( bounds.min.x, bounds.min.y, bounds.min.z, bounds.max.x, bounds.max.y, bounds.max.z );

				// If projection plane is disabled, calculate aperture based from FOV and aspect ratio
				bool bProjPlaneEnable = currentCamera.GetParameterValue( L"projplane" );
				if ( !bProjPlaneEnable )
				{
					dProjPlaneOffsetX = 0.0;
					dProjPlaneOffsetY = 0.0;

					int iFovType = currentCamera.GetParameterValue( L"fovtype" );
					double dFov = currentCamera.GetParameterValue( L"fov" );

					// Get the correct horizontal focal length if our fov is vertical
					if ( iFovType == 0 )
					{
						dFocal = m_pExporter->GetFrameworkPtr()->GetUtils().FocalLengthFromFov( dFov, dHorizAperture );
					}
				}

				spSample->SetChildBounds( bounds );
				spSample->SetClippingPlanes( dNear, dFar );
				spSample->SetLensSqueezeRatio( dAspect );
				spSample->SetAperture( dHorizAperture, dVertAperture );
				spSample->SetFocalLength( dFocal );
				spSample->SetFilmOffset( dProjPlaneOffsetX, dProjPlaneOffsetY );	
				spSample->SetShutterOpen( dShutterOpen );
				spSample->SetShutterClose( dShutterClose );
				spAbcCamera->AddCameraSample( spSample );


			}
		}

		XSI::Primitive prim = cameraObj.GetActivePrimitive( in_dFrame );
		XSI::Geometry cameraGeom = prim.GetGeometry( in_dFrame );
		WriteAttributeSample( prim, cameraGeom );
		return CStatus::OK;
	}

	return CStatus::Fail;
}
//===========================================================================================================================================================================
// ICE ATTRIBUTES -> PROPERTIES
//===========================================================================================================================================================================


XSI::CStatus ExportTreeNode::CreateProperties( IAbcOGeom* in_pAbcGeom, XSI::SIObject& in_obj, XSI::CRefArray& in_arefAttrs, const AttributeNameSet* in_pUserAttrs /*= NULL*/, const AttributeNameSet* in_pExcludedAttrs /*= NULL */ )
{
	CAbcPtr<IAbcOCompoundProperty> spArbGeomParams;
	in_pAbcGeom->GetArbGeomParams( &spArbGeomParams );

	for ( LONG i=0; i< in_arefAttrs.GetCount(); ++i )
	{
		XSI::ICEAttribute attr( in_arefAttrs[i] );
		CString csName = attr.GetName();
		if ( in_pUserAttrs && in_pUserAttrs->Find(csName) )
		{
			m_AttrNameSet.Insert( csName );
		}
	}

	if ( in_pExcludedAttrs )
	{
		for ( AttributeNameSet::ConstIterator it = in_pExcludedAttrs->Begin(); it != in_pExcludedAttrs->End(); ++it )
		{
			m_AttrNameSet.Erase( *it );
		}
	}

	AttributeNameSet validAttrs;

	ULONG SUPPORTED_TYPES = 
		siICENodeDataBool | 
		siICENodeDataLong | 
		siICENodeDataFloat | 
		siICENodeDataVector2 | 
		siICENodeDataVector3 | 
		siICENodeDataVector4 | 
		siICENodeDataQuaternion | 
		siICENodeDataRotation | 
		siICENodeDataMatrix33 | 
		siICENodeDataMatrix44 | 
		siICENodeDataColor4 | 
		siICENodeDataShape	|
		siICENodeDataString ;

	for (AttributeNameSet::ConstIterator it = m_AttrNameSet.Begin(); it != m_AttrNameSet.End(); ++it )
	{
		const XSI::CString& csAttrName = *it;
		
		XSI::ICEAttribute attr = in_arefAttrs.GetItem( csAttrName );
		
		if ( attr.IsDefined() && !attr.IsReadonly() )
		{
			if ( attr.GetDataType() & SUPPORTED_TYPES )
			{
				CAbcPtr<IAbcOCompoundProperty> spArbGeomParams;
				in_pAbcGeom->GetArbGeomParams( &spArbGeomParams );

				if ( attr.GetDataType() != XSI::siICENodeDataShape )
				{
					EAbcGeomScope scope;
					EAbcDataTraits traits;
					FindAbcGeomParamType( scope, traits, attr );

					if ( attr.GetStructureType() == XSI::siICENodeStructureSingle )
					{
						CAbcPtr<IAbcOGeomParamBase> spGeomParam;
						
						// For per-Sample attributes, we reverse the winding before saving so that we don't have to save the indices
						spArbGeomParams->CreateGeomParam( &spGeomParam, csAttrName.GetAsciiString(), false, traits, scope, 1 );

						validAttrs.Insert( csAttrName );

						CAbcPtr<IAbcOProperty> spProp;
						spGeomParam->GetAsProperty( &spProp );
						m_AttrToPropMap[ csAttrName ] = spProp.GetPtr();
					}
					else if ( attr.GetStructureType() == XSI::siICENodeStructureArray )
					{
						CAbcPtr<IAbcOProperty> spArray2DProp;
						spArbGeomParams->Create2DGeomParam( &spArray2DProp, csAttrName.GetAsciiString(), traits, scope );

						validAttrs.Insert( csAttrName );

						m_AttrToPropMap[ csAttrName ] = spArray2DProp.GetPtr();
					}
				}
				else
				{
					// Shape is exported as a compound with 3 properties: shapeType, shapeInstanceId, shapeInstanceHierarchy (per point);
					// Also, the names of instanced geometries are stored in property "shapeInstancedGeoms" (per object)
					CAbcPtr<IAbcOParticleShapeProperty> spShapeProp;
					spArbGeomParams->CreateParticleShapeProperty( &spShapeProp, csAttrName.GetAsciiString() );
					m_pExporter->AddGeomInstancingPtcloud( in_obj.GetRef(), (IAbcOPoints*)in_pAbcGeom, spShapeProp );

					validAttrs.Insert( csAttrName );

					m_AttrToPropMap[ csAttrName ] = spShapeProp.GetPtr();
				}
			}
			else
			{
				XSI::Application().LogMessage( CUtils::Translate( L"Attribute data type is not supported", L"ABCIMPORTEXPORT" ) + L": " + attr.GetName(), siWarningMsg);
			}
		}
	}

	m_pExporter->MarkExportedAttributes( validAttrs );
	m_AttrNameSet.Swap( validAttrs );

	return XSI::CStatus::OK;
}

XSI::CStatus ExportTreeNode::WriteAttributeSample( XSI::Primitive& in_prim, XSI::Geometry& in_geom )
{
	CAbcPtr<IAbcOGeom> spOGeom = static_cast<IAbcOGeom*>( m_spAbcOPrim.GetPtr() );
	CAbcPtr<IAbcOCompoundProperty> spArbGeomParams;
	spOGeom->GetArbGeomParams( &spArbGeomParams );
	
	bool bIsPolyMesh = spOGeom->GetType() == EOObject_Polymesh;

	for ( AttrToPropMap::const_iterator it = m_AttrToPropMap.begin(); it != m_AttrToPropMap.end(); ++it )
	{
		XSI::ICEAttribute attr;
		if ( in_geom.IsValid() )
			attr = in_geom.GetICEAttributeFromName( it->first );
		else
			attr = in_prim.GetICEAttributeFromName( it->first );

		if ( !attr.IsDefined() )
			continue;

		CAbcPtr<IAbcOProperty> spProp = it->second;

		if ( !spProp )
			continue;
		
		bool bIsPerSampleAttr = bIsPolyMesh && ( attr.GetContextType() == siICENodeContextComponent0D2D );

		switch( attr.GetStructureType() )
		{
		case siICENodeStructureSingle:
			{
				CICEAttributeDataArrayBuffer buffer( attr );

				switch (attr.GetDataType())
				{
				// binary compatible types
				case XSI::siICENodeDataLong:
					{
						if ( bIsPerSampleAttr )
							ICEAttributeDataArrayLongConverter::ConvertAndWritePerSampleDataArray( buffer.m_pBuffer, spProp.GetPtr(), m_FaceCounts.GetArray(), m_FaceCounts.GetCount() ); 
						else
							ICEAttributeDataArrayLongConverter::WriteDataArray( buffer.m_pBuffer, spProp.GetPtr() ); 
					}
					break;
				case XSI::siICENodeDataFloat:
					{
						if ( spOGeom->GetType() == EOObject_Points && it->first.IsEqualNoCase(L"Age") )
							ConvertAndWriteAgeDataArray( buffer.m_pBuffer, spProp.GetPtr(), in_geom );
						else if ( bIsPerSampleAttr )
							ICEAttributeDataArrayFloatConverter::ConvertAndWritePerSampleDataArray( buffer.m_pBuffer, spProp.GetPtr(), m_FaceCounts.GetArray(), m_FaceCounts.GetCount() ); 
						else
							ICEAttributeDataArrayFloatConverter::WriteDataArray( buffer.m_pBuffer, spProp.GetPtr() ); 
					}
					break;
				case XSI::siICENodeDataVector2:
					{
						if ( bIsPerSampleAttr )
							ICEAttributeDataArrayVec2fConverter::ConvertAndWritePerSampleDataArray( buffer.m_pBuffer, spProp.GetPtr(), m_FaceCounts.GetArray(), m_FaceCounts.GetCount() ); 
						else
							ICEAttributeDataArrayVec2fConverter::WriteDataArray( buffer.m_pBuffer, spProp.GetPtr() ); 
					}
					break;
				case XSI::siICENodeDataVector3:
					{
						if ( bIsPerSampleAttr )
							ICEAttributeDataArrayVec3fConverter::ConvertAndWritePerSampleDataArray( buffer.m_pBuffer, spProp.GetPtr(), m_FaceCounts.GetArray(), m_FaceCounts.GetCount() ); 
						else
							ICEAttributeDataArrayVec3fConverter::WriteDataArray( buffer.m_pBuffer, spProp.GetPtr() ); 
					}
					break;

				case XSI::siICENodeDataQuaternion:
					{
						if ( bIsPerSampleAttr )
							ICEAttributeDataArrayQuatfConverter::ConvertAndWritePerSampleDataArray( buffer.m_pBuffer, spProp.GetPtr(), m_FaceCounts.GetArray(), m_FaceCounts.GetCount() ); 
						else
							ICEAttributeDataArrayQuatfConverter::WriteDataArray( buffer.m_pBuffer, spProp.GetPtr() ); 
					}
					break;
				case XSI::siICENodeDataMatrix33:
					{
						if ( bIsPerSampleAttr )
							ICEAttributeDataArrayMatrix33fConverter::ConvertAndWritePerSampleDataArray( buffer.m_pBuffer, spProp.GetPtr(), m_FaceCounts.GetArray(), m_FaceCounts.GetCount() ); 
						else
							ICEAttributeDataArrayMatrix33fConverter::WriteDataArray( buffer.m_pBuffer, spProp.GetPtr() ); 
					}
					break;
				case XSI::siICENodeDataColor4:
					{
						if ( bIsPerSampleAttr )
							ICEAttributeDataArrayColor4fConverter::ConvertAndWritePerSampleDataArray( buffer.m_pBuffer, spProp.GetPtr(), m_FaceCounts.GetArray(), m_FaceCounts.GetCount() ); 
						else
							ICEAttributeDataArrayColor4fConverter::WriteDataArray( buffer.m_pBuffer, spProp.GetPtr() ); 
						
					}
					break;


				// incompatible types
				case XSI::siICENodeDataBool:
					{
						if ( bIsPerSampleAttr )
							ICEAttributeDataArrayBoolConverter::ConvertAndWritePerSampleDataArray( buffer.m_pBuffer, spProp.GetPtr(), m_FaceCounts.GetArray(), m_FaceCounts.GetCount() ); 
						else
							ICEAttributeDataArrayBoolConverter::ConvertAndWriteDataArray( buffer.m_pBuffer, spProp.GetPtr() ); 
					}
					break;
				case XSI::siICENodeDataVector4:
					{
						if ( bIsPerSampleAttr )
							ICEAttributeDataArrayVec4fConverter::ConvertAndWritePerSampleDataArray( buffer.m_pBuffer, spProp.GetPtr(), m_FaceCounts.GetArray(), m_FaceCounts.GetCount() ); 
						else
							ICEAttributeDataArrayVec4fConverter::ConvertAndWriteDataArray( buffer.m_pBuffer, spProp.GetPtr() ); 
					}
					break;
				case XSI::siICENodeDataMatrix44:
					{
						if ( bIsPerSampleAttr )
							ICEAttributeDataArrayMatrix44fConverter::ConvertAndWritePerSampleDataArray( buffer.m_pBuffer, spProp.GetPtr(), m_FaceCounts.GetArray(), m_FaceCounts.GetCount() ); 
						else
							ICEAttributeDataArrayMatrix44fConverter::ConvertAndWriteDataArray( buffer.m_pBuffer, spProp.GetPtr() ); 
					}
					break;
				case XSI::siICENodeDataRotation:
					{
						if ( bIsPerSampleAttr )
							ICEAttributeDataArrayRotationfConverter::ConvertAndWritePerSampleDataArray( buffer.m_pBuffer, spProp.GetPtr(), m_FaceCounts.GetArray(), m_FaceCounts.GetCount() ); 
						else
							ICEAttributeDataArrayRotationfConverter::ConvertAndWriteDataArray( buffer.m_pBuffer, spProp.GetPtr() ); 
					}
					break;
				case XSI::siICENodeDataString:
					{
						if ( bIsPerSampleAttr )
							ICEAttributeDataArrayStringConverter::ConvertAndWritePerSampleDataArray( buffer.m_pBuffer, spProp.GetPtr(), m_FaceCounts.GetArray(), m_FaceCounts.GetCount() ); 
						else
							ICEAttributeDataArrayStringConverter::ConvertAndWriteDataArray( buffer.m_pBuffer, spProp.GetPtr() ); 
					}
					break;
				case XSI::siICENodeDataShape:		
					{
						if ( spOGeom->GetType() == EOObject_Points )
						{
							ConvertAndWriteShapeDataArray( buffer.m_pBuffer, spProp.GetPtr(), static_cast<IAbcOPoints*>(spOGeom.GetPtr()) );
						}
					}
					break;
				default: break;
				}

			}
			break;

		case siICENodeStructureArray:
			{
				CICEAttributeDataArrayBuffer buffer( attr );

				switch (attr.GetDataType())
				{
				case XSI::siICENodeDataBool:		ICEAttributeDataArrayBoolConverter::ConvertAndWriteDataArray2D( buffer.m_pBuffer, spProp.GetPtr() ); break;

				case XSI::siICENodeDataLong:		ICEAttributeDataArrayLongConverter::ConvertAndWriteDataArray2D( buffer.m_pBuffer, spProp.GetPtr() ); break;
				case XSI::siICENodeDataFloat:		ICEAttributeDataArrayFloatConverter::ConvertAndWriteDataArray2D( buffer.m_pBuffer, spProp.GetPtr() ); break;
				case XSI::siICENodeDataVector2:		ICEAttributeDataArrayVec2fConverter::ConvertAndWriteDataArray2D( buffer.m_pBuffer, spProp.GetPtr() ); break;
				case XSI::siICENodeDataVector3:		ICEAttributeDataArrayVec3fConverter::ConvertAndWriteDataArray2D( buffer.m_pBuffer, spProp.GetPtr() ); break;
				case XSI::siICENodeDataVector4:		ICEAttributeDataArrayVec4fConverter::ConvertAndWriteDataArray2D( buffer.m_pBuffer, spProp.GetPtr() ); break;

				case XSI::siICENodeDataQuaternion:	ICEAttributeDataArrayQuatfConverter::ConvertAndWriteDataArray2D( buffer.m_pBuffer, spProp.GetPtr() ); break;;
				case XSI::siICENodeDataMatrix33: 	ICEAttributeDataArrayMatrix33fConverter::ConvertAndWriteDataArray2D( buffer.m_pBuffer, spProp.GetPtr() ); break;
				case XSI::siICENodeDataMatrix44: 	ICEAttributeDataArrayMatrix44fConverter::ConvertAndWriteDataArray2D( buffer.m_pBuffer, spProp.GetPtr() ); break;
				case XSI::siICENodeDataColor4: 		ICEAttributeDataArrayColor4fConverter::ConvertAndWriteDataArray2D( buffer.m_pBuffer, spProp.GetPtr() ); break;

				case XSI::siICENodeDataRotation:	ICEAttributeDataArrayRotationfConverter::ConvertAndWriteDataArray2D( buffer.m_pBuffer, spProp.GetPtr() ); break;

				case XSI::siICENodeDataString:		ICEAttributeDataArrayStringConverter::ConvertAndWriteDataArray2D( buffer.m_pBuffer, spProp.GetPtr() ); break;

				default: break;
				}

			}
		}
	}

	return XSI::CStatus::OK;
}

//===========================================================================================================================================================================
// POINT CLOUD
//===========================================================================================================================================================================
XSI::CStatus ExportTreeNode::CreatePointCloud( IAbcOPoints** out_ppPoints, XSI::SIObject in_obj, IAbcOObject* in_pParent, const XSI::CString& in_csName )
{
	CAbcPtr<IAbcOPoints> spAbcPoints;
	in_pParent->CreateChild( (IAbcOObject**)&spAbcPoints, EOObject_Points, in_csName.GetAsciiString() );

	spAbcPoints->SetTimeSampling( 1.0 / (m_pExporter->GetFrameRate() * GetNbSubframes()), m_pExporter->GetStartTime() );

	XSI::X3DObject x3dObj( in_obj.GetRef() );
	XSI::Geometry pointcloudGeom( x3dObj.GetActivePrimitive().GetGeometry( m_pExporter->GetStartFrame(), siConstructionModeSecondaryShape ) );

	XSI::CRefArray arefAttrs = pointcloudGeom.GetICEAttributes();
	CreateProperties( (IAbcOGeom*)spAbcPoints.GetPtr(), in_obj, arefAttrs, &(m_pExporter->GetUserSpecifiedAttributesPointcloud()), &(m_pExporter->GetExcludedAttributesPointcloud()) );

	// Add the object to the list of animated items
	m_pExporter->AddAnimatedItem( this );


	spAbcPoints->AddRef();
	*out_ppPoints = spAbcPoints;

	return CStatus::OK;
}


XSI::CStatus ExportTreeNode::WritePointcloudSample( double in_dFrame, double in_dTime )
{
	assert( m_spAbcOPrim->GetType() == EOObject_Points );
	CAbcPtr<IAbcOPoints> in_pAbcOPoints = static_cast<IAbcOPoints*>( m_spAbcOPrim.GetPtr() );
	
	XSI::X3DObject x3DObj( m_Obj );

	XSI::Primitive prim = x3DObj.GetActivePrimitive( in_dFrame );
	XSI::Geometry pointcloudGeom = prim.GetGeometry( in_dFrame, siConstructionModeSecondaryShape );

	XSI::ICEAttribute PointPositionAttr = pointcloudGeom.GetICEAttributeFromName( "PointPosition" );
	ULONG ulElemCnt = PointPositionAttr.GetElementCount();

	// Get the point position from ICE attribute
	float* pPos = NULL;
	ExportHelper::FloatVector fvecPos;

	if ( ulElemCnt > 0)
	{
		XSI::CICEAttributeDataArrayVector3f pointPositionDataArray;
		PointPositionAttr.GetDataArray( pointPositionDataArray );

		if ( pointPositionDataArray.IsConstant() )
		{
			fvecPos.Resize( 3 * ulElemCnt );
			XSI::MATH::CVector3f constPos = pointPositionDataArray[0];

			float* pFloat = &fvecPos[0];
			for (LONG i=ulElemCnt; i>0; --i)
			{
				*(pFloat++) = constPos.GetX();
				*(pFloat++) = constPos.GetY();
				*(pFloat++) = constPos.GetZ();
			}

			pPos = &fvecPos[0];
		}
		else
		{
			pPos = (float*)&pointPositionDataArray[0];
		}

		// Get point ID
		unsigned long long* pIDs = NULL;

		XSI::ICEAttribute IDAttr = pointcloudGeom.GetICEAttributeFromName( "ID" );
		XSI::CICEAttributeDataArrayLong IDDataArray;
		IDAttr.GetDataArray( IDDataArray );
		std::vector<unsigned long long> vecID;

		if ( IDDataArray.GetCount() > 0 && IDDataArray.IsConstant() == false )
		{
			vecID.resize( ulElemCnt );
			CastedCopy< unsigned long long, LONG > ( vecID.data(), &IDDataArray[0], ulElemCnt );
		}
		else
		{
			vecID.resize( ulElemCnt, IDDataArray.GetCount() > 0 ? IDDataArray[0] : 0 );
		}
		pIDs = vecID.data();

		// Get point velocity
		float* pVel = NULL;
		XSI::ICEAttribute pointVelocityAttr;
		XSI::CICEAttributeDataArrayVector3f pointVelocityDataArray;
		std::vector<XSI::MATH::CVector3f> fvecVel;

		const AttributeNameSet& userPtCloudAttrs = m_pExporter->GetUserSpecifiedAttributesPointcloud();

		if ( userPtCloudAttrs.Find( L"PointVelocity" ) )
		{
			pointVelocityAttr = pointcloudGeom.GetICEAttributeFromName( "PointVelocity" );
			pointVelocityAttr.GetDataArray( pointVelocityDataArray );

			if ( pointVelocityDataArray.GetCount() > 0 && pointVelocityDataArray.IsConstant() == false )
			{
				pVel = (float*)&pointVelocityDataArray[0];
			}
			else
			{
				fvecVel.resize( ulElemCnt, pointVelocityDataArray.GetCount() > 0 ? pointVelocityDataArray[0] : XSI::MATH::CVector3f() );
				pVel = (float*)fvecVel.data();
			}
		}

		// Get point size
		float* pSizes = NULL;
		ULONG ulSizeCount = 0;
		std::vector<float> fvecSize;
		XSI::ICEAttribute sizeAttr;
		XSI::CICEAttributeDataArrayFloat sizeDataArray;

		if ( userPtCloudAttrs.Find( L"Size" ) )
		{
			sizeAttr = pointcloudGeom.GetICEAttributeFromName( "Size" );
			sizeAttr.GetDataArray( sizeDataArray );

			if ( sizeDataArray.GetCount() > 0 && sizeDataArray.IsConstant() == false )
			{
				pSizes = &sizeDataArray[0];
				ulSizeCount = sizeDataArray.GetCount();
			}
			else if ( sizeDataArray.GetCount() > 0 )
			{
				fvecSize.resize( ulElemCnt, sizeDataArray[0] );
				pSizes = fvecSize.data();
				ulSizeCount = ulElemCnt;
			}
		}

		// Get the bounding box
		double centerx;
		double centery;
		double centerz;
		double extentx;
		double extenty;
		double extentz;
		MATH::CTransformation XfoObjectToBBoxSpace;
		pointcloudGeom.GetBoundingBox( centerx, centery, centerz, extentx, extenty, extentz, XfoObjectToBBoxSpace );

		Alembic::Abc::Box3d AbcBBox(
			Alembic::Abc::V3d( centerx - extentx, centery - extenty, centerz - extentz ),
			Alembic::Abc::V3d( centerx + extentx, centery + extenty, centerz + extentz )
			);

		in_pAbcOPoints->AddSample(
			pPos, ulElemCnt,
			vecID.data(), ulElemCnt,
			pVel, ulElemCnt,
			pSizes, ulSizeCount,
			AbcBBox
			);
	}
	else
	{
		Alembic::Abc::Box3d AbcBBox;

		in_pAbcOPoints->AddSample(
			NULL, 0,
			NULL, 0,
			NULL, 0,
			NULL, 0,
			AbcBBox
			);
	}

	WriteAttributeSample( prim, pointcloudGeom );

	return XSI::CStatus::OK;
}



AbcXsiExporter::AbcXsiExporter( const XSI::CString& in_csFileName, bool in_bIncludeParentHier, EAbcArchiveType in_eArchiveType )
	: AbcXsiIO()
	, m_spOArchive( NULL )
	, m_csFileName( in_csFileName )
	, m_bExportParentHier( in_bIncludeParentHier )
	, m_ulNbPointcloudsExported( 0 )
	, m_ulNbPolymeshesExported( 0 )
{
	m_acsExcludedAttributes_Polymesh += CString( EXCLUDED_ATTRIBUTES_POLYMESH ).Split( "," );
	m_acsExcludedAttributes_Pointcloud += CString( EXCLUDED_ATTRIBUTES_POINTCLOUD ).Split( "," );

	GetFrameworkPtr()->OpenOArchive( in_csFileName.GetAsciiString(), &m_spOArchive, in_eArchiveType );

	if ( m_spOArchive != NULL )
	{
		m_csFileName = m_spOArchive->GetName( );
	}
}

AbcXsiExporter::~AbcXsiExporter()
{
	m_AnimatedItems.clear();
	m_OPointsWithGeomInstanced.clear();
}

XSI::CStatus AbcXsiExporter::ExpandBranchSelection( XSI::CRefArray& out_objs, const XSI::CRefArray& in_objs )
{
	XSI::CRef crefSceneRoot( Application().GetActiveSceneRoot().GetRef() );

	for (LONG i=0; i<in_objs.GetCount(); ++i)
	{
		// If the scene root is selected/specified, export everything regardless if it's branch selected.
		if( in_objs[i] == crefSceneRoot )
		{
			out_objs.Clear();
			return GetAllSceneObjects( out_objs );
		}
		XSI::ProjectItem projItem( in_objs[i] );
		if ( projItem.IsValid() )
		{
			out_objs.Add( projItem.GetRef() );
			if ( projItem.GetSelected( XSI::siBranch ) )
				AddNestedObjects( out_objs, projItem.GetRef() );
		}
	}

	return ( out_objs.GetCount() > 0 )? CStatus::OK : CStatus::False;
}

XSI::CStatus AbcXsiExporter::AddNestedObjects( XSI::CRefArray& out_objs, const XSI::CRef& in_obj )
{
	XSI::CRefArray nestedObjs;
	XSI::X3DObject x3dobj( in_obj );

	// for debugging
	XSI::CString csName = x3dobj.GetFullName();

	if ( x3dobj.IsValid() )
	{
		nestedObjs += x3dobj.GetChildren();
	}
	else
	{
		XSI::Group group( in_obj );
		if ( group.IsValid() )
			nestedObjs += group.GetMembers();
	}

	for ( LONG j=0; j<nestedObjs.GetCount(); ++j )
	{
		XSI::SceneItem sceneItem( nestedObjs[j] );
		if ( sceneItem.IsValid() )
		{
			out_objs.Add( sceneItem.GetRef() );
			AddNestedObjects( out_objs, sceneItem.GetRef() );
		}
	}

	return XSI::CStatus::OK;
}

XSI::CStatus AbcXsiExporter::ExportObjects( const XSI::CRefArray& in_objects, LONG in_lStartFrame, LONG in_lEndFrame )
{
	if ( GetFrameworkPtr() == NULL ||  m_spOArchive == NULL )
		return CStatus::Fail;

	Application app;

	// Check the frame rate and scene name
	{
		// Get the current project
		Project prj = app.GetActiveProject();

		// The PlayControl property set is stored with scene data under the project
		CRefArray proplist = prj.GetProperties();
		Property playctrl( proplist.GetItem(L"Play Control") );

		m_dFrameRate = playctrl.GetParameterValue( L"Rate" );

		XSI::Scene scene( prj.GetActiveScene() );
		CParameterRefArray params = scene.GetParameters();

		// Save the scene name
		Parameter nameParam = params.GetItem( L"Name" );
		m_csSceneName = nameParam.GetValue( double(1) );

		m_lSceneInFrame = playctrl.GetParameterValue( L"In");
	}

	m_lStartFrame = in_lStartFrame;
	m_lEndFrame = in_lEndFrame;

	CRefArray allObjs;
	if( in_objects.GetCount() > 0 )
	{
		ExpandBranchSelection( allObjs, in_objects );
	}
	else
	{
		// If no object is specified, export the whole scene
		GetAllSceneObjects( allObjs );
	}

	if( allObjs.GetCount() == 0 )
	{
		app.LogMessage( CUtils::Translate( L"No supported object found to export", L"ABCIMPORTEXPORT" ), siErrorMsg );
		return CStatus::InvalidArgument;
	}
	std::auto_ptr<ExportTreeRootNode> pRootExportNode( new ExportTreeRootNode( this, allObjs, m_bExportParentHier ) ); 

	CAbcPtr<IAbcOObject> spTop;
	m_spOArchive->GetTop( &spTop );
	
	CStatus status;

	m_ProgressBar.SetMin( 0 );
	m_ProgressBar.SetMax( GetEndFrame() - GetStartFrame() );
	m_ProgressBar.SetValue( 0 );
	m_ProgressBar.SetVisible( true );
	m_ProgressBar.SetCaption( L"Caching objects..." );

	// Export hierarchy
	status = pRootExportNode->CreateOObject( spTop );

	// Export animation
	status = ExportAnimation();

	m_ProgressBar.SetVisible( false );

	if ( status != XSI::CStatus::OK )
		return status;

	// Check if any pointcloud produces a list of instanced geometries
	// Collect the CRef of the instanced geometries
	XSI::CRefArray refInstancedGeoms;

	for ( GeomInstancingPtcloudMap::iterator it = m_OPointsWithGeomInstanced.begin(); it != m_OPointsWithGeomInstanced.end(); ++it )
	{
		CAbcPtr<IAbcOPoints> spAbcOPoints = it->second.m_spOPoints.GetPtr();
		ULONG ulNbInstancedGeoms = spAbcOPoints->GetNbInstancedGeom();

		if ( ulNbInstancedGeoms > 0 )
		{
			std::vector<unsigned int> instancedGeomDBIDVec( ulNbInstancedGeoms );
			unsigned int* pDBIDs = instancedGeomDBIDVec.data();
			bool* pInstancedHierarchy = new bool[ ulNbInstancedGeoms ];

			spAbcOPoints->GetInstancedGeom( &pDBIDs, &pInstancedHierarchy );

			for ( ULONG i=0; i<ulNbInstancedGeoms; ++i )
			{
				XSI::ProjectItem projItem = app.GetObjectFromID( instancedGeomDBIDVec[i] );
				if ( projItem.IsValid() )
				{
					refInstancedGeoms.Add( projItem.GetRef() );
					if ( pInstancedHierarchy[i])
						AddNestedObjects( refInstancedGeoms, projItem.GetRef() );
				}
			}

			delete [] pInstancedHierarchy;
		}
	}
	
	// Export the instanced geometries which haven't been
	if( refInstancedGeoms.GetCount() > 0 )
	{
		pRootExportNode->CreateHierarchy( refInstancedGeoms, m_bExportParentHier );		

		status = pRootExportNode->CreateOObject( spTop );

		// Export the names of instanced geometries
		for ( GeomInstancingPtcloudMap::const_iterator it = m_OPointsWithGeomInstanced.begin(); it!=m_OPointsWithGeomInstanced.end(); ++it )
		{
			CAbcPtr<IAbcOPoints> spAbcOPoints = it->second.m_spOPoints;

			ULONG ulNbInstancedGeoms = spAbcOPoints->GetNbInstancedGeom();

			typedef std::vector<unsigned int> UIntVec;
			UIntVec instancedGeomDBIDs( ulNbInstancedGeoms );
			unsigned int* pDBID = instancedGeomDBIDs.data();
			spAbcOPoints->GetInstancedGeom( &pDBID, NULL );

			typedef std::vector<std::string> StringVec;
			StringVec exportedInstancedGeomNames;

			for ( UIntVec::const_iterator itDBID = instancedGeomDBIDs.begin(); itDBID != instancedGeomDBIDs.end(); ++itDBID )
			{
				XSI::ProjectItem projItem = app.GetObjectFromID( *itDBID );
				CAbcPtr<IAbcOObject> spExportedOObj = m_AnimatedItems[ projItem.GetRef() ].m_pExportTreeNode->GetOObject();
				exportedInstancedGeomNames.push_back( spExportedOObj->GetFullName() );
			}

			CAbcPtr< IAbcOTypedArrayProperty<Alembic::Abc::StringPODTraits> > spInstancedGeomsProp;
			spAbcOPoints->GetInstancedGeomProperty( (IAbcOProperty**)&spInstancedGeomsProp );

			spInstancedGeomsProp->AddSample( exportedInstancedGeomNames.data(), (int)exportedInstancedGeomNames.size() );
		}

		m_ProgressBar.SetMin( 0 );
		m_ProgressBar.SetMax( GetEndFrame() - GetStartFrame() );
		m_ProgressBar.SetValue( 0 );
		m_ProgressBar.SetCaption( L"Caching instanced geometries..." );

		status = ExportAnimation();

		m_ProgressBar.SetVisible( false );
	}
	
	return status;
}

void AbcXsiExporter::AddAnimatedItem( ExportTreeNode* in_pNode )
{
	AnimatedItemMap::iterator it = m_AnimatedItems.find( in_pNode->GetCRef() );
	if ( it == m_AnimatedItems.end() )
	{
		m_AnimatedItems[in_pNode->GetCRef()] = AnimatedItem( in_pNode );
	}

	XSI::SIObject siObj( in_pNode->GetCRef() );

	XSI::CString csType( siObj.GetType() );

	if ( csType.IsEqualNoCase(XSI::siPolyMeshType) )
		m_ulNbPolymeshesExported++;
	else if ( csType.IsEqualNoCase(XSI::siPointCloudPrimType) )
		m_ulNbPointcloudsExported++;
	else
		m_ulNbOtherPrimExported++;
}


XSI::CStatus AbcXsiExporter::ExportAnimation()
{
	FirstFrame();

	double dFrame;

	for (dFrame = m_lSceneInFrame; dFrame < GetStartFrame(); dFrame += 1.0 )
	{
		NextFrame();
	}

	bool bIsCanceled = false;
	for (dFrame = GetStartFrame(); dFrame <= GetEndFrame() && !bIsCanceled; dFrame += 1.0, NextFrame() )
	{
		for ( AnimatedItemMap::iterator it = m_AnimatedItems.begin(); it != m_AnimatedItems.end(); ++it )
		{
			if ( it->second.m_bExported == false )
			{
				ULONG ulNbSubframes = it->second.m_pExportTreeNode->GetNbSubframes();
				for ( ULONG subFrame = 0; subFrame < ulNbSubframes; ++subFrame )
				{
					double dFractionalFrame = dFrame + (double)subFrame/ulNbSubframes;
					it->second.m_pExportTreeNode->WriteSample( dFractionalFrame, ( dFrame * ulNbSubframes + subFrame ) / GetFrameRate() );
				}
			}
		}

		m_ProgressBar.Increment();
		XSI::CString statusText("Frame ");
		statusText += XSI::CString( dFrame );

		m_ProgressBar.SetStatusText( statusText );

		bIsCanceled |= m_ProgressBar.IsCancelPressed();
	}

	for ( AnimatedItemMap::iterator it = m_AnimatedItems.begin(); it != m_AnimatedItems.end(); ++it )
	{
		it->second.m_bExported = true;
	}

	if ( bIsCanceled )
		return XSI::CStatus::Abort;

	return XSI::CStatus::OK;
}

XSI::CStatus AbcXsiExporter::RunCacheToFileCommand( XSI::CRefArray& in_refObjs, LONG in_lStartFrame, LONG in_lEndFrame, const XSI::CValueArray& in_attrs, const XSI::CString& in_csTargetCacheObjNames )
{
	if ( in_refObjs.GetCount() > 0 )
	{
		// call command CacheObjectsIntoFile
		XSI::Application app;

		XSI::CValue returnVal;
		XSI::CValueArray args(12);

		args[0] = in_refObjs;
		args[1] = 5;
		args[2] = in_lStartFrame;
		args[3] = in_lEndFrame;
		args[4] = 1;
		args[5] = true;	// overwrite if Abc file exists. Must be true so that BaseCache::GetCacheInfo() won't wait for file lock
		args[6] = true;   // show progress bar
		args[7] = in_attrs;
		args[8] = GetFrameworkPtr()->GetAlembicVersionString();
		args[9] = GetFileName();
		args[10] = false;
		args[11] = in_csTargetCacheObjNames;

		XSI::CStatus status = app.ExecuteCommand( L"CacheObjectsIntoFile", args, returnVal );
		return status;
	}

	return XSI::CStatus::OK;
}

void AbcXsiExporter::AddGeomInstancingPtcloud( const XSI::CRef& in_refXSIObj, IAbcOPoints* in_pAbcOPoints, IAbcOParticleShapeProperty* in_pOProp )
{
	GeomInstancingPtcloudMap::iterator it = m_OPointsWithGeomInstanced.find( in_refXSIObj );
	if ( it == m_OPointsWithGeomInstanced.end() )
	{
		m_OPointsWithGeomInstanced[ in_refXSIObj ] = GeomInstancingPointcloundInfo( in_pAbcOPoints, in_pOProp );
	}
	else
	{
		assert( false );
	}
}

void AbcXsiExporter::SetUserSpecifiedAttributes( const XSI::CString& in_csPolymeshAttrs, const XSI::CString& in_csPointcloudAttrs, const XSI::CString& in_csOtherPrimAttrs )
{
	m_acsUserSpecifiedAttributes_Polymesh += in_csPolymeshAttrs.Split( "," );
	m_acsUserSpecifiedAttributes_Pointcloud += in_csPointcloudAttrs.Split( "," );
	m_acsUserSpecifiedAttributes_OtherPrim += in_csOtherPrimAttrs.Split( "," );
}

CStatus AbcXsiExporter::GetAllSceneObjects( XSI::CRefArray& out_objs )
{
	CStringArray strFamilies;
	strFamilies.Add( si3DObjectFamily );
	out_objs = Application().GetActiveSceneRoot().FindChildren2(L"", L"", strFamilies, true);
	return ( out_objs.GetCount() > 0 )? CStatus::OK : CStatus::False;
}

void AbcXsiExporter::MarkExportedAttributes( const AttributeNameSet& in_attrs )
{
	m_acsExportedAttributes.Insert( in_attrs );
}

void AbcXsiExporter::GetUnexportedAttributes( AttributeNameSet& out_attrs ) const
{
	AttributeNameSet diff;
	if ( m_ulNbPolymeshesExported > 0 )
	{
		m_acsUserSpecifiedAttributes_Polymesh.Difference( m_acsExportedAttributes, diff );
		out_attrs.Insert( diff );
	}

	if ( m_ulNbPointcloudsExported > 0 )
	{
		m_acsUserSpecifiedAttributes_Pointcloud.Difference( m_acsExportedAttributes, diff );
		out_attrs.Insert( diff );
	}

	if ( m_ulNbOtherPrimExported > 0 )
	{
		m_acsUserSpecifiedAttributes_OtherPrim.Difference( m_acsExportedAttributes, diff );
		out_attrs.Insert( diff );
	}
}
