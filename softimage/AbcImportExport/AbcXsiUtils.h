//*****************************************************************************
/*!
	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*

#ifndef _AbcXsiUtils_h_
#define _AbcXsiUtils_h_

#include <xsi_decl.h>
#include <xsi_progressbar.h>
#include <xsi_string.h>
#include <xsi_math.h>
#include <stack>
#include <vector>
#include <map>

#include "IAbcInput.h"

#define POSITION_EPSILON	( (double)1e-3 )
#define SCALE_EPSILON		( (double)1e-3 )
#define ROTATION_EPSILON	( (double)1e-3 )
#define MATRIX_EPSILON		( (double)1e-8 )

/*! \class CLog
	\brief Helper class to print log messages with indentation
*/
class CLog
{
public:
	/*! Constructor
	*/
	CLog() : m_iLogLevel(0) {}

	/*! Increment the indentation level
	*/
	void Indent();

	/*! Decrement the indentation level
	*/
	void UnIndent();

	/*! Log a message
		\param in_bIndent Use indentation
		\param in_SeverityType Severity
		\param in_szFormat Format string
	*/
	void Log ( bool in_bIndent, XSI::siSeverityType in_SeverityType, const char * in_szFormat, ... );
private:
	int m_iLogLevel;
};

class CXsiProgressBar
{
public:
	CXsiProgressBar();
	~CXsiProgressBar();

	void Increment();
	void SetMin( long in_lMin );
	void SetMax( long in_lMax );
	void SetValue( long in_lVal );
	void SetStatusText( const XSI::CString& in_csText );
	void SetCaption( const XSI::CString& in_csText );
	void SetVisible( bool in_bVisible );

	long GetMin() const;
	long GetMax() const;
	long GetValue() const;
	float GetPercent() const;

	bool IsCancelPressed() const;
private:
	struct SProgressBarState
	{
		XSI::CString m_csCaption;
		XSI::CString m_csText;
		long m_lMin;
		long m_lMax;
		long m_lCurrent;
	};
	static std::stack<SProgressBarState> m_States;
	static XSI::ProgressBar m_ProgressBar;
	static bool m_bInit;
	static void Push();
	static void Pop();
};

/*! \class CPolyMeshDefinition 
	\brief Helper class for Alembic -> Softimage mesh conversons
*/
class CPolyMeshDefinition
{
public:
	/*! Constructor
	*/
	CPolyMeshDefinition() : m_pParentDef( 0 ) {}
	
	/*! Sets the parent definition of this mesh. 
		If the position/index/facecount/uv/normal buffers are empty, the Get* functions will then
		look into the parent to get the actual buffers. This is used to save memory if the buffers are constant
		over time.
		\param in_pParent The parent definition
	*/
	void SetParent( CPolyMeshDefinition* in_pParent );

	/*! Setters for our buffers, SetNormals and SetUVs can automatically fix the winding order of the vertices from Alembic winding to Softimage winding 
		\param in_pfVals The positions
		\param in_NumPos The number of position values
	*/
	void SetPositions( const float* in_pfVals, size_t in_NumPos );

	/*! Set positions
		\param in_pdVals The positions
		\param in_NumPos The number of position values
	*/
	void SetPositions( const double* in_pdVals, size_t in_NumPos );

	/*! Set vertex indices and face counts
		\param in_plIndices The array of vertex indices per face
		\param in_IndexCount The number of indices
		\param in_plFaceCounts The array of vertex count per face
		\param in_FaceCount The number of faces
	*/
	void SetIndicesAndFaceCounts( const LONG* in_plIndices, size_t in_IndexCount, const LONG* in_plFaceCounts, size_t in_FaceCount );

	/*! Set normals
		\param in_pfNormals The array of normals
		\param in_NumNormals The number of normal values
		\param in_bInvertWinding True if the winding needs to be reversed
	*/
	void SetNormals( const float* in_pfNormals, size_t in_NumNormals, bool in_bInvertWinding = true );

