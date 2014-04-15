//*****************************************************************************
/*!
   \file IAbcInput.h
   \brief Contains the interface definition and enumerations for the ABC framework.
   Each Alembic IObject is wrapped by an AbcFramework pure virtual interface, e.g.
   Alembic IPolyMesh is wrapped by AbcFramework IAbcIPolyMesh. This way the internal
   code will work with any version of Alembic.

	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*****************************************************************************
#ifndef ABCINPUTINTERFACE_H
#define ABCINPUTINTERFACE_H

#include "IAbcFramework.h"
#include "AbcFrameworkDataTypes.h"
#include <cstddef>

/*! Enumeration for Alembic Object Types */
enum EAbcIObjectType
{
	EIObject_Unknown,			/*! Unknown to the framework */
	EIObject_Polymesh,			
	EIObject_Subd,				
	EIObject_Points,			
	EIObject_Material,			
	EIObject_Camera,			
	EIObject_Curves,			
	EIObject_NuPatch,
	EIObject_FaceSet,
	EIObject_Xform
};

/*! Enumeration for Faceset exclusivity, equivalent to Abc::AbcGeom::FaceSetExclusivity */
enum EAbcIFaceSetExclusivity
{
	EIFaceSet_NonExclusive,		/*! Faces are not exclusive to this face set*/
	EIFaceSet_Exclusive,		/*! Faces are exclusive to this face set */
};

class IAbcIPolyMesh;
class IAbcISchemaObject;
class IAbcICompoundPropertyAccessor;

//*****************************************************************************
/*! \class IAbcIObject
	\brief Base interface for objects contained in IAbcIArchives
*/
//*****************************************************************************
class IAbcIObject : public IBase
{
public:
	/*! Gets the name of the object
	\return The name of the object
	*/
	virtual const char*		GetName() const = 0;
	
	/*! Gets the name of the object with the full path
	\return The name of the object
	*/
	virtual const char*		GetFullName() const = 0;

	/*! Gets the number of objects parented under this object
	\return The number of child objects
	*/
	virtual size_t			GetNumChildren() const = 0;

	/*! Gets the child object at the specified index
	\param in_Index The index of the object
	\param out_ppObject Pointer to a pointer that will hold the returned object
	\return ::EResult_Success on success, please see ::EAbcResult for more return code information

	\eg Usage:
	\code
		IAbcIObject* pObj; // Assume pObj points to a valid object
		CAbcPtr<IAbcIObject> pChild;
		pObject->GetChild( 0, &pChild );
	\endcode
	*/
	virtual EAbcResult		GetChild( size_t in_Index, IAbcIObject** out_ppObject ) const = 0;

	/*! Gets the child object with the specified name
	\param in_szName The name of the object to find
	\param out_ppObject Pointer to a pointer that will hold the returned object
	\return ::EResult_Success on success, please see ::EAbcResult for more return code information
	
	\eg Usage:
	\code
		IAbcIObject* pObj; // Assume pObj points to a valid object
		CAbcPtr<IAbcIObject> pChild;
		pObject->GetChild( "camera", &pChild );
	\endcode
	*/
	virtual EAbcResult		GetNamedChild( const char* in_szName, IAbcIObject** out_ppObject ) const = 0;

	/*! Gets the metadata string value with the specified key
	\param in_szKey The Key
	\param out_ppObject Optional. Pointer to a pointer that will hold the returned object
	\param out_pCount	Optional. Pointer to a size_t variable that would contain the length of the returned string
	\return ::EResult_Success on success, please see ::EAbcResult for more return code information

	\eg Usage:
	\code
		IAbcIObject* pObj; // Assume pObj points to a valid object
		size_t stringSize = 0;

		// Get the size of the string first
		pObj->GetMetaDataValue( "key", 0, &stringSize );
		
		if ( stringSize > 0 )
		{
			// Get the actual string
			char* pszValue = new char[stringSize + 1]; // +1 for null terminator
			pObj->GetMetaDataValue( "key", pszValue, 0 );
			delete[] pszValue;
		}
	\endcode
	*/
	virtual void			GetMetaDataValue( const char* in_szKey, char* out_szValue, size_t* out_pCount ) const = 0;

	/*! Gets the underlying Alembic type of the object
	\return The type of the object
	*/
	virtual	EAbcIObjectType	GetType() const = 0;

