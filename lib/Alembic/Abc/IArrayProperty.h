//-*****************************************************************************
//
// Copyright (c) 2009-2010,
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
#ifndef _Alembic_Abc_IArrayProperty_h_
#define _Alembic_Abc_IArrayProperty_h_

#include <Alembic/Abc/Foundation.h>
#include <Alembic/Abc/Base.h>
#include <Alembic/Abc/IArgument.h>
#include <Alembic/Abc/ISampleSelector.h>
#include <Alembic/Abc/IBaseProperty.h>
#include <Alembic/Abc/ICompoundProperty.h>

namespace Alembic {
namespace Abc {

//-*****************************************************************************
class IArrayProperty
    : public IBasePropertyT<AbcA::ArrayPropertyReaderPtr>
{
public:
    //! By convention we always define this_type in Abc classes
    //! Used by unspecified-bool-type conversion below
    typedef IArrayProperty this_type;

    //-*************************************************************************
    // CONSTRUCTION, DESTRUCTION, ASSIGNMENT
    //-*************************************************************************

    //! The default constructor creates an empty IArrayProperty function set.
    //! ...
    IArrayProperty() : IBasePropertyT<AbcA::ArrayPropertyReaderPtr>() {}

    //! This templated, explicit function creates a new array property reader.
    //! The first argument is any Abc (or AbcCoreAbstract) object
    //! which can intrusively be converted to a CompoundPropertyReaderPtr
    //! to use as a parent, from which the error handler policy for
    //! inheritance is also derived.  The remaining optional arguments
    //! can be used to override the ErrorHandlerPolicy, to specify
    //! protocol matching policy, and that's it.
    template <class OBJECT_PTR>
    IArrayProperty( OBJECT_PTR iParentObject,
                    const std::string &iName,

                    const IArgument &iArg0 = IArgument(),
                    const IArgument &iArg1 = IArgument() );

    //! This attaches an IArrayProperty wrapper around an existing
    //! ArrayPropertyReaderPtr, with an optional error handling policy.
    IArrayProperty(
        //! The pointer
        //! ...
        AbcA::ArrayPropertyReaderPtr iPtr,

        //! The flag indicating that wrapping is intended.
        //! Even though it's nonambiguous here, we use it anyway
        //! for readability
        WrapExistingFlag iWrapFlag,

        //! Optional error handling policy
        //! ...
        ErrorHandler::Policy iPolicy = ErrorHandler::kThrowPolicy )
      : IBasePropertyT<AbcA::ArrayPropertyReaderPtr>( iPtr,
                                                      iWrapFlag,
                                                      iPolicy ) {}

    //! Default copy constructor used
    //! Default assignment operator used.

    //! Destructor
    //! ...
    ~IArrayProperty();

    //-*************************************************************************
    // ARRAY PROPERTY READER FUNCTIONALITY
    //-*************************************************************************

    //! Return the number of samples contained in the property.
    //! This can be any number, including zero.
    //! This returns the number of samples that were written, independently
    //! of whether or not they were constant.
    size_t getNumSamples();

    //! Ask if we're constant - no change in value amongst samples,
    //! regardless of the time sampling.
    bool isConstant();

    //! Ask if we are like a scalar - we have 1 and only 1 element (which
    //! may have an extent greater than one, eg, a single V3f in a
    //! V3fArrayProperty) per sample.  Alternately, an ArrayProperty with
    //! no samples at all is also considered scalar-like.
    bool isScalarLike();

    //! Time information.
    //! This will be valid regardless of TimeSamplingType or number of samples.
    AbcA::TimeSampling getTimeSampling();

    //! Get a sample into the address of a datum.
    //! ...
    void get( AbcA::ArraySamplePtr& oSample,
              const ISampleSelector &iSS = ISampleSelector() );

    //! Get a key from an address of a datum.
    //! ...
    bool getKey( AbcA::ArraySampleKey& oKey,
              const ISampleSelector &iSS = ISampleSelector() );

    //! Return the parent compound property, handily wrapped in a
    //! ICompoundProperty wrapper.
    ICompoundProperty getParent();

private:
    void init( AbcA::CompoundPropertyReaderPtr iParentObject,
               const std::string &iName,

               ErrorHandler::Policy iParentPolicy,

               const IArgument &iArg0,
               const IArgument &iArg1 );
};

//-*****************************************************************************
// TEMPLATE AND INLINE FUNCTIONS
//-*****************************************************************************

//-*****************************************************************************
template <class CPROP_PTR>
inline IArrayProperty::IArrayProperty( CPROP_PTR iParentProp,
                                       const std::string &iName,
                                       const IArgument &iArg0,
                                       const IArgument &iArg1 )
{
    init( GetCompoundPropertyReaderPtr( iParentProp ),
          iName,

          GetErrorHandlerPolicy( iParentProp ),
          iArg0, iArg1 );
}

} // End namespace Abc
} // End namespace Alembic

#endif
