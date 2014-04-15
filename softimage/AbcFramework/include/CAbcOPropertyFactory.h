//*****************************************************************************
/*!
	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*

#ifndef CABCOPROPERTYFACTORY_H
#define CABCOPROPERTYFACTORY_H

#include "IAbcFramework.h"
#include "IAbcOutput.h"
#include "IAbcOProperty.h"
#include <Alembic/Abc/OCompoundProperty.h>

class CAbcOPropertyFactory
{
public:
	static EAbcResult CreateOProperty( 
		IAbcOProperty** out_ppProp, 
		Alembic::Abc::OCompoundProperty& in_parent, 
		const char* in_pszName, 
		EAbcPropertyType in_DataType, 
		EAbcDataTraits in_PodType
	);

	static EAbcResult WrapOProperty( 
		IAbcOProperty** out_ppProp, 
		Alembic::AbcCoreAbstract::BasePropertyWriterPtr in_ptr
	);

	static EAbcResult CreateOGeomParam( 
		IAbcOGeomParamBase** out_ppGeomParam, 
		Alembic::Abc::OCompoundProperty& in_parent, 
		const char* in_pszName, 
		bool in_bIsIndex,
		EAbcDataTraits in_DataType, 
		EAbcGeomScope in_geomScope,
		size_t in_szExtent 
	);

	static EAbcResult WrapOGeomParam( 
		IAbcOGeomParamBase** out_ppGeomParam, 
		IAbcOProperty* in_pProp
		);
};


#endif