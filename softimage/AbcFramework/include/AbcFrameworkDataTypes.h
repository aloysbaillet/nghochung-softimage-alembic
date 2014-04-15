//*****************************************************************************
/*!
	\file AbcFrameworkDataTypes.h
	\brief Expose the data types supported by Alembic (uint32, int32, etc).

	AbcFramework exposes these types as they are in Alembic source, they don't 
	need wrappers like OObject and IObject types cause it's unlikely that they will change.

	The purpose is to avoid having to include all Alembic headers (yes, all headers!) 
	just to understand the data types that Alembic supports. The AbcFramework headers 
	should be enough for the internal/plugin code to do that.

	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*

#ifndef ABCFRAMEWORKDATATYPES_H
#define ABCFRAMEWORKDATATYPES_H

// stdint.h is not in anything prior to Visual Studio 2010
#if !defined(_MSC_VER) || _MSC_VER >= 1600
#include <stdint.h>
#endif

#ifndef ABCFRAMEWORK_IMPL

#include "AbcFrameworkPlainOldDataType.h"
#include "AbcFrameworkImportedTypes.h"
#include "AbcFrameworkCoreAbstractDataType.h"
#include "AbcFrameworkTypedPropertyTraits.h"

#else

#include <Alembic/Abc/Foundation.h>
#include <Alembic/Abc/TypedPropertyTraits.h>

#endif

#include "AbcFrameworkDataTypeExtension.h"

namespace AbcFramework{

	template<class TRAITS> EAbcDataTraits GetEDataTraits();

	template<> __forceinline EAbcDataTraits GetEDataTraits<Alembic::Abc::BooleanTPTraits>() { return EDataTraits_Bool;}
	template<> __forceinline EAbcDataTraits GetEDataTraits<Alembic::Abc::Uint8TPTraits>() { return EDataTraits_UInt8;}
	template<> __forceinline EAbcDataTraits GetEDataTraits<Alembic::Abc::Int8TPTraits>() { return EDataTraits_Int8;}
	template<> __forceinline EAbcDataTraits GetEDataTraits<Alembic::Abc::Uint16TPTraits>() { return EDataTraits_UInt16;}
	template<> __forceinline EAbcDataTraits GetEDataTraits<Alembic::Abc::Int16TPTraits>() { return EDataTraits_Int16;}
	template<> __forceinline EAbcDataTraits GetEDataTraits<Alembic::Abc::Uint32TPTraits>() { return EDataTraits_UInt32;}
	template<> __forceinline EAbcDataTraits GetEDataTraits<Alembic::Abc::Int32TPTraits>() { return EDataTraits_Int32;}
	template<> __forceinline EAbcDataTraits GetEDataTraits<Alembic::Abc::Uint64TPTraits>() { return EDataTraits_UInt64;}
	template<> __forceinline EAbcDataTraits GetEDataTraits<Alembic::Abc::Int64TPTraits>() { return EDataTraits_Int64;}
	template<> __forceinline EAbcDataTraits GetEDataTraits<Alembic::Abc::Float16TPTraits>() { return EDataTraits_Float16;}
	template<> __forceinline EAbcDataTraits GetEDataTraits<Alembic::Abc::Float32TPTraits>() { return EDataTraits_Float32;}
	template<> __forceinline EAbcDataTraits GetEDataTraits<Alembic::Abc::Float64TPTraits>() { return EDataTraits_Float64;}

	template<> __forceinline EAbcDataTraits GetEDataTraits<Alembic::Abc::StringTPTraits>() { return EDataTraits_StringPtr;}
	template<> __forceinline EAbcDataTraits GetEDataTraits<Alembic::Abc::WstringTPTraits>() { return EDataTraits_WStringPtr;}
	template<> __forceinline EAbcDataTraits GetEDataTraits<Alembic::Abc::V2sTPTraits>() { return EDataTraits_V2s;}
	template<> __forceinline EAbcDataTraits GetEDataTraits<Alembic::Abc::V2iTPTraits>() { return EDataTraits_V2i;}
	template<> __forceinline EAbcDataTraits GetEDataTraits<Alembic::Abc::V2fTPTraits>() { return EDataTraits_V2f;}
	template<> __forceinline EAbcDataTraits GetEDataTraits<Alembic::Abc::V2dTPTraits>() { return EDataTraits_V2d;}
	
	template<> __forceinline EAbcDataTraits GetEDataTraits<Alembic::Abc::V3sTPTraits>() { return EDataTraits_V3s;}
	template<> __forceinline EAbcDataTraits GetEDataTraits<Alembic::Abc::V3iTPTraits>() { return EDataTraits_V3i;}
	template<> __forceinline EAbcDataTraits GetEDataTraits<Alembic::Abc::V3fTPTraits>() { return EDataTraits_V3f;}
	template<> __forceinline EAbcDataTraits GetEDataTraits<Alembic::Abc::V3dTPTraits>() { return EDataTraits_V3d;}
	
	template<> __forceinline EAbcDataTraits GetEDataTraits<Alembic::Abc::P2sTPTraits>() { return EDataTraits_P2s;}
	template<> __forceinline EAbcDataTraits GetEDataTraits<Alembic::Abc::P2iTPTraits>() { return EDataTraits_P2i;}
	template<> __forceinline EAbcDataTraits GetEDataTraits<Alembic::Abc::P2fTPTraits>() { return EDataTraits_P2f;}
	template<> __forceinline EAbcDataTraits GetEDataTraits<Alembic::Abc::P2dTPTraits>() { return EDataTraits_P2d;}
	
	template<> __forceinline EAbcDataTraits GetEDataTraits<Alembic::Abc::P3sTPTraits>() { return EDataTraits_P3s;}
	template<> __forceinline EAbcDataTraits GetEDataTraits<Alembic::Abc::P3iTPTraits>() { return EDataTraits_P3i;}
	template<> __forceinline EAbcDataTraits GetEDataTraits<Alembic::Abc::P3fTPTraits>() { return EDataTraits_P3f;}
	template<> __forceinline EAbcDataTraits GetEDataTraits<Alembic::Abc::P3dTPTraits>() { return EDataTraits_P3d;}	

	template<> __forceinline EAbcDataTraits GetEDataTraits<Alembic::Abc::Box2sTPTraits>() { return EDataTraits_Box2s;}
	template<> __forceinline EAbcDataTraits GetEDataTraits<Alembic::Abc::Box2iTPTraits>() { return EDataTraits_Box2i;}
	template<> __forceinline EAbcDataTraits GetEDataTraits<Alembic::Abc::Box2fTPTraits>() { return EDataTraits_Box2f;}
	template<> __forceinline EAbcDataTraits GetEDataTraits<Alembic::Abc::Box2dTPTraits>() { return EDataTraits_Box2d;}
	
	template<> __forceinline EAbcDataTraits GetEDataTraits<Alembic::Abc::Box3sTPTraits>() { return EDataTraits_Box3s;}
	template<> __forceinline EAbcDataTraits GetEDataTraits<Alembic::Abc::Box3iTPTraits>() { return EDataTraits_Box3i;}
	template<> __forceinline EAbcDataTraits GetEDataTraits<Alembic::Abc::Box3fTPTraits>() { return EDataTraits_Box3f;}
	template<> __forceinline EAbcDataTraits GetEDataTraits<Alembic::Abc::Box3dTPTraits>() { return EDataTraits_Box3d;}
	
	template<> __forceinline EAbcDataTraits GetEDataTraits<Alembic::Abc::M33fTPTraits>() { return EDataTraits_M33f;}
	template<> __forceinline EAbcDataTraits GetEDataTraits<Alembic::Abc::M33dTPTraits>() { return EDataTraits_M33d;}
	template<> __forceinline EAbcDataTraits GetEDataTraits<Alembic::Abc::M44fTPTraits>() { return EDataTraits_M44f;}
	template<> __forceinline EAbcDataTraits GetEDataTraits<Alembic::Abc::M44dTPTraits>() { return EDataTraits_M44d;}
	
	template<> __forceinline EAbcDataTraits GetEDataTraits<Alembic::Abc::QuatfTPTraits>() { return EDataTraits_Quatf;}
	template<> __forceinline EAbcDataTraits GetEDataTraits<Alembic::Abc::QuatdTPTraits>() { return EDataTraits_Quatd;}
	
	template<> __forceinline EAbcDataTraits GetEDataTraits<Alembic::Abc::C3hTPTraits>() { return EDataTraits_C3h;}
	template<> __forceinline EAbcDataTraits GetEDataTraits<Alembic::Abc::C3fTPTraits>() { return EDataTraits_C3f;}
	template<> __forceinline EAbcDataTraits GetEDataTraits<Alembic::Abc::C3cTPTraits>() { return EDataTraits_C3c;}
	
	template<> __forceinline EAbcDataTraits GetEDataTraits<Alembic::Abc::C4hTPTraits>() { return EDataTraits_C4h;}
	template<> __forceinline EAbcDataTraits GetEDataTraits<Alembic::Abc::C4fTPTraits>() { return EDataTraits_C4f;}
	template<> __forceinline EAbcDataTraits GetEDataTraits<Alembic::Abc::C4cTPTraits>() { return EDataTraits_C4c;}

	template<> __forceinline EAbcDataTraits GetEDataTraits<Alembic::Abc::N2fTPTraits>() { return EDataTraits_N2f;}
	template<> __forceinline EAbcDataTraits GetEDataTraits<Alembic::Abc::N2dTPTraits>() { return EDataTraits_N2d;}

	template<> __forceinline EAbcDataTraits GetEDataTraits<Alembic::Abc::N3fTPTraits>() { return EDataTraits_N3f;}
	template<> __forceinline EAbcDataTraits GetEDataTraits<Alembic::Abc::N3dTPTraits>() { return EDataTraits_N3d;}

	template<> __forceinline EAbcDataTraits GetEDataTraits<AbcFramework::V4fTPTraits>() { return EDataTraits_V4f;}
	template<> __forceinline EAbcDataTraits GetEDataTraits<AbcFramework::VRotationfTraits>() { return EDataTraits_Rotf; }

	template<EAbcDataTraits TRAITENUM>
	struct TPTRAITSFromEnum {};

	// Actual specializations
	template <> struct TPTRAITSFromEnum<EDataTraits_Bool> : public Alembic::Abc::BooleanTPTraits {};
	template <> struct TPTRAITSFromEnum<EDataTraits_UInt8> : public Alembic::Abc::Uint8TPTraits {};
	template <> struct TPTRAITSFromEnum<EDataTraits_Int8> : public Alembic::Abc::Int8TPTraits {};
	template <> struct TPTRAITSFromEnum<EDataTraits_UInt16> : public Alembic::Abc::Uint16TPTraits {};
	template <> struct TPTRAITSFromEnum<EDataTraits_Int16> : public Alembic::Abc::Int16TPTraits {};
	template <> struct TPTRAITSFromEnum<EDataTraits_UInt32> : public Alembic::Abc::Uint32TPTraits {};
	template <> struct TPTRAITSFromEnum<EDataTraits_Int32> : public Alembic::Abc::Int32TPTraits {};
	template <> struct TPTRAITSFromEnum<EDataTraits_UInt64> : public Alembic::Abc::Uint64TPTraits {};
	template <> struct TPTRAITSFromEnum<EDataTraits_Int64> : public Alembic::Abc::Int64TPTraits {};
	template <> struct TPTRAITSFromEnum<EDataTraits_Float16> : public Alembic::Abc::Float16TPTraits {};
	template <> struct TPTRAITSFromEnum<EDataTraits_Float32> : public Alembic::Abc::Float32TPTraits {};
	template <> struct TPTRAITSFromEnum<EDataTraits_Float64> : public Alembic::Abc::Float64TPTraits {};
	template <> struct TPTRAITSFromEnum<EDataTraits_StringPtr> : public Alembic::Abc::StringTPTraits {};
	template <> struct TPTRAITSFromEnum<EDataTraits_WStringPtr> : public Alembic::Abc::WstringTPTraits {};

} // End namespace Alembic

#endif
