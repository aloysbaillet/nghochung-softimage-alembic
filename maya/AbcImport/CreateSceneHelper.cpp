//-*****************************************************************************
//
// Copyright (c) 2009-2013,
//  Sony Pictures Imageworks, Inc. and
//  Industrial Light & Magic, a division of Lucasfilm Entertainment Company Ltd.
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// *       Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// *       Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
// *       Neither the name of Sony Pictures Imageworks, nor
// Industrial Light & Magic nor the names of their contributors may be used
// to endorse or promote products derived from this software without specific
// prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//-*****************************************************************************

#include "util.h"
#include "CameraHelper.h"
#include "LocatorHelper.h"
#include "MeshHelper.h"
#include "NurbsCurveHelper.h"
#include "NurbsSurfaceHelper.h"
#include "PointHelper.h"
#include "XformHelper.h"
#include "CreateSceneHelper.h"

#include <Alembic/AbcGeom/Visibility.h>

#include <maya/MString.h>
#include <maya/MStringArray.h>
#include <maya/MIntArray.h>
#include <maya/MDoubleArray.h>
#include <maya/MPlug.h>
#include <maya/MDGModifier.h>
#include <maya/MFnCamera.h>
#include <maya/MFnDoubleArrayData.h>
#include <maya/MFnDagNode.h>
#include <maya/MFnIntArrayData.h>
#include <maya/MFnStringData.h>
#include <maya/MFnTransform.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnNurbsCurve.h>
#include <maya/MFnNurbsSurface.h>
#include <maya/MFnSet.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnSingleIndexedComponent.h>
#include <maya/MItSelectionList.h>
#include <maya/MItDependencyGraph.h>

#include <map>
#include <set>
#include <string>
#include <vector>


namespace CreateSceneHelper
{
    void copyIndicesToNode(MIntArray& iIndices, const MObject& iNode,
                           MObject& iSet)
    {
        MStatus status;

        MFnDagNode mFnNode(iNode);

        MDagPath dpShape;
        status = mFnNode.getPath(dpShape);

        // Empty the set
        MFnSet         fnSet( iSet );
        MSelectionList selList;
        fnSet.getMembers   ( selList, false );
        MItSelectionList iterSelList( selList );
        for( ; iterSelList.isDone()!=true; iterSelList.next() )
        {
            MDagPath curDag;
            MObject  curCompObj;
            iterSelList.getDagPath(curDag, curCompObj);

            if ((curDag==dpShape) && curCompObj.isNull())
                fnSet.removeMember(curDag, curCompObj);
        }

        // Feed the indexed component
        MFnSingleIndexedComponent fnSComp;
        MObject comp = fnSComp.create( MFn::kMeshPolygonComponent );
        fnSComp.addElements( iIndices );

        // Remove the elements from all other sets (exclusivity constraint)
        MObjectArray   connSGObjs( getOutConnectedSG(dpShape) );
        MObject	   connSGObj;
        MFnSet	   fnOtherSet;
        MSelectionList setList, otherSetList, xorList;
        setList.add( dpShape, comp );
        int nSG = connSGObjs.length();
        for( int i=0; i<nSG; i++ )
        {
            // Get the current SG Object
            connSGObj = connSGObjs[i];
            if( connSGObj != iSet )
            {
                // Build the intersection list and remove it from the set
                otherSetList.clear();
                fnOtherSet.setObject( connSGObj );
                fnOtherSet.getMembers( otherSetList, false );

                // test if it's a full partition
                if (otherSetList.length()>=1)
                {
                    MItSelectionList itSelList( otherSetList );
                    for( ; itSelList.isDone()!=true; itSelList.next() )
                    {
                        MDagPath dp;
                        MObject  compObj;
                        itSelList.getDagPath(dp, compObj);

                        if (!(dp==dpShape) || !compObj.isNull())
                            continue;

                        fnOtherSet.removeMember( dp, compObj );

                        // create a component with the full list
                        MFnMesh fnMesh(dpShape);
                        MFnSingleIndexedComponent fnFullSComp;
                        compObj = fnFullSComp.create(
                                        MFn::kMeshPolygonComponent );
                        fnFullSComp.setCompleteData(fnMesh.numPolygons());

                        // fill the sel list
                        otherSetList.clear();
                        otherSetList.add( dpShape, compObj );
                        break;
                    }
                }

                xorList = otherSetList;
                xorList.merge( setList, MSelectionList::kXORWithList );
                otherSetList.merge( xorList, MSelectionList::kRemoveFromList);
                if( !otherSetList.isEmpty() )
                    fnOtherSet.removeMembers( otherSetList );
            }
        }

        // Feed the set
        fnSet.addMember(dpShape, comp);
    }

