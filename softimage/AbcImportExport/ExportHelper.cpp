//*****************************************************************************
/*!
	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*

#include "ExportHelper.h"
#include <xsi_application.h>
#include <xsi_geometryaccessor.h>
#include <xsi_iceattributedataarray.h>
#include <xsi_iceattributedataarray2D.h>
#include <xsi_kinematics.h>
#include <xsi_kinematicstate.h>
#include <xsi_library.h>
#include <xsi_material.h>
#include <xsi_model.h>
#include <xsi_polygonmesh.h>
#include <xsi_primitive.h>
#include <xsi_project.h>
#include <xsi_scene.h>
#include <xsi_shader.h>
#include <xsi_shaderparameter.h>
#include <xsi_status.h>
#include <xsi_x3dobject.h>
#include <ctype.h>
#include <map>
#include <set>

XSI::CStatus ExportHelper::PrepareGeomData( FloatVector& out_vertexPos, LongVector& out_faceVertices, LongVector& out_faceVertexCount, const XSI::CGeometryAccessor& in_geomAccessor )
{
	// get the number of vertices for each polygons
    in_geomAccessor.GetPolygonVerticesCount(out_faceVertexCount);
	out_faceVertexCount.UpdatePtr();

    // get the vertex indices
    in_geomAccessor.GetVertexIndices(out_faceVertices);
	out_faceVertices.UpdatePtr();

    // get nb polygons
    LONG nPolyCount = in_geomAccessor.GetPolygonCount();

	DoubleVector l_vPos;
	in_geomAccessor.GetVertexPositions( l_vPos );
	l_vPos.UpdatePtr();

	// Alembic uses clockwise winding, we have to reverse the vertex order from GeomAccessor to match that
	ReverseFaceWinding<LONG>( out_faceVertices.GetArray(), out_faceVertices.GetCount(), out_faceVertexCount.GetArray(), out_faceVertexCount.GetCount() );

	// Convert the vertex positions from double to float
	LONG l_lVPosCount = l_vPos.GetCount();
	if ( l_lVPosCount )
	{
		out_vertexPos.Resize( l_lVPosCount );
		const double* l_pVPosSrc = &l_vPos[0];
		float* l_pVPosDest = &out_vertexPos[0];

		for (LONG i=l_lVPosCount; i>0; --i)
		{
			*(l_pVPosDest++) = (float)(*(l_pVPosSrc++));
		}
	}

	return XSI::CStatus::OK;
}

XSI::CStatus ExportHelper::PrepareUVData( 
	FloatVector& out_UVs, 
	LongVector& out_nodeIndices, 
	ClusterInfoVector& out_clusterInfoVec, 
	LongVector& in_faceVertexCount, 
	const XSI::CGeometryAccessor& in_geomAccessor, 
	const XSI::Geometry& in_geom )
{
	XSI::CRefArray l_materials;
	LongVector l_materialID;

	GetMaterialData( l_materials, l_materialID, out_clusterInfoVec, in_geomAccessor, in_geom );
	ULONG l_nbMaterials = l_materials.GetCount();

	XSI::CString l_csUVName;
	XSI::CRef l_refUV;

	FloatVector l_UVWsClusterPropData;
	XSI::CICEAttributeDataArrayVector3f l_UVWsICEAttrData;
	bool l_bUVIsClusterProp = false;
	bool l_bUVIsICEAttr = false;

	for (ULONG nMaterial = 0; nMaterial < l_nbMaterials && !(l_bUVIsClusterProp || l_bUVIsICEAttr); nMaterial++)
	{
		const XSI::Material l_material( l_materials[nMaterial] );
		
		XSI::Parameter l_tspaceidParam;
		int l_nbImageClip = l_material.GetImageClips().GetCount();

		if( l_nbImageClip == 0 )
			continue;

		if ( GetFirstTexSpaceIDParam( l_tspaceidParam, l_material ) == XSI::CStatus::OK )
		{
			// From the object material, we found an image shader
			if ( l_tspaceidParam.HasInstanceValue() )
			{
				XSI::Primitive l_primitive( in_geom.GetParent() );
				l_csUVName = l_tspaceidParam.GetInstanceValue( l_primitive.GetParent3DObject() );
			}
			else
				l_csUVName = l_tspaceidParam.GetValue().GetAsText();

			if ( l_csUVName == "*" )
			{
				// UV name is wildcard *
				// wildcard "*" is only used for ClusterProp UV
				XSI::CRefArray l_uvRefArray = in_geomAccessor.GetUVs();
				if ( l_uvRefArray.GetCount() > 0 )
				{
					l_refUV = l_uvRefArray[0];
					XSI::ClusterProperty l_clusterProp( l_refUV );
					l_csUVName = l_clusterProp.GetName();

					l_clusterProp.GetValues( l_UVWsClusterPropData );
					l_UVWsClusterPropData.UpdatePtr();

					l_bUVIsClusterProp = true;
				}
			}
			else
			{
				// UV name is literal
				l_refUV = in_geomAccessor.GetUV(l_csUVName);

				if ( l_refUV.IsValid() )
				{
					// UV is a cluster prop
					XSI::ClusterProperty l_clusterProp( l_refUV );

					l_clusterProp.GetValues( l_UVWsClusterPropData );
					l_UVWsClusterPropData.UpdatePtr();

					l_bUVIsClusterProp = true;
				}
				else
				{
					// UV is a per-Sample ICE attribute
					XSI::ICEAttribute l_iceAttrTextureProjection;
					l_iceAttrTextureProjection = in_geom.GetICEAttributeFromName(l_csUVName);

					if ( l_iceAttrTextureProjection.IsValid() && 
						(l_iceAttrTextureProjection.GetContextType() & XSI::siICENodeContextSingletonOrComponent0D2D) && 
						l_iceAttrTextureProjection.GetDataType() == XSI::siICENodeDataVector3 &&
						l_iceAttrTextureProjection.GetStructureType() == XSI::siICENodeStructureSingle )
					{
						l_iceAttrTextureProjection.GetDataArray( l_UVWsICEAttrData );
						l_bUVIsICEAttr = true;
					}
				}
			}
		}
	}

	// nguyenh 2013: the FBX exporter performs an optimization on the UV array to remove duplicate UV coordinates
	// This is useful for mesh with duplicates, e.g. Crowd, but incurs an overhead during the export

	if ( !l_bUVIsClusterProp && !l_bUVIsICEAttr )
	{
		// No image shader found in the object materials, use
		// the first cluster prop UV
		XSI::CRefArray l_uvRefArray = in_geomAccessor.GetUVs();
		if ( l_uvRefArray.GetCount() > 0 )
		{
			l_refUV = l_uvRefArray[0];
			XSI::ClusterProperty l_clusterProp( l_refUV );
			l_csUVName = l_clusterProp.GetName();

			l_clusterProp.GetValues( l_UVWsClusterPropData );
			l_UVWsClusterPropData.UpdatePtr();

			l_bUVIsClusterProp = true;
		}
	}

	if ( l_bUVIsClusterProp || l_bUVIsICEAttr )
	{
		PrepareNodeIndices( out_nodeIndices, in_faceVertexCount, in_geomAccessor );
	}

	if ( l_bUVIsClusterProp )
	{
		PackUVWtoUV( out_UVs, l_UVWsClusterPropData );
		return XSI::CStatus::OK;

	}
	else if ( l_bUVIsICEAttr )
	{
		PackUVWtoUV( out_UVs, l_UVWsICEAttrData );
		return XSI::CStatus::OK;
	}
	else
	{
		// refUV is invalid && uvName is empty		
		return XSI::CStatus::False;
	}
		


	return XSI::CStatus::OK;
}

XSI::CStatus ExportHelper::PrepareNodeIndices( LongVector& out_nodeIndices, const LongVector& in_faceVertexCount, const XSI::CGeometryAccessor& in_geomAccessor )
{
	// node array   : node0, node1, node2, ...
	// node indices : (poly0)node5, node6, node7, node8, (poly1)node0, node1, node2, (poly2)...
	// UV array     : node0.U, node0.V, node0.W, node1.U, node1.V, node1.W, ...

	// nguyenh 2013: tested in OM SDK, TurnInternalEdge doesn't seem to affect the order in node indices array.
	// So we'll leave the array as is
	in_geomAccessor.GetNodeIndices( out_nodeIndices );
	out_nodeIndices.UpdatePtr();
	ReverseFaceWinding<LONG>( out_nodeIndices.GetArray(), out_nodeIndices.GetCount(), in_faceVertexCount.GetArray(), in_faceVertexCount.GetCount() );

	return XSI::CStatus::OK;
}

XSI::CStatus ExportHelper::PrepareNormalData( FloatVector& out_Normals, LongVector& out_nodeIndices, LongVector& in_faceVertexCount, const XSI::CGeometryAccessor& in_geomAccessor, const XSI::Geometry& in_geom )
{
	bool l_bNormalIsClusterProp = false;
	bool l_bNormalIsICEAttr = false;

	// First, find out if there is defined ICE attribute "NodeUserNormal"
	XSI::ICEAttribute l_iceAttrNodeUserNormal = in_geom.GetICEAttributeFromName(L"NodeUserNormal");
	
	ULONG l_ulNodeUserNormalArraySize = 0;
	XSI::CICEAttributeDataArrayVector3f l_NodeUserNormalDataArray;

	if (l_iceAttrNodeUserNormal.IsDefined() && 
		(l_iceAttrNodeUserNormal.GetContextType() & XSI::siICENodeContextSingletonOrComponent0D2D) &&
		l_iceAttrNodeUserNormal.GetStructureType() == XSI::siICENodeStructureSingle &&
		l_iceAttrNodeUserNormal.GetDataType() == XSI::siICENodeDataVector3)
	{
		l_iceAttrNodeUserNormal.GetDataArray(l_NodeUserNormalDataArray);
		l_ulNodeUserNormalArraySize = l_NodeUserNormalDataArray.GetCount();
	}

	if (l_ulNodeUserNormalArraySize > 0 )
	{
		l_bNormalIsICEAttr = true;

		out_Normals.Resize( 3 * l_ulNodeUserNormalArraySize );

		if ( l_NodeUserNormalDataArray.IsConstant() )
		{
			float* l_pOutNormal = &out_Normals[0];

			const XSI::MATH::CVector3f l_vConstNormal = l_NodeUserNormalDataArray[0];
			for (ULONG i=0; i<l_ulNodeUserNormalArraySize; ++i)
			{
				*(l_pOutNormal++) = l_vConstNormal.GetX();
				*(l_pOutNormal++) = l_vConstNormal.GetY();
				*(l_pOutNormal++) = l_vConstNormal.GetZ();
			}
		}
		else
		{
			float* l_pOutNormal = &out_Normals[0];
			
			for (ULONG i=0; i<l_ulNodeUserNormalArraySize; ++i)
			{
				const XSI::MATH::CVector3f l_vNormal = l_NodeUserNormalDataArray[i];
				*(l_pOutNormal++) = l_vNormal.GetX();
				*(l_pOutNormal++) = l_vNormal.GetY();
				*(l_pOutNormal++) = l_vNormal.GetZ();
			}

		}

	}
	else
	{
		// ICE attribute is empty
		
		l_bNormalIsClusterProp = true;

		XSI::CRefArray l_refAllUserNormals = in_geomAccessor.GetUserNormals();

		if ( l_refAllUserNormals.GetCount() > 0 )
		{
			// Note: GetNodeNormals() is already including the UserNormal clusterProp
			// So we don't need to explicitly query the UserNormal
			XSI::CStatus status = in_geomAccessor.GetNodeNormals( out_Normals );
			out_Normals.UpdatePtr();

		}
	}

	if ( l_bNormalIsClusterProp || l_bNormalIsICEAttr )
	{
		PrepareNodeIndices( out_nodeIndices, in_faceVertexCount, in_geomAccessor );
	}

	return XSI::CStatus::False;
}

XSI::CStatus ExportHelper::GetFirstTexSpaceIDParam( XSI::Parameter& out_tspaceidParam, const XSI::Material& in_material )
{
	XSI::Parameter param = in_material.GetParameter("surface");
	if (param.IsValid())
	{
		XSI::ShaderParameter shaderParam(param.GetSource());
		if ( shaderParam.IsValid() )
		{
			XSI::Shader shader( shaderParam.GetParent() );
			return GetFirstTexSpaceIDParam( out_tspaceidParam, shader );
		}
	}

	return XSI::CStatus::False;
}

void ExportHelper::PackUVWtoUV( FloatVector& out_UVs, const XSI::CICEAttributeDataArrayVector3f& in_UVWArray )
{
	LONG l_lInUVWArraySize = in_UVWArray.GetCount();

	if ( l_lInUVWArraySize == 0 )
		return;

	out_UVs.Resize( 2*l_lInUVWArraySize );

	if ( in_UVWArray.IsConstant() )
	{
		float* l_pOutUV = &out_UVs[0];
		const XSI::MATH::CVector3f l_InVec3f = in_UVWArray[0];

		for (LONG i=l_lInUVWArraySize; i>0; --i)
		{
			*(l_pOutUV++) = l_InVec3f.GetX();
			*(l_pOutUV++) = l_InVec3f.GetY();
		}
	}
	else
	{
		float* l_pOutUV = &out_UVs[0];
		const XSI::MATH::CVector3f* l_pInVec3f = &in_UVWArray[0];

		for (LONG i=l_lInUVWArraySize; i>0; --i)
		{
			*(l_pOutUV++) = l_pInVec3f->GetX();
			*(l_pOutUV++) = l_pInVec3f->GetY();
			l_pInVec3f++;
		}
	}
}

void ExportHelper::PackUVWtoUV( FloatVector& out_UVs, const FloatVector& in_UVWArray )
{
	LONG l_lInUVWArraySize = in_UVWArray.GetCount();
	if ( l_lInUVWArraySize == 0 )
		return;

	out_UVs.Resize( 2*l_lInUVWArraySize/3 );
	
	const float* l_pInUV = &in_UVWArray[0];
	float* l_pOutUV = &out_UVs[0];

	for (LONG i=l_lInUVWArraySize/3; i>0; --i)
	{
		*(l_pOutUV++) = *(l_pInUV++);
		*(l_pOutUV++) = *(l_pInUV++);
		l_pInUV++;
	}
}

XSI::CStatus ExportHelper::GetFirstTexSpaceIDParam( XSI::Parameter& out_tspaceidParam, const XSI::Shader& in_shader )
{
	XSI::Parameter param = in_shader.GetParameter( "tspace_id" );
	if ( param.IsValid() )
	{
		out_tspaceidParam = param;
		return XSI::CStatus::OK;
	}

	const ULONG SHADER_PARAMS_COUNT = 3;
	static const char* SHADER_PARAMS[SHADER_PARAMS_COUNT] = {
		"diffuse",
		"ambient",
		"specular"
	};

	for (ULONG i=0; i<SHADER_PARAMS_COUNT; ++i)
	{
		XSI::Parameter param = in_shader.GetParameter( SHADER_PARAMS[i] );
		if ( param.IsValid() )
		{
			XSI::ShaderParameter shaderParam( param.GetSource() );
			if (shaderParam.IsValid())
			{
				XSI::Shader shader( shaderParam.GetParent() );
				if ( GetFirstTexSpaceIDParam( out_tspaceidParam, shader ) == XSI::CStatus::OK )
				{
					return XSI::CStatus::OK;
				}
			}
		}
	}

	return XSI::CStatus::False;
}

XSI::CStatus ExportHelper::GetMaterialData( 
	XSI::CRefArray& out_materials, 
	LongVector& out_materialID, 
	ClusterInfoVector& out_clusterInfoVec,
	const XSI::CGeometryAccessor& in_geomAccessor, 
	const XSI::Geometry& in_geom )
{
	// Create clusterInfo
	XSI::CRefArray l_clusters = in_geom.GetClusters();
	for ( ULONG i = 0; i < (ULONG)l_clusters.GetCount(); ++i )
	{
		XSI::Cluster l_cluster( l_clusters[i] );
		if ( l_cluster.GetType().IsEqualNoCase( XSI::siPolygonCluster ) )
		{
			XSI::CClusterElementArray l_elemArray = l_cluster.GetElements();
			ClusterInfo* l_pClusterInfo = new ClusterInfo( l_cluster.GetName(), l_elemArray.GetArray() );

			out_clusterInfoVec.push_back( l_pClusterInfo );
		}
	}

	XSI::ICEAttribute l_iceAttrMaterialID = in_geom.GetICEAttributeFromName(L"MaterialID");
	
	ULONG l_ulMaterialIDArraySize = 0;
	XSI::CICEAttributeDataArrayLong l_dataArrayMaterialID;

	if (l_iceAttrMaterialID.IsDefined() && l_iceAttrMaterialID.GetDataType() == XSI::siICENodeDataLong)
	{
		l_iceAttrMaterialID.GetDataArray(l_dataArrayMaterialID);
		l_ulMaterialIDArraySize = l_dataArrayMaterialID.GetCount();
	}

	if (l_ulMaterialIDArraySize > 0 )
	{
		// nguyenh 2013: ICE Materials list can overlap with the non-ICE one.
		// To make sure we only export unique materials, we first add the ICE Materials
		// to unique array, then add the non-ICE ones that are referenced by MaterialID
		CMaterialArray l_materialsToExport;
		ULONG l_nbICEMaterials = 0;

		// First, add to export list the materials specified in ICE attribute "Materials"
		{
			SceneMaterialFinder l_matFinder;
			l_matFinder.CollectICEMaterials( l_materialsToExport, in_geom );
			l_nbICEMaterials = l_materialsToExport.GetAllArray().GetCount();
		}
		
		// Secondly, add the non-ICE materials (on object/clusters)
		// nguyen 2013: We add all the non-ICE materials in, regardless of whether they are
		// assigned to any polygon.
		{
			const XSI::CRefArray l_nonICEMaterials = in_geomAccessor.GetMaterials();

			// This call guarantees that the material won't be added again if it already exists
			l_materialsToExport.Append( l_nonICEMaterials );
		}
		


		ClusterInfoVector l_clusterFromICE( l_nbICEMaterials );
		for ( ULONG i=0; i<l_nbICEMaterials; ++i )
		{
			XSI::Material l_material( l_materialsToExport.GetAllArray()[i] );
			l_clusterFromICE[i] = new ClusterInfo( l_material.GetName() );
		}

		// Lastly, iterate over MaterialID array
		// If materialID references a valid index in l_iceMaterials, it's kept
		// Else it's overridden by the non-ICE materialIndex
		LongVector l_polyMatIndicesNonICE;
		in_geomAccessor.GetPolygonMaterialIndices(l_polyMatIndicesNonICE);
		l_polyMatIndicesNonICE.UpdatePtr();

		out_materialID.Resize(l_ulMaterialIDArraySize);

		if ( l_dataArrayMaterialID.IsConstant() )
		{
			LONG l_lmatID = l_dataArrayMaterialID[0];
			
			if ( l_lmatID >= 1 && l_lmatID <= (LONG)l_nbICEMaterials )
			{
				// Valid MaterialID
				// ICE Material ID starts from 1, we shall subtract 1 to match it with Material Array
				LONG l_lExportedMatID;
				l_lExportedMatID = l_materialsToExport.GetIndexInUniqueArray( l_lmatID - 1 );

				XSI::CLongArray& l_polyArray = l_clusterFromICE[ l_lmatID - 1 ]->m_Elements;
				l_polyArray.Resize( l_ulMaterialIDArraySize );
				for ( ULONG i = 0; i < l_ulMaterialIDArraySize; ++i )
				{
					l_polyArray[i] = i;
				}

				for (ULONG i = 0; i<l_ulMaterialIDArraySize; i++)
					out_materialID[i] = l_lExportedMatID;
			}
			else
			{
				// Invalid MaterialID, resort to using object material or cluster material
				// Non-ICE materials are appended after ICE materials, so the index needs to be offset
				if ( l_nbICEMaterials > 0 )
				{
					for (ULONG i = 0; i<l_ulMaterialIDArraySize; i++)
						out_materialID[i] = l_materialsToExport.GetIndexInUniqueArray( l_nbICEMaterials + l_polyMatIndicesNonICE[i] );
				}
			}
		}
		else
		{
			for (ULONG i = 0; i<l_ulMaterialIDArraySize; i++)
			{
				LONG matID = l_dataArrayMaterialID[i];

				if ( matID >= 1 && matID <= (LONG)l_nbICEMaterials )
				{
					// Valid MaterialID
					// ICE Material ID starts from 1, we shall subtract 1 to match it with Material Array
					out_materialID[i] = l_materialsToExport.GetIndexInUniqueArray( matID - 1 );

					l_clusterFromICE[ matID - 1]->m_Elements.Add( i );
				}
				else
				{
					// Invalid MaterialID, resort to using object material or cluster material
					// Non-ICE materials are appended after ICE materials, so the index needs to be offset
					out_materialID[i] = l_materialsToExport.GetIndexInUniqueArray( l_nbICEMaterials + l_polyMatIndicesNonICE[i] );
				}
			}

		}
		out_materials += l_materialsToExport.GetUniqueArray();

		// Add ICE clusters to out_clusters
		for ( ULONG i = 0; i < l_clusterFromICE.size(); ++i )
		{
			if ( l_clusterFromICE[i]->m_Elements.GetCount() == 0 )
			{
				delete l_clusterFromICE[i];
			}
			else
			{
				out_clusterInfoVec.push_back( l_clusterFromICE[i] );
			}
		}

		l_clusterFromICE.clear();
	}
	else
	{
		// ICE attribute "Materials" is empty
		out_materials += in_geomAccessor.GetMaterials();
		
		in_geomAccessor.GetPolygonMaterialIndices( out_materialID );
		out_materialID.UpdatePtr();
	}

	return XSI::CStatus::OK;
}

void ExportHelper::CalculateLocalTransform( XSI::MATH::CTransformation& out_xformChildLocal, const XSI::MATH::CTransformation& in_xformChildGlobal, const XSI::MATH::CTransformation& in_xformParentGlobal )
{
	// We use SetMatrix4 on a clean CTransform so that it won't get the SIScaling flag
	XSI::MATH::CTransformation l_classicChildGlobal;
	XSI::MATH::CTransformation l_classicParentGlobal;

	XSI::MATH::CMatrix4 l_matTemp = in_xformChildGlobal.GetMatrix4();
	l_classicChildGlobal.SetMatrix4( l_matTemp );
	l_classicChildGlobal.SetScaling( XSI::MATH::CVector3( 1.0, 1.0, 1.0 ) );

	l_matTemp = in_xformParentGlobal.GetMatrix4();
	l_classicParentGlobal.SetMatrix4( l_matTemp );
	l_classicParentGlobal.SetScaling( XSI::MATH::CVector3( 1.0, 1.0, 1.0 ) );

	out_xformChildLocal = MapWorldPoseToObjectSpace( l_classicParentGlobal, l_classicChildGlobal );
}

SceneMaterialFinder::SceneMaterialFinder()
{
	XSI::Application l_app;
	m_refMaterialLibraries = l_app.GetActiveProject().GetActiveScene().GetMaterialLibraries();
}


XSI::CRef SceneMaterialFinder::Find( const XSI::CString& in_csName ) const
{
	ULONG l_lNbMaterialLibs = m_refMaterialLibraries.GetCount();
	for (ULONG l_idMaterialLibs=0; l_idMaterialLibs<l_lNbMaterialLibs; l_idMaterialLibs++)
	{
		const XSI::Library l_matLibrary = m_refMaterialLibraries[l_idMaterialLibs];
		const XSI::CRefArray l_refSceneMaterials = l_matLibrary.GetItems();

		LONG l_NbTotalMaterial = l_refSceneMaterials.GetCount();

		for (LONG l_idSceneMaterial=0; l_idSceneMaterial<l_NbTotalMaterial; l_idSceneMaterial++)
		{
			const XSI::Material l_sceneMaterial( l_refSceneMaterials.GetItem(l_idSceneMaterial) );
 
			// The name can be very flexible, all of these are acceptable names:
			// - Sources.Materials.DefaultLib.Scene_Material
			// - Materials.DefaultLib.Scene_Material
			// - DefaultLib.Scene_Material
			// - Scene_Material
			// - sphere.polymsh.cls.Cluster1.Material

			XSI::CString l_csSceneMaterialFullName = l_sceneMaterial.GetFullName();
			LONG l_lNameSearchStart = (LONG)l_csSceneMaterialFullName.Length() - (LONG)in_csName.Length();

			bool l_bNameFound = l_csSceneMaterialFullName.FindString( in_csName, l_lNameSearchStart ) == l_lNameSearchStart;
			bool l_bNameIsLast = l_lNameSearchStart == 0 || (l_lNameSearchStart>0 && l_csSceneMaterialFullName.GetAt(l_lNameSearchStart-1) == '.');

			if ( l_bNameFound && l_bNameIsLast )
			{
				return l_sceneMaterial.GetRef();
			}
		}
	}

	return XSI::CRef();
}

XSI::CStatus SceneMaterialFinder::CollectICEMaterials( CMaterialArray& out_materials, const XSI::Geometry& in_geom ) const
{
	XSI::ICEAttribute l_iceAttrMaterials = in_geom.GetICEAttributeFromName(L"Materials");
	assert(l_iceAttrMaterials.GetDataType()==XSI::siICENodeDataString);

	XSI::CICEAttributeDataArray2DString l_dataArray2DMaterials;
	l_iceAttrMaterials.GetDataArray2D(l_dataArray2DMaterials);

	// ICE Attribute "Materials" is a per-object 2D array, so it has only 1 subarray
	if ( l_dataArray2DMaterials.GetCount() > 0 )
	{
		XSI::CICEAttributeDataArrayString l_dataArrayMaterials;
		l_dataArray2DMaterials.GetSubArray(0, l_dataArrayMaterials);
		ULONG l_lNbICEAttributeMaterialElems = l_dataArrayMaterials.GetCount();

		for (ULONG l_idMaterial1D=0; l_idMaterial1D<l_lNbICEAttributeMaterialElems; l_idMaterial1D++ )
		{
			XSI::CRef l_sceneMaterial = Find( l_dataArrayMaterials[l_idMaterial1D] );
			if ( l_sceneMaterial.IsValid() )
				out_materials.Add( l_sceneMaterial );
		}
	}

	return XSI::CStatus::OK;
}

bool CaseInsensitiveLess::operator()( const XSI::CString& x, const XSI::CString& y ) const
{
	ULONG ulXlen = x.Length();
	ULONG ulYlen = y.Length();

	bool bXShorter;
	ULONG ulMinLen;

	if ( ulXlen < ulYlen )
	{
		bXShorter = true;
		ulMinLen = ulXlen;
	}
	else
	{
		bXShorter = false;
		ulMinLen = ulYlen;
	}

	for ( ULONG i=0; i < ulMinLen; ++i )
	{
		wchar_t xi, yi;
		xi = toupper(x[i]);
		yi = toupper(y[i]);

		if ( xi == yi )
			continue;
		else if ( xi < yi )
			return true;
		else
			return false;
	}

	if ( bXShorter )
		return true;

	return false;
}