	/*! Transforms the current object to the requested object type
	\param in_objType The type of object to transform to
	\param out_ppObject The returned object
	\return ::EResult_Success on success, please see ::EAbcResult for more return code information

	\eg Usage:
	\code
		IAbcIArchive* pArchive; // Assume pArchive points to a valid archive
		CAbcPtr<IAbcIObject> spObj;
		if ( pArchive->FindObject( "camera", &spObj ) == EResult_Success )
		{
			// Get the camera interface
			CAbcPtr<IAbcICamera> spCamera;
			pArchive->TransformInto( EIObject_Camera, (IAbcIObject**)&spCamera );
		}
	\endcode
	*/
	virtual EAbcResult		TransformInto( EAbcIObjectType in_objType, IAbcIObject** out_ppObject ) = 0;

	/*! Gets the associated Schema Object of this object
	\param out_ppObject The returned object
	\return ::EResult_Success on success, please see ::EAbcResult for more return code information

	\eg Usage:
	\code
		IAbcIArchive* pArchive; // Assume pArchive points to a valid archive
		CAbcPtr<IAbcIObject> spObj;
		if ( pArchive->FindObject( "camera", &spObj ) == EResult_Success )
		{
			// Get the camera interface
			CAbcPtr<IAbcISchemaObject> spSchema;
			pArchive->GetSchemaObject( EIObject_Camera, (IAbcIObject**)&spSchema );
		}
	\endcode
	*/
	virtual EAbcResult		GetSchemaObject( IAbcISchemaObject** out_ppObject ) const = 0;

	/*! Compares if two objects are the same object, two objects may be equal even if their pointers are different
	\param in_pOther The object to compare to
	\return true if they are equal, false if not
	*/
	virtual bool			IsEqualObject( IAbcIObject* in_pOther ) = 0;

	/*! Compares if two objects are the same object and of the same type, two objects may not be of the same type if you are comparing a transformed object to a regular object
	\param in_pOther The object to compare to
	\return true if they are equal, false if not
	*/
	virtual bool			IsEqualObjectAndType( IAbcIObject* in_pOther ) = 0;

	/*! Gets the parent of this object
	\param out_ppObject The returned parent of the object
	\return ::EResult_Success on success, please see ::EAbcResult for more return code information
	*/
	virtual EAbcResult		GetParent( IAbcIObject** out_ppObject ) = 0;

	/*! Gets the compound property accessor for this object
	\param out_ppPropertyAccessor The returned property accessor for the object
	\return ::EResult_Success on success, please see ::EAbcResult for more return code information
	*/
	virtual EAbcResult		GetPropertyAccessor( IAbcICompoundPropertyAccessor** out_ppPropertyAccessor ) = 0;
};


//*****************************************************************************
/*! \class IAbcISampleSelector
	\brief Interface for sample selection when using the GetSample functions
*/
//*****************************************************************************
class IAbcISampleSelector : public IBase
{
public:
	/*! Sets the requested sample index
	\param in_llIndex The sample index
	*/
	virtual void SetRequestedIndex( long long in_llIndex ) = 0;

	/*! Sets the requested sample time
	\param in_eTimeIndexType Determines which sample to get
	\param in_dTime The requested time
	*/
	virtual void SetRequestedTime( EAbcTimeIndex in_eTimeIndexType, double in_dTime ) = 0;

	/*! Gets the requested sample index
	\param out_llIndex The returned value of the currently requested index
	*/
	virtual void GetRequestedIndex( long long& out_llIndex ) const = 0;

	/*! Gets the requested sample time
	\param in_eTimeIndexType Determines which sample to get
	\param out_dTime The returned value of the currently requested time
	*/
	virtual void GetRequestedTime( EAbcTimeIndex& out_eTimeIndexType, double& out_dTime ) const = 0;
};


//*****************************************************************************
/*! \class IAbcTimeSampling
	\brief Interface to query information about the Time Sampling of a property
*/
//*****************************************************************************
class IAbcTimeSampling : public IBase
{
public:
	/*! Returns the number of time samples on the internal TimeSampling object
	\return The number of samples
	*/
	virtual size_t	GetNumTimeSamples() const = 0;

	/*! Returns the time at a specified index
	\param in_Index The requested sample index
	\return The time at the specified index
	*/
	virtual double	GetTimeAt( long long in_Index ) const = 0;

	/*! Returns the floor sample index of a specified time
	\param in_dTime The requested time
	\param in_llNumSamples The number of samples on the property
	\param out_llIndex The returned index value
	\param out_dTime The returned time at the returned index
	*/
	virtual void	GetFloorIndex( double in_dTime, long long in_llNumSamples, long long& out_llIndex, double& out_dTime ) const = 0;

	/*! Returns the ceiling sample index of a specified time
	\param in_dTime The requested time
	\param in_llNumSamples The number of samples on the property
	\param out_llIndex The returned index value
	\param out_dTime The returned time at the returned index
	*/
	virtual void	GetCeilIndex( double in_dTime, long long in_llNumSamples, long long& out_llIndex, double& out_dTime ) const = 0;

