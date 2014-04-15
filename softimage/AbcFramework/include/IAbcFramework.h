//*****************************************************************************
/*!
   \file IAbcFramework.h
   \brief Contains the interface definition and enumerations for the ABC framework

	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*****************************************************************************


#ifndef ABCFRAMEWORKINTERFACE_H
#define ABCFRAMEWORKINTERFACE_H

#include "AbcFrameworkDll.h"
#include "AbcFrameworkImportedTypes.h"
#include "AbcFrameworkPlainOldDataType.h"

#include <stdint.h>
#include <stddef.h>

#ifdef unix
#define __forceinline inline
#endif

class IAbcIArchive;
class IAbcOArchive;
class IAbcOProperty;
class IAbcIPropertyAccessor;

//*****************************************************************************
/*! \class IBase
	\brief Base interface for all ABC framework classes
*/
//*****************************************************************************
class IBase
{
public:
	virtual void AddRef() = 0;
	virtual void Release() = 0;
	virtual int  GetRefCount() const = 0;
};

/*! Return codes for framework functions */
enum EAbcResult
{
	EResult_Success,			/*!< The operation was successful. */
	EResult_Fail,				/*!< The operation has failed. */
	EResult_OutOfMemory,		/*!< A memory allocation within the operation failed. */
	EResult_InvalidPtr,			/*!< An input pointer parameter is invalid. */
	EResult_OutOfRange,			/*!< An input index parameter is invalid. */
	EResult_InvalidCast,		/*!< The object cannot be cast to the requested type. */
	EResult_InvalidProp,		/*!< A property being requested is invalid. */
	EResult_NotApplicable,		/*!< The operation is not applicable with one or more of the input objects. */
	EResult_NotImpl				/*!< The requested function is not yet implemented */
};

/*! Enum for the types of Topology variance of Polygon meshes, equivalent to Alembic::AbcGeom::MeshTopologyVariance */
enum EAbcTopoVariance
{
	ETopoVariance_Constant = 0,			/*!< Mesh topology and vertex positions doesn't change */
	ETopoVariance_Homogenous = 1,		/*!< Mesh topology is constant but vertex positions may change */
	ETopoVariance_Heterogenous = 2		/*!< Mesh topology and vertex positions may change */
};

/*! Enumeration for the various Alembic property types */
enum EAbcPropertyType
{
	EPropertyType_Compound = 0,			/*!< A compound property which may contain other properties */
	EPropertyType_Scalar = 1,			/*!< A single value property */
	EPropertyType_Array = 2,				/*!< A property with an array of values */
	EPropertyType_Unknown = 127			/*!< Unknown to the framework */
};

/*! Enumeration for the various Alembic POD types, equivalent to Alembic::Util::PlainOldDataType */
enum EAbcPodType
{
	EPodType_Bool = 0,
	EPodType_UInt8,
	EPodType_Int8,
	EPodType_UInt16,
	EPodType_Int16,
	EPodType_UInt32,
	EPodType_Int32,
	EPodType_UInt64,
	EPodType_Int64,
	EPodType_Float16,
	EPodType_Float32,
	EPodType_Float64,
	EPodType_StringPtr,
	EPodType_WStringPtr,
	EPodType_NumPods,
	EPodType_Unknown
};

/*! Enumeration for the various Alembic type traits, please see TypedPropertyTraits.h in the Alembic headers */
enum EAbcDataTraits
{
	EDataTraits_Bool = 0,
	
	EDataTraits_UInt8,
	EDataTraits_Int8,
	EDataTraits_UInt16,
	EDataTraits_Int16,
	
	EDataTraits_UInt32,
	EDataTraits_Int32,
	
	EDataTraits_UInt64,
	EDataTraits_Int64,
	
	EDataTraits_Float16,
	EDataTraits_Float32,
	EDataTraits_Float64,
	
	EDataTraits_StringPtr,
	EDataTraits_WStringPtr,

	EDataTraits_V2s,
	EDataTraits_V2i,  
	EDataTraits_V2f,  
	EDataTraits_V2d,  

