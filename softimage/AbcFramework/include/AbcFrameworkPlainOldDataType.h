//*****************************************************************************
/*!
	\file AbcFrameworkPlainOldDataType.h
	\brief Extraction of Alembic plain old data types (bool, uint32, int32, etc)

	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*

#ifndef ABCFRAMEWORKPLAINOLDDATATYPE_H
#define ABCFRAMEWORKPLAINOLDDATATYPE_H

#include <assert.h>
#include <OpenEXR/half.h>

// =========================================================================================
// PLAIN OLD DATA TYPES AND THEIR TRAITS
// =========================================================================================
#ifndef ABCFRAMEWORK_IMPL

namespace Alembic {
namespace Util {
namespace ALEMBIC_VERSION_NS {


	//-*****************************************************************************
	//! Bytes are unsigned chars, by definition.
	//! We use bytes in Alembic as the name of anonymous storage memory, since
	//! it is not possible to create arrays of voids. 
	typedef unsigned char           byte_t;

	//-*****************************************************************************
	//-*****************************************************************************
	//! BOOLEAN BASE TYPE - since C++ doesn't explicitly demand that bool
	//! be a given bit depth, but we need it to be here, we make our own
	//! bool type, which is a bit silly. This is purely for storage reasons.
	//-*****************************************************************************
	class bool_t
	{
	public:
		bool_t() : m_byte( 0 ) {}

		bool_t( bool tf ) : m_byte( static_cast<byte_t>( tf ) ) {}
		bool_t( byte_t b ) : m_byte( b ) {}


		//! Using default copy constructor
		//! ...

		//! Using default assignment operator
		//! ...

		bool_t& operator=( bool tf )
		{
			m_byte = static_cast<byte_t>( tf );
			return *this;
		}

		bool_t& operator=( byte_t b )
		{
			m_byte = b;
			return *this;
		}

		bool_t operator!( void )
		{
			return bool_t( m_byte == 0 );
		}

		bool asBool() const { return ( m_byte != 0 ); }
		operator bool() const { return ( m_byte != 0 ); }

	private:
		byte_t m_byte;
	};

	//-*****************************************************************************
	inline bool operator==( const bool_t &a, const bool_t &b )
	{
		return a.asBool() == b.asBool();
	}

	//-*****************************************************************************
	inline bool operator==( const bool_t &a, bool b )
	{
		return a.asBool() == b;
	}

	//-*****************************************************************************
	inline bool operator==( bool a, const bool_t &b )
	{
		return a == b.asBool();
	}

	//-*****************************************************************************
	inline bool operator!=( const bool_t &a, const bool_t &b )
	{
		return a.asBool() != b.asBool();
	}

	//-*****************************************************************************
	inline bool operator!=( const bool_t &a, bool b )
	{
		return a.asBool() != b;
	}

	//-*****************************************************************************
	inline bool operator!=( bool a, const bool_t &b )
	{
		return a != b.asBool();
	}

#if !defined(_MSC_VER)
	using ::uint8_t;
	using ::int8_t;
	using ::uint16_t;
	using ::int16_t;
	using ::uint32_t;
	using ::int32_t;
	using ::uint64_t;
	using ::int64_t;
#else
	typedef unsigned char           uint8_t;
	typedef signed char             int8_t;
	typedef unsigned short          uint16_t;
	typedef signed short            int16_t;
	typedef unsigned int            uint32_t;
	typedef int                     int32_t;
	typedef unsigned long long      uint64_t;
	typedef long long               int64_t;
#endif

	typedef half                    float16_t;
	typedef float                   float32_t;
	typedef double                  float64_t;

	//! Last, but not least, standard strings.
	//! These are CLEARLY not "Plain Old Data Types", however, "strings" are
	//! such ubiquitous components of programming, and without an enclosing
	//! structure like std::string, they're so difficult to use from an API
	//! point of view (call first time to find out length! allocate your own array!
	//! call second time to get string value!), that I'm going to put my foot down
	//! and say - from Alembic's point of view, std::string and std::wstring are
	//! "Kinda Sorta POD types". Please pardon the abuse of the idiom.
	using std::string;
	using std::wstring;


	//-*****************************************************************************
	//! I'm using explicit names here because the terms 'int', 'short', 'long', etc,
	//! have different bit-depths on different machine architectures. To avoid
	//! any ambiguity whatsoever, I'm just making these explicit. End users will
	//! rarely see these anyway, so it's okay to be a bit pedantic.
	//!
	//! These are always represented in the endian-ness of the host machine when
	//! resident in working memory, but need to have an explicit endian-ness when
	//! being written out. That's hidden from the user by HDF5.
	enum PlainOldDataType
	{
		//! Booleans are difficult to store in arrays in a 'one bit per bool'
		//! kind of way, so we actually file them as bytes (uint8).  But again
		//! this is entirely hidden from the end user. Implemented via the
		//! "bool_t" type defined above.
		kBooleanPOD,

		//! Char/UChar
		kUint8POD,
		kInt8POD,

		//! Short/UShort
		kUint16POD,
		kInt16POD,

		//! Int/UInt
		kUint32POD,
		kInt32POD,

		//! Long/ULong
		kUint64POD,
		kInt64POD,

		//! Half/Float/Double
		kFloat16POD,
		kFloat32POD,
		kFloat64POD,

		//! String Pointer
		kStringPOD,

		//! Wide String Pointer
		kWstringPOD,

		//! Number of POD
		kNumPlainOldDataTypes,

		//! Unknown
		kUnknownPOD = 127
	};

	//-*****************************************************************************
	//-*****************************************************************************
	//-*****************************************************************************
	// A little traits class that binds these things together.
	//-*****************************************************************************
	//-*****************************************************************************
	//-*****************************************************************************
	template <PlainOldDataType PODT, class T > struct PODTraits {};

	//-*****************************************************************************
	//! Unfortunately, C++ only allows for static const declaration of constants
	//! with integral types, not floating. Therefore, we have the whole
	//! inlined static function for default values.
#define DECLARE_TRAITS( PENUM, PTYPE, PNAME, DFLT, PTDEF )                    \
template <>                                                                   \
	struct PODTraits< PENUM , PTYPE >                                             \
	{                                                                             \
	static const PlainOldDataType       pod_enum = PENUM ;                    \
	typedef PTYPE                       value_type ;                          \
	static const char *                 name() { return PNAME ; }             \
	static PTYPE                        default_value()                       \
	{ return ( DFLT ) ; }                                                     \
	static size_t                       numBytes()                            \
	{ return sizeof( PTYPE ) ; }                                              \
	};                                                                            \
	typedef PODTraits< PENUM , PTYPE > PTDEF 

	//-*****************************************************************************
	// Actual specialized traits
	DECLARE_TRAITS( kBooleanPOD, bool_t,    "bool_t",    false, BooleanPODTraits );
	DECLARE_TRAITS( kUint8POD,   uint8_t,   "uint8_t",   0,     Uint8PODTraits );
	DECLARE_TRAITS( kInt8POD,    int8_t,    "int8_t",    0,     Int8PODTraits );
	DECLARE_TRAITS( kUint16POD,  uint16_t,  "uint16_t",  0,     Uint16PODTraits );
	DECLARE_TRAITS( kInt16POD,   int16_t,   "int16_t",   0,     Int16PODTraits );
	DECLARE_TRAITS( kUint32POD,  uint32_t,  "uint32_t",  0,     Uint32PODTraits );
	DECLARE_TRAITS( kInt32POD,   int32_t,   "int32_t",   0,     Int32PODTraits );
	DECLARE_TRAITS( kUint64POD,  uint64_t,  "uint64_t",  0,     Uint64PODTraits );
	DECLARE_TRAITS( kInt64POD,   int64_t,   "int64_t",   0,     Int64PODTraits );
	DECLARE_TRAITS( kFloat16POD, float16_t, "float16_t", 0,     Float16PODTraits );
	DECLARE_TRAITS( kFloat32POD, float32_t, "float32_t", 0,     Float32PODTraits );
	DECLARE_TRAITS( kFloat64POD, float64_t, "float64_t", 0,     Float64PODTraits );
	DECLARE_TRAITS( kStringPOD,  string,    "string",    "",    StringPODTraits );
	DECLARE_TRAITS( kWstringPOD, wstring,   "wstring",   L"",   WstringPODTraits );

#undef DECLARE_TRAITS  

	//-*****************************************************************************
	//-*****************************************************************************
	// FROM TYPES
	//-*****************************************************************************
	//-*****************************************************************************
	template <class PTYPE>
	struct PODTraitsFromType {};

	//-*****************************************************************************
	// Actual specializations
	template <> struct PODTraitsFromType<bool_t> : public BooleanPODTraits {};
	template <> struct PODTraitsFromType<uint8_t> : public Uint8PODTraits {};
	template <> struct PODTraitsFromType<int8_t> : public Int8PODTraits {};
	template <> struct PODTraitsFromType<uint16_t> : public Uint16PODTraits {};
	template <> struct PODTraitsFromType<int16_t> : public Int16PODTraits {};
	template <> struct PODTraitsFromType<uint32_t> : public Uint32PODTraits {};
	template <> struct PODTraitsFromType<int32_t> : public Int32PODTraits {};
	template <> struct PODTraitsFromType<uint64_t> : public Uint64PODTraits {};
	template <> struct PODTraitsFromType<int64_t> : public Int64PODTraits {};
	template <> struct PODTraitsFromType<float16_t> : public Float16PODTraits {};
	template <> struct PODTraitsFromType<float32_t> : public Float32PODTraits {};
	template <> struct PODTraitsFromType<float64_t> : public Float64PODTraits {};
	template <> struct PODTraitsFromType<string> : public StringPODTraits {};
	template <> struct PODTraitsFromType<wstring> : public WstringPODTraits {};

	//-*****************************************************************************
	//-*****************************************************************************
	// Some runtime stuff, for when templates won't help.
	//-*****************************************************************************
	//-*****************************************************************************
	inline size_t PODNumBytes( PlainOldDataType pod )
	{
		switch ( pod )
		{
		case kBooleanPOD: return BooleanPODTraits::numBytes();
		case kUint8POD: return Uint8PODTraits::numBytes();
		case kInt8POD: return Int8PODTraits::numBytes();
		case kUint16POD: return Uint16PODTraits::numBytes();
		case kInt16POD: return Int16PODTraits::numBytes();
		case kUint32POD: return Uint32PODTraits::numBytes();
		case kInt32POD: return Int32PODTraits::numBytes();
		case kUint64POD: return Uint64PODTraits::numBytes();
		case kInt64POD: return Int64PODTraits::numBytes();
		case kFloat16POD: return Float16PODTraits::numBytes();
		case kFloat32POD: return Float32PODTraits::numBytes();
		case kFloat64POD: return Float64PODTraits::numBytes();
		case kStringPOD: return StringPODTraits::numBytes();
		case kWstringPOD: return WstringPODTraits::numBytes();
		default:
			// Badness!
			assert( false );
			return 0;
		};
	}


} // End namespace ALEMBIC_VERSION_NS
	using namespace ALEMBIC_VERSION_NS;
}// End namespace Util
}// End namespace Alembic

#else

#include <Alembic/Util/PlainOldDataType.h>

#endif // !ABCFRAMEWORK_IMPL

#endif