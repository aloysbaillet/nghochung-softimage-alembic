//*****************************************************************************
/*!
	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*

#ifndef _AbcXsiImporter_h_
#define _AbcXsiImporter_h_
#include "AbcXsiIO.h"
#include "AbcXsiUtils.h"
#include <IAbcInput.h>
#include <AbcFrameworkUtil.h>
#include <xsi_ref.h>
#include <xsi_siobject.h>
#include <xsi_string.h>
#include <xsi_math.h>
#include <xsi_vector3.h>
#include <xsi_rotation.h>
#include <xsi_x3dobject.h>
#include <vector>
#include <string>

// Import Nodes
const int TreeNodeType_Base = 1;
const int TreeNodeType_Xform = 2;
const int TreeNodeType_Renderable = 3;
const int TreeNodeType_PolyMesh = 4;
const int TreeNodeType_PointCloud = 5;
const int TreeNodeType_Locator = 6;
const int TreeNodeType_Camera = 7;
const int TreeNodeType_FaceSet = 8;

/*! Defines the options for processing the time range
*/
enum ETimeRangeFitting
{
	ETimeRange_Expand = 0,  /*!< Expand timeline if shorter than source  */
	ETimeRange_Fit = 1,		/*!< Set timeline to source */
	ETimeRange_NoFit = 2,	/*!< Don't set the timeline */
	ETimeRange_Max
};

/*! Defines the options for importing animation
*/
enum EAnimationImportMode
{
	EImport_CacheOnFile = 0, /*!< Use ICE node Alembic Cache to read animation*/
	EImport_Recreate = 1,    /*!< Use keyframing. This is deprecated.*/
	EImport_Max
};

/*! Defines the transform components to be keyed
*/
enum ETransformKeyFlags
{
	ETransformKey_None = 0,
	ETransformKey_Position = 1,
	ETransformKey_Rotation = 2,
	ETransformKey_Scale = 4
};

/*! Defines the options for creating ICE trees that read the Alembic cache
*/
enum EImportICETreeMode
{
	EImport_CreateICETreePerObj, /*!< Create multiple ICE trees, one for each object in the imported archive */
	EImport_SingleICETree		 /*!< Create a single ICE tree for all objects in the imported archive  */
};

/*! Encapsulates a transform key
*/
struct STransformKey
{
	XSI::MATH::CVector3 m_Position;
	XSI::MATH::CVector3 m_Rotation;
	XSI::MATH::CVector3 m_Scale;
	long				m_KeyFlags;
};
typedef std::map< long, STransformKey > TKeyMap;
typedef std::vector< STransformKey > TKeyList;

/*! \class AbcImportTreeNode
	\brief Base class for the Alembic objects to be imported into the XSI scene.

	A hierarchy of AbcImportTreeNode is created as we traverse the Alembic object hierarchy. Since Alembic organizes objects and transforms
	differently from how XSI does it, the hierarchy of AbcImportTreeNode needs to be processed into one that matches XSI object hierarchy.
	::BuildImportTree and ::SimplifyImportTree provides details on how the hierarchy is created and processed.
*/
class AbcImportTreeNode
{
protected:
	typedef std::vector<AbcImportTreeNode*> TChildList;
	std::string m_strName;
	TChildList m_children;
	AbcImportTreeNode* m_pParent;

	CAbcPtr<IAbcIObject> m_spObject;
	XSI::CRef m_Ref;
public:
	/*! Constructor
	\param pObject The Alembic object represented by this AbcImportTreeNode
	*/
	AbcImportTreeNode( IAbcIObject* pObject ) : m_strName( pObject->GetName() ), m_pParent(NULL), m_spObject( pObject ) {}
	DECL_BASE_TYPE_INFO( TreeNodeType_Base );

	/*! Get the name of the Alembic object
	\return The name of the Alembic object
	*/
	const char*			GetName() const;

	/*! Get the Alembic object
	\return Pointer to the Alembic object
	*/
	IAbcIObject*		GetObject();

	/*! Get the Alembic object
	\return Pointer to the Alembic object
	*/
	const IAbcIObject*	GetObject() const;

	/*! Add a child node
	\param pNode The child node
	*/
	void				AddChild( AbcImportTreeNode* pNode );

	/*! Reparent the children of this node to another node
	\param pNewParent The new parent
	*/
	void				ReparentChildren( AbcImportTreeNode* pNewParent );

	/*! Remove a child node
	\param pNode The child node
	\return bDelete Delete the child node
	*/
	void				RemoveChild( AbcImportTreeNode* pNode, bool bDelete );

	/*! Get the number of children
	\return The number of childrent
	*/
	size_t				GetNumChildren() const;

	/*! Get the parent node
	\return The parent node casted into type T
	*/
	template<typename T> T* GetParent() const;