	/*! Returns the nearest sample index at a specified time
	\param in_dTime The requested time
	\param in_llNumSamples The number of samples on the property
	\param out_llIndex The returned index value
	\param out_dTime The returned time at the returned index
	*/
	virtual void	GetNearIndex( double in_dTime, long long in_llNumSamples, long long& out_llIndex, double& out_dTime ) const = 0;
};

//*****************************************************************************
/*! \class IAbcIArchive
	\brief Interface for an IArchive object
*/
//*****************************************************************************
class IAbcIArchive : public IBase
{
public:
	/*! Gets the top object in the archive
	\param out_ppObject The returned object
	\return ::EResult_Success on success, please see ::EAbcResult for more return code information
	*/
	virtual EAbcResult		GetTop( IAbcIObject** out_ppObject ) const = 0;

	/*! Gets the name of the archive
	\return A const char* string containing the name of the archive
	*/
	virtual const char*		GetName() const = 0;

	/*! Gets the number of TimeSampling objects in the archive
	\return The number of time sampling objects in the archive
	*/
	virtual unsigned int	GetNumTimeSampling() = 0;

	/*! Gets a TimeSampling object from the archive, it is recommended to use the GetTimeSampling() function of IAbcISchemaObjects instead
	\param in_uiIndex The index if the TimeSampling object
	\param out_ppSampling The returned IAbcTimeSampling object
	\return ::EResult_Success on success, please see ::EAbcResult for more return code information
	*/
	virtual EAbcResult		GetTimeSampling( unsigned int in_uiIndex, IAbcTimeSampling** out_ppSampling ) = 0;

	/*! Creates a sample selector to be used with the GetSample() function of schema objects
	\param out_ppSelector The returned sample selector object
	\return ::EResult_Success on success, please see ::EAbcResult for more return code information
	*/
	virtual EAbcResult		CreateSampleSelector( IAbcISampleSelector** out_ppSelector ) const = 0;

	/*! Finds an object with the specified name in the archive
	\param in_pszObject The name of the object to find
	\param out_ppObject The returned object if found
	\return ::EResult_Success on success, please see ::EAbcResult for more return code information

	\eg Usage:
	\code
		IAbcIArchive* pArchive; // Assume pArchive points to a valid archive
		CAbcPtr<IAbcIObject> spObj;
		if ( pArchive->FindObject( "camera", &spObj ) == EResult_Success )
		{
			// Get the camera interface
			CAbcPtr<IAbcISchemaObject> spSchema;
			pArchive->GetSchemaObject( EIObject_Camera, (IAbcIObject**)&spSchema );
		}
	\endcode
	*/
	virtual EAbcResult		FindObject( const char* in_pszObject, IAbcIObject** out_ppObject ) = 0;

	/*! Gets the start and end times of the archive
	\param out_ppdStartTime		Optional. The time at the earliest sample in the archive
	\param out_ppdEndTime		Optional. The time at the latest sample in the archive
	*/
	virtual void			GetArchiveStartAndEndTime( double* out_ppdStartTime, double* out_ppdEndTime ) const = 0;

	/*! Gets the Alembic API version used to create this archive
	\return The API version used to create this archive
	*/
	virtual unsigned int	GetAbcApiVersion() const = 0;

	/*! Gets the name of the application used to write this archive. It is not required for applications to provide this info.
	\return A const char* string containing the name of the application
	*/
	virtual const char*		GetApplicationWriter() const = 0;

	/*! Get the Alembic version used to create this acrhive
	\return A const char* string containing the version
	*/
	virtual const char*		GetAbcVersion() const = 0;

	/*! Gets the date when the archive was written
	\return A const char* string containing the date
	*/
	virtual const char*		GetDateWritten() const = 0;

	/*! Gets the user description contained in the archive
	\return A const char* string containing the description
	*/
	virtual const char*		GetUserDescription() const = 0;
};

//*****************************************************************************
/*! \class IAbcSampleBuffer
	\brief Interface for accessing the data contained in an Alembic Sample
*/
//*****************************************************************************
class IAbcSampleBuffer : public IBase
{
public:
	/*! Gets a pointer to the internal buffer of the sample
	\return The buffer to the sample data
	*/
	virtual const void*			GetBuffer() const = 0;
	/*! Gets the number of elements in the buffer
	\return The number of elements (e.g. If the sample is a Vector3 sample with 10 vectors, this function will return 10)
	*/
	virtual size_t				GetNumElements() const = 0;

	/*! Gets the data type of the underlying sample data
	\return The data type of the samples
	*/
	virtual const SAbcDataType& GetDataType() const = 0;