	EDataTraits_V3s,  
	EDataTraits_V3i,  
	EDataTraits_V3f,  
	EDataTraits_V3d,  

	EDataTraits_P2s,  
	EDataTraits_P2i,  
	EDataTraits_P2f,  
	EDataTraits_P2d,  

	EDataTraits_P3s,  
	EDataTraits_P3i,  
	EDataTraits_P3f,  
	EDataTraits_P3d,  

	EDataTraits_Box2s,
	EDataTraits_Box2i,
	EDataTraits_Box2f,
	EDataTraits_Box2d,

	EDataTraits_Box3s,
	EDataTraits_Box3i,
	EDataTraits_Box3f,
	EDataTraits_Box3d,

	EDataTraits_M33f, 
	EDataTraits_M33d, 
	EDataTraits_M44f, 
	EDataTraits_M44d, 

	EDataTraits_Quatf,
	EDataTraits_Quatd,

	EDataTraits_C3h,  
	EDataTraits_C3f,  
	EDataTraits_C3c,  

	EDataTraits_C4h,  
	EDataTraits_C4f,  
	EDataTraits_C4c,  

	EDataTraits_N2f,  
	EDataTraits_N2d,  

	EDataTraits_N3f,  
	EDataTraits_N3d,  

    EDataTraits_V4f,

	EDataTraits_Rotf,

	EDataTraits_NumDataTypes,
	EDataTraits_Unknown
};

/*! Enumeration of Alembic transform operation types, equivalent to Abc::AbcGeom::XformOperationType */
enum EAbcXformOpType
{
	EXformOp_Scale,	
	EXformOp_Translate,
	EXformOp_Rotate,
	EXformOp_Matrix,
	EXformOp_RotateX,
	EXformOp_RotateY,
	EXformOp_RotateZ
};

/*! Enumeration of possible return values for IAbcIXformOp::GetHint(), equivalent to Alembic::AbcGeom::MatrixHint */
enum EAbcMatrixHint
{
	EMatrixHint_Default = 0,		/*! The matrix operation is a regular transform operation */
	EMatrixHint_MayaShear = 1		/*! The matrix operation is a shear operation from Maya's Alembic export */
};

/*! Enumeration of possible return values for IAbcIXformOp::GetHint(), equivalent to Alembic::AbcGeom::RotateHint */
enum EAbcRotateHint
{
	ERotateHint_Default = 0,		/*! The rotation operation is a regular operation */
	ERotateHint_Orientation = 1		/*! The rotation operation is an adjustment rotation that orients the local rotation space */
};

/*! Enumeration of possible return values for IAbcIXformOp::GetHint(), equivalent to Alembic::AbcGeom::ScaleHint */
enum EAbcScaleHint
{
	EScaleHint_Default = 0			/*! The scale operation is a regular scale operation */
};

/*! Enumeration of possible return values for IAbcIXformOp::GetHint(), equivalent to Alembic::AbcGeom::TranslateHint */
enum EAbcTranslateHint
{
	ETranslateHint_Default = 0,						/*! The operation is a regular translation operation */
	ETranslateHint_ScalePivotPoint = 1,				/*! The translate operation is used for scaling around a pivot */
	ETranslateHint_ScalePivotTranslate = 2,			/*! The translate operation is used to preserve existing scale transforms when moving the pivot */
	ETranslateHint_RotatePivotPoint = 3,			/*! The translate operation is used to rotate around a pivot */
	ETranslateHint_RotatePivotTranslation = 4		/*! The translate operation is used to preserve existing rotate transforms when moving the pivot */
};

/*! Enumeration of the possible geometry scopes for properties, equivalent to Alembic::AbcGeom:GeometryScope 
	Please see GeometryScope.h in the Alembic header files for more information
*/
enum EAbcGeomScope
{
	EGeomScope_Constant,		
	EGeomScope_Uniform,
	EGeomScope_Varying,
	EGeomScope_Vertex,
	EGeomScope_FaceVarying,
	EGeomScope_Unknown = 127
};