	/*! Set UV
		\param in_pfUVs The UV values
		\param in_NumUVs The number of UV values
		\param in_bInvertWinding True if the winding needs to be reversed.
	*/
	void SetUVs( const float* in_pfUVs, size_t in_NumUVs, bool in_bInvertWinding = true );

	/*! Set indexed normals
		\param in_pfNormals The array of normals
		\param in_NumNormals The number of normal values
	*/
	void SetIndexedNormals( const float* in_pfNormals, size_t in_NumNormals );

	/*! Set indexed UV
		\param in_pfUVs The UV values
		\param in_NumUVs The number of UV values
	*/
	void SetIndexedUVs( const float* in_pfUVs, size_t in_NumUVs );

	/*! Get the parent definition
		\return The parent definition 
	*/
	CPolyMeshDefinition* GetParent();

	/*! Get the position array
		\return Pointer to the position array
	*/
	double* GetPositions();

	/*! Get the UV array
		\return Pointer to the UV array
	*/
	float*  GetUVs();

	/*! Get the normal array
		\return Pointer to the normal array
	*/
	float*  GetNormals();

	/*! Get the face vertex indices array
		\return Pointer to the face vertex indices array
	*/
	LONG*	GetIndices();

	/*! Get the face vertex count array
		\return Pointer to the face vertex count array
	*/
	LONG*	GetFaceCounts();

	/*! Get the number of position values
		\return The number of position values
	*/
	size_t GetPositionsCount();

	/*! Get the number of UV values
		\return The number of UV values
	*/
	size_t GetUVsCount();

	/*! Get the number of normal values
		\return The number of normal values
	*/
	size_t GetNormalsCount();

	/*! Get the number of face vertex index values
		\return The number of face vertex index values
	*/
	size_t GetIndicesCount();

	/*! Get the number of face vertex count values
		\return The number of face vertex count values
	*/
	size_t GetFaceCount();
private:
	std::vector<double> m_vecPositions;
	std::vector<float>	m_vecUVs;
	std::vector<float>	m_vecNormals;
	std::vector<LONG>	m_vecIndices;
	std::vector<LONG>	m_vecFaceCounts;

	CPolyMeshDefinition* m_pParentDef;

	std::vector<double>& GetPositionsContainer();
	std::vector<float>& GetUVsContainer();
	std::vector<float>& GetNormalsContainer();
	std::vector<LONG>& GetIndicesContainer();
	std::vector<LONG>& GetFaceCountsContainer();	
};

/*! \class CPolyMeshDefinitionCache
	\brief Helper class to cache samples read from Alembic. When we import a polymesh shape animation, we first determine which frame range in Softimage time
	we need to import. We then query the Alembic archive which samples we need for the current Softimage frame and interpolate between the samples if 
	needed. This class helps cache the samples that we already got from Alembic.
*/
class CPolyMeshDefinitionCache
{
public:
	/*! Constructor
	*/
	CPolyMeshDefinitionCache();

	/*! Destructor
	*/
	~CPolyMeshDefinitionCache();

	/*! Get a cache entry from sample index
		\param in_llSample The sample index
		\param out_ppDefinition Pointer to hold the found entry
		\return True if the entry is found
	*/
	bool GetEntryBySample( long long in_llSample, CPolyMeshDefinition** out_ppDefinition ) const;

	/*! Return the root entry
		\param out_ppDefinition Pointer to hold the root entry
		\return True if there is at least an entry in the cache
	*/
	bool GetRootEntry( CPolyMeshDefinition** out_ppDefinition ) const;

	/*! Create a cache entry at an index
		\param in_llSample The sample index
		\param out_ppDefinition Pointer to hold the definition created
		\return True if a new entry is created.
	*/
	bool CreateEntry( long long in_llSample, CPolyMeshDefinition** out_ppDefinition );

	/*! Return the number of entries
		\return The number of entries
	*/
	size_t GetNumEntries() const;
	
private:
	typedef std::map< long long, CPolyMeshDefinition* > TCacheMap;
	TCacheMap m_Entries;
};

