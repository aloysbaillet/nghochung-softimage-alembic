//*****************************************************************************
/*!
	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*

#ifndef _ExportHelper_h_
#define _ExportHelper_h_

#include <xsi_string.h>
#include <xsi_doublearray.h>
#include <xsi_floatarray.h>
#include <xsi_iceattributedataarray.h>
#include <xsi_iceattributedataarray2D.h>
#include <xsi_longarray.h>
#include <xsi_ref.h>
#include <xsi_status.h>
#include <map>
#include <vector>

namespace XSI
{
	class CGeometryAccessor;

	class Geometry;
	class Material;

	class Parameter;
	class Shader;
}

/*! \class CRefUniqueArray
	\brief The CRefUniqueArray extends the CRefArray class to allow only unique references

*/
class CRefUniqueArray
{
public:
	typedef std::map< XSI::CRef, ULONG > CRefToIndexMap;


	/*! Add a reference
		\param in_ref The reference to add. This can be the same as an existing reference in the list.
		\return The index of the reference in a unique list
	*/
	inline ULONG Add( const XSI::CRef& in_ref )
	{
		CRefToIndexMap::const_iterator it = m_refToIndexMap.find( in_ref );
		if ( it == m_refToIndexMap.end() )
		{
			ULONG index = m_refArray.GetCount();
			m_refToIndexMap[ in_ref ] = index;
			m_refArray.Add( in_ref );
			return index;
		}
		else
		{
			return it->second;
		}
	}

	/*! Append an array of references to the list
		\param in_refArray The array of references
	*/
	inline void Append( const XSI::CRefArray& in_refArray )
	{
		for (ULONG i=0; i<(ULONG)in_refArray.GetCount(); ++i)
			Add( in_refArray[i] );
	}

	/*! Get the index of a reference in the unique list
		\param in_ref The reference
		\return The index of the reference if it is found. -1 if it is not found. 
	*/
	inline ULONG GetIndex( const XSI::CRef& in_ref ) const
	{
		CRefToIndexMap::const_iterator it = m_refToIndexMap.find( in_ref );

		if ( it == m_refToIndexMap.end() )
			return -1;
		else
			return it->second;
	}

	/*! Return the array of unique references
		\return The array of references
	*/
	inline const XSI::CRefArray& GetRefArray() const { return m_refArray;}

	/*! Return the number of references
	*/
	inline ULONG GetCount() const { return m_refArray.GetCount();}

	/*! Return the reference-to-index map
	*/
	inline const CRefToIndexMap& GetRefToIndexMap() const { return m_refToIndexMap;}

protected:
	XSI::CRefArray m_refArray;
	CRefToIndexMap m_refToIndexMap;
};

/*! \class CMaterialArray
	\brief Helper class to keep a list of materials referenced by mesh polygons
	
	Since material list (either from cluster or from ICE attribute "Materials") are allowed to contain duplicates,
	two MaterialID values can reference the same material.
	
	To avoid exporting duplicates, we keep a map from non-unique material index to a unique index.
	The lookup time is constant and shouldn't affect the performance when the mesh have 10000 polygons.
*/
class CMaterialArray
{
public:
	typedef std::vector<ULONG> IndexArray;

	/*! Constructor
	*/
	CMaterialArray()
		: m_allIndices( 0 )
	{

	}

	/*! Add a material to the list
	*/
	inline void Add( const XSI::CRef& in_ref )
	{
		m_allRef.Add( in_ref );
		m_allIndices.push_back( m_uniqueRef.Add (in_ref) );
	}

	/*! Append an array of materials to the list
	*/
	inline void Append( const XSI::CRefArray& in_refArray )
	{
		for (ULONG i=0; i<(ULONG)in_refArray.GetCount(); ++i)
			Add( in_refArray[i] );
	}


	/*! Get the unique index of a material
	*/
	inline ULONG GetIndexInUniqueArray( ULONG in_ulMatID ) const
	{
		return m_allIndices[in_ulMatID];
	}

	/*! Get the array of materials with possible duplicates
	*/
	inline const XSI::CRefArray& GetAllArray() const { return m_allRef;}

