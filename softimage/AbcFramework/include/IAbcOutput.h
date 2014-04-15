//*****************************************************************************
/*!
   \file IAbcOutput.h
   \brief Contains the interface definition and enumerations for the ABC framework
   Each Alembic OObject is wrapped by an AbcFramework pure virtual interface, e.g.
   Alembic OPolyMesh is wrapped by AbcFramework OAbcIPolyMesh. This way the internal
   code will work with any version of Alembic.

	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*****************************************************************************
#ifndef IABCOUTPUT_H
#define IABCOUTPUT_H

#include "AbcFrameworkDataTypes.h"
#include <string>

/*! Enumeration for Alembic Object Types */
enum EAbcOObjectType
{
	EOObject_Unknown,
	EOObject_Polymesh,
	EOObject_Subd,
	EOObject_Points,
	EOObject_Material,
	EOObject_Camera,
	EOObject_Xform,
	EOObject_FaceSet
};

class IAbcOProperty;
class IAbcOCompoundProperty;

//*****************************************************************************
/*! \class IAbcOObject
	\brief Base interface for objects contained in IAbcOArchive
*/
//*****************************************************************************
class IAbcOObject : public IBase
{
public:
	/*! Destructor*/
	virtual ~IAbcOObject() = 0;

	/*! Gets the underlying Alembic type of the object
	\return The type of the object
	*/
	virtual	EAbcOObjectType	GetType() const = 0;

	/*! Create a child object with specified type and name
	\param out_ppOObject Pointer to an IAbcOObject pointer that will hold the returned object
	\param in_type The child object type
	\param in_pszChildName The name of the object to find
	\return ::EResult_Success on success, please see ::EAbcResult for more return code information
	*/
	virtual EAbcResult	CreateChild( IAbcOObject** out_ppOObject, EAbcOObjectType in_type, const char* in_pszName ) = 0;
	/*! Gets the child object with the specified name
	\param out_ppOObject Pointer to an IAbcOObject pointer that will hold the returned object
	\param in_pszChildName The name of the object to find
	\return ::EResult_Success on success, please see ::EAbcResult for more return code information
	
	\eg Usage:
	\code
		IAbcOObject* pObj; // Assume pObj points to a valid object
		CAbcPtr<IAbcOObject> pChild;
		pObject->GetChild( "camera", &pChild );
	\endcode
	*/
	virtual EAbcResult	GetChild( IAbcOObject** out_ppOObject, const char* in_pszChildName ) = 0;
	
	/*! Gets the parent of this object
	\param out_ppParent The returned parent of the object
	\return ::EResult_Success on success, please see ::EAbcResult for more return code information
	*/
	virtual EAbcResult	GetParent( IAbcOObject** out_ppParent ) = 0;

	/*! This returns the single top-level OCompoundProperty that exists automatically as part of the object.
	\param out_ppProp The compound property containing the object properties
	\return ::EResult_Success on success, please see ::EAbcResult for more return code information
	*/
	virtual EAbcResult	GetProperties( IAbcOCompoundProperty** out_ppProp ) = 0;

	/*! Gets the name of the object
	\return The name of the object
	*/
	virtual const char* GetName( ) const = 0;

	/*! Gets the name of the object with the full path
	\return The name of the object
	*/
	virtual const char* GetFullName( ) const = 0;
};

/*! \class IAbcOSchemaObj
	\brief The base interface for Alembic output schemas
*/
class IAbcOSchemaObject : public IAbcOObject
{
public:
	/*! Gets the schema property compound associated with this object
	\param out_ppProp The returned compound property representing this object
	\return ::EResult_Success on success, please see ::EAbcResult for more return code information
	*/
	virtual EAbcResult GetSchemaProperties( IAbcOCompoundProperty** out_ppProp ) = 0;
};