	/*! Gets the geometry scope of the sample data 
	\return The SAbcDataType struct describing the data type
	*/
	virtual EAbcGeomScope		GetGeomScope() const = 0;

	/*! Queries whether the sample data is indexed or not
	\return true if the sample data is indexed, false if not
	*/
	virtual bool				IsIndexed() const = 0;
};

/*! \class IAbcIPropertyAccessor
	\brief Interface for accessing the properties of an Alembic object
*/
class IAbcIPropertyAccessor : public IBase
{
public:
	/*! Gets the name of the property
	\return A const char* string containing the property name
	*/
	virtual const char*			GetName() const = 0;

	/*! Gets the Plain Old Datatype (POD) of the property
	\return The POD type
	*/
	virtual EAbcPodType			GetPodType() const = 0;

	/*! Gets the type of the property
	\return ::EPropertyType_Compound if the property is a compound
	\return	::EPropertyType_Array if the property is an array
	\return	::EPropertyType_Scalar if the property is a scalar
	*/
	virtual EAbcPropertyType	GetPropertyType() const = 0;

	/*! Gets the datatype of the property
	\return The SAbcDataType struct describing the datatype
	*/
	virtual const SAbcDataType& GetDataType() const = 0;

	/*! Gets the parent property if this property if any
	\param out_ppProp The returned parent property if any
	\return ::EResult_Success on success, please see ::EAbcResult for more return code information
	*/
	virtual EAbcResult			GetParentProperty( IAbcIPropertyAccessor** out_ppProp ) const = 0;

	/*! Gets the geometry scope of the sample data 
	\return The SAbcDataType struct describing the data type
	*/
	virtual EAbcGeomScope		GetGeomScope() const = 0;

};

/*! \class IAbcISimplePropertyAccessor
	\brief Base interface for IAbcScalarPropertyAccessor and IAbcIArrayAccessor
*/
class IAbcISimplePropertyAccessor : public IAbcIPropertyAccessor
{
public:
	/*! Gets the time sampling interface for this property
	\param out_ppSampling The returned time sampling interface for the property
	\return ::EResult_Success on success, please see ::EAbcResult for more return code information
	*/
	virtual EAbcResult	GetTimeSampling( IAbcTimeSampling** out_ppSampling ) = 0;

	/*! Gets the number of samples in contained in the property
	\return The number of samples
	*/
	virtual size_t		GetNumSamples() const = 0;

	/*! Query whether the property is constant throughout the whole duration of the cached simulation/animation
	\return true of the property is constant
	*/
	virtual bool		IsConstant() const = 0;
};

/*! \class IAbcIScalarPropertyAccessor
	\brief Interface for accessing scalar properties
*/
class IAbcIScalarPropertyAccessor : public IAbcISimplePropertyAccessor
{
public:
	/*! Gets a sample from the property
	\param in_pSampleSelector Optional. The sample selector to determine the time at which to get the sample
	\param out_pDest The address of a buffer large enough to hold the sample data
	\return ::EResult_Success on success, please see ::EAbcResult for more return code information
	*/
	virtual EAbcResult	GetSample( IAbcISampleSelector* in_pSampleSelector, void* out_pDest ) const = 0;
};

/*! \class IAbcIArrayPropertyAccessor
	\brief Interface for accessing array properties
*/
class IAbcIArrayPropertyAccessor : public IAbcISimplePropertyAccessor
{
public:
	/*! Gets a sample from the property
	\param in_pSampleSelector Optional. The sample selector to determine the time at which to get the sample
	\param out_ppDest The returned IAbcSampleBuffer for accessing the contained samples
	\return ::EResult_Success on success, please see ::EAbcResult for more return code information
	*/
	virtual EAbcResult	GetSample( IAbcISampleSelector* in_pSampleSelector, IAbcSampleBuffer** out_ppDest ) const = 0;
};

/*! \class IAbcICompoundPropertyAccessor
	\brief Interface for accessing compound properties
*/
class IAbcICompoundPropertyAccessor : public IAbcIPropertyAccessor
{
public:
	/*! Gets the number of child properties of this compound property
	\return The number of child properties
	*/
	virtual size_t				GetNumChildrenProperties() const = 0;

	/*! Gets a child property given an index
	\param in_index The index of the property
	\param out_ppProp The returned property accessor of the property at in_index
	\return ::EResult_Success on success, please see ::EAbcResult for more return code information
	*/
	virtual EAbcResult			GetChildProperty( size_t in_index, IAbcIPropertyAccessor** out_ppProp ) const = 0;

