//*****************************************************************************
/*!
	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*

#ifndef _AbcXsiExporter_h_
#define _AbcXsiExporter_h_

#include <AbcFrameworkUtil.h>
#include <IAbcFramework.h>
#include <IAbcOProperty.h>
#include <IAbcOutput.h>

#include "AbcXsiIO.h"
#include "AbcXsiUtils.h"
#include "ExportHelper.h"

#include <xsi_geometry.h>
#include <xsi_ref.h>
#include <xsi_string.h>

#include <map>
#include <set>
#include <vector>

class IAbcOObject;
class ExportTreeNode;

void SplitAndTrim( XSI::CStringArray& out_csArray, const XSI::CString& in_cs );

/*! \class AttributeNameSet
	\brief Container for a set of unique ICE attribute names. Comparison for unique-ness is case-insensitive string comparison.
	AttributeNameSet supports wildcard "*" which means all attributes are match.
	*/
class AttributeNameSet : private std::set< XSI::CString, CaseInsensitiveLess >
{
public:
	typedef std::set< XSI::CString, CaseInsensitiveLess > TheBaseClass;
	typedef TheBaseClass::const_iterator ConstIterator;

	/*! Get the iterator to beginning of the set
	\return AttributeNameSet::ConstIterator The iterator to the beginning
	*/
	ConstIterator Begin() const { return begin();}

	/*! Get the iterator to end of the set
	\return AttributeNameSet::ConstIterator The iterator to end
	*/
	ConstIterator End() const { return end();}

	/*! Default constructor
	*/
	AttributeNameSet() : m_bMatchAll( false ) {}

	/*! Insert into the set the attribute names stored in an CStringArray
	*/	
	const AttributeNameSet& operator+=( const XSI::CStringArray& in_strArray )
	{
		for ( LONG i=0; i<in_strArray.GetCount(); ++i )
		{
			Insert( in_strArray[i] );
		}

		return *this;
	}

	/*! Write the attribute names to a CString
	\param out_csAllAttrs The CString where the names are written to
	*/
	void ToString( XSI::CString& out_csAllAttrs ) const
	{
		for ( AttributeNameSet::const_iterator it = TheBaseClass::begin(); it != TheBaseClass::end(); ++it)
		{
			out_csAllAttrs += *it;
			out_csAllAttrs += ',';
		}
	}

	/*! Test if a name exists in the set
	\param in_csAttr The attribute name
	\return true if the name is found in the set
	*/
	bool Find( const XSI::CString& in_csAttr ) const
	{
		if ( m_bMatchAll )
			return true;
		else
			return ( find(in_csAttr) != end() );
	}

	/*! Insert all attributes of a geometry
	\param in_geom The XSI geometry
	*/
	void InsertAllAttributes( const XSI::Geometry& in_geom )
	{
		XSI::CRefArray attrs = in_geom.GetICEAttributes();
		for ( LONG i=0; i< attrs.GetCount(); ++i )
		{
			XSI::ICEAttribute attr( attrs[i] );
			insert( attr.GetName() );
		}
	}

	/*! Return whether this set is match-all
	\return True if the set is match-all
	*/	
	bool IsMatchAll() const { return m_bMatchAll;}

	/*! Insert an attribute name
	\param in_csAttr The attribute name
	\return True if the name did not exist in the set before
	*/
	bool Insert( const XSI::CString& in_csAttr )
	{
		XSI::CString trimmed = in_csAttr;
		trimmed.TrimLeft( L" " );
		trimmed.TrimRight( L" " );
		if ( trimmed.Length() > 0 )
		{
			if ( in_csAttr.IsEqualNoCase(L"*") )
			{
				if ( !m_bMatchAll )
				{
					m_bMatchAll = true;
					return true;
				}
			}
			else
			{
				return insert( trimmed ).second;
			}
		}

		return false;
	}