	/*! Get the array of unique materials
	*/
	inline const XSI::CRefArray& GetUniqueArray() const { return m_uniqueRef.GetRefArray();}

protected:
	CRefUniqueArray m_uniqueRef;
	XSI::CRefArray m_allRef; // array of non-unique materials
	IndexArray m_allIndices;
};

/*! \class SceneMaterialFinder
	\brief Helper class to find a material from name
*/
class SceneMaterialFinder
{
public:
	SceneMaterialFinder();

	XSI::CRef Find( const XSI::CString& in_csName ) const;
	XSI::CStatus CollectICEMaterials( CMaterialArray& out_materials, const XSI::Geometry& in_geom ) const;

private:
	XSI::CRefArray m_refMaterialLibraries;
};

/*! \class CDirectAccessArray
	\brief Helper class that provides direct access to elements in XSI::CLongArray, XSI::CFloatArray, XSI::CDoubleArray

*/
template <class sicppArrayT, typename CElemT>
class CDirectAccessArray
{
public:
	/*! Constructor
	*/
	CDirectAccessArray()
		: m_ArrayImpl()
	{
		UpdatePtr();
	}

	/*! Construcutor
		\param in_array The XSI C++ SDK array object
	*/
	CDirectAccessArray( const sicppArrayT& in_array )
		: m_ArrayImpl( in_array )
	{
		UpdatePtr();
	}

	/*! Returns the number of elements
	*/
	inline LONG GetCount() const { return m_ArrayImpl.GetCount();}

	/*! Returns the pointer to the data array
	*/
	inline const CElemT* GetArray() const { return m_pPtr;}

	/*! Returns the pointer to the data array
	*/
	inline CElemT* GetArray() { return m_pPtr;}

	/*! Const element access
	*/
	inline const CElemT& operator[]( LONG in_lIndex ) const { return m_pPtr[in_lIndex];}

	/*! Non-const element access
	*/
	inline CElemT& operator[]( LONG in_lIndex ) { return m_pPtr[in_lIndex];}
	
	/*! Cast operator to the original XSI C++ SDK array
	*/
	operator sicppArrayT&() { return m_ArrayImpl;}

	/*! Cast operator to the original XSI C++ SDK array
	*/
	operator const sicppArrayT& () const { return m_ArrayImpl;}

	/*! Resize
	*/
	inline XSI::CStatus Resize( LONG in_lNewSize )
	{
		XSI::CStatus status = m_ArrayImpl.Resize( in_lNewSize );
		if (status == XSI::CStatus::OK)
		{
			UpdatePtr();
		}
		return status;
	}

	/*! Update the pointer used for direct access. This function should be called when the XSI array size is changed.
	*/
	inline void UpdatePtr()
	{
		m_pPtr = (CElemT*)m_ArrayImpl.GetArray();
	}

	/*! Add new element
	*/
	inline void Add( const CElemT& in_val )
	{
		m_ArrayImpl.Add( in_val );
	}

private:
	sicppArrayT m_ArrayImpl;
	CElemT* m_pPtr;
};

/*! \class ExportHelper
	\brief Helper class with functions to prepare data from mesh
*/
class ExportHelper
{
public:
	/*! Helper class to provide direct access to XSI::CLongArray elements
	*/
	typedef CDirectAccessArray<XSI::CLongArray, LONG> LongVector;

	/*! Helper class to provide direct access to XSI::CFloatArray elements
	*/
	typedef CDirectAccessArray<XSI::CFloatArray, float> FloatVector;

	/*! Helper class to provide direct access to XSI::CDoubleArray elements
	*/
	typedef CDirectAccessArray<XSI::CDoubleArray, double> DoubleVector;
	
	/*! \struct ClusterInfo
		\brief Represent a cluster to be created in Alembic
	
	*/
	struct ClusterInfo
	{
		XSI::CString m_csName;
		XSI::CLongArray m_Elements;

		ClusterInfo( const XSI::CString& in_csName ) 
			: m_csName( in_csName )
		{

		}

		ClusterInfo( const XSI::CString& in_csName, const XSI::CLongArray& in_elemArray ) 
			: m_csName( in_csName )
			, m_Elements( in_elemArray )
		{ }
	};