/*! Enumeration of the time index request types for IAbcSampleSelector::GetRequestedTime() and IAbcSampleSelection::SetRequestedTime() */
enum EAbcTimeIndex
{
	ETimeIndex_Floor,	/*! Get the floor sample index for the requested time */
	ETimeIndex_Ceil,	/*! Get the ceiling sample index for the requested time */
	ETimeIndex_Near		/*! Get the nearest sample index for the requested time */
};

//*****************************************************************************
/*! \struct SAbcDataType
	\brief Struct containing data type information about an Alembic property
*/
//*****************************************************************************
struct SAbcDataType
{
	/*! Describes the POD (e.g. Bool, UInt8, Float32, etc )*/
	EAbcPodType		m_eType;
	/*! Describes the type traits (e.g. Vector3 float, Vector4 float, etc */
	EAbcDataTraits  m_eTraits;
	/*! The number of elements this datatype has (e.g. 3 for Vector3, 4 for Vector4) */
	unsigned char	m_ucExtent;
	/*! The total size in bytes for this data type (calculated sizeof(PODTYPE) * m_ucExtent) */
	size_t			m_numBytes;

	inline bool operator == ( const SAbcDataType& other ) const
	{
		return m_eType == other.m_eType && m_eTraits == other.m_eTraits && m_ucExtent == other.m_ucExtent && m_numBytes == other.m_numBytes;
	}
	inline bool operator != ( const SAbcDataType& other ) const
	{
		return !( operator==( other ) );
	}
};

/*! Enumeration of the possible Alembic archive types */
enum EAbcArchiveType
{
	EArchiveType_HDF5,				/*! Archive is an HDF5 archive */
	EArchiveType_Ogawa,				/*! Archive is an Ogawa archive */
	EArchiveType_Any = 127			/*! Don't know what archive type it is */
};

/*! Standard Defines for Softimage-specific property names */

/*! For 2D Array Props */
#define	PROPNAME_VALS					"vals"
#define PROPNAME_SUBARRAYINDICES		"subArrayIndices"

/*! For point cloud shape instancing */
#define	PROPNAME_SHAPETYPE				"shapeType"
#define PROPNAME_SHAPEINSTANCEID		"shapeInstanceId"
#define PROPNAME_SHAPEINSTANCEHIERARCHY	"shapeInstanceHierarchy"
#define PROPNAME_SHAPEINSTANCEDGEOMS	"shapeInstancedGeoms"

//*****************************************************************************
/*! \struct SAbcBBox
	\brief An axis-aligned bounding box type for use with ABC framework functions
*/
//*****************************************************************************
struct SAbcBBox
{
	/*! Default Constructor */
	SAbcBBox()
	{
		m_dMinX = 0.0;
		m_dMinY = 0.0; 
		m_dMinZ = 0.0;
		m_dMaxX = 0.0; 
		m_dMaxY = 0.0; 
		m_dMaxZ = 0.0;
	}

	/*! Initializes the bounding box with initial values */
	SAbcBBox( 
		double in_dMinX,
		double in_dMinY, 
		double in_dMinZ,
		double in_dMaxX, 
		double in_dMaxY, 
		double in_dMaxZ
		)
	{
		m_dMinX = in_dMinX;
		m_dMinY = in_dMinY; 
		m_dMinZ = in_dMinZ;
		m_dMaxX = in_dMaxX; 
		m_dMaxY = in_dMaxY; 
		m_dMaxZ = in_dMaxZ;
	}

	double m_dMinX;
	double m_dMinY; 
	double m_dMinZ;
	double m_dMaxX; 
	double m_dMaxY; 
	double m_dMaxZ;
};

//*****************************************************************************
/*! \class IAbcUtils
	\brief An interface for utility functions used by the ABC framework and its clients

	\eg Using IAbcUtils
	\code
		IAbcFramework* framework; // Assume that we have the interface
		double radians = framework->GetUtils().DegreesToRadians( 180.0 );
	\endcode
*/
//*****************************************************************************
class IAbcUtils
{
public:
	/*! Converts an angle in radians to degrees
	\param in_dAngle An angle in radians
	\return The equivalent angle in degrees
	*/
	virtual double RadiansToDegrees( double in_dAngle ) const = 0;