	/*! Insert another AttributeNameSet into this set
	\param in_attrSet The other AttributeNameSet
	*/
	void Insert( const AttributeNameSet& in_attrSet )
	{
		insert( in_attrSet.begin(), in_attrSet.end() );
	}

	/*! Erase an attribute name from the set
	\param in_csAttr The attribute name
	\return True if the name was found
	*/	
	bool Erase( const XSI::CString& in_csAttr )
	{
		iterator it = find( in_csAttr );
		if ( it != end() )
		{
			erase( it );
			return true;
		}

		return false;
	}

	/*! Swap the content of this set with another
	\param in_attrSet The other AttributeNameSet
	*/
	void Swap( AttributeNameSet& in_attrSet )
	{
		std::swap( m_bMatchAll, in_attrSet.m_bMatchAll );
		swap( in_attrSet );
	}

	/*! Get the difference between this set and another. The difference are names which exists in this set but not in the other.
	\param in_otherSet The other set
	\param out_diff The difference set
	*/	
	void Difference( const AttributeNameSet& in_otherSet, AttributeNameSet& out_diff ) const
	{
		out_diff.clear();
		if ( IsMatchAll() || in_otherSet.IsMatchAll() )
		{
			return;
		}
		else
		{
			for ( const_iterator it=begin(); it!=end(); ++it )
			{
				if ( in_otherSet.Find(*it) == false )
				{
					out_diff.Insert(*it);
				}
			}
		}
	}

private:
	bool m_bMatchAll;
};

/*! \class AbcXsiExporter
	\brief The AbcXsiExporter class handles the process of creating the Alembic output objects from XSI objects.
	
	The exporter creates the hierarchy of Alembic objects in the OArchive, it then increments the global time 
	one frame at a time to cache the animation over time.

	If a shape-instancing PointCloud is exported, the instanced geometries will be exported along with it.

	ICE attributes are exported as Alembic properties. ICE attribute data types that do not have a matching
	type in Alembic are handled as following:
	- XSI::siICENodeDataRotation: exported into custom type AbcFramework::VRotationf
	- XSI::siICENodeDataVector4: exported into custom type AbcFramework::V4f
	- XSI::siICENodeDataShape: exported as a compound of 3 properties: shape type (uint8_t), shape instance id (uint32_t), shape instance hierarchy (bool_t)
	- XSI::siICENodeDataGeometry, XSI::siICENodeDataLocation: not exported
	Disclaimer: this can be changed in later releases to support better inter-op with other software.
	*/
class AbcXsiExporter : public AbcXsiIO
{
public:
	/*! Define the list of standard attributes on a PointCloud
	*/
	static const wchar_t* DEFAULT_ATTRIBUTES_POINTCLOUD;

	/*! Define the list of standard attributes on a Polymesh
	*/
	static const wchar_t* DEFAULT_ATTRIBUTES_POLYMESH;

	/*! Define the list of Polymesh attributes excluded from export
	*/
	static const wchar_t* EXCLUDED_ATTRIBUTES_POLYMESH;

	/*! Define the list of Pointcloud attributes excluded from export
	*/
	static const wchar_t* EXCLUDED_ATTRIBUTES_POINTCLOUD;

	/*! Constructor
	\param in_csFileName The full name of the Alembic archive file
	\param in_bIncludeParentHier True if the parent objects of the selected ones should be exported
	\param in_eArchiveType The archive format (HDF5 or Ogawa)
	\return 
	*/
	AbcXsiExporter( const XSI::CString& in_csFileName, bool in_bIncludeParentHier, EAbcArchiveType in_eArchiveType );

	/*! Destructor
	*/
	~AbcXsiExporter();

	/*! Export a list of objects
	\param in_objects The objects to be exported
	\param in_lStartFrame The start frame
	\param in_lEndFrame The end frame
	\return CStatus::OK Succeeded.
	*/	
	XSI::CStatus ExportObjects( const XSI::CRefArray& in_objects, LONG in_lStartFrame, LONG in_lEndFrame );