    void addFaceSets(MObject & iNode, Alembic::Abc::IObject & iObj)
    {
        MStatus status;

        MFnDependencyNode mesh(iNode, &status);

        if (status != MS::kSuccess)
            return;

        std::size_t numChildren = iObj.getNumChildren();
        for ( std::size_t i = 0 ; i < numChildren; ++i )
        {
            Alembic::Abc::IObject child = iObj.getChild(i);
            if (Alembic::AbcGeom::IFaceSet::matches(child.getHeader()))
            {
                Alembic::AbcGeom::IFaceSet faceSet(child,
                    Alembic::Abc::kWrapExisting);

                // create a shading group for this faceset.
                MObject shadingGroup;
                const MString& faceSetName(faceSet.getName().c_str());

                // check if this SG already exists.
                status = getObjectByName(faceSetName, shadingGroup);

                // retrive face indices.
                Alembic::AbcGeom::IFaceSetSchema::Sample samp;
                faceSet.getSchema().get(samp);

                MFnIntArrayData fnData;
                const int* faceArray((int *)samp.getFaces()->getData());
                const size_t size = samp.getFaces()->size();

                MIntArray arr(faceArray, static_cast<unsigned int>(size));
                
                // Use sets command to create the shadingGroup
                {
                    // Feed the indexed component 
                    MFnSingleIndexedComponent fnSComp;
                    MObject comp = fnSComp.create( MFn::kMeshPolygonComponent );
                    fnSComp.addElements( arr );

                    MSelectionList setList;

                    MFnDagNode mFnNode(iNode);

                    MDagPath dpShape;
                    status = mFnNode.getPath(dpShape);

                    setList.add( dpShape, comp );

                    status = MGlobal::setActiveSelectionList( setList );

                    if (shadingGroup.isNull())
                    {
                        MString setCmd = "sets -r true -n \"";
                        setCmd += faceSetName;
                        setCmd += "\"";
                        status = MGlobal::executeCommand( setCmd, true );

                        status = getObjectByName(faceSetName, shadingGroup);
                    }
                    else
                    {
                        MString setCmd = "sets -add \"";
                        setCmd += faceSetName;
                        setCmd += "\"";
                        status = MGlobal::executeCommand( setCmd, true );
                    }
                }
                Alembic::Abc::ICompoundProperty arbProp =
                    faceSet.getSchema().getArbGeomParams();

                Alembic::Abc::ICompoundProperty userProp =
                    faceSet.getSchema().getUserProperties();

                addProps(arbProp, shadingGroup, false);
                addProps(userProp, shadingGroup, false);
            }
        }
    }

    void removeDagNode(MDagPath & dagPath)
    {

        MStatus status = deleteDagNode(dagPath);
        if ( status != MS::kSuccess )
        {
            MString theError = dagPath.partialPathName();
            theError += MString(" removal not successful");
            printError(theError);
        }
    }

    Alembic::Abc::IScalarProperty getVisible(Alembic::Abc::IObject & iNode,
        bool isObjConstant,
        std::vector<Prop> & oPropList,
        std::vector<Alembic::AbcGeom::IObject> & oAnimVisStaticObj)
    {
        Alembic::AbcGeom::IVisibilityProperty visProp =
            Alembic::AbcGeom::GetVisibilityProperty(iNode);

        if (visProp && !visProp.isConstant())
        {
            Prop prop;
            prop.mScalar = visProp;
            oPropList.push_back(prop);
            if (isObjConstant)
            {
                oAnimVisStaticObj.push_back(iNode);
            }
        }
        return visProp;
    }

    void setConstantVisibility(Alembic::Abc::IScalarProperty iVisProp,
        MObject & iParent)
    {
        if (iVisProp.valid() && iVisProp.isConstant())
        {
            Alembic::Util::int8_t visVal;
            iVisProp.get(&visVal);
            MFnDependencyNode dep(iParent);
            MPlug plug = dep.findPlug("visibility");
            if (!plug.isNull())
            {
                plug.setBool(visVal != 0);
            }
        }
    }

    bool matchesNameWithRegex (const MString& iName,
                               const MStringArray & iPatterns)
    {
        unsigned int length = iPatterns.length();
        if (length == 0)
            return true;

        for (unsigned int i=0; i<length; ++i)
        {
            // use 'match' function provided in the maya mel script.
            MString scriptStr, result;
            scriptStr.format("match \"^1s\" \"^2s\";", iPatterns[i], iName);
            MGlobal::executeCommand(scriptStr, result);

            // found a match!
            if (result.length() > 0)
            {
                return true;
            }
        }

        return false;
    }
}

using namespace CreateSceneHelper;

CreateSceneVisitor::CreateSceneVisitor(double iFrame,
    bool iUnmarkedFaceVaryingColors, const MObject & iParent,
    Action iAction, MString iRootNodes,
    MString iIncludeFilterString, MString iExcludeFilterString) :
    mFrame(iFrame), mParent(iParent),
    mUnmarkedFaceVaryingColors(iUnmarkedFaceVaryingColors), mAction(iAction)
{
    mAnyRoots = false;

    // parse the input string to extract the nodes that need (re)connection
    if (iRootNodes != MString() && iRootNodes != MString("/"))
    {
        MStringArray theArray;
        if (iRootNodes.split(' ', theArray) == MS::kSuccess)
        {
            unsigned int len = theArray.length();
            for (unsigned int i = 0; i < len; i++)
            {
                MString name = theArray[i];
                // the name could be either a partial path or a full path
                MDagPath dagPath;
                if ( getDagPathByName( name, dagPath ) == MS::kSuccess )
                {
                    name = dagPath.partialPathName();
                }

                mRootNodes.insert(name.asChar());
                mAnyRoots = true;
            }
        }
    }
    else if (iRootNodes == MString("/"))
    {
        mAnyRoots = true;
    }

    mOnlyPatterns.clear();
    if (iIncludeFilterString != MString() &&
        iIncludeFilterString != MString("*"))
    {
        iIncludeFilterString.split(' ', mOnlyPatterns);
    }

    mExceptPatterns.clear();
    if (iExcludeFilterString != MString() &&
        iExcludeFilterString != MString("*"))
    {
        iExcludeFilterString.split(' ', mExceptPatterns);
    }
}

CreateSceneVisitor::~CreateSceneVisitor()
{
}

void CreateSceneVisitor::getData(WriterData & oData)
{
    oData = mData;
}

bool CreateSceneVisitor::hasSampledData()
{

    // Currently there's no support for bringing in particle system simulation
    return (mData.mPropList.size() > 0
        || mData.mXformList.size() > 0
        || mData.mSubDList.size() > 0
        || mData.mPolyMeshList.size() > 0
        || mData.mCameraList.size() > 0
        || mData.mNurbsList.size() > 0
        || mData.mCurvesList.size() > 0
        || mData.mLocList.size() > 0);
}

// re-add the selection back to the sets
void CreateSceneVisitor::applyShaderSelection()
{
    std::map < MObject, MSelectionList, ltMObj >::iterator i =
        mShaderMeshMap.begin();

    std::map < MObject, MSelectionList, ltMObj >::iterator end =
        mShaderMeshMap.end();

    for (; i != end; ++i)
    {
        MFnSet curSet(i->first);
        curSet.addMembers(i->second);
    }
    mShaderMeshMap.clear();
}

