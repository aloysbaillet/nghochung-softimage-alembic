//*****************************************************************************
/*!
	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*

#include "CAbcOObjectFactory.h"
#include "CAbcOCamera.h"
#include "CAbcOPolyMesh.h"
#include "CAbcOPoints.h"
#include "CAbcOXform.h"
#include "CAbcOFaceSet.h"

#include <boost/exception/exception.hpp>

EAbcResult CAbcOObjectFactory::CreateOObject( IAbcOObject** out_ppObj, IAbcOObject* in_pParent, EAbcOObjectType in_type, Alembic::Abc::OObject& in_ParentOObj, const char* in_pszName )
{
	if ( out_ppObj == NULL || in_pszName == NULL )
		return EResult_InvalidPtr;

	try
	{
		IAbcOObject* l_pOObj = NULL;

		switch (in_type)
		{
		case EOObject_Polymesh:
			{
				l_pOObj = new CAbcOPolyMesh( in_pParent, in_ParentOObj, in_pszName );
			}
			break;
		case EOObject_Points:
			{
				l_pOObj = new CAbcOPoints( in_pParent, in_ParentOObj, in_pszName );
			}
			break;
		case EOObject_Xform:
			{
				l_pOObj = new CAbcOXform( in_pParent, in_ParentOObj, in_pszName );
			}
			break;
		case EOObject_Camera:
			{
				l_pOObj = new CAbcOCamera( in_pParent, in_ParentOObj, in_pszName );
			}
			break;
		case EOObject_FaceSet:
			{
				l_pOObj = new CAbcOFaceSet( in_pParent, in_ParentOObj, in_pszName );
			}
			break;
		default:
			break;		
		}

		if ( l_pOObj )
		{
			l_pOObj->AddRef();
			*out_ppObj = l_pOObj;
			return EResult_Success;
		}
		else
		{
			return EResult_Fail;
		}
	}
	catch (...)
	{
		return EResult_Fail;
	}

}