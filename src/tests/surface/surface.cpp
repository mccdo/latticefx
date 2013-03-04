/*************** <auto-copyright.rb BEGIN do not edit this line> **************
 *
 * Copyright 2012-2012 by Ames Laboratory
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 *************** <auto-copyright.rb END do not edit this line> ***************/

#include <latticefx/core/DataSet.h>
#include <latticefx/core/ChannelData.h>
#include <latticefx/core/ChannelDataOSGArray.h>
#include <latticefx/core/SurfaceRenderer.h>
#include <latticefx/core/TransferFunctionUtils.h>
#include <latticefx/core/Log.h>
#include <latticefx/core/LogMacros.h>

#include <osgViewer/Viewer>
#include <osgGA/TrackballManipulator>

#include <boost/foreach.hpp>

#include <osg/io_utils>
#include <iostream>


using namespace lfx::core;


const std::string logstr( "lfx.demo" );


class ScalarComputation : public RTPOperation
{
public:
    ScalarComputation() : RTPOperation( RTPOperation::Channel ) {}
    ScalarComputation( const ScalarComputation& rhs ) : RTPOperation( rhs ) {}

    ChannelDataPtr channel( const ChannelDataPtr maskIn )
    {
        ChannelDataPtr warpData( _inputs[ 0 ] );
        osg::Array* warpBaseArray( warpData->asOSGArray() );
        osg::Vec3Array* warpArray( static_cast< osg::Vec3Array* >( warpBaseArray ) );

        // Create an array of the warp lengths.
        osg::FloatArray* scalar( new osg::FloatArray );
        osg::Vec3Array::const_iterator it;
        for( it=warpArray->begin(); it != warpArray->end(); ++it )
        {
            const float l( it->length() );
            scalar->push_back( l );
        }

        return( ChannelDataOSGArrayPtr( new ChannelDataOSGArray( "scalar", scalar ) ) );
    }
};



const int triangleCount( 5 );

void subtract( osg::Vec3Array* a, const osg::Vec3Array* b )
{
    for( unsigned int idx=0; idx<a->size(); ++idx )
        (*a)[ idx ] -= (*b)[ idx ];
}

void createTriangles( osg::Vec3Array* verts, osg::Vec3Array* norms )
{
    const float deltaX( 2.f / float( triangleCount ) );

    for( int idx=0; idx<triangleCount; idx++ )
    {
        const float xVal( float( idx ) / float( triangleCount ) * 2.f - 1.f );
        verts->push_back( osg::Vec3( xVal, 0., 0. ) );
        verts->push_back( osg::Vec3( xVal+deltaX, 0., 0. ) );
        verts->push_back( osg::Vec3( xVal, 0., deltaX ) );
        norms->push_back( osg::Vec3( 0., -1., 0. ) );
        norms->push_back( osg::Vec3( 0., -1., 0. ) );
        norms->push_back( osg::Vec3( 0., -1., 0. ) );
    }
}
void createWarpTriangles( osg::Vec3Array* verts, osg::Vec3Array* norms )
{
    const float deltaX( 2.f / float( triangleCount ) );

    for( int idx=0; idx<triangleCount; idx++ )
    {
        const float theta0( osg::PI * float( idx ) / float( triangleCount ) );
        const osg::Matrix m0( osg::Matrix::rotate( theta0, osg::Vec3( 0., 0., 1. ) ) );
        const float theta1( osg::PI * float( idx+1 ) / float( triangleCount ) );
        const osg::Matrix m1( osg::Matrix::rotate( theta1, osg::Vec3( 0., 0., 1. ) ) );

        const osg::Vec3 baseVec( osg::Vec3( -1., 0., 0. ) );
        const osg::Vec3 a( baseVec * m0 );
        const osg::Vec3 b( baseVec * m1 );

        verts->push_back( a );
        verts->push_back( b );
        verts->push_back( osg::Vec3( a.x(), a.y(), deltaX ) );
        norms->push_back( a );
        norms->push_back( b );
        norms->push_back( a );
    }
}