void CreateSceneVisitor::addToPropList(std::size_t iFirst, MObject & iObject)
{
    std::size_t last = mData.mPropList.size();
    std::vector<std::string> attrList;
    for (std::size_t i = iFirst; i < last; ++i)
    {
        if (mData.mPropList[i].mArray.valid())
        {
            attrList.push_back(mData.mPropList[i].mArray.getName());
        }
        else
        {
            attrList.push_back(mData.mPropList[i].mScalar.getName());
        }
    }
    mData.mPropObjList.push_back(SampledPair(iObject, attrList));
}

// remembers what sets a mesh was part of, gets those sets as a selection
// and then clears the sets for reassignment later  this is only used when
// hooking up an Alembic node to a previous hierarchy (swapping)
void CreateSceneVisitor::checkShaderSelection(MFnMesh & iMesh,
    unsigned int iInst)
{
    MObjectArray sets;
    MObjectArray comps;

    iMesh.getConnectedSetsAndMembers(iInst, sets, comps, false);
    unsigned int setsLength = sets.length();
    for (unsigned int i = 0; i < setsLength; ++i)
    {
        MObject & curSetObj = sets[i];
        if (mShaderMeshMap.find(curSetObj) == mShaderMeshMap.end())
        {
            MFnSet curSet(curSetObj);
            MSelectionList & curSel = mShaderMeshMap[curSetObj];
            curSet.getMembers(curSel, true);

            // clear before hand so that when we add the selection to the
            // set later, it will take.  This is to get around a problem where
            // shaders are assigned per face dont shade correctly
            curSet.clear();
        }
    }
}

void CreateSceneVisitor::visit(AlembicObjectPtr iObject)
{
    Alembic::Abc::IObject iObj = iObject->object();

    if ( Alembic::AbcGeom::IXform::matches(iObj.getHeader()) )
    {
        Alembic::AbcGeom::IXform xform(iObj, Alembic::Abc::kWrapExisting);
        (*this)(xform, iObject);
    }
    else if ( Alembic::AbcGeom::ISubD::matches(iObj.getHeader()) )
    {
        Alembic::AbcGeom::ISubD mesh(iObj, Alembic::Abc::kWrapExisting);
        (*this)(mesh);
    }
    else if ( Alembic::AbcGeom::IPolyMesh::matches(iObj.getHeader()) )
    {
        Alembic::AbcGeom::IPolyMesh mesh(iObj, Alembic::Abc::kWrapExisting);
        (*this)(mesh);
    }
    else if ( Alembic::AbcGeom::ICamera::matches(iObj.getHeader()) )
    {
        Alembic::AbcGeom::ICamera cam(iObj, Alembic::Abc::kWrapExisting);
        (*this)(cam);
    }
    else if ( Alembic::AbcGeom::ICurves::matches(iObj.getHeader()) )
    {
        Alembic::AbcGeom::ICurves curves(iObj, Alembic::Abc::kWrapExisting);
        (*this)(curves);
    }
    else if ( Alembic::AbcGeom::INuPatch::matches(iObj.getHeader()) )
    {
        Alembic::AbcGeom::INuPatch nurbs(iObj, Alembic::Abc::kWrapExisting);
        (*this)(nurbs);
    }
    else if ( Alembic::AbcGeom::IPoints::matches(iObj.getHeader()) )
    {
        Alembic::AbcGeom::IPoints pts(iObj, Alembic::Abc::kWrapExisting);
        (*this)(pts);
    }
    else if ( iObj.getHeader().getMetaData().get("schema") == "" )
    {
        createEmptyObject(iObject);
    }
    else
    {
        MString theWarning(iObj.getName().c_str());
        theWarning += " is an unsupported schema, skipping: ";
        theWarning += iObj.getMetaData().get("schema").c_str();
        printWarning(theWarning);
    }
}

AlembicObjectPtr CreateSceneVisitor::previsit(AlembicObjectPtr iParentObject)
{
    Alembic::Abc::IObject parent = iParentObject->object();
    const MString name = parent.getFullName().c_str();
    const size_t numChildren = parent.getNumChildren();

    // Apply exclude filters first as a preorder traversal.
    if (mExceptPatterns.length() > 0 &&
        matchesNameWithRegex(name, mExceptPatterns))
    {
        return AlembicObjectPtr();
    }

    for (size_t i = 0; i < numChildren; ++i)
    {
        Alembic::Abc::IObject child = parent.getChild(i);
        AlembicObjectPtr childObject =
            previsit(AlembicObjectPtr(new AlembicObject(child)));

        if (childObject)
        {
            iParentObject->addChild(childObject);
        }
    }

    // We traverse a tree in postorder. The invarient is that iParentObject
    // will have no child unless any descendent of it has the matching name.
    if (iParentObject->getNumChildren() == 0)
    {
        if (!matchesNameWithRegex(name, mOnlyPatterns))
        {
            return AlembicObjectPtr();
        }
    }

    return iParentObject;
}

 // root of file, no creation of DG node