	/*! Get the child node at a given index
	\param index The index
	\return The child node casted into type T
	*/
	template<typename T> T* GetChild( size_t index );

	/*! Set the CRef reference to the XSI object created
	\param in_Ref The CRef reference
	*/
	void SetRef( const XSI::CRef& in_Ref );

	/*! Get the CRef reference to the XSI object created
	\return The CRef reference
	*/
	XSI::CRef GetRef() const;

	/*! Destructor
	*/
	virtual ~AbcImportTreeNode();
};

/*! \class AbcImportXform
	\brief The AbcImportXform object represents an Alembic Xform object to be imported into a Kinematics in XSI
*/
class AbcImportXform : public AbcImportTreeNode
{
public:
	DECL_TYPE_INFO( TreeNodeType_Xform, AbcImportTreeNode );
	bool	m_bInheritParent;
	bool	m_bHasShearing;
	long	m_lStartFrame;
	XSI::MATH::CMatrix4 m_LocalMat;

	TKeyMap	m_TransformKeys;

	AbcImportXform( IAbcIXform* pObject ) : AbcImportTreeNode( pObject ), m_bInheritParent( true ), m_bHasShearing( false ), m_lStartFrame( 0 ) {}

	void ResetTransform( AbcImportXform* pNewXform, bool bResetName );
	bool IsConstantIdentity() const;
	XSI::MATH::CMatrix4 GetGlobalTransform() const;
};

/*! \class AbcImportFaceSet
	\brief The AbcImportFaceSet represents a faceset to be imported into the XSI scene as a Cluster
*/
class AbcImportFaceSet : public AbcImportTreeNode
{
public:
	DECL_TYPE_INFO( TreeNodeType_FaceSet, AbcImportTreeNode );
	
	AbcImportFaceSet( IAbcIFaceSet* pObject ) : AbcImportTreeNode( pObject ) {}
};

/*! \class AbcImportRenderable
	\brief The AbcImportRenderable represents a 3D object to be imported into the XSI scene
*/
class AbcImportRenderable : public AbcImportTreeNode
{
public:
	DECL_TYPE_INFO( TreeNodeType_Renderable, AbcImportTreeNode );
	AbcImportXform* m_pXform;
	AbcImportRenderable( IAbcIObject* pObject ) : AbcImportTreeNode( pObject ), m_pXform( NULL ) {}

	virtual ~AbcImportRenderable()
	{
		if ( m_pXform )
			delete m_pXform;
	}
};

/*! \class AbcImportPolymesh
	\brief The AbcImportPolymesh represents a polymesh object to be imported into the XSI scene
*/
class AbcImportPolymesh : public AbcImportRenderable
{
public:
	DECL_TYPE_INFO( TreeNodeType_PolyMesh, AbcImportRenderable );
	AbcImportPolymesh( IAbcIPolyMesh* pObject ) : AbcImportRenderable( pObject ) {}
};

/*! \class AbcImportPointCloud
	\brief The AbcImportPointCloud represents a pointcloud object to be imported into the XSI scene
*/
class AbcImportPointCloud : public AbcImportRenderable 
{
public:
	DECL_TYPE_INFO( TreeNodeType_PointCloud, AbcImportRenderable );
	AbcImportPointCloud( IAbcIPoints* pObject ) : AbcImportRenderable( pObject ) {}
};

/*! \class AbcImportLocator
	\brief The AbcImportLocator represents a locator object to be imported into the XSI scene as a Null
*/
class AbcImportLocator : public AbcImportRenderable 
{
public:
	DECL_TYPE_INFO( TreeNodeType_Locator, AbcImportRenderable );
	AbcImportLocator( IAbcIXform* pObject ) : AbcImportRenderable( pObject ) {}
};

/*! \class AbcImportCamera
	\brief The AbcImportCamera object represents a camera object to be imported into the XSI scene
*/
class AbcImportCamera : public AbcImportRenderable
{
public:
	DECL_TYPE_INFO( TreeNodeType_Camera, AbcImportRenderable );
	AbcImportCamera( IAbcICamera* pObject ) : AbcImportRenderable( pObject ) {}
};

template<typename T>
T* node_cast( AbcImportTreeNode* pNode )
{
	if ( isTypeOf( pNode->GetInstanceTypeInfo(), T::GetTypeInfo() ) )
		return (T*)pNode;
	return NULL;
}

template<typename T>
bool isNodeA( AbcImportTreeNode* pNode )
{
	return isTypeOf( pNode->GetInstanceTypeInfo(), T::GetTypeInfo() );
}

bool isNodeRenderable( AbcImportTreeNode* pNode );

/*! \struct STimeRange
	\brief This struct represents the time range for export
	*/
struct STimeRange
{
	double m_dStart;
	double m_dEnd;
};