template< typename TDest, typename TSrc >
void CastedCopy( TDest* in_pDest, const TSrc* in_pSrc, size_t in_numElems )
{
	for ( size_t i = 0; i < in_numElems; i++ )
	{
		in_pDest[i] = (TDest)in_pSrc[i];
	}
}
template< typename T, typename F >
T LerpSingleValue( T in_From, T in_To, F in_Factor )
{
	return (T)( (F)in_From + in_Factor * (F)( in_To - in_From ) );
}

template< typename T, typename F >
void LerpArray( T* in_pDest, const T* in_pSrcFrom, const T* in_pSrcTo, F in_Factor, size_t in_NumItmes )
{
	for ( size_t i = 0; i < in_NumItmes; i++ )
	{
		in_pDest[i] = (T)( (F)in_pSrcFrom[i] + in_Factor * (F)( in_pSrcTo[i] - in_pSrcFrom[i] ) );
	}
}

inline double WrapAngle( double in_dAngle )
{
	while ( in_dAngle > 360.0 )
		in_dAngle -= 360.0;
	while ( in_dAngle < 0.0 )
		in_dAngle += 360.0;
	return in_dAngle;
}

inline double LerpAngle( double in_dFrom, double in_dTo, double in_dFactor )
{
	in_dFrom = WrapAngle( in_dFrom );
	in_dTo = WrapAngle( in_dTo );

	double l_dDistance = in_dTo - in_dFrom;
	double l_dDistanceCW = WrapAngle( l_dDistance );
	double l_dDistanceCCW = WrapAngle( -l_dDistance );

	double l_dMovement = l_dDistanceCW < l_dDistanceCCW ? l_dDistanceCW : -l_dDistanceCCW;

	return WrapAngle( in_dFrom + in_dFactor * l_dMovement );
}

bool HasShearing( IAbcIXformSample* in_pSample );

class CTransformHelper
{
public:
	CTransformHelper() {};
	CTransformHelper( IAbcIXformSample* in_pSample );
	void AddTranslation( double in_dX, double in_dY, double in_dZ );
	void AddRotationAxisAngle( double in_dX, double in_dY, double in_dZ, double in_dAngle );
	void AddScaling( double in_dX, double in_dY, double in_dZ );
	void AddRotX( double in_dAngle );
	void AddRotY( double in_dAngle );
	void AddRotZ( double in_dAngle );
	void AddMatrix( const double* in_dMatrix44 );
	XSI::MATH::CTransformation GetTransform() const;
protected:
	std::vector<XSI::MATH::CMatrix4> m_Transforms;
};

/*! \struct GenericTypeInfo
	\brief Type info for AbcImportTreeNode
*/
struct GenericTypeInfo
{
	int m_TypeID;
	const GenericTypeInfo* m_BaseType;
	GenericTypeInfo( int in_type, const GenericTypeInfo* in_pBase ) : m_TypeID( in_type ), m_BaseType( in_pBase ) {}

	bool operator == ( const GenericTypeInfo& other ) const
	{
		return m_TypeID == other.m_TypeID;
	}
	bool operator != ( const GenericTypeInfo& other ) const
	{
		return m_TypeID != other.m_TypeID;
	}
};

#define DECL_TYPE_FUNCS(x) \
	static const GenericTypeInfo*	GetTypeInfo() { static T_##x sType; return &sType; }		\
	virtual const GenericTypeInfo* GetInstanceTypeInfo() const { return GetTypeInfo(); }	

#define DECL_BASE_TYPE_INFO(x)																	\
	struct T_##x : public GenericTypeInfo													\
	{																						\
		T_##x() : GenericTypeInfo(x, NULL)	{}												\
	};																						\
	DECL_TYPE_FUNCS(x)


#define DECL_TYPE_INFO(x, b)																\
	struct T_##x : public GenericTypeInfo													\
	{																						\
		T_##x() : GenericTypeInfo(x, b::GetTypeInfo())	{}									\
	};																						\
	DECL_TYPE_FUNCS(x)	

bool isTypeOf( const GenericTypeInfo* a, const GenericTypeInfo* b );

#endif
