//*****************************************************************************
/*!
	\file AbcFrameworkImportedTypes.h
	\brief Expose Alembic imported types (Imath V2f, V3f, etc)

	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*

#ifndef ABCFRAMEWORKIMPORTEDTYPES_H
#define ABCFRAMEWORKIMPORTEDTYPES_H

// headers for OpenEXR data types
#include <OpenEXR/ImathVec.h>
#include <OpenEXR/ImathBox.h>
#include <OpenEXR/ImathMatrix.h>
#include <OpenEXR/ImathQuat.h>
#include <OpenEXR/ImathColor.h>

// =========================================================================================
// IMPORTING OPEN EXR TYPES
// =========================================================================================
#ifndef ABCFRAMEWORK_IMPL

namespace Alembic
{

namespace Abc {
	namespace ALEMBIC_VERSION_NS {

		// IMPORTED IMATH TYPES
		//-*****************************************************************************
		//-*****************************************************************************
		//-*****************************************************************************
		using Imath::V2s;
		using Imath::V2i;
		using Imath::V2f;
		using Imath::V2d;

		using Imath::V3s;
		using Imath::V3i;
		using Imath::V3f;
		using Imath::V3d;

		using Imath::Box2s;
		using Imath::Box2i;
		using Imath::Box2f;
		using Imath::Box2d;

		using Imath::Box3s;
		using Imath::Box3i;
		using Imath::Box3f;
		using Imath::Box3d;

		using Imath::M33f;
		using Imath::M33d;
		using Imath::M44f;
		using Imath::M44d;

		using Imath::Quatf;
		using Imath::Quatd;

		using Imath::C3h;
		using Imath::C3f;
		using Imath::C3c;

		using Imath::C4h;
		using Imath::C4f;
		using Imath::C4c;

		typedef V3f N3f;
		typedef V3d N3d;  

	}
}
}
#else

#include <Alembic/Abc/Foundation.h>

#endif // !ABCFRAMEWORK_IMPL
#endif