/*! \class IAbcOGeom
	\brief This class is used to encapsulate common functionality of the real Geometry schema classes, like IAbcOPoints and IAbcOPolyMesh and so on
*/
class IAbcOGeom : public IAbcOSchemaObject
{
public:
	/*! Get the "arbGeomParams" compound
		\param out_ppProp Pointer to an IAbcOCompoundProperty to hold the compound
		\return ::EResult_Success on success, please see ::EAbcResult for more return code information
	*/
	virtual EAbcResult GetArbGeomParams( IAbcOCompoundProperty** out_ppProp ) = 0;

	/*! Get the "userProperties" compound
		\param out_ppProp Pointer to an IAbcOCompoundProperty to hold the compound
		\return ::EResult_Success on success, please see ::EAbcResult for more return code information
	*/
	virtual EAbcResult GetUserProperties( IAbcOCompoundProperty** out_ppProp ) = 0;
};

class IAbcOFaceSet : public IAbcOGeom
{
public:
	virtual void AddSample( const Alembic::Util::int32_t* in_pIndices, int in_iIndicesCount ) = 0;
};

/*! \class IAbcOPolyMesh
	\brief The interface for accessing Alembic OPolyMesh objects
*/
class IAbcOPolyMesh : public IAbcOGeom
{
public:
	/*! Sets uniform time sampling with the specified time per cycle and the start time.
	\param in_dTimePerCycle
	\param in_dStartTime
	\return ::EResult_Success on success, please see ::EAbcResult for more return code information
	*/
	virtual EAbcResult SetTimeSampling( double in_dTimePerCycle, double in_dStartTime ) = 0;

	/*! Create a face set object for this polymesh.
	\param in_pszName The name of the face set
	\param out_ppOFaceSet The pointer to IAbcOFaceSet pointer which holds the created face set
	\return ::EResult_Success on success, please see ::EAbcResult for more return code information
	*/
	virtual EAbcResult CreateFaceSet( IAbcOFaceSet** out_ppOFaceSet, const char* in_pszName ) = 0;

	/*! Get an existing face set of this polymesh.
	\param in_pszName The name of the face set
	\param out_ppOFaceSet The pointer to IAbcOFaceSet pointer which holds the created face set
	\return ::EResult_Success on success, please see ::EAbcResult for more return code information
	*/
	virtual EAbcResult GetFaceSet( IAbcOFaceSet** out_ppOFaceSet, const char* in_pszName ) = 0;

	/*! Add a sample of this polymesh.
	\param in_pfVertexPositions The array of vertex positions
	\pram in_iVertexCount The number of vertices
	\param in_BBox The bounding box of the sample
	*/
	virtual void AddSample( 
		const float* in_pfVertexPositions, int in_iVertexCount,
		const Alembic::Abc::Box3d& in_BBox
		) = 0;

	/*! Add a sample of this polymesh.
	\param in_pfVertexPositions The array of vertex positions
	\param in_iVertexCount The number of vertices
	\param in_pFaceVertices The array of face vertices
	\param in_pFaceVerticesSize The number of elements in in_pFaceVertices
	\param in_pFaceCounts The array of face vertex count
	\param in_iFaceCountSize The number of elements in in_pFaceCounts
	\param in_BBox The bounding box of the sample
	*/
	virtual void AddSample( 
		const float* in_pfVertexPositions, int in_iVertexCount,
		const int* in_pFaceVertices, int in_iFaceVerticesSize,
		const int* in_pFaceCounts, int in_iFaceCountsSize,
		const Alembic::Abc::Box3d& in_BBox
		) = 0;