/*! \struct SScenePlaybackSettings
	\brief This struct encapsulates the playback settings of the current XSI scene
	*/
struct SScenePlaybackSettings
{
	double	m_dFrameIn;
	double	m_dFrameOut;
	double	m_dGlobalIn;
	double	m_dGlobalOut;
	double	m_dFrameRate;

	SScenePlaybackSettings();
	void Apply();

	double TimeToFrame( double in_dTime );
	double FrameToTime( double in_dFrame );
};

/*! \class AbcImportTreeTraverser
	\brief This class handles the traversing of a hierarchy of AbcImportTreeNode to 
	simplify it into an XSI-friendly structure before the XSI objects are created.
*/
template< typename TVisitor >
class AbcImportTreeTraverser
{
protected:
	XSI::CStatus m_Status;

	struct PrePostCaller
	{
		TVisitor& m_Visitor;
		AbcImportTreeNode* m_pNode;
		PrePostCaller( TVisitor& in_Visitor, AbcImportTreeNode* in_pNode ) : m_Visitor( in_Visitor ), m_pNode( in_pNode )
		{
			m_Visitor.OnPreImportNode( m_pNode );
		}
		~PrePostCaller()
		{
			m_Visitor.OnPostImportNode( m_pNode );
		}
	};
public:
	/*! Constructor
	*/
	AbcImportTreeTraverser( ) { Reset(); }
	
	/*! Reset the traversal status
	*/
	void Reset()
	{
		m_Status = XSI::CStatus::False;
	}

	/*! Return the traversal result
		\return CStatus::OK Success
	*/
	XSI::CStatus GetResult()
	{
		return m_Status;
	}

	/*! Traverse an AbcImportTreeNode hierarchy recursively
		\param in_Visitor The visitor class which implements the callbacks OnPreImportNode, OnImportNode and OnPostImportNode
		\param in_pNode The node from which the traversal begins
		\sa AbcXsiImporter::OnPreImportNode, AbcXsiImporter::OnImportNode and AbcXsiImporter::OnPostImportNode 
	*/
	void Traverse( TVisitor& in_Visitor, AbcImportTreeNode* in_pNode )
	{
		PrePostCaller l_Caller( in_Visitor, in_pNode );

		XSI::CStatus l_status = in_Visitor.OnImportNode( in_pNode );
		if ( l_status.Succeeded() )
		{
			for ( size_t i = 0; i < in_pNode->GetNumChildren(); i++ )
			{
				Traverse( in_Visitor, in_pNode->GetChild<AbcImportTreeNode>(i) );
				if ( !GetResult().Succeeded() )
					return;
			}
		}
		else
		{
			m_Status = l_status;
			return;
		}
	}
};

/*! \class AbcXsiImporter
	\brief The AbcXsiImporter class handles the process of importing an Alembic archive into the XSI scene.
	*/
class AbcXsiImporter : public AbcXsiIO
{
public:
	/*! Constructor
	*/
	AbcXsiImporter();

	/*! Destructor
	*/
	~AbcXsiImporter();

	/*! Set the name of the Alembic archive for import
		\param in_csFileName The full name of the archive
		\return CStatus::OK Succeeded 
	*/
	XSI::CStatus SetFilename( const XSI::CString& in_csFileName );

	/*! Set the import mode for animation
		\param in_eImportMode
		\return CStatus::OK Success
		\sa ::EAnimationImportMode
	*/
	XSI::CStatus SetAnimationImportMode( EAnimationImportMode in_eImportMode );

	/*! Set the name of the Alembic archive for import
		\param in_csFileName The full name of the archive
		\return CStatus::OK Success
	*/
	XSI::CStatus SetTimeParams( bool in_bAnimated, double in_dStart, double in_dEnd, ETimeRangeFitting in_eTimeFitting );

	/*! Set the option to pad shape clips
		\param in_bPad True if the clip should be padded
	*/
	void		 SetPadShapeClips( bool in_bPad );

	/*! Set the option to create a root model to contain the import objects
		\param in_bCreate True if the model should be created
	*/
	void		SetCreateNewRootModel( bool in_bCreate );

	/*! Set the parent 3D object under which the imported objects are nested
		\param in_objParent The XSI 3D object
	*/
	void		SetRootParent( XSI::X3DObject& in_objParent );

	/*! Set the option for creating the cache-reading ICE Trees
		\param in_eCreateMode The create mode
	*/
	void		SetCreateICETreeMode( EImportICETreeMode in_eCreateMode );

	/*! Import the Alembic objects from the archive into the XSI scene
		\return CStatus::OK Success 
	*/
	XSI::CStatus ImportObjects();