	/*! Add an XSI object to the list of animated items
	\param in_refXSIObject The XSI object
	\param in_pAbcOObj The Alembic object created from the XSI object
	\param in_pAttrs The attribute list to be cached
	*/	
	void AddAnimatedItem( ExportTreeNode* in_pNode );

	/*! Add an XSI Pointcloud to the list of geometry-instancing PointClouds
	\param in_refXSIObj The CRef to the XSI PointCloud
	\param in_pAbcOPoints The Alembic OPoints created from the XSI PointCloud
	\param in_pOProp The shape property that stores the shape instancing data
	*/	
	void AddGeomInstancingPtcloud( const XSI::CRef& in_refXSIObj, IAbcOPoints* in_pAbcOPoints, IAbcOParticleShapeProperty* in_pOProp );

	/*! Return the name of the Alembic archive file
	\return The full name of the Alembic archive
	*/	
	const XSI::CString& GetFileName() const		{ return m_csFileName;}

	/*! Return the name of the XSI scene
	\return The scene name
	*/	
	const XSI::CString& GetSceneName() const	{ return m_csSceneName;}

	/*! Return the start frame to be exported
	\return The start frame
	*/	
	LONG GetStartFrame() const					{ return m_lStartFrame;}

	/*! Return the end frame
	\return The end frame
	*/	
	LONG GetEndFrame() const					{ return m_lEndFrame;}

	/*! Return the start time in seconds
	\return The start time
	*/	
	double GetStartTime() const					{ return (double)m_lStartFrame/GetFrameRate();}

	/*! Return the scene playback frame-rate
		\return The scene playback frame rate 
	*/
	double GetFrameRate() const					{ return m_dFrameRate;}
	
	/*! Return the frame-rate at which the XSI object is exported. The frame-rate is calculated from the number of subframes of the object.
	\param in_refXSIObj The XSI object
	\return The frame-rate
	*/	
	double GetObjectFPS( const XSI::CRef& in_refXSIObj ) const;

	/*! Return the number of subframes at which the XSI object is sampled for exported. For XSI PointCloud object, this is a per-object setting defined in attribute SimulationSettings
	\param in_refXSIObj The XSI object
	\return The number of subframes
	*/	
	LONG GetObjectNbSubSamples( const XSI::CRef& in_refXSIObj ) const;

	/*! Return the AttributeNameSet which the user specifies for Polymesh
	\return The attribute name set
	*/
	const AttributeNameSet& GetUserSpecifiedAttributesPolymesh() const { return m_acsUserSpecifiedAttributes_Polymesh;}

	/*! Return the AttributeNameSet which the user specifies for Pointcloud
	\return The attribute name set
	*/
	const AttributeNameSet& GetUserSpecifiedAttributesPointcloud() const { return m_acsUserSpecifiedAttributes_Pointcloud;}

	/*! Return the AttributeNameSet which the user specifies for Pointcloud
	\return The attribute name set
	*/
	const AttributeNameSet& GetUserSpecifiedAttributesOtherPrim() const { return m_acsUserSpecifiedAttributes_OtherPrim;}

	/*! Return the AttributeNameSet which is excluded when exporting Pointcloud
	\return The attribute name set
	*/
	const AttributeNameSet& GetExcludedAttributesPointcloud() const { return m_acsExcludedAttributes_Pointcloud;}

	/*! Return the AttributeNameSet which is excluded when exporting Polymesh
	\return The attribute name set
	*/
	const AttributeNameSet& GetExcludedAttributesPolymesh() const { return m_acsExcludedAttributes_Polymesh;}

	/*! Set attributes to be exported
	\param in_csPolymeshAttrs The string of Polymesh attribute names entered by the user
	\param in_csPointcloudAttrs The string of Pointcloud attribute names entered by the user
	*/
	void SetUserSpecifiedAttributes( const XSI::CString& in_csPolymeshAttrs, const XSI::CString& in_csPointcloudAttrs, const XSI::CString& in_csOtherPrimAttrs );