	/*! Converts an angle in degrees to radians
	\param in_dAngle An angle in degrees
	\return The equivalent angle in radians
	*/
	virtual double DegreesToRadians( double in_dAngle ) const = 0;
	virtual double FocalLengthFromFov( double in_dFov, double in_dAperture ) const = 0;

	/*! A helper function to get a non-wide string being pointed to by alembic string properties
	    These functions are here to ensure that we always access the string using the std string class compiled
	    with Alembic and the Framework.

	\param in_Index The sample index
	\param in_pBuffer The sample buffer containing the string
	\return The const char* buffer pointing to a string contained in the buffer
	*/
	virtual const char* GetStringFromSample( size_t in_Index, const class IAbcSampleBuffer* in_pBuffer ) const = 0;

	/*! A helper function to get a wide string being pointed to by alembic string properties
	    These functions are here to ensure that we always access the string using the std string class compiled
	    with Alembic and the Framework.

	\param in_Index The sample index
	\param in_pBuffer The sample buffer containing the wide-string
	\return The const wchar_t* buffer pointing to a wide-string contained in the buffer
	*/
	virtual const wchar_t* GetWideStringFromSample( size_t in_Index, const class IAbcSampleBuffer* in_pBuffer ) const = 0;

	/*! Gets the total size of the contained strings in a sample buffer
	\param in_pBuffer The input buffer
	\param in_startIndex The start index
	\param in_endIndex The end index
	\param out_TotalSize The total size of the strings
	\param out_pPerStringSize A pre-allocated array of unsigned ints to hold the inidividual lengths of the strings
	*/
	virtual void GetTotalStringSize( const class IAbcSampleBuffer* in_pBuffer, 
		Alembic::Util::uint32_t in_startIndex, Alembic::Util::uint32_t in_endIndex, 
		Alembic::Util::uint32_t& out_TotalSize, Alembic::Util::uint32_t* out_pPerStringSize ) const = 0;

	/*! Takes in a source buffer and a target buffer and creates a new buffer containing linearly interpolated values from the source to the target
	\param in_pBufferFrom The source buffer
	\param in_pBufferTo The target buffer, should be the same type as the source buffer
	\param out_ppBufferDest Pointer to the new buffer, the size will be the same as the smaller buffer between in_pBufferFrom and in_pBufferTo
	\param in_fAlpha The amount of interpolation
	\return Returns ::EResult_Success if successful. For other return codes, please see ::EAbcResult
	*/
	virtual EAbcResult GetInterpolatedBuffer( const class IAbcSampleBuffer* in_pBufferFrom, const class IAbcSampleBuffer* in_pBufferTo, class IAbcSampleBuffer** out_ppBufferDest, float in_fAlpha ) const = 0;

	/*! Takes in two transform samples and returns a linearly interpolated 4x4 Matrix between the two transforms
	\param in_pBufferFrom The source transform
	\param in_pBufferTo The target transform
	\param out_pdMat A pre-allocated array to put the output matrix, should have enough storage for 16 doubles
	\param in_fAlpha The amount of interpolation
	\return Returns ::EResult_Success if successful. For other return codes, please see ::EAbcResult
	*/
	virtual EAbcResult GetInterpolatedTransformMat44( const class IAbcIXformSample* in_pBufferFrom, const class IAbcIXformSample* in_pBufferTo, double* out_pdMat, float in_fAlpha ) const = 0;

	/*! Test if an IAbcOProperty is a valid Array2D property
	\param in_pOProp The input property
	\param out_ppValProp The pointer to hold the value property if any
	\param out_ppSubArrayIndicesProp The pointer to hold the subarray indices property if any
	\return True if the property is a valid Array2D
	*/
	virtual bool IsValidArray2DProp( IAbcOProperty* in_pOProp, IAbcOProperty** out_ppValProp, IAbcOProperty** out_ppSubArrayIndicesProp ) const = 0;

