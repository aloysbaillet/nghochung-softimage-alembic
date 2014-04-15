//*****************************************************************************
/*!
	\file AbcFrameworkCoreAbstractDataType.h
	\brief Expose Alembic struct AbcCoreAbstract::DataType, which is used
	to describe how a data element of a sample is stored.

	Copyright 2013 Autodesk, Inc.  All rights reserved.
	Use of this software is subject to the terms of the Autodesk license agreement
	provided at the time of installation or download, or which otherwise accompanies
	this software in either electronic or hard copy form.
*/
//*

#ifndef ABCFRAMEWORKCOREABSTRACTDATATYPE_H
#define ABCFRAMEWORKCOREABSTRACTDATATYPE_H

#include "AbcFrameworkPlainOldDataType.h"

#ifndef ABCFRAMEWORK_IMPL

namespace Alembic {
namespace Util {
	namespace ALEMBIC_VERSION_NS {
			// similiar to boost::totally_ordered
			// only need < and == operators and this fills in the rest
			template < class T >
			class totally_ordered
			{
				friend bool operator > ( const T& x, const T& y )
				{
					return y < x;
				}

				friend bool operator <= ( const T& x, const T& y )
				{
					return !( y < x );
				}

				friend bool operator >= ( const T& x, const T& y )
				{
					return !( x < y );
				}

				friend bool operator != ( const T& x, const T& y )
				{
					return !( x == y );
				}
			};  
	} // End namespace ALEMBIC_VERSION_NS
		using namespace ALEMBIC_VERSION_NS;
	} // End namespace Util

namespace AbcCoreAbstract {
	namespace ALEMBIC_VERSION_NS {
			//-*****************************************************************************
			//! The DataType class is a description of how an element of a sample in a
			//! Scalar or an Array property is stored. It does not contain an interpretation
			//! this is left to the metadata of the properties themselves.
			class DataType : Alembic::Util::totally_ordered<DataType>
			{
			public:
				//! Default constructor
				//! Sets the DataType to an unknown DataType with extent 0.
				//! This is obviously an invalid storage description, and is used in
				//! cases where we need to indicated that a DataType could not be
				//! determined.
				DataType()
					: m_pod( Alembic::Util::kUnknownPOD ), m_extent( 0 ) {}

				//! Explicit constructor.
				//! Takes a pod and an extent.
				//! By default the extent is 1.
				//! For String and Wstring types, the extent _must_ be 1.
				explicit DataType( Alembic::Util::PlainOldDataType iPod, uint8_t iExtent = 1 )
					: m_pod( iPod ), m_extent( iExtent ) {}

				//! Default copy constructor used.
				//! Default assignment operator used.

				//! Return the PlainOldDataType enum
				//! ...
				Alembic::Util::PlainOldDataType getPod() const { return m_pod; }

				//! Set the PlainOldDataType
				//! ...
				void setPod( Alembic::Util::PlainOldDataType iPod ) { m_pod = iPod; }

				//! Return the 8-bit extent
				//! ...
				uint8_t getExtent() const { return m_extent; }

				//! Set the 8-bit extent
				//! ...
				void setExtent( uint8_t iExtent ) { m_extent = iExtent; }

				//! Returns the number of bytes occupied by a single datum. (element)
				//! The assumption that each element has a fixed size in memory is a
				//! core assumption in Alembic.
				//!
				//! String DataTypes are a troublesome problem. A single string datum
				//! does not have a fixed number of bytes associated with it. So we
				//! are returning, here, the size of the std::string and std::wstring
				//! datatypes, respectively.
				size_t getNumBytes() const
				{
					return Alembic::Util::PODNumBytes( m_pod ) * ( size_t )m_extent;
				}

				//! Equality operator
				//! ...
				bool operator==( const DataType &b ) const
				{
					return ( ( m_pod == b.m_pod ) &&
						( m_extent == b.m_extent ) );
				}

				//-*************************************************************************
				//! Returns whether one datatype is lexigraphically "less" than
				//! another - this has meaning only so that DataType instances can
				//! be meaningfully sorted.
				bool operator<( const DataType &b ) const
				{
					if ( m_pod < b.m_pod ) { return true; }
					else if ( m_pod > b.m_pod ) { return false; }
					else { return ( m_extent < b.m_extent ); }
				}

			private:
				//! An Enum indicating which PlainOldDataType is our
				//! super-storage-class.
				Alembic::Util::PlainOldDataType m_pod;

				//! An 8-bit extent indicating the cardinality of
				//! a single element of data.
				uint8_t m_extent;
			};
	} // End namespace ALEMBIC_VERSION_NS
		using namespace ALEMBIC_VERSION_NS;
	} // End namespace AbcCoreAbstract
}
#endif //ABCFRAMEWORK_IMPL

#endif