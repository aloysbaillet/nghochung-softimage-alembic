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

#include <Alembic/AbcGeom/All.h>
#include <Alembic/AbcCoreHDF5/All.h>
#include <Alembic/Abc/Tests/Assert.h>

#include <ImathMath.h>

#include <limits>

static const double VAL_EPSILON = std::numeric_limits<double>::epsilon() \
    * 1024.0;

bool almostEqual( const double &a, const double &b,
                  const double &epsilon = VAL_EPSILON )
{
    return Imath::equalWithAbsError( a, b, epsilon );
}

using namespace Alembic::AbcGeom;

//-*****************************************************************************
void xformOut()
{
    OArchive archive( Alembic::AbcCoreHDF5::WriteArchive(), "Xform1.abc" );
    OXform a( OObject( archive, kTop ), "a" );
    OXform b( a, "b" );
    OXform c( b, "c" );
    OXform d( c, "d" );
    OXform e( d, "e" );

    XformOp transop( kTranslateOperation, kTranslateHint );
    XformOp scaleop( kScaleOperation, kScaleHint );

    XformSample asamp;
    for ( size_t i = 0; i < 20; ++i )
    {
        asamp.addOp( transop, V3d( 12.0, i + 42.0, 20.0 ) );

        a.getSchema().set( asamp, OSampleSelector( i ) );
    }

    XformSample bsamp;
    for ( size_t i = 0 ; i < 20 ; ++i )
    {
        bsamp.setInheritsXforms( (bool)(i&1) );

        b.getSchema().set( bsamp, OSampleSelector( i ) );
    }

    // for c we write nothing

    XformSample dsamp;
    dsamp.addOp( scaleop, V3d( 3.0, 6.0, 9.0 ) );
    d.getSchema().set( dsamp );

    XformSample esamp;
    M44d identmat;
    identmat.makeIdentity();

    esamp.addOp( transop, V3d( 0.0, 0.0, 0.0 ) );
    esamp.addOp( XformOp( kMatrixOperation, kMatrixHint ), identmat );
    esamp.addOp( scaleop, V3d( 1.0, 1.0, 1.0 ) );
    e.getSchema().set( esamp );

}

//-*****************************************************************************
void xformIn()
{
    IArchive archive( Alembic::AbcCoreHDF5::ReadArchive(), "Xform1.abc" );

    Abc::M44d identity;
    XformSample xs;

    IXform a( IObject( archive, kTop ), "a" );
    TESTING_ASSERT( a.getSchema().getNumOps() == 1 );
    TESTING_ASSERT( a.getSchema().getInheritsXforms() );
    for ( index_t i = 0; i < 20; ++i )
    {
        XformSample xs;
        a.getSchema().get( xs, Abc::ISampleSelector( i ) );
        TESTING_ASSERT( xs.getNumOps() == 1 );
        TESTING_ASSERT( xs[0].isTranslateOp() );
        TESTING_ASSERT( xs[0].isYAnimated() == true );
        TESTING_ASSERT( xs[0].isXAnimated() == false );
        TESTING_ASSERT( xs[0].isZAnimated() == false );

        TESTING_ASSERT( xs.getTranslation() == V3d( 12.0, i+42.0, 20.0 ) );
        TESTING_ASSERT( xs.getMatrix() ==
                        Abc::M44d().setTranslation( V3d(12.0, i+42.0, 20.0)) );
    }

    IXform b( a, "b" );
    b.getSchema().get( xs );
    TESTING_ASSERT( b.getSchema().getTimeSampling().getTimeSamplingType().isIdentity() );
    // the schema is not static, because set() was called 20 times on it.
    TESTING_ASSERT( !b.getSchema().getTimeSampling().isStatic() );
    TESTING_ASSERT( xs.getNumOps() == 0 );
    TESTING_ASSERT( b.getSchema().getNumOps() == 0 );
    TESTING_ASSERT( xs.getMatrix() == identity );
    for (size_t i = 0; i < 20; ++i)
    {
        AbcA::index_t j = i;
        TESTING_ASSERT( b.getSchema().getInheritsXforms( ISampleSelector( j ) )
                        == (i&1) );
    }

    IXform c( b, "c" );
    xs = c.getSchema().getValue();
    TESTING_ASSERT( xs.getNumOps() == 0 );
    TESTING_ASSERT( c.getSchema().getNumOps() == 0 );
    TESTING_ASSERT( xs.getMatrix() == identity );
    TESTING_ASSERT( c.getSchema().getInheritsXforms() );
    TESTING_ASSERT( c.getSchema().isConstantIdentity() );


    IXform d( c, "d" );
    xs = d.getSchema().getValue();
    TESTING_ASSERT( xs.getNumOps() == 1 );
    TESTING_ASSERT( d.getSchema().getNumOps() == 1 );
    TESTING_ASSERT( xs[0].isScaleOp() );
    TESTING_ASSERT( ! ( xs[0].isXAnimated() || xs[0].isYAnimated()
                        || xs[0].isZAnimated() ) );
    TESTING_ASSERT( xs.getScale().equalWithAbsError( V3d( 3.0, 6.0, 9.0 ),
                                                     VAL_EPSILON ) );
    TESTING_ASSERT( xs.getMatrix() ==
                    Abc::M44d().setScale( V3d(3.0, 6.0, 9.0)) );
    TESTING_ASSERT( d.getSchema().getInheritsXforms() );

    IXform e( d, "e" );
    TESTING_ASSERT( e.getSchema().isConstantIdentity() );
    TESTING_ASSERT( e.getSchema().isConstant() );
    TESTING_ASSERT( e.getSchema().getNumOps() == 3 );
}

