/*************** <auto-copyright.rb BEGIN do not edit this line> *************
*
* latticeFX is (C) Copyright 2011-2012 by Ames Laboratory
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Library General Public
* License as published by the Free Software Foundation; 
* version 2.1 of the License.
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
* -----------------------------------------------------------------
* Date modified: $Date$
* Version:       $Rev$
* Author:        $Author$
* Id:            $Id$
* -----------------------------------------------------------------
*
*************** <auto-copyright.rb END do not edit this line> **************/

#include <latticefx/core/DataSet.h>
#include <latticefx/core/ChannelData.h>
#include <latticefx/core/ChannelDataOSGArray.h>
#include <latticefx/core/RTPOperation.h>
#include <latticefx/core/VectorRenderer.h>
#include <latticefx/core/TransferFunctionUtils.h>
#include <latticefx/core/PlayControl.h>
#include <latticefx/core/Log.h>
#include <latticefx/core/LogMacros.h>

#include <osgViewer/Viewer>
#include <osgGA/TrackballManipulator>
#include <osg/ClipNode>

#include <iostream>
#include <sstream>



const std::string logstr( "lfx.demo" );



class DepthComputation : public lfx::RTPOperation
{
public:
    DepthComputation() : lfx::RTPOperation( lfx::RTPOperation::Channel ) {}
    DepthComputation( const DepthComputation& rhs ) : lfx::RTPOperation( rhs ) {}

    lfx::ChannelDataPtr channel( const lfx::ChannelDataPtr maskIn )
    {
        lfx::ChannelDataPtr posData( getInput( "positions" ) );
        osg::Array* posBaseArray( posData->asOSGArray() );
        osg::Vec3Array* posArray( static_cast< osg::Vec3Array* >( posBaseArray ) );

        // Depth value is the z value. Create an array of the z values.
        osg::FloatArray* depth( new osg::FloatArray );
        osg::Vec3Array::const_iterator it;
        for( it=posArray->begin(); it != posArray->end(); ++it )
        {
            const float z( it->z() );
            depth->push_back( z );
        }

        return( lfx::ChannelDataOSGArrayPtr( new lfx::ChannelDataOSGArray( depth, "depth" ) ) );
    }
};


unsigned int computeDynamicPositions( osg::Vec3Array* a,
        const unsigned int w, const unsigned int h, const unsigned int d, const double t )
{
    a->resize( w*h*d );
    unsigned int index( 0 );
    unsigned int wIdx, hIdx, dIdx;
    for( wIdx=0; wIdx<w; ++wIdx )
    {
        for( hIdx=0; hIdx<h; ++hIdx )
        {
            for( dIdx=0; dIdx<d; ++dIdx )
            {
                const double x( ((double)wIdx)/(w-1.) * (double)w - (w*.5) );
                const double y( ((double)hIdx)/(h-1.) * (double)h - (h*.5) );
                const double z( ((double)dIdx)/(d-1.) * (double)d - (d*.5) );
                (*a)[ index ].set( x + sin( (x+y+t)*.8 ), y + sin( (x+y+t) ), z + sin( (x+y+t)*1.2 ) );
                ++index;
            }
        }
    }
    return( index );
}