	/*! Mark a set of attribute names as exported. Un-exported ones will be logged as a warning for the user.
	\param in_attrs The attribute names which are exported
	*/
	void MarkExportedAttributes( const AttributeNameSet& in_attrs );

	/*! Return the unexported attribute names
	\param out_attrs The AttributeNameSet where the unexported attribute names are stored
	*/
	void GetUnexportedAttributes( AttributeNameSet& out_attrs ) const;

private:
	// Export animation is done in one go by command CacheObjectsIntoFile
	XSI::CStatus ExportAnimation();
	XSI::CStatus ExpandBranchSelection( XSI::CRefArray& out_objs, const XSI::CRefArray& in_objs );
	XSI::CStatus AddNestedObjects( XSI::CRefArray& out_objs, const XSI::CRef& in_obj );
	XSI::CStatus ExportInstancedGeoms();
	XSI::CStatus RunCacheToFileCommand( XSI::CRefArray& in_objs, LONG in_lStartFrame, LONG in_lEndFrame, const XSI::CValueArray& in_attrs, const XSI::CString& in_csTargetCacheObjNames  );
	XSI::CStatus GetAllSceneObjects( XSI::CRefArray& out_objs );

	struct AnimatedItem
	{
		AnimatedItem() { }

		AnimatedItem( ExportTreeNode* in_pNode )
			: m_pExportTreeNode( in_pNode )
			, m_bExported( false )
		{
		}
		
		ExportTreeNode*			m_pExportTreeNode;
		bool					m_bExported;
	};
	typedef std::vector< AnimatedItem > AnimatedItemVec;
	typedef std::map< XSI::CRef, AnimatedItem > AnimatedItemMap;

	struct GeomInstancingPointcloundInfo
	{
		CAbcPtr<IAbcOPoints> m_spOPoints;
		std::vector< CAbcAdapt< CAbcPtr<IAbcOParticleShapeProperty> > > m_spShapeProps;

		GeomInstancingPointcloundInfo() {}
		GeomInstancingPointcloundInfo( IAbcOPoints* in_pOPoints ) : m_spOPoints( in_pOPoints ) {}
		GeomInstancingPointcloundInfo( IAbcOPoints* in_pOPoints, IAbcOParticleShapeProperty* in_pOProp ) 
			: m_spOPoints( in_pOPoints )
		{
			m_spShapeProps.push_back( CAbcPtr<IAbcOParticleShapeProperty>( in_pOProp ) );
		}
		
		GeomInstancingPointcloundInfo( const GeomInstancingPointcloundInfo& in_other )
			: m_spOPoints( in_other.m_spOPoints )
			, m_spShapeProps( in_other.m_spShapeProps )
		{}
	};
	typedef std::map< XSI::CRef, GeomInstancingPointcloundInfo > GeomInstancingPtcloudMap;

	CAbcPtr<IAbcOArchive>	m_spOArchive;
	AnimatedItemMap			m_AnimatedItems;
	GeomInstancingPtcloudMap m_OPointsWithGeomInstanced;

	CXsiProgressBar			m_ProgressBar;

	AttributeNameSet		m_acsUserSpecifiedAttributes_Polymesh;
	AttributeNameSet		m_acsUserSpecifiedAttributes_Pointcloud;
	AttributeNameSet		m_acsUserSpecifiedAttributes_OtherPrim;
	
	AttributeNameSet		m_acsExcludedAttributes_Polymesh;
	AttributeNameSet		m_acsExcludedAttributes_Pointcloud;
	
	AttributeNameSet		m_acsExportedAttributes;

	XSI::CString			m_csFileName;
	XSI::CString			m_csSceneName;

	LONG					m_lSceneInFrame;

	LONG					m_lStartFrame;
	LONG					m_lEndFrame;
	double					m_dFrameRate;

	ULONG					m_ulNbPolymeshesExported;
	ULONG					m_ulNbPointcloudsExported;
	ULONG					m_ulNbOtherPrimExported;

	bool					m_bExportParentHier;
};

#endif