MStatus CreateSceneVisitor::walk(Alembic::Abc::IArchive & iRoot)
{
    MStatus status = MS::kSuccess;

    MObject saveParent = mParent;

    if (!iRoot.valid()) return MS::kFailure;

    // preload the cache hierarchy with an optional filtering.
    AlembicObjectPtr topObject =
        previsit(AlembicObjectPtr(new AlembicObject(iRoot.getTop())));

    if (!topObject) return status;

    size_t numChildren = topObject->getNumChildren();

    if (numChildren == 0) return status;

    if (mAction == NONE)  // simple scene creation mode
    {
        for (size_t i = 0; i < numChildren; i++)
        {
            this->visit(topObject->getChild(i));
            mParent = saveParent;
        }
        return status;
    }

    // doing connections
    std::set<std::string> connectUpdateNodes;
    std::set<std::string> connectCurNodesInFile;

    std::set<std::string>::iterator fileEnd =
        connectCurNodesInFile.end();
    for (size_t i = 0; i < numChildren; i++)
    {
        AlembicObjectPtr object = topObject->getChild(i);
        std::string name = object->object().getName();
        connectCurNodesInFile.insert(name);

        // see if this name is part of the input to AlembicNode
        if (!mAnyRoots || ( mAnyRoots && (mRootNodes.empty() ||
          (mRootNodes.find(name) != mRootNodes.end())) ))
        {
            // Find out if this node exists in the current scene
            MDagPath dagPath;

            if (mAnyRoots &&
                getDagPathByName(MString(name.c_str()), dagPath) ==
                MS::kSuccess)
            {
                connectUpdateNodes.insert(name);
                mConnectDagNode = dagPath;
                mConnectDagNode.pop();
                this->visit(object);
                mParent = saveParent;
            }
            else if (mAction != CREATE && mAction != CREATE_REMOVE)
            {
                MString theWarning("Could not find: ");
                theWarning += name.c_str();
                theWarning += " in the scene.";
                theWarning += "  Skipping it and all descendants.";
                printWarning(theWarning);
            }
            else
            {
                mConnectDagNode = MDagPath();
                connectUpdateNodes.insert(name);
                this->visit(object);
                mParent = saveParent;

            }
        }
    }  // for-loop

    if (mRootNodes.size() > connectUpdateNodes.size() &&
        (mAction == REMOVE || mAction == CREATE_REMOVE))
    {
        std::set<std::string>::iterator iter =
            mRootNodes.begin();
        const std::set<std::string>::iterator fileEndIter =
            connectCurNodesInFile.end();
        MDagPath dagPath;
        for ( ; iter != mRootNodes.end(); iter++)
        {
            std::string name = *iter;
            bool existInFile =
                (connectCurNodesInFile.find(name) != fileEndIter);
            bool existInScene =
                (getDagPathByName(MString(name.c_str()), dagPath)
                    == MS::kSuccess);
            if (existInScene && !existInFile)
            {
                removeDagNode(dagPath);
            }
            else if (!existInScene && !existInFile)
            {
                MString theWarning(name.c_str());
                theWarning +=
                    " exists neither in file nor in the scene";
                printWarning(theWarning);
            }
        }
    }

    return status;
}

MStatus CreateSceneVisitor::operator()(Alembic::AbcGeom::ICamera & iNode)
{
    MStatus status = MS::kSuccess;
    MObject cameraObj = MObject::kNullObj;

    bool isConstant = iNode.getSchema().isConstant();

    // add animated camera to the list
    if (!isConstant)
    {
        mData.mCameraList.push_back(iNode);
    }

    Alembic::Abc::ICompoundProperty arbProp =
        iNode.getSchema().getArbGeomParams();

    Alembic::Abc::ICompoundProperty userProp =
        iNode.getSchema().getUserProperties();

    std::size_t firstProp = mData.mPropList.size();
    getAnimatedProps(arbProp, mData.mPropList, false);
    getAnimatedProps(userProp, mData.mPropList, false);
    Alembic::Abc::IScalarProperty visProp = getVisible(iNode, isConstant,
        mData.mPropList, mData.mAnimVisStaticObjList);

    bool hasDag = false;
    if (mAction != NONE && mConnectDagNode.isValid())
    {
        hasDag = getDagPathByChildName(mConnectDagNode, iNode.getName());
        if (hasDag)
        {
            cameraObj = mConnectDagNode.node();
            if (!isConstant)
            {
                mData.mCameraObjList.push_back(cameraObj);
            }
        }
    }

    if (mAction == CREATE || mAction == CREATE_REMOVE)
    {
        cameraObj = create(iNode, mParent);
        if (!isConstant)
        {
            mData.mCameraObjList.push_back(cameraObj);
        }
    }

    if (cameraObj != MObject::kNullObj)
    {
        setConstantVisibility(visProp, cameraObj);
        addProps(arbProp, cameraObj, false);
        addProps(userProp, cameraObj, false);
    }

    if ( mAction >= CONNECT )
    {
        MFnCamera fn(cameraObj, &status);

        // check that the data types are compatible, they might not be
        // if we have a weird hierarchy, where the node in the scene
        // differs from the node on disk
        if ( status != MS::kSuccess )
        {
            MString theError("No connection done for node '");
            theError += MString(iNode.getName().c_str());
            theError += MString("' with ");
            theError += mConnectDagNode.fullPathName();
            printError(theError);
            return status;
        }

        addToPropList(firstProp, cameraObj);
    }

    return status;
}

