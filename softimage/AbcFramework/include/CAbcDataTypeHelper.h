//*****************************************************************************
/*!
	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*

#ifndef CABCDATATYPEHELPER_H
#define CABCDATATYPEHELPER_H

#include <Alembic/Abc/OObject.h>
#include <Alembic/Abc/TypedPropertyTraits.h>

#include "IAbcFramework.h"
#include "IAbcOutput.h"

EAbcOObjectType GetOObjectType( const Alembic::Abc::ObjectHeader& in_ObjHeader );
EAbcDataTraits GetDataTraits( const Alembic::AbcCoreAbstract::PropertyHeader& in_PropHeader );
EAbcPodType GetPodType( const Alembic::AbcCoreAbstract::DataType& in_DataType );

#endif