	/*! Callback function for processing the hierarchy of AbcImportTreeNode. This callback is called before the Alembic object is created. The callback handles the updating of progress bar.
		\param in_pNode The AbcImportTreeNode object
		\sa AbcImportTreeNode, AbcImportTreeTraverser
	*/
	void OnPreImportNode( AbcImportTreeNode* in_pNode );

	/*! Callback function for processing the hierarchy of AbcImportTreeNode. This callback is called after the Alembic object is created. The callback handles the updating of progress bar.
		\param in_pNode The AbcImportTreeNode object
		\sa AbcImportTreeNode, AbcImportTreeTraverser
	*/
	void OnPostImportNode( AbcImportTreeNode* in_pNode );

	/*! Callback function for processing the hierarchy of AbcImportTreeNode. This callback is called to create XSI objects from Alembic objects.
		\param in_pNode The AbcImportTreeNode object
		\return CStatus::OK Success
		\sa AbcImportTreeNode , AbcImportTreeTraverser
	*/
	XSI::CStatus OnImportNode( AbcImportTreeNode* in_pNode );

private:
	XSI::CStatus ProcessRoot( AbcImportTreeNode* in_pNode );
	XSI::CStatus ProcessPolymesh( AbcImportPolymesh* in_pNode );
	XSI::CStatus ProcessFaceSet( AbcImportFaceSet* in_pNode );
	XSI::CStatus ProcessTransform( AbcImportXform* in_pNode, bool in_bCreateNull );
	XSI::CStatus ProcessPointCloud( AbcImportPointCloud* in_pNode );
	XSI::CStatus ProcessLocator( AbcImportLocator* in_pNode );
	XSI::CStatus ProcessCamera( AbcImportCamera* in_pNode );

	void		 ApplyTransform( XSI::X3DObject& in_Object, AbcImportXform* in_pNode );

	XSI::CStatus CachePolyMesh( IAbcISampleSelector* in_pSampleSelector, IAbcIPolyMesh* in_pPolyMesh, bool in_bRoot, CPolyMeshDefinitionCache& in_Cache, CPolyMeshDefinition* io_pDefinition );
	XSI::CStatus CreatePolyMesh( const XSI::CString& in_csPolyMeshName, CPolyMeshDefinition& in_rPolymeshDef, XSI::X3DObject& in_x3dParentObj, XSI::X3DObject& in_x3dObj, AbcImportPolymesh* in_pNode );
	XSI::CStatus CreateShapeClip( XSI::X3DObject& in_Object, XSI::CRef &in_refShapeKey, double in_dFrame, long in_lNumKeys, const XSI::CString& in_csShapeKeyName );
	XSI::CRef	 CreatePolyMeshShapeKey( const XSI::CString& in_csShapeKeyName, double in_dTime, double in_dFrequency, long in_lNumKeys, CPolyMeshDefinition& in_rPolymeshDef, XSI::X3DObject& in_x3dObj );

	XSI::CStatus UpdatePolyMeshShapeKey( XSI::CRef& in_refKey, long in_lKeyIndex, CPolyMeshDefinition& in_rPolymeshDef );
	XSI::CString GetSchemaString( IAbcIObject* in_pObject );

	XSI::CStatus CreateCachedICETree( XSI::X3DObject& in_TargetObject, const XSI::CString& in_csAlembicFile, IAbcIObject* in_pAbcObject, bool in_bCreateKineCnx );

	XSI::CStatus CreateRootCachedICETree( AbcImportTreeNode* in_pRootNode, const XSI::CString& in_csAlembicFile );

	void		 AddObjectMapping( XSI::CRef in_ref, IAbcIObject* in_pAbcIObj, bool in_bCreateKineCnx );

	SScenePlaybackSettings	m_PlaybackSettings;
	CAbcPtr<IAbcIArchive> m_spIArchive;
	CAbcPtr<IAbcIXform> m_spModelXform;
	XSI::CRef			m_refNewRootModel;
	XSI::CString		m_csModelName;
	XSI::CString		m_csFileName;
	CXsiProgressBar		m_ProgressBar;
	double m_dStartTime;
	double m_dEndTime;
	double m_dMinTime;
	double m_dMaxTime;
	ETimeRangeFitting m_eTimeRangeFitting;
	EAnimationImportMode m_eAnimationImportMode;
	bool m_bAnimated;
	bool m_bPadShapeClips;
	bool m_bCreateRootModel;
	EImportICETreeMode m_bCreateICETreeMode;
	XSI::X3DObject		m_objRootParent;
	struct SCacheObjectInfo
	{
		XSI::CRef m_Ref;
		CAbcPtr<IAbcIObject> m_spAbcIObj;
		bool m_bCreateKineCnx;
	};

	typedef std::vector< SCacheObjectInfo > CacheObjectInfoVector;
	CacheObjectInfoVector m_ObjectMapping;
};

#endif
