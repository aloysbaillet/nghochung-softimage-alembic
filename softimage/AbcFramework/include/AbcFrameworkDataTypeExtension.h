//*****************************************************************************
/*!
	\file AbcFrameworkDataTypeExtension.h
	\brief extend data type for siICENodeDataVector4

	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*

#ifndef ABCFRAMEWORKDATATYPEEXTENSION_H
#define ABCFRAMEWORKDATATYPEEXTENSION_H


#ifndef ABCFRAMEWORK_IMPL

#include "AbcFrameworkTypedPropertyTraits.h"

#else

#include <Alembic/Abc/TypedPropertyTraits.h>

#endif

namespace AbcFramework {

	namespace AbcA = ::Alembic::AbcCoreAbstract::ALEMBIC_VERSION_NS;
	using namespace Alembic::Util::ALEMBIC_VERSION_NS;
    using Imath::V4f;

#define ABCFRAMEWORK_DATA_TYPE_EXTENSION_TRAITS( VAL, POD, EXTENT, INTERP, PTDEF ) \
	struct PTDEF                                                                \
	{                                                                           \
	static const PlainOldDataType pod_enum = POD;                       \
	static const int extent = EXTENT;                                   \
	typedef VAL         value_type;                                     \
	static const char * interpretation()  { return ( INTERP ) ; }       \
	static const char * name() { return #PTDEF; }                       \
	static AbcA::DataType     dataType()                                \
	{ return AbcA::DataType( POD, EXTENT ) ; }                          \
	static value_type   defaultValue()                                  \
	{ value_type v; return v; }                                         \
    }

    ABCFRAMEWORK_DATA_TYPE_EXTENSION_TRAITS( V4f, kFloat32POD, 4, "vector", V4fTPTraits );

	struct VRotationf {
		float x, y, z, w, r;
	};

	ABCFRAMEWORK_DATA_TYPE_EXTENSION_TRAITS( VRotationf, kFloat32POD, 5, "vrotation", VRotationfTraits );

}


#endif