DataSetPtr prepareDataSet()
{
    osg::ref_ptr< osg::Vec3Array > verts( new osg::Vec3Array );
    osg::ref_ptr< osg::Vec3Array > norms( new osg::Vec3Array );
    createTriangles( verts.get(), norms.get() );
    ChannelDataOSGArrayPtr cdv( new ChannelDataOSGArray( "vertices", verts.get() ) );
    ChannelDataOSGArrayPtr cdn( new ChannelDataOSGArray( "normals", norms.get() ) );

    osg::ref_ptr< osg::Vec3Array > warpVerts( new osg::Vec3Array );
    osg::ref_ptr< osg::Vec3Array > warpNorms( new osg::Vec3Array );
    createWarpTriangles( warpVerts.get(), warpNorms.get() );
    subtract( warpVerts.get(), verts.get() );
    subtract( warpNorms.get(), norms.get() );
    ChannelDataOSGArrayPtr cdwv( new ChannelDataOSGArray( "warp vertices", warpVerts.get() ) );
    ChannelDataOSGArrayPtr cdwn( new ChannelDataOSGArray( "warp normals", warpNorms.get() ) );

    // Create a data set and add the vertex data.
    DataSetPtr dsp( new DataSet() );
    dsp->addChannel( cdv );
    dsp->addChannel( cdn );
    dsp->addChannel( cdwv );
    dsp->addChannel( cdwn );

    // Add RTP operation to create a scalar channel to use as input to the transfer function.
    ScalarComputation* sc( new ScalarComputation() );
    sc->addInput( cdwv->getName() );
    dsp->addOperation( RTPOperationPtr( sc ) );

    SurfaceRendererPtr renderOp( new SurfaceRenderer() );
    renderOp->setInputNameAlias( SurfaceRenderer::VERTEX, cdv->getName() );
    renderOp->setInputNameAlias( SurfaceRenderer::NORMAL, cdn->getName() );
    renderOp->setInputNameAlias( SurfaceRenderer::WARP_VERTEX, cdwv->getName() );
    renderOp->setInputNameAlias( SurfaceRenderer::WARP_NORMAL, cdwn->getName() );

    renderOp->setTransferFunctionInput( "scalar" );
    renderOp->setTransferFunction( loadImageFromDat( "01.dat" ) );
    renderOp->setTransferFunctionDestination( Renderer::TF_RGBA );

    renderOp->addInput( cdv->getName() );
    renderOp->addInput( cdn->getName() );
    renderOp->addInput( cdwv->getName() );
    renderOp->addInput( cdwn->getName() );
    renderOp->addInput( "scalar" );
    dsp->setRenderer( renderOp );

    return( dsp );
}



int main( int argc, char** argv )
{
    Log::instance()->setPriority( Log::PrioInfo, Log::Console );

    DataSetPtr dsp( prepareDataSet() );
    osg::Node* lfxParent( dsp->getSceneData() );
    // By calling getSceneGraph() before dumpUniformInfo(), we allow the
    // Renderer to establish correct default values for its uniforms, many
    // of which can't be determined until the DataSet is completely processed.
    dsp->getRenderer()->dumpUniformInfo( std::cout );

    osg::Group* root( new osg::Group );
    root->addChild( lfxParent );

    osg::ref_ptr< osg::Uniform > warpScale( new osg::Uniform( "warpScale", 0.f ) );
    warpScale->setDataVariance( osg::Object::DYNAMIC );
    root->getOrCreateStateSet()->addUniform( warpScale.get(),
        osg::StateAttribute::OVERRIDE );

    osgViewer::Viewer viewer;
    viewer.setUpViewInWindow( 20, 30, 800, 460 );
    viewer.setCameraManipulator( new osgGA::TrackballManipulator() );
    viewer.setSceneData( root );

    double lastTime( 0. );
    while( !viewer.done() )
    {
        const double currentTime( viewer.getFrameStamp()->getSimulationTime() );
        const double elapsed( currentTime - lastTime );
        lastTime = currentTime;

        float scale;
        warpScale->get( scale );
        scale += (float)elapsed;
        if( scale > 1.f )
            scale -= 1.f;
        warpScale->set( scale );

        viewer.frame();
    }
    return( 0 );
}