	/*! Add a sample of this polymesh.
	\param in_pfVertexPositions The array of vertex positions
	\param in_iVertexCount The number of vertices
	\param in_pFaceVertices The array of face vertices
	\param in_pFaceVerticesSize The number of elements in in_pFaceVertices
	\param in_pFaceCounts The array of face vertex count
	\param in_iFaceCountSize The number of elements in in_pFaceCounts
	\param in_pUVs The array of (u,v) coordinates. in_pUVs[2*i] is the (u,v) coordinate of face vertex at in_pFaceVertexIndices[i]
	\param in_iUVCount The number of (u,v) pairs in in_pUVs
	\param in_pFaceVertexIndices The array of face vertices indexed by in_pUVs and in_pNormals
	\param in_iFaceVertexIndicesCount The number of elements in in_pFaceVertexIndices
	\param in_pNormals The array of normals
	\param in_iNormalCount The number of elements in in_pNormals
	\param in_BBox The bounding box of the sample
	*/
	virtual void AddSample( 
		const float* in_pfVertexPositions, int in_iVertexCount,
		const int* in_pFaceVertices, int in_iFaceVerticesSize,
		const int* in_pFaceCounts, int in_iFaceCountsSize,
		const float* in_pUVs, int in_iUVCount,
		const unsigned int* in_pFaceVertexIndices, int in_iFaceVertexIndicesCount,
		const unsigned int* in_pVertexNormalIndices, int in_iVertexNormalIndicesCount,
		const float* in_pNormals, int in_iNormalCount,
		const Alembic::Abc::Box3d& in_BBox
		) = 0;
};

/*! \class IAbcOPoints
	\brief The interface for accessing Alembic OPoints objects
	The interface provides additional functions to handle shape instancing which is not covered by the Alembic API.
	The following properties are reserved for shape instancing:
	"shapeInstancedGeoms": string property containing the names of geometries instanced by this pointcloud. The geometries are exported in the same OArchive.
	"shapeType": uint8 property storing the shape type
	"shapeInstanceId": uint32 property storing the index of the geometry in property "shapeInstancedGeoms"
	"shapeInstanceHierarchy": boolean property storing the hierarchy-instancing flag
*/
class IAbcOPoints : public IAbcOGeom
{
public:
	/*! Get the index of an instanced geometry.
	\param in_ulDBID The object ID of the geometry
	\return The index in property "shapeInstancedGeoms"
	*/
	virtual unsigned int	GetInstancedGeomIndex( unsigned int in_ulDBID ) = 0;

	/*! Set the hierarchy-instancing flag of an instanced geometry.
	\param in_ulDBID The object ID of the geometry
	*/
	virtual void			SetHierarchyInstancedGeom( unsigned int in_ulDBID ) = 0;
	
	/*! Get the number of an instanced geometry.
	\return The number of instanced geometries
	*/	
	virtual unsigned int	GetNbInstancedGeom() const = 0;

	/*! Get the array of instanced geometries' DBID and their hierarchy-instancing flag.
	\param out_ppDBIDs The pointer to an allocated array where the DBIDs will be stored
	\param out_ppInstancedHierary The pointer to an allocated array where the hierarchy-instancing flags will be stored
	\return ::EResult_Success on success, please see ::EAbcResult for more return code information
	*/
	virtual EAbcResult		GetInstancedGeom( unsigned int** out_ppDBIDs, bool** out_ppInstancedHierarchy ) const = 0;

	/*! Get the property which stores the names of the instanced geometries
	\param out_ppProp Pointer to an IAbcOProperty pointer to hold the property
	\return ::EResult_Success on success, please see ::EAbcResult for more return code information
	*/
	virtual EAbcResult		GetInstancedGeomProperty( IAbcOProperty** out_ppProp ) = 0;

	/*! Sets uniform time sampling with the specified time per cycle and the start time.
	\param in_dTimePerCycle
	\param in_dStartTime
	\return ::EResult_Success on success, please see ::EAbcResult for more return code information
	*/
	virtual EAbcResult SetTimeSampling( double in_dTimePerCycle, double in_dStartTime ) = 0;

	/*! Add a sample of this pointcloud.
	\param in_pfPointPositions The array of point positions
	\param in_iPointCount The number of points
	\param in_BBox The bounding box of the sample
	*/
	virtual void AddSample( const float* in_pfPointPositions, int in_iPointCount, const Alembic::Abc::Box3d& in_BBox ) = 0;
	