	/*! Test if an IAbcIPropertyAccessor is a valid Array2D property
	\param in_pOProp The input property
	\param out_ppValProp The pointer to hold the value property if any
	\param out_ppSubArrayIndicesProp The pointer to hold the subarray indices property if any
	\return True if the property is a valid Array2D
	*/
	virtual bool IsValidArray2DProp( IAbcIPropertyAccessor* in_pOProp, IAbcIPropertyAccessor** out_ppValProp, IAbcIPropertyAccessor** out_ppSubArrayIndicesProp ) const = 0;

	/*! Reverse the face winding of a buffer
	\param io_pBuffer The buffer
	\param in_pFaceCounts The array of face counts
	\param in_szNbFaces The number of faces
	\return Returns ::EResult_Success if successful. For other return codes, please see ::EAbcResult
	*/
	virtual EAbcResult ReverseFaceWinding( IAbcSampleBuffer* io_pBuffer, const Alembic::Util::int32_t* in_pFaceCounts, size_t in_szNbFaces ) const = 0;
};

//*****************************************************************************
/*! \class IAbcFramework
	\brief The interface definition for the ABC Framework

	\eg Using IAbcFramework
	\code
		// Load the dynamic library
		MODULE handle = AbcFrameworkLoader::InitFramework();

		// Get the interface pointer
		IAbcFramework* pFramework = AbcFrameworkLoader::GetFramework( handle );

		// Do stuff with the framework
		...
		
		// Cleanup
		pFramework->Release();
		AbcFrameworkLoader::CloseFramework( handle );
	\endcode
*/
//*****************************************************************************
class IAbcFramework : public IBase
{
public:
	/*! Returns const char* string buffer containing the ABC Framework version */
	virtual const char*			GetFrameworkVersionString() const = 0;
	
	/*! Returns const char* string buffer containing the Alembic version compiled with the framework */
	virtual const char*			GetAlembicVersionString() const = 0;

	/*! Opens an archive for reading and returns a valid archive object if successful
	\param in_pszFilename The filename of the archive
	\param out_ppArchive The returned archive if successful
	\return Returns ::EResult_Success if successful. For other return codes, please see ::EAbcResult

	\eg Usage:
	\code
		IAbcFramework* pFramework; // Assume we have a valid interface pointer
		CAbcPtr< IAbcIArchive > spArchive;
		if ( pFramework->OpenIArchive( "test.abc", &spArchive ) == EResult_Success )
		{
			// Loaded successfully
		}
		else
		{
			// Error
		}
	\endcode
	*/
	virtual EAbcResult			OpenIArchive( const char* in_pszFileName, IAbcIArchive** out_ppArchive ) = 0;

	/*! Opens or creates an archive for writing and returns a valid archive object if successful
	\param in_pszFilename The filename of the archive
	\param out_ppArchive The returned archive if successful
	\param in_eArchiveType The archive type to create/open
	\return Returns ::EResult_Success if successful. For other return codes, please see ::EAbcResult

	\eg Usage:
	\code
		IAbcFramework* pFramework; // Assume we have a valid interface pointer
		CAbcPtr< IAbcOArchive > spArchive;
		
		// Create an ogawa archive
		if ( pFramework->OpenOArchive( "test.abc", &spArchive, EArchiveType_Ogawa ) == EResult_Success )
		{
			// Created successfully
		}
		else
		{
			// Error
		}
	\endcode
	*/
	virtual EAbcResult			OpenOArchive( const char* in_pszFileName, IAbcOArchive** out_ppArchive, EAbcArchiveType in_eArchiveType = EArchiveType_Ogawa ) = 0;

	/*! Gets a reference to the IAbcUtils interface which could be used to access various utility functions
	\return A reference to an IAbcUtils object
	*/
	virtual const IAbcUtils&	GetUtils() const = 0;
};

#endif // ABCFRAMEWORKINTERFACE_H
