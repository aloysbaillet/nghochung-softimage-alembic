//*****************************************************************************
/*!
	\file AbcFrameworkDataTypeConverter.h
	\brief Define the base converter template between Alembic and Softimage.

	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*

#ifndef ABCFRAMEWORKDATATTYPECONVERTER_H
#define ABCFRAMEWORKDATATTYPECONVERTER_H

namespace AbcFramework
{

template< class XsiDataT, class AbcDataT >
class DataTypeConverter
{
public:
	__forceinline static void Convert( AbcDataT& out, const XsiDataT& in )
	{
		out = (AbcDataT&)in;
	}
};

template<>
class DataTypeConverter< bool, Alembic::Abc::bool_t >
{
public:
	__forceinline static void Convert( Alembic::Abc::bool_t& out, const bool& in )
	{
		out = in;
	}
};

} // End namespace AbcFramework
#endif