	/*! Add a sample of this pointcloud
	\param in_pfPointPositions The array of point positions
	\param in_iPointCount The number of points
	\param in_pIds The array of point IDs
	\param in_iIdCount The number of elements in in_pIds
	\param in_BBox The bounding box of the sample
	*/
	virtual void AddSample( 
		const float* in_pfPointPositions, int in_iPointCount,
		const unsigned long long* in_pIds, int in_iIdCount,
		const Alembic::Abc::Box3d& in_BBox
		) = 0;

	/*! Add a sample of this pointcloud.
	\param in_pfPointPositions The array of point positions
	\param in_iPointCount The number of points
	\param in_pVelocities The array of point velocities
	\param in_iVelocityCount The number of velocity values
	\param in_pWidths The array of face vertex count
	\param in_pWidthCount The number of width values
	\param in_BBox The bounding box of the sample
	*/
	virtual void AddSample( 
		const float* in_pfPointPositions, int in_iPointCount,
		const float* in_pVelocities, int in_iVelocityCount,
		const float* in_pWidths, int in_iWidthCount,
		const Alembic::Abc::Box3d& in_BBox
		) = 0;

	/*! Add a sample of this pointcloud.
	\param in_pfPointPositions The array of point positions
	\param in_iPointCount The number of points
	\param in_pIds The array of point IDs
	\param in_iIdCount The number of elements in in_pIds
	\param in_pVelocities The array of point velocities
	\param in_iVelocityCount The number of velocity values
	\param in_pWidths The array of face vertex count
	\param in_pWidthCount The number of width values
	\param in_BBox The bounding box of the sample
	*/
	virtual void AddSample( 
		const float* in_pfPointPositions, int in_iPointCount,
		const unsigned long long* in_pIds, int in_iIdCount,
		const float* in_pVelocities, int in_iVelocityCount,
		const float* in_pWidths, int in_iWidthCount,
		const Alembic::Abc::Box3d& in_BBox
		) = 0;

};

/*! \class IAbcOXform
	\brief The interface for accessing Alembic OXform objects
*/
class IAbcOXform : public IAbcOGeom
{
public:
	/*! Sets uniform time sampling with the specified time per cycle and the start time.
	\param in_dTimePerCycle
	\param in_dStartTime
	\return ::EResult_Success on success, please see ::EAbcResult for more return code information
	*/
	virtual EAbcResult SetTimeSampling( double in_dTimePerCycle, double in_dStartTime ) = 0;

	/*! Add a sample of this Xform
	\param in_matLocal The local transform matrix
	*/
	virtual void AddSampleFromMatrix( const Alembic::Abc::M44d& in_matLocal ) = 0;

	/*! Add a sample of this Xform
	\param in_matGlobal,in_matParentGlobal The object and its parent global transform. The local transform will be calculated from these two values.
	*/
	virtual void AddSampleFromMatrix( const Alembic::Abc::M44d& in_matGlobal, const Alembic::Abc::M44d& in_matParentGlobal ) = 0;
};

/*! \class IAbcOXform
	\brief The interface for accessing Alembic CameraSample objects
*/
class IAbcOCameraSample : public IBase
{
public:
	/*! Set focal length
	\param in_dFocalLength The focal length
	*/
	virtual void SetFocalLength( double in_dFocalLength ) = 0;

	/*! Set aperture
	\param in_dHorizAperture,in_dVertAperture The horizontal and vertical aperture
	*/
	virtual void SetAperture( double in_dHorizAperture, double in_dVertAperture ) = 0;

	/*! Set film offset
	\param in_dHorizOffset,in_dVertOffset The horizontal and vertical film offset
	*/
	virtual void SetFilmOffset( double in_dHorizOffset, double in_dVertOffset ) = 0;