	/*! Searches for a child property given a name
	\param in_szName The name of the property to search for
	\param out_ppProp The returned property accessor of the searched property
	\return ::EResult_Success on success, please see ::EAbcResult for more return code information
	*/
	virtual EAbcResult			GetChildProperty( const char* in_szName, IAbcIPropertyAccessor** out_ppProp ) const = 0;
};


/*! \class IAbcIGeomParam
	\brief Interface for accessing geom param
*/
class IAbcIGeomParam : public IBase
{
public:
		/*! Gets the name of the property
	\return A const char* string containing the property name
	*/
	virtual const char*			GetName() const = 0;

	/*! Gets the Plain Old Datatype (POD) of the property
	\return The POD type
	*/
	virtual EAbcPodType			GetPodType() const = 0;

	/*! Returns if the geom param is indexed
	\return	True if the param is indexed, false if it is not
	*/
	virtual bool IsIndexed() const = 0;

	/*! Gets the datatype of the property
	\return The SAbcDataType struct describing the datatype
	*/
	virtual const SAbcDataType& GetDataType() const = 0;

	/*! Gets the parent property if this property if any
	\param out_ppProp The returned parent property if any
	\return ::EResult_Success on success, please see ::EAbcResult for more return code information
	*/
	virtual EAbcResult			GetParentProperty( IAbcIPropertyAccessor** out_ppProp ) const = 0;

	/*! Gets a sample from the geometry param
	\param in_bExpand Set to true to return an expanded buffer if the samples are indexed
	\param in_pSampleSelector Optional. The sample selector to determine the time at which to get the sample
	\param out_ppDest The returned IAbcSampleBuffer for accessing the contained samples
	\return ::EResult_Success on success, please see ::EAbcResult for more return code information
	*/
	virtual EAbcResult			GetSample( bool in_bExpand, IAbcISampleSelector* in_pSampleSelector, IAbcSampleBuffer** out_ppBuffer ) = 0;

	/*! Gets the value property
	\param out_ppProp The returned value property 
	\return ::EResult_Success on success, please see ::EAbcResult for more return code information
	*/
	virtual EAbcResult GetValueProperty( IAbcIPropertyAccessor** out_ppProp ) = 0;

	/*! Gets the index property if this geom param is indexed
	\param out_ppProp The returned property if any
	\return ::EResult_Success on success, please see ::EAbcResult for more return code information
	*/
	virtual EAbcResult GetIndexProperty( IAbcIPropertyAccessor** out_ppProp ) = 0;

};

/*! \class IAbcISchemaObject
	\brief The base interface for Alembic input schemas
*/
class IAbcISchemaObject : public IAbcIObject
{
public:
	/*! Gets the number of samples contained in this object
	\return The number of samples in the object
	*/
	virtual size_t				GetNumSamples() const = 0;

	/*! Gets the time sampling information associated with this object
	\param out_ppSampling The returned sampling information for this object
	\return ::EResult_Success on success, please see ::EAbcResult for more return code information
	*/
	virtual EAbcResult			GetTimeSampling( IAbcTimeSampling** out_ppSampling ) = 0;

	/*! Gets the property accessor associated with this object
	\param out_ppPropertyAccessor The returned compound property representing this object
	\return ::EResult_Success on success, please see ::EAbcResult for more return code information
	*/
	virtual EAbcResult			GetSchemaPropertyAccessor( IAbcICompoundPropertyAccessor** out_ppPropertyAccessor ) = 0;
};

/*! \class IAbcIPolyMesh
	\brief The interface for accessing Alembic IPolyMesh objects
*/
class IAbcIPolyMesh : public IAbcISchemaObject
{
public:
	/*! Gets the topology variance of this mesh
	\return The topology variance of this mesh. See ::EAbcTopoVariance
	*/
	virtual EAbcTopoVariance	GetTopologyVariance() const = 0;

	/*! Queries if this mesh is constant for the whole duration of the Alembic simulation/animation
	\return true if the constant, false otherwise
	*/
	virtual bool				IsConstant() const = 0;
	
	/*! Gets the UVs for this object
	\param in_bExpand Set to true to return an expanded buffer if the samples are indexed
	\param in_pSampleSelector Optional. A sample selector object to determine which time to get the sample from.
	\param out_ppBuffer The returned sample buffer containing the requested UVs
	\return ::EResult_Success on success, please see ::EAbcResult for more return code information
	*/
	virtual EAbcResult			GetUVs( bool in_bExpand, IAbcISampleSelector* in_pSampleSelector, IAbcSampleBuffer** out_ppBuffer ) = 0;

	/*! Gets the normals for this object
	\param in_bExpand Set to true to return an expanded buffer if the samples are indexed
	\param in_pSampleSelector Optional. A sample selector object to determine which time to get the sample from.
	\param out_ppBuffer The returned sample buffer containing the requested normals
	\return ::EResult_Success on success, please see ::EAbcResult for more return code information
	*/
	virtual EAbcResult			GetNormals( bool in_bExpand, IAbcISampleSelector* in_pSampleSelector, IAbcSampleBuffer** out_ppBuffer ) = 0;