MStatus CreateSceneVisitor::operator()(Alembic::AbcGeom::ICurves & iNode)
{
    MStatus status = MS::kSuccess;
    MObject curvesObj = MObject::kNullObj;

    bool isConstant = iNode.getSchema().isConstant();

    // read sample 0 to determine and use it to set the number of total
    // curves.  We can't support changing the number of curves over time.
    Alembic::AbcGeom::ICurvesSchema::Sample samp;
    iNode.getSchema().get(samp);

    Alembic::Abc::ICompoundProperty arbProp =
        iNode.getSchema().getArbGeomParams();

    Alembic::Abc::ICompoundProperty userProp =
        iNode.getSchema().getUserProperties();

    Alembic::AbcGeom::IFloatGeomParam::Sample widthSamp;
    if (iNode.getSchema().getWidthsParam())
    {
        iNode.getSchema().getWidthsParam().getExpanded(widthSamp);
    }
    std::size_t numCurves = samp.getNumCurves();

    if (numCurves == 0)
    {
        MString theWarning(iNode.getName().c_str());
        theWarning += " has no curves, skipping.";
        printWarning(theWarning);
        return MS::kFailure;
    }
    // add animated curves to the list
    else if (!isConstant)
    {
        mData.mNumCurves.push_back(numCurves);
        mData.mCurvesList.push_back(iNode);
    }

    std::size_t firstProp = mData.mPropList.size();
    getAnimatedProps(arbProp, mData.mPropList, false);
    getAnimatedProps(userProp, mData.mPropList, false);
    Alembic::Abc::IScalarProperty visProp = getVisible(iNode, isConstant,
        mData.mPropList, mData.mAnimVisStaticObjList);

    bool hasDag = false;
    if (mAction != NONE && mConnectDagNode.isValid())
    {
        hasDag = getDagPathByChildName(mConnectDagNode, iNode.getName());
        if (hasDag)
        {
            curvesObj = mConnectDagNode.node();
            if (!isConstant)
            {
                if (numCurves == 1)
                {
                    mData.mNurbsCurveObjList.push_back(curvesObj);
                }
                else
                {
                    unsigned int childCurves = mConnectDagNode.childCount();

                    for (unsigned int i = 0; i < numCurves; ++i)
                    {
                        if (i < childCurves)
                        {
                            mData.mNurbsCurveObjList.push_back(
                                mConnectDagNode.child(i));
                        }
                        else
                        {
                            // push a null object since we have more Alembic
                            // curves than we have Maya curves
                            MObject obj;
                            mData.mNurbsCurveObjList.push_back(obj);
                        }
                    }
                } // else
            }
        } // if hasDag
    }

    if (!hasDag && (mAction == CREATE || mAction == CREATE_REMOVE))
    {
        curvesObj = createCurves(iNode.getName(), samp, widthSamp, mParent,
            mData.mNurbsCurveObjList, !isConstant);
    }

    if (curvesObj != MObject::kNullObj)
    {
        setConstantVisibility(visProp, curvesObj);
        addProps(arbProp, curvesObj, false);
        addProps(userProp, curvesObj, false);
    }


    if (mAction >= CONNECT)
    {

        MFnNurbsCurve fncurve(curvesObj, &status);

        // not a single curve, try the transform for a group of curves
        if (status != MS::kSuccess)
        {
            MFnTransform fntrans(curvesObj, &status);
        }

        // check that the data types are compatible, they might not be
        // if we have a weird hierarchy, where the node in the scene
        // differs from the node on disk
        if (status != MS::kSuccess)
        {
            MString theError("No connection done for node '");
            theError += MString(iNode.getName().c_str());
            theError += MString("' with ");
            theError += mConnectDagNode.fullPathName();
            printError(theError);
            return status;
        }

        if (!fncurve.object().isNull())
        {
            MPlug dstPlug = fncurve.findPlug("create");
            disconnectAllPlugsTo(dstPlug);
            disconnectProps(fncurve, mData.mPropList, firstProp);
            addToPropList(firstProp, curvesObj);
        }
    }

    if (hasDag)
    {
        mConnectDagNode.pop();
    }

    return status;
}

MStatus CreateSceneVisitor::operator()(Alembic::AbcGeom::IPoints& iNode)
{
    MStatus status = MS::kSuccess;
    MObject particleObj = MObject::kNullObj;

    bool isConstant = iNode.getSchema().isConstant();
    if (!isConstant)
        mData.mPointsList.push_back(iNode);

    // since we don't really support animated points, don't bother
    // with the animated properties on it

    bool hasDag = false;
    if (mAction != NONE && mConnectDagNode.isValid())
    {
        hasDag = getDagPathByChildName(mConnectDagNode, iNode.getName());
        if (hasDag)
        {
            particleObj = mConnectDagNode.node();
        }
    }

    if (!hasDag && (mAction == CREATE || mAction == CREATE_REMOVE))
    {

        status = create(mFrame, iNode, mParent, particleObj);
        if (!isConstant)
        {
            mData.mPointsObjList.push_back(particleObj);
        }
    }

    // don't currently care about anything animated on a particleObj
    std::vector<Prop> fakePropList;
    std::vector<Alembic::AbcGeom::IObject> fakeObjList;

    if (particleObj != MObject::kNullObj)
    {
        Alembic::Abc::IScalarProperty visProp =
            getVisible(iNode, false, fakePropList, fakeObjList);

        setConstantVisibility(visProp, particleObj);

        Alembic::Abc::ICompoundProperty arbProp =
            iNode.getSchema().getArbGeomParams();
        Alembic::Abc::ICompoundProperty userProp =
            iNode.getSchema().getUserProperties();

        addProps(arbProp, particleObj, false);
        addProps(userProp, particleObj, false);
    }

    if (hasDag)
    {
        mConnectDagNode.pop();
    }

    return status;
}