	/*! Set the lens squeeze ratio
	\param in_dRatio The lens squeeze ratio
	*/
	virtual void SetLensSqueezeRatio( double in_dRatio ) = 0;

	/*! Set overscan
	\param in_dLeft, in_dRight, in_dTop, in_dBottom  The left, right, top and bottom of the overscan
	*/
	virtual void SetOverscan( double in_dLeft, double in_dRight, double in_dTop, double in_dBottom ) = 0;

	/*! Set the f-stop value
	\param in_dFStop The f-stop value
	*/
	virtual void SetFStop( double in_dFStop ) = 0;

	/*! Set the focus distance
	\param in_dFocusDistance The focus distance
	*/
	virtual void SetFocusDistance( double in_dFocusDistance ) = 0;

	/*! Set the shutter open time
	\param in_dTime The shutter open time
	*/
	virtual void SetShutterOpen( double in_dTime ) = 0;

	/*! Set the shutter close time
	\param in_dTime The shutter close time
	*/
	virtual void SetShutterClose( double in_dTime ) = 0;

	/*! Set the near and far clipping planes
	\param in_dNear,in_dFar The near and far clipping plane
	*/
	virtual void SetClippingPlanes( double in_dNear, double in_dFar ) = 0;

	/*! Set the child bounding box
	\param in_BBox The child bounding box
	*/
	virtual void SetChildBounds( const Alembic::Abc::Box3d& in_BBox ) = 0;
};

/*! \class IAbcOCamera
	\brief The interface for accessing Alembic OCamera objects
*/
class IAbcOCamera : public IAbcOGeom
{
public:
	/*! Sets uniform time sampling with the specified time per cycle and the start time.
	\param in_dTimePerCycle Time per cycle
	\param in_dStartTime Start time
	\return ::EResult_Success on success, please see ::EAbcResult for more return code information
	*/
	virtual EAbcResult SetTimeSampling( double in_dTimePerCycle, double in_dStartTime ) = 0;

	/*! Create a CameraSample object
	\param out_ppSample The pointer to an IAbcOCameraSample pointer to hold the created sample
	\return ::EResult_Success on success, please see ::EAbcResult for more return code information
	*/
	virtual EAbcResult CreateCameraSample( IAbcOCameraSample** out_ppSample ) = 0;

	/*! Add a sample for this OCamera
	\param in_pSample Pointer to an IAbcOCameraSample object
	\return ::EResult_Success on success, please see ::EAbcResult for more return code information
	*/
	virtual void AddCameraSample( const IAbcOCameraSample* in_pSample ) = 0;
};

/*! \class IAbcOArchive
	\brief The interface for accessing Alembic OArchive objects
*/
class IAbcOArchive : public IBase
{
public:
	/*! Destructor */
	virtual ~IAbcOArchive();

	/*! Gets the name of the archive
	\return The name of the archive
	*/
	virtual const char* GetName( ) const = 0;

	/*! Add a uniform time sampling with the specified time per cycle and the start time.
	\param in_dTimePerCycle Time per cycle
	\param in_dStartTime Start time
	\return ::EResult_Success on success, please see ::EAbcResult for more return code information
	*/
	virtual EAbcResult AddTimeSampling( double in_dTimePerCycle, double in_dStartTime ) = 0;

	/*! Get the top object in the archive
	\param out_ppOObject Pointer to an IAbcOObject pointer to hold the top object
	\return ::EResult_Success on success, please see ::EAbcResult for more return code information
	*/
	virtual EAbcResult GetTop( IAbcOObject** out_ppOObject ) = 0;

	/*! Find an object in the archive
	\param in_pszName The full name of the object
	\param out_ppObject Pointer to an IAbcOObject pointer to hold the result object
	\return ::EResult_Success on success, please see ::EAbcResult for more return code information
	*/
	virtual EAbcResult FindObject( const char* in_pszName, IAbcOObject** out_ppOObject ) = 0;
};



#endif