lfx::DataSetPtr prepareSimplePoints()
{
    osg::ref_ptr< osg::Vec3Array > vertArray( new osg::Vec3Array );
    const unsigned int w( 73 ), h( 41 ), d( 11 );
    {
        std::ostringstream ostr;
        ostr << "Creating data set. Dimensions: " << w << " x " << h << " x " << d;
        LFX_INFO_STATIC( logstr, ostr.str() );
    }

    lfx::DataSetPtr dsp( new lfx::DataSet() );
    const double maxTime( 8. );
    const double sampleRate( 60. );
    {
        std::ostringstream ostr;
        ostr << "Creating time series data. " << maxTime << "s, sample rate: " << sampleRate << "hz.";
        LFX_INFO_STATIC( logstr, ostr.str() );
    }

    unsigned int totalSamples( 0 );
    double time;
    for( time=0.; time<maxTime; time += 1./sampleRate )
    {
        osg::ref_ptr< osg::Vec3Array > posArray( new osg::Vec3Array );
        unsigned int count( computeDynamicPositions( posArray.get(), w, h, d, time ) );
        totalSamples += count;
        lfx::ChannelDataOSGArrayPtr posData( lfx::ChannelDataOSGArrayPtr( new lfx::ChannelDataOSGArray( posArray.get(), "positions" ) ) );
        dsp->addChannel( posData, time );
    }
    {
        std::ostringstream ostr;
        ostr << "Total samples: " << totalSamples;
        LFX_INFO_STATIC( logstr, ostr.str() );
    }

    // Add RTP operation to create a depth channel to use as input to the transfer function.
    DepthComputation* dc( new DepthComputation() );
    dc->addInput( "positions" );
    dsp->addOperation( lfx::RTPOperationPtr( dc ) );


    lfx::VectorRendererPtr renderOp( new lfx::VectorRenderer() );
    renderOp->setPointStyle( lfx::VectorRenderer::SIMPLE_POINTS );
    renderOp->addInput( "positions" );
    renderOp->addInput( "depth" ); // From DepthComputation channel creator

    // Configure transfer function.
    renderOp->setTransferFunctionInput( "depth" );
    renderOp->setTransferFunction( lfx::loadImageFromDat( "01.dat" ) );
    renderOp->setTransferFunctionDestination( lfx::Renderer::TF_RGBA );

    dsp->setRenderer( renderOp );


    return( dsp );
}
lfx::DataSetPtr preparePointSprites()
{
    lfx::DataSetPtr dsp( (lfx::DataSet*) NULL );
    return( dsp );
}
lfx::DataSetPtr prepareSpheres()
{
    const unsigned int w( 15 ), h( 12 ), d( 9 );
    const unsigned int samplesPerTime( w*h*d );
    {
        std::ostringstream ostr;
        ostr << "Creating data set. Dimensions: " << w << " x " << h << " x " << d;
        LFX_INFO_STATIC( logstr, ostr.str() );
    }

    lfx::DataSetPtr dsp( new lfx::DataSet() );
    unsigned int totalSamples( 0 );

    const double maxTime( 8. );
    const double sampleRate( 30. );
    {
        std::ostringstream ostr;
        ostr << "Creating time series data. " << maxTime << "s, sample rate: " << sampleRate << "hz.";
        LFX_INFO_STATIC( logstr, ostr.str() );
    }

    double time;
    for( time=0.; time<maxTime; time += 1./sampleRate )
    {
        osg::ref_ptr< osg::Vec3Array > posArray( new osg::Vec3Array );
        unsigned int count( computeDynamicPositions( posArray.get(), w, h, d, time ) );
        totalSamples += count;
        lfx::ChannelDataOSGArrayPtr posData( lfx::ChannelDataOSGArrayPtr( new lfx::ChannelDataOSGArray( posArray.get(), "positions" ) ) );
        dsp->addChannel( posData, time );

        // Array of radius values.
        osg::ref_ptr< osg::FloatArray > radArray( new osg::FloatArray );
        radArray->resize( samplesPerTime );
        unsigned int wIdx, hIdx, dIdx, index( 0 );
        for( wIdx=0; wIdx<w; ++wIdx )
        {
            for( hIdx=0; hIdx<h; ++hIdx )
            {
                for( dIdx=0; dIdx<d; ++dIdx )
                {
                    const double x( ((double)wIdx)/(w-1.) );
                    const double y( ((double)hIdx)/(h-1.) );
                    const double rad( osg::absolute( sin( x+y+time ) ) ); 
                    (*radArray)[ index ] = rad * .33;
                    ++index;
                }
            }
        }
        lfx::ChannelDataOSGArrayPtr radData( lfx::ChannelDataOSGArrayPtr( new lfx::ChannelDataOSGArray( radArray.get(), "radii" ) ) );
        dsp->addChannel( radData, time );
    }
    {
        std::ostringstream ostr;
        ostr << "Total samples: " << totalSamples;
        LFX_INFO_STATIC( logstr, ostr.str() );
    }

    // Add RTP operation to create a depth channel to use as input to the transfer function.
    DepthComputation* dc( new DepthComputation() );
    dc->addInput( "positions" );
    dsp->addOperation( lfx::RTPOperationPtr( dc ) );

    lfx::VectorRendererPtr renderOp( new lfx::VectorRenderer() );
    renderOp->setPointStyle( lfx::VectorRenderer::SPHERES );
    renderOp->addInput( "positions" );
    renderOp->addInput( "radii" );
    renderOp->addInput( "depth" ); // From DepthComputation channel creator

    // Configure transfer function.
    renderOp->setTransferFunctionInput( "depth" );
    renderOp->setTransferFunction( lfx::loadImageFromDat( "01.dat" ) );
    renderOp->setTransferFunctionDestination( lfx::Renderer::TF_RGBA );

    // Configure hardware mask.
    renderOp->setHardwareMaskInputSource( lfx::Renderer::HM_SOURCE_RED );
    renderOp->setHardwareMaskOperator( lfx::Renderer::HM_OP_OFF );
    renderOp->setHardwareMaskReference( 0.f );

    dsp->setRenderer( renderOp );

    return( dsp );
}
lfx::DataSetPtr prepareDirectionVectors()
{
    osg::ref_ptr< osg::Vec3Array > vertArray( new osg::Vec3Array );
    const unsigned int w( 73 ), h( 41 ), d( 11 );
    {
        std::ostringstream ostr;
        ostr << "Creating data set. Dimensions: " << w << " x " << h << " x " << d;
        LFX_INFO_STATIC( logstr, ostr.str() );
    }

    unsigned int samplesPerTime( w*h*d );
    vertArray->resize( samplesPerTime );
    unsigned int wIdx, hIdx, dIdx, index( 0 );
    for( wIdx=0; wIdx<w; ++wIdx )
    {
        for( hIdx=0; hIdx<h; ++hIdx )
        {
            for( dIdx=0; dIdx<d; ++dIdx )
            {
                const float x( ((double)wIdx)/(w-1.) * (double)w - (w*.5) );
                const float y( ((double)hIdx)/(h-1.) * (double)h - (h*.5) );
                const float z( ((double)dIdx)/(d-1.) * (double)d - (d*.5) );
                (*vertArray)[ index ].set( x, y, z );
                ++index;
            }
        }
    }

    lfx::ChannelDataOSGArrayPtr vertData( new lfx::ChannelDataOSGArray( vertArray.get(), "positions" ) );
    lfx::DataSetPtr dsp( new lfx::DataSet() );
    dsp->addChannel( vertData );

    const double maxTime( 8. );
    const double sampleRate( 60. );
    {
        std::ostringstream ostr;
        ostr << "Creating time series data. " << maxTime << "s, sample rate: " << sampleRate << "hz.";
        LFX_INFO_STATIC( logstr, ostr.str() );
    }

    int count( 0 );
    double time;
    for( time=0.; time<maxTime; time += 1./sampleRate )
    {
        osg::ref_ptr< osg::Vec3Array > dirArray( new osg::Vec3Array );
        dirArray->resize( w*h*d );
        index = 0;
        for( wIdx=0; wIdx<w; ++wIdx )
        {
            for( hIdx=0; hIdx<h; ++hIdx )
            {
                for( dIdx=0; dIdx<d; ++dIdx )
                {
                    const float x( ((double)wIdx)/(w-1.) * (double)w - (w*.5) );
                    const float y( ((double)hIdx)/(h-1.) * (double)h - (h*.5) );
                    (*dirArray)[ index ].set( sin( x+y+time ), sin( (x+y+time)*1.2 ), .5 );
                    ++index;
                    ++count;
                }
            }
        }
        lfx::ChannelDataOSGArrayPtr dirData( lfx::ChannelDataOSGArrayPtr( new lfx::ChannelDataOSGArray( dirArray.get(), "directions" ) ) );
        dsp->addChannel( dirData, time );
    }
    {
        std::ostringstream ostr;
        ostr << "Total samples: " << count;
        LFX_INFO_STATIC( logstr, ostr.str() );
    }

    // Add RTP operation to create a depth channel to use as input to the transfer function.
    DepthComputation* dc( new DepthComputation() );
    dc->addInput( "positions" );
    dsp->addOperation( lfx::RTPOperationPtr( dc ) );

    lfx::VectorRendererPtr renderOp( new lfx::VectorRenderer() );
    renderOp->setPointStyle( lfx::VectorRenderer::DIRECTION_VECTORS );
    renderOp->addInput( "positions" );
    renderOp->addInput( "directions" );
    renderOp->addInput( "depth" ); // From DepthComputation channel creator

    // Configure transfer function.
    renderOp->setTransferFunctionInput( "depth" );
    renderOp->setTransferFunction( lfx::loadImageFromDat( "01.dat" ) );
    renderOp->setTransferFunctionDestination( lfx::Renderer::TF_RGBA );

    dsp->setRenderer( renderOp );

    return( dsp );
}