MStatus CreateSceneVisitor::operator()(Alembic::AbcGeom::ISubD& iNode)
{
    MStatus status = MS::kSuccess;
    MObject subDObj = MObject::kNullObj;

    SubDAndColors subdColors;
    subdColors.mMesh = iNode;

    Alembic::Abc::ICompoundProperty arbProp =
        iNode.getSchema().getArbGeomParams();

    Alembic::Abc::ICompoundProperty userProp =
        iNode.getSchema().getUserProperties();

    bool colorAnim = getColorAttrs(arbProp, subdColors.mC3s,
        subdColors.mC4s, mUnmarkedFaceVaryingColors);

    bool isConstant = iNode.getSchema().isConstant();

    // add animated SubDs to the list
    if (!isConstant || colorAnim)
    {
        mData.mSubDList.push_back(subdColors);
    }

    std::size_t firstProp = mData.mPropList.size();

    getAnimatedProps(arbProp, mData.mPropList, mUnmarkedFaceVaryingColors);
    getAnimatedProps(userProp, mData.mPropList, mUnmarkedFaceVaryingColors);
    Alembic::Abc::IScalarProperty visProp = getVisible(iNode, isConstant,
        mData.mPropList, mData.mAnimVisStaticObjList);

    bool hasDag = false;
    if (mAction != NONE && mConnectDagNode.isValid())
    {
        hasDag = getDagPathByChildName(mConnectDagNode, iNode.getName());
        if (hasDag)
        {
            subDObj = mConnectDagNode.node();
            if (!isConstant || colorAnim)
            {
                mData.mSubDObjList.push_back(subDObj);
            }
        }
    }

    if (!hasDag && (mAction == CREATE || mAction == CREATE_REMOVE))
    {
        subDObj = createSubD(mFrame, subdColors, mParent);
        if (!isConstant || colorAnim)
        {
            mData.mSubDObjList.push_back(subDObj);
        }
    }

    if (subDObj != MObject::kNullObj)
    {
        setConstantVisibility(visProp, subDObj);
        addProps(arbProp, subDObj, mUnmarkedFaceVaryingColors);
        addProps(userProp, subDObj, mUnmarkedFaceVaryingColors);
        addFaceSets(subDObj, iNode);
    }

    if ( mAction >= CONNECT )
    {
        MFnMesh fn(subDObj, &status);

        // check that the data types are compatible, they might not be
        // if we have a weird hierarchy, where the node in the scene
        // differs from the node on disk
        if (!subDObj.hasFn(MFn::kMesh))
        {
            MString theError("No connection done for node '");
            theError += MString(iNode.getName().c_str());
            theError += MString("' with ");
            theError += mConnectDagNode.fullPathName();
            printError(theError);
            return MS::kFailure;
        }

        if (mConnectDagNode.isValid())
        {
            checkShaderSelection(fn, mConnectDagNode.instanceNumber());
        }

        disconnectMesh(subDObj, mData.mPropList, firstProp);
        if (isConstant && CONNECT == mAction)
        {
            readSubD(mFrame, fn, subDObj, subdColors, false);
        }
        addToPropList(firstProp, subDObj);
    }

    if (hasDag)
    {
        mConnectDagNode.pop();
    }

    return status;
}

MStatus CreateSceneVisitor::operator()(Alembic::AbcGeom::IPolyMesh& iNode)
{
    MStatus status = MS::kSuccess;
    MObject polyObj = MObject::kNullObj;

    PolyMeshAndColors meshColors;
    meshColors.mMesh = iNode;

    bool isConstant = iNode.getSchema().isConstant();

    Alembic::Abc::ICompoundProperty arbProp =
        iNode.getSchema().getArbGeomParams();

    Alembic::Abc::ICompoundProperty userProp =
        iNode.getSchema().getUserProperties();

    bool colorAnim = getColorAttrs(arbProp, meshColors.mC3s,
        meshColors.mC4s, mUnmarkedFaceVaryingColors);

    // add animated poly mesh to the list
    if (!isConstant || colorAnim)
        mData.mPolyMeshList.push_back(meshColors);

    std::size_t firstProp = mData.mPropList.size();
    getAnimatedProps(arbProp, mData.mPropList, mUnmarkedFaceVaryingColors);
    getAnimatedProps(userProp, mData.mPropList, mUnmarkedFaceVaryingColors);
    Alembic::Abc::IScalarProperty visProp = getVisible(iNode, isConstant,
        mData.mPropList, mData.mAnimVisStaticObjList);

    bool hasDag = false;
    if (mAction != NONE && mConnectDagNode.isValid())
    {
        hasDag = getDagPathByChildName(mConnectDagNode, iNode.getName());
        if (hasDag)
        {
            polyObj = mConnectDagNode.node();
            if (!isConstant || colorAnim)
            {
                mData.mPolyMeshObjList.push_back(polyObj);
            }
        }
    }

    if (!hasDag && (mAction == CREATE || mAction == CREATE_REMOVE))
    {
        polyObj = createPoly(mFrame, meshColors, mParent);
        if (!isConstant || colorAnim)
        {
            mData.mPolyMeshObjList.push_back(polyObj);
        }
    }

    if (polyObj != MObject::kNullObj)
    {
        setConstantVisibility(visProp, polyObj);
        addProps(arbProp, polyObj, mUnmarkedFaceVaryingColors);
        addProps(userProp, polyObj, mUnmarkedFaceVaryingColors);
        if (iNode.getSchema().getNumSamples() <= 1)
            addFaceSets(polyObj, iNode);
    }

    if ( mAction >= CONNECT )
    {
        MFnMesh fn(polyObj, &status);

        // check that the data types are compatible, they might not be
        // if we have a weird hierarchy, where the node in the scene
        // differs from the node on disk
        if ( status != MS::kSuccess )
        {
            MString theError("No connection done for node '");
            theError += MString(iNode.getName().c_str());
            theError += MString("' with ");
            theError += mConnectDagNode.fullPathName();
            printError(theError);
            return status;
        }

        if (mConnectDagNode.isValid())
            checkShaderSelection(fn, mConnectDagNode.instanceNumber());

        disconnectMesh(polyObj, mData.mPropList, firstProp);
        if (isConstant && CONNECT == mAction)
        {
            readPoly(mFrame, fn, polyObj, meshColors, false);
        }
        addToPropList(firstProp, polyObj);
    }

    if (hasDag)
    {
        mConnectDagNode.pop();
    }

    return status;
}