	typedef std::vector< ClusterInfo* > ClusterInfoVector;
	
	/*! Prepare geometry data
		\param out_vertexPos Array of vertex positions
		\param out_faceVertices Array of face vertices
		\param out_faceVertexCount Array of vertex count per face
		\param in_geomAccessor Geometry to be exported
		\return CStatus::OK Success
	*/
	static XSI::CStatus PrepareGeomData( FloatVector& out_vertexPos, LongVector& out_faceVertices, LongVector& out_faceVertexCount, const XSI::CGeometryAccessor& in_geomAccessor );

	/*! Prepare UV and cluster data to export. This function supports material and UV data defined in ICE.
		\param out_UVs Array of u,v coordiates
		\param out_nodeIndices Array of node indices
		\param out_clusterInfoVec Array of ClusterInfo.
		\param in_faceVertexCount Array of vertex count perface
		\param in_geomAccess The geometry accessor
		\param in_geom The geometry
		\return CStatus::OK Success
	*/
	static XSI::CStatus PrepareUVData( FloatVector& out_UVs, LongVector& out_nodeIndices, ClusterInfoVector& out_clusterInfoVec, LongVector& in_faceVertexCount, const XSI::CGeometryAccessor& in_geomAccessor, const XSI::Geometry& in_geom );
	
	/*! Prepare normal data to export. This function supports UserNormal defined in ICE.
		\param out_UVs Array of normals
		\param out_nodeIndices Array of node indices
		\param in_faceVertexCount Array of vertex count perface
		\param in_geomAccess The geometry accessor
		\param in_geom The geometry
		\return CStatus::OK Success
	*/
	static XSI::CStatus PrepareNormalData( FloatVector& out_UVs, LongVector& out_nodeIndices, LongVector& in_faceVertexCount, const XSI::CGeometryAccessor& in_geomAccessor, const XSI::Geometry& in_geom );

	/*! Prepare node indices to match Alembic clock-wise winding
		\param out_nodeIndices Array of node indices
		\param in_faceVertexCount Array of vertex count per face
		\param in_geomAccess The geometry accessor
		\return CStatus::OK Success
	*/
	static XSI::CStatus PrepareNodeIndices( LongVector& out_nodeIndices, const LongVector& in_faceVertexCount, const XSI::CGeometryAccessor& in_geomAccessor );
	
	/*! Get the first "texture space ID" (texture projection) parameter in a material.
		\param out_tspaceidParam The result texspaceID parameter
		\param in_material The material
		\return CStatus::OK Success
	*/
	static XSI::CStatus GetFirstTexSpaceIDParam( XSI::Parameter& out_tspaceidParam, const XSI::Material& in_material );

	/*! Get the first "texture space ID" (texture projection) parameter in a shader.
		\param out_tspaceidParam The result texspaceID parameter
		\param in_material The shader
		\return CStatus::OK Success
	*/
	static XSI::CStatus GetFirstTexSpaceIDParam( XSI::Parameter& out_tspaceidParam, const XSI::Shader& in_shader );
	
	/*! Get the material list and material-per-cluster info from a geometry
		\param out_materials The array of materials
		\param out_materialID The array of material index per polygon
		\param out_clusterInfoVec The array of ClusterInfo
		\param in_geomAccessor The geometry accessor
		\param in_geom The geometry
		\return CStatus::OK Success
	*/
	static XSI::CStatus GetMaterialData( 
		XSI::CRefArray& out_materials, 
		LongVector& out_materialID, 
		ClusterInfoVector& out_clusterInfoVec,
		const XSI::CGeometryAccessor& in_geomAccessor, 
		const XSI::Geometry& in_geom );

	/*! Reverse face winding to match Alembic's
		\param io_indices The array of node/vertex indices
		\param in_faceVertexCount The array of vertex count per polygon
	*/
	template< class ElemT >
	static void ReverseFaceWinding( ElemT* io_Data, ULONG in_ulElemCount, const LONG* in_pFaceCounts, ULONG in_NbFaces );