	/*! Gets the face counts for this object
	\param in_pSampleSelector Optional. A sample selector object to determine which time to get the sample from.
	\param out_ppBuffer The returned sample buffer containing the requested face counts
	\return ::EResult_Success on success, please see ::EAbcResult for more return code information
	*/
	virtual EAbcResult			GetFaceCounts( IAbcISampleSelector* in_pSampleSelector, IAbcSampleBuffer** out_ppBuffer ) = 0;

	/*! Gets the face indices for this object
	\param in_pSampleSelector Optional. A sample selector object to determine which time to get the sample from.
	\param out_ppBuffer The returned sample buffer containing the requested face indices
	\return ::EResult_Success on success, please see ::EAbcResult for more return code information
	*/
	virtual EAbcResult			GetFaceIndices( IAbcISampleSelector* in_pSampleSelector, IAbcSampleBuffer** out_ppBuffer ) = 0;

	/*! Gets the vertex positions for this object
	\param in_pSampleSelector Optional. A sample selector object to determine which time to get the sample from.
	\param out_ppBuffer The returned sample buffer containing the requested vertex positions
	\return ::EResult_Success on success, please see ::EAbcResult for more return code information
	*/
	virtual EAbcResult			GetPositions( IAbcISampleSelector* in_pSampleSelector, IAbcSampleBuffer** out_ppBuffer ) = 0;

	/*! Gets the vertex velocities for this object
	\param in_pSampleSelector Optional. A sample selector object to determine which time to get the sample from.
	\param out_ppBuffer The returned sample buffer containing the requested vertex velocities
	\return ::EResult_Success on success, please see ::EAbcResult for more return code information
	*/
	virtual EAbcResult			GetVelocities( IAbcISampleSelector* in_pSampleSelector, IAbcSampleBuffer** out_ppBuffer ) = 0;

	/*! Gets the UVs param for this object
	\param out_ppGeomParam The returned geom param
	\return ::EResult_Success on success, please see ::EAbcResult for more return code information
	*/
	virtual EAbcResult			GetUVsParam( IAbcIGeomParam** out_ppGeomParam ) = 0;

	/*! Gets the Normals param for this object
	\param out_ppGeomParam The returned geom param
	\return ::EResult_Success on success, please see ::EAbcResult for more return code information
	*/
	virtual EAbcResult			GetNormalsParam( IAbcIGeomParam** out_ppGeomParam ) = 0;
};

/*! \class IAbcIPoints
	\brief The interface for accessing Alembic IPoints objects
*/
class IAbcIPoints : public IAbcISchemaObject
{
public:
	/*! Queries if this point cloud is constant for the whole duration of the Alembic simulation/animation
	\return true if the constant, false otherwise
	*/
	virtual bool	IsConstant() const = 0;
	
	/*! Gets the point positions for this object
	\param in_pSampleSelector	Optional. A sample selector object to determine which time to get the sample from.
	\param out_ppBuffer			The returned sample buffer containing the requested point positions
	\return ::EResult_Success on success, please see ::EAbcResult for more return code information
	*/	
	virtual EAbcResult	GetPositions( IAbcISampleSelector* in_pSampleSelector, IAbcSampleBuffer** out_ppBuffer ) = 0;

	/*! Gets the point velocities for this object
	\param in_pSampleSelector	Optional. A sample selector object to determine which time to get the sample from.
	\param out_ppBuffer			The returned sample buffer containing the requested point velocities
	\return ::EResult_Success on success, please see ::EAbcResult for more return code information
	*/	
	virtual EAbcResult	GetVelocities( IAbcISampleSelector* in_pSampleSelector, IAbcSampleBuffer** out_ppBuffer ) = 0;

	/*! Gets the point IDs for this object
	\param in_pSampleSelector	Optional. A sample selector object to determine which time to get the sample from.
	\param out_ppBuffer			The returned sample buffer containing the requested point IDs
	\return ::EResult_Success on success, please see ::EAbcResult for more return code information
	*/	
	virtual EAbcResult	GetIds( IAbcISampleSelector* in_pSampleSelector, IAbcSampleBuffer** out_ppBuffer ) = 0;
};

/*! \class IAbcICameraSample
	\brief A helper interface for accessing Alembic ICamera samples
*/
class IAbcICameraSample : public IBase
{
public:
	/*! Gets the focal length of the camera
	\return The focal length of the camera
	*/
	virtual double	GetFocalLength( ) const = 0;