MStatus CreateSceneVisitor::operator()(Alembic::AbcGeom::INuPatch& iNode)
{
    MStatus status = MS::kSuccess;
    MObject nurbsObj = MObject::kNullObj;

    bool isConstant = iNode.getSchema().isConstant();

    // add animated poly mesh to the list
    if (!isConstant)
        mData.mNurbsList.push_back(iNode);

    Alembic::Abc::ICompoundProperty arbProp =
        iNode.getSchema().getArbGeomParams();

    Alembic::Abc::ICompoundProperty userProp =
        iNode.getSchema().getUserProperties();

    std::size_t firstProp = mData.mPropList.size();
    getAnimatedProps(arbProp, mData.mPropList, false);
    getAnimatedProps(userProp, mData.mPropList, false);
    Alembic::Abc::IScalarProperty visProp = getVisible(iNode, isConstant,
        mData.mPropList, mData.mAnimVisStaticObjList);

    bool hasDag = false;
    if (mAction != NONE && mConnectDagNode.isValid())
    {
        hasDag = getDagPathByChildName(mConnectDagNode, iNode.getName());
        if (hasDag)
        {
            nurbsObj = mConnectDagNode.node();
            if (!isConstant)
            {
                mData.mNurbsObjList.push_back(nurbsObj);
            }
        }
    }

    if (!hasDag && (mAction == CREATE || mAction == CREATE_REMOVE))
    {
        nurbsObj = createNurbs(mFrame, iNode, mParent);
        if (!isConstant)
        {
            mData.mNurbsObjList.push_back(nurbsObj);
        }
    }

    if (nurbsObj != MObject::kNullObj)
    {
        addProps(arbProp, nurbsObj, false);
        addProps(userProp, nurbsObj, false);
        setConstantVisibility(visProp, nurbsObj);
    }


    if ( mAction >= CONNECT )
    {
        MFnNurbsSurface fn(nurbsObj, &status);

        // check that the data types are compatible, they might not be
        // if we have a weird hierarchy, where the node in the scene
        // differs from the node on disk
        if ( status != MS::kSuccess )
        {
            MString theError("No connection done for node '");
            theError += MString(iNode.getName().c_str());
            theError += MString("' with ");
            theError += mConnectDagNode.fullPathName();
            printError(theError);
            return status;
        }

        MPlug dstPlug = fn.findPlug("create");
        disconnectAllPlugsTo(dstPlug);
        disconnectProps(fn, mData.mPropList, firstProp);
        addToPropList(firstProp, nurbsObj);
    }

    if (hasDag)
    {
        mConnectDagNode.pop();
    }
    return status;
}

MStatus CreateSceneVisitor::operator()(Alembic::AbcGeom::IXform & iNode,
                                       AlembicObjectPtr iNodeObject)
{
    MStatus status = MS::kSuccess;
    MObject xformObj = MObject::kNullObj;

    Alembic::Abc::ICompoundProperty arbProp =
        iNode.getSchema().getArbGeomParams();

    Alembic::Abc::ICompoundProperty userProp =
        iNode.getSchema().getUserProperties();

    std::size_t firstProp = mData.mPropList.size();
    getAnimatedProps(arbProp, mData.mPropList, false);
    getAnimatedProps(userProp, mData.mPropList, false);

    if (iNode.getProperties().getPropertyHeader("locator") != NULL)
    {
        Alembic::Abc::ICompoundProperty props = iNode.getProperties();
        const Alembic::AbcCoreAbstract::PropertyHeader * locHead =
            props.getPropertyHeader("locator");
        if (locHead != NULL && locHead->isScalar() &&
            locHead->getDataType().getPod() == Alembic::Util::kFloat64POD &&
            locHead->getDataType().getExtent() == 6)
        {
            Alembic::Abc::IScalarProperty locProp(props, "locator");
            bool isConstant = locProp.isConstant();

            Alembic::Abc::IScalarProperty visProp = getVisible(iNode,
                isConstant, mData.mPropList, mData.mAnimVisStaticObjList);

            // add animated locator to the list
            if (!isConstant)
                mData.mLocList.push_back(iNode);

            bool hasDag = false;
            if (mAction != NONE && mConnectDagNode.isValid())
            {
                hasDag = getDagPathByChildName(mConnectDagNode,
                    iNode.getName());
                if (hasDag)
                {
                    xformObj = mConnectDagNode.node();
                    if (!isConstant)
                    {
                        mData.mLocObjList.push_back(xformObj);
                    }
                }
            }

            if (!hasDag && (mAction == CREATE || mAction == CREATE_REMOVE))
            {
                xformObj = create(iNode, mParent, locProp);
                if (!isConstant)
                {
                    mData.mLocObjList.push_back(xformObj);
                }
            }

            if (xformObj != MObject::kNullObj)
            {
                addProps(arbProp, xformObj, false);
                addProps(userProp, xformObj, false);
                setConstantVisibility(visProp, xformObj);
            }

            if ( mAction >= CONNECT )
            {
                if (!xformObj.hasFn(MFn::kLocator))
                {
                    MString theError("No connection done for node '");
                    theError += MString(iNode.getName().c_str());
                    theError += MString("' with ");
                    theError += mConnectDagNode.fullPathName();
                    printError(theError);
                    return status;
                }

                addToPropList(firstProp, xformObj);
            }

            if (hasDag)
            {
                mConnectDagNode.pop();
            }
        }
    }
    else    // transform node
    {
        MString name(iNode.getName().c_str());

        size_t numChildren = iNodeObject->getNumChildren();
        bool isConstant = iNode.getSchema().isConstant();

        Alembic::Abc::IScalarProperty visProp = getVisible(iNode,
            isConstant, mData.mPropList, mData.mAnimVisStaticObjList);

        Alembic::AbcGeom::XformSample samp;
        iNode.getSchema().get(samp, 0);
        if (!isConstant)
        {
            mData.mXformList.push_back(iNode);
            mData.mIsComplexXform.push_back(isComplex(samp));
        }

        if (isConstant && visProp.valid() && !visProp.isConstant())
        {
             mData.mAnimVisStaticObjList.push_back(iNode);
        }

        bool hasDag = false;
        if (mAction != NONE && mConnectDagNode.isValid())
        {
            hasDag = getDagPathByChildName(mConnectDagNode, iNode.getName());
            if (hasDag)
            {
                xformObj = mConnectDagNode.node();
            }
        }

        // There might be children under the current DAG node that
        // doesn't exist in the file.
        // Remove them if the -removeIfNoUpdate flag is set
        if ((mAction == REMOVE || mAction == CREATE_REMOVE) && hasDag)
        {
            unsigned int numDags = mConnectDagNode.childCount();
            std::vector<MDagPath> dagToBeRemoved;

            // get names of immediate children so we can compare with
            // the hierarchy in the scene
            std::set< std::string > childNodesInFile;
            for (size_t j = 0; j < numChildren; ++j)
            {
                Alembic::Abc::IObject child = iNodeObject->getChild(j)->object();
                childNodesInFile.insert(child.getName());
            }

            for (unsigned int i = 0; i < numDags; i++)
            {
                MObject child = mConnectDagNode.child(i);
                MFnDagNode fn(child, &status);
                if ( status == MS::kSuccess )
                {
                    std::string childName = fn.fullPathName().asChar();
                    size_t found = childName.rfind("|");

                    if (found != std::string::npos)
                    {
                        childName = childName.substr(
                            found+1, childName.length() - found);
                        if (childNodesInFile.find(childName)
                            == childNodesInFile.end())
                        {
                            MDagPath dagPath;
                            getDagPathByName(fn.fullPathName(), dagPath);
                            dagToBeRemoved.push_back(dagPath);
                        }
                    }
                }
            }
            if (dagToBeRemoved.size() > 0)
            {
                unsigned int dagSize =
                    static_cast<unsigned int>(dagToBeRemoved.size());
                for ( unsigned int i = 0; i < dagSize; i++ )
                    removeDagNode(dagToBeRemoved[i]);
            }
        }

        // just create the node
        if (!hasDag && (mAction == CREATE || mAction == CREATE_REMOVE ))
        {
            MFnTransform trans;
            xformObj = trans.create(mParent, &status);

            if (status != MS::kSuccess)
            {
                MString theError("Failed to create transform node ");
                theError += name;
                printError(theError);
                return status;
            }

            trans.setName(name);
        }

        if (xformObj != MObject::kNullObj)
        {
            setConstantVisibility(visProp, xformObj);
            addProps(arbProp, xformObj, false);
            addProps(userProp, xformObj, false);
        }

        if (mAction >= CONNECT)
        {
            if (xformObj.hasFn(MFn::kTransform))
            {
                std::vector<std::string> transopNameList;
                connectToXform(samp, isConstant, xformObj, transopNameList,
                    mData.mPropList, firstProp);

                if (!isConstant)
                {
                    SampledPair sampPair(xformObj, transopNameList);
                    mData.mXformOpList.push_back(sampPair);
                }
                addToPropList(firstProp, xformObj);
            }
            else if (xformObj != MObject::kNullObj)
            {
                MString theError = mConnectDagNode.partialPathName();
                theError += MString(" is not compatible as a transform node. ");
                theError += MString("Connection failed.");
                printError(theError);
                return MS::kFailure;
            }

        }

        MObject saveParent = xformObj;
        for (size_t i = 0; i < numChildren; ++i)
        {
            mParent = saveParent;

            this->visit(iNodeObject->getChild(i));
        }

        if (hasDag)
        {
            mConnectDagNode.pop();
        }
    }

    return status;
}

