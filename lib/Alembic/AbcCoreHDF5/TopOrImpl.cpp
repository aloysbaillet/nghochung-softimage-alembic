//-*****************************************************************************
//
// Copyright (c) 2009-2010,
//  Sony Pictures Imageworks Inc. and
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
// Industrial Light & Magic, nor the names of their contributors may be used
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

#include <Alembic/AbcCoreHDF5/TopOrImpl.h>
#include <Alembic/AbcCoreHDF5/ArImpl.h>
#include <Alembic/AbcCoreHDF5/ReadUtil.h>
#include <Alembic/AbcCoreHDF5/HDF5Util.h>

namespace Alembic {
namespace AbcCoreHDF5 {
namespace ALEMBIC_ABCHDF5_VERSION_NS {

//-*****************************************************************************
//-*****************************************************************************
// OBJECT READER IMPLEMENTATION
//-*****************************************************************************
//-*****************************************************************************

//-*****************************************************************************
// This means we're reading the top group.
// The Top Group has a special name.
TopOrImpl::TopOrImpl( ArImpl &iArchive,
                      hid_t iRootGroup )
  : BaseOrImpl( MakeProtoObjectReaderPtr( iRootGroup, "", "ABC" ) )
  , m_archiveRef( iArchive )
{
    // Check validity of all inputs.
    ABCA_ASSERT( iRootGroup >= 0, "Invalid parent (archive) root group" );
}

//-*****************************************************************************
AbcA::ArchiveReaderPtr TopOrImpl::getArchive()
{
    return m_archiveRef.asArchivePtr();
}

//-*****************************************************************************
AbcA::ObjectReaderPtr TopOrImpl::getParent()
{
    return AbcA::ObjectReaderPtr();
}

//-*****************************************************************************
AbcA::ObjectReaderPtr TopOrImpl::asObjectPtr()
{
    return m_archiveRef.getTop();
}

//-*****************************************************************************
TopOrImpl::~TopOrImpl()
{
    // Nothing.
}

} // End namespace ALEMBIC_ABCHDF5_VERSION_NS
} // End namespace AbcCoreHDF5
} // End namespace Alembic