	/*! Reverse face winding to match Alembic's
		\param out_indices The array of output node/vertex indices
		\param in_indices The array of input node/vertex indices
		\param in_faceVertexCount The array of vertex count per polygon
	*/
	template< class ElemT >
	static void ReverseFaceWinding( ElemT* out_Data, const ElemT* in_Data, ULONG in_ulElemCount, const LONG* in_pFaceCounts, ULONG in_NbFaces );

	/*! Compace UVW coordinates into UV vector
		\param out_UVs The output u,v array
		\param in_array The input u,v,w array
	*/
	static void PackUVWtoUV( FloatVector& out_UVs, const FloatVector& in_array );

	/*! Compace UVW coordinates into UV vector
		\param out_UVs The output u,v array
		\param in_array The input ICE attribute data for u,v,w
	*/
	static void PackUVWtoUV( FloatVector& out_UVs, const XSI::CICEAttributeDataArrayVector3f& in_array );

	/*! Calculate local transform from global
		\param out_xformChildLocal The child local transform
		\param in_xformChildGlobal The child global transform
		\param in_xformParentGlobal The parent global transform
	*/
	static void CalculateLocalTransform( XSI::MATH::CTransformation& out_xformChildLocal, const XSI::MATH::CTransformation& in_xformChildGlobal, const XSI::MATH::CTransformation& in_xformParentGlobal );
};

template< class ElemT >
void ExportHelper::ReverseFaceWinding( ElemT* io_data, ULONG in_ElemCount, const LONG* in_pFaceCounts, ULONG in_NbFaces )
{
	if ( in_NbFaces == 0 || in_ElemCount == 0 )
		return;

	const LONG* l_pVertexCount = &in_pFaceCounts[0];
	ElemT* l_pElem = (ElemT*)&io_data[0];

	for (LONG j = in_NbFaces; j>0; --j, ++l_pVertexCount)
	{
		const LONG l_lCount = *l_pVertexCount;

		if ( l_lCount == 4 )
		{
			std::swap( *l_pElem, *(l_pElem + 3) );
			std::swap( *(l_pElem + 1), *(l_pElem + 2) );
			l_pElem += 4;
		}
		else if ( l_lCount == 3 )
		{
			std::swap( *l_pElem, *(l_pElem + 2) );
			l_pElem += 3;
		}
		else
		{
			ElemT* l_lLeft = l_pElem;
			ElemT* l_lRight = l_pElem + l_lCount - 1;

			for (LONG j=l_lCount/2; j>0; --j, ++l_lLeft, --l_lRight)
			{
				std::swap( *l_lLeft, *l_lRight );
			}

			l_pElem += l_lCount;
		}
	}
}

template< class ElemT >
void ExportHelper::ReverseFaceWinding( ElemT* out_pData, const ElemT* in_pData, ULONG in_ElemCount, const LONG* in_pFaceCounts, ULONG in_NbFaces )
{
	if ( in_NbFaces == 0 || in_ElemCount == 0 )
		return;

	ElemT* l_pOut = out_pData;
	ElemT* l_pIn = in_pData + in_pFaceCounts[0] - 1;

	for (LONG i = 0; i < in_NbFaces ; ++i )
	{
		const LONG l_lCount = in_pFaceCounts[i];

		for (LONG j = 0; j < l_lCount; ++j)
		{
			*l_pOut = *l_pIn;
			l_pOut++;
			l_pIn--;
		}

		l_pIn += l_lCount;
	}
}

/*! \struct CaseInsensitiveLess
	\brief Binary functor class that provides case-insensitive string comparison
*/
struct CaseInsensitiveLess : std::binary_function<XSI::CString, XSI::CString, bool>
{
	/*! Compare two XSI::CString and returns if the first string is less than the second.
		\param x The first string
		\param y The second string
		\return True if the first is less than the second
	*/
	bool operator() ( const XSI::CString& x, const XSI::CString& y ) const;
};