MStatus CreateSceneVisitor::createEmptyObject(AlembicObjectPtr iNodeObject)
{
    Alembic::Abc::IObject iNode = iNodeObject->object();

    MStatus status = MS::kSuccess;
    MObject xformObj = MObject::kNullObj;

    MString name(iNode.getName().c_str());

    size_t numChildren = iNodeObject->getNumChildren();

    bool hasDag = false;

    if (mAction != NONE && mConnectDagNode.isValid())
    {
        hasDag = getDagPathByChildName(mConnectDagNode, iNode.getName());
        if (hasDag)
        {
            xformObj = mConnectDagNode.node();
        }
    }

    // There might be children under the current DAG node that
    // doesn't exist in the file.
    // Remove them if the -removeIfNoUpdate flag is set
    if ((mAction == REMOVE || mAction == CREATE_REMOVE) &&
        mConnectDagNode.isValid())
    {
        unsigned int numDags = mConnectDagNode.childCount();
        std::vector<MDagPath> dagToBeRemoved;

        // get names of immediate children so we can compare with
        // the hierarchy in the scene
        std::set< std::string > childNodesInFile;
        for (size_t j = 0; j < numChildren; ++j)
        {
            Alembic::Abc::IObject child = iNodeObject->getChild(j)->object();
            childNodesInFile.insert(child.getName());
        }

        for (unsigned int i = 0; i < numDags; i++)
        {
            MObject child = mConnectDagNode.child(i);
            MFnDagNode fn(child, &status);
            if ( status == MS::kSuccess )
            {
                std::string childName = fn.fullPathName().asChar();
                size_t found = childName.rfind("|");

                if (found != std::string::npos)
                {
                    childName = childName.substr(
                        found+1, childName.length() - found);
                    if (childNodesInFile.find(childName)
                        == childNodesInFile.end())
                    {
                        MDagPath dagPath;
                        getDagPathByName(fn.fullPathName(), dagPath);
                        dagToBeRemoved.push_back(dagPath);
                    }
                }
            }
        }
        if (dagToBeRemoved.size() > 0)
        {
            unsigned int dagSize =
                static_cast<unsigned int>(dagToBeRemoved.size());
            for ( unsigned int i = 0; i < dagSize; i++ )
                removeDagNode(dagToBeRemoved[i]);
        }
    }

    // just create the node
    if (!hasDag && (mAction == CREATE || mAction == CREATE_REMOVE ))
    {
        MFnTransform trans;
        xformObj = trans.create(mParent, &status);

        if (status != MS::kSuccess)
        {
            MString theError("Failed to create transform node ");
            theError += name;
            printError(theError);
            return status;
        }

        trans.setName(name);
    }

    MObject saveParent = xformObj;
    for (size_t i = 0; i < numChildren; ++i)
    {
        mParent = saveParent;

        this->visit(iNodeObject->getChild(i));
    }

    if (hasDag)
    {
        mConnectDagNode.pop();
    }
    return status;
}