	/*! Gets the aperture of the camera
	\param out_dHorizAperture	The horizontal aperture
	\param out_dVertAperture	The vertical aperture
	*/
	virtual void	GetAperture( double& out_dHorizAperture, double& out_dVertAperture ) const = 0;

	/*! Gets the film offset of the camera
	\param out_dHorizOffset The horizontal offset
	\param out_dVertOffset	The vertical offset
	*/
	virtual void	GetFilmOffset( double& out_dHorizOffset, double& out_dVertOffset ) const = 0;

	/*! Gets the lens squeeze ratio of the camera (aspect ratio)
	\return The squeeze ratio of the camera
	*/
	virtual double	GetLensSqueezeRatio( ) const = 0;

	/*! Gets the overscan of the camera
	\param out_dLeft	The amount of left overscan
	\param out_dRight	The amount right overscan
	\param out_dTop		The amount of top overscan
	\param out_dBottom	The amount of bottom overscan
	*/
	virtual void	GetOverscan( double& out_dLeft, double& out_dRight, double& out_dTop, double& out_dBottom ) const = 0;

	/*! Gets the FStop of the camera
	\return The FStop
	*/
	virtual double	GetFStop( ) const = 0;

	/*! Gets the focus distance for this camera
	\return The focus distance for this camera
	*/
	virtual double	GetFocusDistance( ) const = 0;

	/*! Gets the open shutter time for this camera (for Motion Blur)
	\return The time it takes for the shutter to open
	*/
	virtual double	GetShutterOpen( ) const = 0;

	/*! Gets the close shutter time for this camera (for Motion Blur)
	\return The time it takes for the shutter to close
	*/
	virtual double	GetShutterClose( ) const = 0;

	/*! Gets the clipping planes for this camera
	\param out_dNear	The nearest distance before the rendering gets clipped
	\param out_dFar		The furthest distance before the rendering gets clipped
	*/
	virtual void	GetClippingPlanes( double& out_dNear, double& out_dFar ) const = 0;

	/*! Gets the child bounds for this camera
	\param out_BBox	The returned bounding box
	*/
	virtual void	GetChildBounds( Alembic::Abc::Box3d& out_BBox ) const = 0;

	/*! Gets the calculated field of view in degrees based from the camera parameters
	\return The FOV in degrees
	*/
	virtual double	GetEffectiveFov( ) const = 0;
};

/*! \class IAbcICamera
	\brief An interface for accessing Alembic ICamera objects
*/
class IAbcICamera : public IAbcISchemaObject
{
public:
	/*! Queries if this camera is constant for the whole duration of the Alembic simulation/animation
	\return true if the constant, false otherwise
	*/
	virtual bool	IsConstant() const = 0;

	/*! Gets a sample from the camera object
	\param in_pSampleSelector	Optional. A sample selector containing information on which sample to get
	\param out_ppSample			The returned IAbcICameraSample
	\return ::EResult_Success on success, please see ::EAbcResult for more return code information
	*/
	virtual EAbcResult GetCameraSample( IAbcISampleSelector* in_pSampleSelector, IAbcICameraSample** out_ppSample ) = 0;

};

/*!	\class IAbcIXformOp
	\brief An interface for accessing an Alembic IXformOp object
*/
class IAbcIXformOp : public IBase
{
public:
	/*! Gets the type of transformation
	\return The type of transformation. See ::EAbcXformOpType for more details
	*/
	virtual EAbcXformOpType	GetType() const = 0;

	/*! Gets a transform hint. Transform hints are application specific values for storing more information on what type of transform is supposed to be applied.
	\return The transform hint
	\return	See ::EAbcMatrixHint, ::EAbcRotateHint, ::EAbcScaleHint and ::EAbcTranslateHint for standard hints
	*/
	virtual unsigned char	GetHint() const = 0;

	virtual bool			IsXAnimated() const = 0;
	virtual bool			IsYAnimated() const = 0;
	virtual bool			IsZAnimated() const = 0;
	virtual bool			IsAngleAnimated() const = 0;
	virtual bool			IsChannelAnimated( size_t in_Index ) const = 0;
	virtual size_t			GetNumChannels() const = 0;
	virtual double			GetDefaultChannelValue( size_t in_Index ) const = 0;
	virtual double			GetChannelValue( size_t in_Index ) const = 0;
	virtual const double*	GetTranslate() const = 0;
	virtual const double*	GetScale() const = 0;
	virtual const double*	GetAxis() const = 0;
	virtual const double*	GetMatrix4x4() const = 0;
	virtual double			GetXRotation() const = 0;
	virtual double			GetYRotation() const = 0;
	virtual double			GetZRotation() const = 0;
	virtual double			GetAngle() const = 0;
};