lfx::DataSetPtr prepareDataSet( const lfx::VectorRenderer::PointStyle& style )
{
    lfx::DataSetPtr dataSet;
    switch( style )
    {
    case lfx::VectorRenderer::POINT_SPRITES:
        LFX_NOTICE_STATIC( logstr, "point sprites not yet implemented." );
    default:
    case lfx::VectorRenderer::SIMPLE_POINTS:
        dataSet = prepareSimplePoints();
        break;
//    case lfx::VectorRenderer::POINT_SPRITES:
//        dataSet = preparePointSprites();
//        break;
    case lfx::VectorRenderer::SPHERES:
        dataSet = prepareSpheres();
        break;
    case lfx::VectorRenderer::DIRECTION_VECTORS:
        dataSet = prepareDirectionVectors();
        break;
    }

    return( dataSet );
}


int main( int argc, char** argv )
{
    lfx::Log::instance()->setPriority( lfx::Log::PrioInfo, lfx::Log::Console );

    LFX_INFO_STATIC( logstr, "With no options, render as simple points." );
    LFX_INFO_STATIC( logstr, "Options:" );
    LFX_INFO_STATIC( logstr, "\t-ps\tRender as point sprites." );
    LFX_INFO_STATIC( logstr, "\t-s\tRender as spheres." );
    LFX_INFO_STATIC( logstr, "\t-d\tRender as direction vectors.\n" );

    osg::ArgumentParser arguments( &argc, argv );
    lfx::VectorRenderer::PointStyle style( lfx::VectorRenderer::SIMPLE_POINTS );
    if( arguments.find( "-ps" ) > 0 ) style = lfx::VectorRenderer::POINT_SPRITES;
    if( arguments.find( "-s" ) > 0 ) style = lfx::VectorRenderer::SPHERES;
    if( arguments.find( "-d" ) > 0 ) style = lfx::VectorRenderer::DIRECTION_VECTORS;

    // Create an example data set.
    osg::Group* root( new osg::Group );
    lfx::DataSetPtr dsp( prepareDataSet( style ) );
    root->addChild( dsp->getSceneData() );

    // Adjust root state.
    {
        osg::StateSet* stateSet( root->getOrCreateStateSet() );

        // Test hardware clip planes
        osg::ClipNode* cn( new osg::ClipNode() );
        cn->addClipPlane( new osg::ClipPlane( 0, 1., 0., 0., 3. ) );
        root->addChild( cn );
        stateSet->setMode( GL_CLIP_PLANE0, osg::StateAttribute::ON );

        // Add uniform to control transfer function min/max range.
        stateSet->addUniform( new osg::Uniform( "tfRange", osg::Vec2( -3., 2. ) ),
            osg::StateAttribute::OVERRIDE );
    }
    
    // Play the time series animation
    lfx::PlayControlPtr playControl( new lfx::PlayControl( dsp->getSceneData() ) );
    playControl->setTimeRange( dsp->getTimeRange() );

    osgViewer::Viewer viewer;
    viewer.setUpViewInWindow( 10, 30, 800, 440 );
    viewer.setCameraManipulator( new osgGA::TrackballManipulator() );
    viewer.setSceneData( root );

    double prevClockTime( 0. );
    while( !( viewer.done() ) )
    {
        const double clockTime( viewer.getFrameStamp()->getReferenceTime() );
        const double elapsed( clockTime - prevClockTime );
        prevClockTime = clockTime;
        playControl->elapsedClockTick( elapsed );

        viewer.frame();
    }
    return( 0 );
}