class CICEAttributeDataArrayBuffer
{
public:
	CICEAttributeDataArrayBuffer( XSI::ICEAttribute& in_attr )
		: m_pBuffer( NULL )
	{
		if ( in_attr.IsDefined() )
		{
			switch (in_attr.GetStructureType())
			{
			case XSI::siICENodeStructureSingle:
				{
					switch (in_attr.GetDataType())
					{
#define CASE_ATTRTYPE( ATTRTYPE, DTYPE ) case ATTRTYPE: m_pBuffer = new XSI::CICEAttributeDataArray<DTYPE>; break
						CASE_ATTRTYPE( XSI::siICENodeDataBool, bool );
						CASE_ATTRTYPE( XSI::siICENodeDataLong, LONG);		
						CASE_ATTRTYPE( XSI::siICENodeDataFloat, float);		
						CASE_ATTRTYPE( XSI::siICENodeDataVector2, XSI::MATH::CVector2f);		
						CASE_ATTRTYPE( XSI::siICENodeDataVector3, XSI::MATH::CVector3f );		
						CASE_ATTRTYPE( XSI::siICENodeDataVector4, XSI::MATH::CVector4f );	

						CASE_ATTRTYPE( XSI::siICENodeDataQuaternion, XSI::MATH::CQuaternionf );	
						CASE_ATTRTYPE( XSI::siICENodeDataMatrix33, XSI::MATH::CMatrix3f ); 	
						CASE_ATTRTYPE( XSI::siICENodeDataMatrix44, XSI::MATH::CMatrix4f ); 	
						CASE_ATTRTYPE( XSI::siICENodeDataColor4, XSI::MATH::CColor4f ); 		
						CASE_ATTRTYPE( XSI::siICENodeDataRotation, XSI::MATH::CRotationf );	
						CASE_ATTRTYPE( XSI::siICENodeDataShape, XSI::MATH::CShape );		
#undef  CASE_ATTRTYPE
					case XSI::siICENodeDataString: m_pBuffer = new XSI::CICEAttributeDataArrayString; break;

					default: break;
					}

					if ( in_attr.GetElementCount() > 0 && m_pBuffer)
						in_attr.GetDataArray( *m_pBuffer );
				}
				break;

			case XSI::siICENodeStructureArray:
				{

					switch (in_attr.GetDataType())
					{
#define CASE_ATTRTYPE( ATTRTYPE, DTYPE ) case ATTRTYPE: m_pBuffer = new XSI::CICEAttributeDataArray2D<DTYPE>; break
						CASE_ATTRTYPE( XSI::siICENodeDataBool, bool );
						CASE_ATTRTYPE( XSI::siICENodeDataLong, LONG);		
						CASE_ATTRTYPE( XSI::siICENodeDataFloat, float);		
						CASE_ATTRTYPE( XSI::siICENodeDataVector2, XSI::MATH::CVector2f);		
						CASE_ATTRTYPE( XSI::siICENodeDataVector3, XSI::MATH::CVector3f );
						CASE_ATTRTYPE( XSI::siICENodeDataVector4, XSI::MATH::CVector4f );			

						CASE_ATTRTYPE( XSI::siICENodeDataQuaternion, XSI::MATH::CQuaternionf );	
						CASE_ATTRTYPE( XSI::siICENodeDataMatrix33, XSI::MATH::CMatrix3f ); 	
						CASE_ATTRTYPE( XSI::siICENodeDataMatrix44, XSI::MATH::CMatrix4f ); 	
						CASE_ATTRTYPE( XSI::siICENodeDataColor4, XSI::MATH::CColor4f ); 		
						CASE_ATTRTYPE( XSI::siICENodeDataRotation, XSI::MATH::CRotationf );	
						CASE_ATTRTYPE( XSI::siICENodeDataShape, XSI::MATH::CShape );		
#undef  CASE_ATTRTYPE
					case XSI::siICENodeDataString: m_pBuffer = new XSI::CICEAttributeDataArray2DString; break;

					default: break;
					}
				
					if ( in_attr.GetElementCount() > 0 && m_pBuffer )
						in_attr.GetDataArray( *m_pBuffer );
				}
				break;
			}

		}
	}

	~CICEAttributeDataArrayBuffer()
	{
		if ( m_pBuffer )
			delete m_pBuffer;
	}

	XSI::CBaseICEAttributeDataArray* m_pBuffer;
};
#endif