//-*****************************************************************************
void someOpsXform()
{
    std::string name = "Xform2.abc";
    {
        OArchive archive( Alembic::AbcCoreHDF5::WriteArchive(), name );

        archive.setCompressionHint( 3 );

        OXform a( OObject( archive, kTop ), "a" );

        XformOp transop( kTranslateOperation, kTranslateHint );
        XformOp scaleop( kScaleOperation, kScaleHint );

        XformSample asamp;

        // scale
        asamp.addOp( scaleop, V3d( 2.0, 1.0, 2.0 ) );

        // Maya-like shear
        XformOp shearmatrixop( kMatrixOperation, kMayaShearHint );
        M44d shearmat;
        shearmat.makeIdentity();

        asamp.addOp( shearmatrixop, shearmat );

        // rotate x axis
        XformOp rotop( kRotateOperation, kRotateHint );
        asamp.addOp( rotop, V3d( 1.0, 0.0, 0.0 ), 1.57 );

        // rotate y axis, angle will be animated
        asamp.addOp( rotop, V3d( 0.0, 1.0, 0.0 ), 0.125 );

        // rotate z axis, use a different hint for fun
        XformOp rotorientop( kRotateOperation, kRotateOrientationHint );
        asamp.addOp( rotorientop, V3d( 0.0, 0.0, 1.0 ), 0.1 );

        // translate with animated y and z, different hint for fun
        XformOp transpivotop( kTranslateOperation, kRotatePivotPointHint );
        asamp.addOp( transpivotop, V3d( 0.0, 0.0, 0.0 ) );

        a.getSchema().set( asamp, OSampleSelector( 0 ) );

        for (size_t i = 1; i < 5; ++i)
        {
            asamp.addOp( scaleop, V3d( 2 * ( i + 1 ),
                                       1.0, 2.0 ) );

            shearmat.x[1][0] = (double)i;
            shearmat.x[2][0] = (double)( (int)i * -1.0 );
            shearmat.x[2][1] = 0.0;

            asamp.addOp( shearmatrixop, shearmat );

            asamp.addOp( rotop, V3d( 1.0, 0.0, 0.0 ),
                         1.57 );
            asamp.addOp( rotop, V3d( 0.0, 1.0, 0.0 ),
                         0.125 * ( i + 1 ) );
            asamp.addOp( rotorientop, V3d( 0.0, 0.0, 1.0 ),
                         0.1 * ( i + 1 ) );

            asamp.addOp( transpivotop, V3d( 0.0, 3.0 * i, 4.0 * i ) );

            a.getSchema().set( asamp, OSampleSelector( i ) );
        }

    }

    {
        IArchive archive( Alembic::AbcCoreHDF5::ReadArchive(), name );
        IXform a( IObject( archive, kTop ), "a" );

        XformSample asamp;

        a.getSchema().get( asamp );

        TESTING_ASSERT( a.getSchema().getNumOps() == 6 );

        TESTING_ASSERT( asamp[0].isScaleOp() );
        TESTING_ASSERT( asamp[0].getHint() == kScaleHint );

        TESTING_ASSERT( asamp[1].isMatrixOp() );
        TESTING_ASSERT( asamp[1].getHint() == kMayaShearHint );


        TESTING_ASSERT( asamp[2].isRotateOp() );
        TESTING_ASSERT( asamp[2].getHint() == kRotateHint );

        TESTING_ASSERT( asamp[3].getType() == kRotateOperation );
        TESTING_ASSERT( asamp[3].getHint() == kRotateHint );

        TESTING_ASSERT( asamp[4].getType() == kRotateOperation );
        TESTING_ASSERT( asamp[4].getHint() == kRotateOrientationHint );

        TESTING_ASSERT( asamp[5].getType() == kTranslateOperation );
        TESTING_ASSERT( asamp[5].getHint() == kRotatePivotPointHint );

        TESTING_ASSERT( asamp[0].isXAnimated() );
        TESTING_ASSERT( !asamp[0].isYAnimated() );
        TESTING_ASSERT( !asamp[0].isZAnimated() );

        TESTING_ASSERT( !asamp[1].isChannelAnimated(0) );  // [0][0]
        TESTING_ASSERT( !asamp[1].isChannelAnimated(1) );  // [0][1]
        TESTING_ASSERT( !asamp[1].isChannelAnimated(2) );  // [0][2]
        TESTING_ASSERT( !asamp[1].isChannelAnimated(3) );  // [0][3]
        TESTING_ASSERT( asamp[1].isChannelAnimated(4) );   // [1][0]
        TESTING_ASSERT( !asamp[1].isChannelAnimated(5) );  // [1][1]
        TESTING_ASSERT( !asamp[1].isChannelAnimated(6) );  // [1][2]
        TESTING_ASSERT( !asamp[1].isChannelAnimated(7) );  // [1][3]
        TESTING_ASSERT( asamp[1].isChannelAnimated(8) );   // [2][0]
        TESTING_ASSERT( !asamp[1].isChannelAnimated(9) );  // [2][1]
        TESTING_ASSERT( !asamp[1].isChannelAnimated(10) ); // [2][2]
        TESTING_ASSERT( !asamp[1].isChannelAnimated(11) ); // [2][3]
        TESTING_ASSERT( !asamp[1].isChannelAnimated(12) ); // [3][0]
        TESTING_ASSERT( !asamp[1].isChannelAnimated(13) ); // [3][1]
        TESTING_ASSERT( !asamp[1].isChannelAnimated(14) ); // [3][2]
        TESTING_ASSERT( !asamp[1].isChannelAnimated(15) ); // [3][3]

        TESTING_ASSERT( !asamp[2].isXAnimated() );
        TESTING_ASSERT( !asamp[2].isYAnimated() );
        TESTING_ASSERT( !asamp[2].isZAnimated() );
        TESTING_ASSERT( !asamp[2].isAngleAnimated() );

        TESTING_ASSERT( !asamp[3].isXAnimated() );
        TESTING_ASSERT( !asamp[3].isYAnimated() );
        TESTING_ASSERT( !asamp[3].isZAnimated() );
        TESTING_ASSERT( asamp[3].isAngleAnimated() );

        TESTING_ASSERT( !asamp[4].isXAnimated() );
        TESTING_ASSERT( !asamp[4].isYAnimated() );
        TESTING_ASSERT( !asamp[4].isZAnimated() );
        TESTING_ASSERT( asamp[4].isAngleAnimated() );

        TESTING_ASSERT( !asamp[5].isXAnimated() );
        TESTING_ASSERT( asamp[5].isYAnimated() );
        TESTING_ASSERT( asamp[5].isZAnimated() );

        // OK, now check the values came through
        M44d shearmat;
        shearmat.makeIdentity();

        TESTING_ASSERT( asamp[0].getScale() == V3d( 2.0, 1.0, 2.0 ) );

        TESTING_ASSERT( asamp[1].getMatrix() == shearmat );

        TESTING_ASSERT( asamp[2].getAxis() == V3d( 1.0, 0.0, 0.0 ) );
        TESTING_ASSERT( almostEqual( asamp[2].getAngle(), 1.57 ) );

        TESTING_ASSERT( asamp[3].getAxis() == V3d( 0.0, 1.0, 0.0 ) );
        TESTING_ASSERT( almostEqual( asamp[3].getAngle(), 0.125 ) );

        TESTING_ASSERT( asamp[4].getAxis() == V3d( 0.0, 0.0, 1.0 ) );
        TESTING_ASSERT( almostEqual( asamp[4].getAngle(), 0.1 ) );

        TESTING_ASSERT( asamp[5].getTranslate() == V3d( 0.0, 0.0, 0.0 ) );

        for ( index_t i = 1; i < 5; ++i )
        {
            a.getSchema().get( asamp, ISampleSelector( i ) );

            TESTING_ASSERT( asamp[0].getScale()
                            == V3d( 2 * ( i + 1 ), 1.0, 2.0 ) );

            shearmat.x[1][0] = (double)i;
            shearmat.x[2][0] = (double)( (int)i * -1.0 );
            shearmat.x[2][1] = 0.0;

            TESTING_ASSERT( asamp[1].getMatrix() == shearmat );

            TESTING_ASSERT( asamp[2].getAxis() == V3d( 1.0, 0.0, 0.0 ) );
            TESTING_ASSERT( almostEqual( asamp[2].getAngle(), 1.57 ) );

            TESTING_ASSERT( asamp[3].getAxis() == V3d( 0.0, 1.0, 0.0 ) );
            TESTING_ASSERT( almostEqual( asamp[3].getAngle(),
                                         0.125 * ( i + 1 ) ) );

            TESTING_ASSERT( asamp[4].getAxis() == V3d( 0.0, 0.0, 1.0 ) );
            TESTING_ASSERT( almostEqual( asamp[4].getAngle(),
                                         0.1 * ( i + 1 ) ) );

            V3d tvec( 0.0, 3.0 * i, 4.0 * i );

            TESTING_ASSERT( tvec.equalWithAbsError( asamp[5].getTranslate(),
                                                    VAL_EPSILON ) );
        }
    }
}

//-*****************************************************************************
int main( int argc, char *argv[] )
{
    xformOut();
    xformIn();
    someOpsXform();

    return 0;
}