/*!	\class IAbcIXformSample
	\brief An interface for accessing Alembic IXform samples
*/
class IAbcIXformSample : public IBase
{
public:
	/*! Gets the number of transform operations on this sample
	\return The number of transform operations
	*/
	virtual size_t			GetNumOps() const = 0;

	/*! Gets the number of operation channels on this sample
	\return The number of channels
	*/
	virtual size_t			GetNumOpChannels() const= 0;

	/*! Gets an IAbcIXformOp describing the transorm operation to be applied
	\param in_index The index of the operation
	\out_ppXformOp	The returned IAbcIXformOp
	\return ::EResult_Success on success, please see ::EAbcResult for more return code information
	*/
	virtual EAbcResult		GetOp( size_t in_index, IAbcIXformOp** out_ppXformOp ) const = 0;

	/*! Queries whether this transform should inherit its parent transform
	\return true if it should inherit the parent transform, false otherwise
	*/
	virtual bool			GetInheritsXforms() const = 0;

	/*! Gets the translation component of this transform
	\return A const double* array containing 3 elements representing the X, Y and Z components of the translation
	*/
	virtual const double*	GetTranslation() const = 0;

	/*! Gets the axis component of this transform (For Axis-Angle rotations)
	\return A const double* array containing 3 elements representing the X, Y and Z components of the axis
	*/
	virtual const double*	GetAxis() const = 0;

	/*! Gets the scaling component of this transform
	\return A const double* array containing 3 elements representing the X, Y and Z components of the scaling
	*/
	virtual const double*	GetScale() const = 0;

	/*! Calculates a 4x4 matrix representing this transform
	\return A const double* array containing 16 elements representing the calculated 4x4 matrix
	*/
	virtual const double*	GetMatrix4x4() const = 0;

	/*! Gets the angle component of this transform (For Axis-Angle rotations)
	\return The angle of the axis-angle transform
	*/
	virtual double			GetAngle() const = 0;

	/*! Gets the rotation for this transform along the X-axis
	\return The X rotation of this transform
	*/
	virtual double			GetXRotation() const = 0;

	/*! Gets the rotation for this transform along the Y-axis
	\return The Y rotation of this transform
	*/
	virtual double			GetYRotation() const = 0;

	/*! Gets the rotation for this transform along the Z-axis
	\return The Z rotation of this transform
	*/
	virtual double			GetZRotation() const = 0;
};

/*!	\class IAbcIXform
	\brief An interface for accessing Alembic IXform objects
*/
class IAbcIXform : public IAbcISchemaObject
{
public:
	/*! Queries if this transform is constant for the whole duration of the Alembic simulation/animation
	\return true if the constant, false otherwise
	*/
	virtual bool			IsConstant() const =  0;

	/*! Queries if this transform is constant for the whole duration of the Alembic simulation/animation and represents and identity transform
	\return true if the constant-identity, false otherwise
	*/
	virtual bool			IsConstantIdentity() const = 0;

	/*! Gets the number if IAbcIXformOp elements on this transform
	\return The number of operations
	*/
	virtual size_t			GetNumOps() const = 0;

	/*! Gets a transform sample from this object
	\param in_pSampleSelector	Optional. A sample selector with information on which sample to retrieve
	\param out_ppXformSample	The returned IAbcXformSample object
	\return ::EResult_Success on success, please see ::EAbcResult for more return code information
	**/
	virtual EAbcResult		GetSample( IAbcISampleSelector* in_pSampleSelector, IAbcIXformSample** out_ppXformSample ) const = 0;
};

/*!	\class IAbcIFaceSet
	\brief An interface for accessing Alembic IFaceSet objects
*/
class IAbcIFaceSet : public IAbcISchemaObject
{
public:
	/*! Queries if this face set is constant for the whole duration of the Alembic simulation/animation
	\return true if the constant, false otherwise
	*/
	virtual bool					IsConstant() const = 0;

	/*! Queries the exclusivity of the elements in this face set
	\return The exclusivity of the elements in this face set. See ::EAbcIFaceSetExclusivity
	*/
	virtual EAbcIFaceSetExclusivity	GetExclusivity() const = 0;

	/*! Gets a sample buffer containing the face indices on this face set
	\param in_pSampleSelector	Optional. A sample selector containing information on which sample to retrieve
	\param out_ppBuffer			The returned buffer containing the face indices on this face set
	\return ::EResult_Success on success, please see ::EAbcResult for more return code information
	*/
	virtual EAbcResult				GetSample( IAbcISampleSelector* in_pSampleSelector, IAbcSampleBuffer** out_ppBuffer ) = 0;
};
#endif
