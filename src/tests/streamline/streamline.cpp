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

#ifdef LFX_USE_CRUNCHSTORE
#  include <crunchstore/DataManager.h>
#  include <crunchstore/NullCache.h>
#  include <crunchstore/NullBuffer.h>
#  include <crunchstore/SQLiteStore.h>
#  include <latticefx/core/DBCrunchStore.h>
#endif

#include <latticefx/core/DataSet.h>
#include <latticefx/core/ChannelData.h>
#include <latticefx/core/ChannelDataOSGArray.h>
#include <latticefx/core/RTPOperation.h>
#include <latticefx/core/StreamlineRenderer.h>
#include <latticefx/core/TransferFunctionUtils.h>
#include <latticefx/core/PlayControl.h>
#include <latticefx/core/Log.h>
#include <latticefx/core/LogMacros.h>

#include <latticefx/core/DBDisk.h>

#include <Poco/File.h>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>
#include <osg/ClipNode>
#include <osg/LightSource>
#include <osg/Material>
#include <osg/io_utils>

#include <iostream>
#include <sstream>


using namespace lfx::core;


const std::string logstr( "lfx.demo" );


class DepthComputation : public RTPOperation
{
public:
    DepthComputation() : RTPOperation( RTPOperation::Channel ) {}
    DepthComputation( const DepthComputation& rhs ) : RTPOperation( rhs ) {}

    ChannelDataPtr channel( const ChannelDataPtr maskIn )
    {
        ChannelDataPtr posData( getInput( "positions" ) );
        osg::Array* posBaseArray( posData->asOSGArray() );
        osg::Vec3Array* posArray( static_cast< osg::Vec3Array* >( posBaseArray ) );

        // Depth value is the z value. Create an array of the z values.
        osg::FloatArray* depth( new osg::FloatArray );
        osg::Vec3Array::const_iterator it;
        for( it = posArray->begin(); it != posArray->end(); ++it )
        {
            const float z( it->z() );
            depth->push_back( z );
        }

        return( ChannelDataOSGArrayPtr( new ChannelDataOSGArray( "depth", depth ) ) );
    }
};

#define DEV_DATA 1

unsigned int computeDynamicPositions( osg::Vec3Array* a,
                                      const unsigned int samples, const double t )
{
    a->resize( samples );
    unsigned int index( 0 );
    for( unsigned int idx = 0; idx < samples; ++idx )
    {
#ifdef DEV_DATA
        ( *a )[ index ].set(
            idx * 0.05,
            t,
            4. * sin( (double)( idx ) / (double)( samples ) * osg::PI * 2. )
            );
#else
        const double x( ( double )( idx % 2323 ) / 2323. );
        const double y( ( double )( idx % 3701 ) / 3701. );
        const double z( ( double )( idx % 1097 ) / 1097. );
        ( *a )[ index ].set(
            ( x + sin( ( x + y + t ) * .8 ) ) * 5.,
            ( y + sin( ( x + y + t ) ) ) * 5.,
            z + sin( ( x + y + t ) * 1.2 )
            );
#endif
        ++index;
    }
    return( index );
}

DataSetPtr prepareStreamline( DBBasePtr dbBase )
{
#ifdef DEV_DATA
    const unsigned int samplesPerTime( 100 );
#else
    const unsigned int samplesPerTime( 10000 );
#endif
    {
        std::ostringstream ostr;
        ostr << "Creating data set. Samples: " << samplesPerTime;
        LFX_INFO_STATIC( logstr, ostr.str() );
    }

    DataSetPtr dsp( new DataSet() );
    unsigned int totalSamples( 0 );

    const double maxTime( 8. );
    const double sampleRate( 30. );
    {
        std::ostringstream ostr;
        ostr << "Creating time series data. " << maxTime << "s, sample rate: " << sampleRate << "hz.";
        LFX_INFO_STATIC( logstr, ostr.str() );
    }

    double time;
    for( time = 0.; time < maxTime; time += 1. / sampleRate )
    {
        osg::ref_ptr< osg::Vec3Array > posArray( new osg::Vec3Array );
        unsigned int count( computeDynamicPositions( posArray.get(), samplesPerTime, time ) );
        totalSamples += count;
        ChannelDataOSGArrayPtr posData( ChannelDataOSGArrayPtr( new ChannelDataOSGArray( "positions", posArray.get() ) ) );
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
    dsp->addOperation( RTPOperationPtr( dc ) );

    StreamlineRendererPtr renderOp( new StreamlineRenderer() );
    renderOp->addInput( "positions" );
    renderOp->addInput( "depth" ); // From DepthComputation channel creator

    // Configure transfer function.
    renderOp->setTransferFunctionInput( "depth" );
    renderOp->setTransferFunction( loadImageFromDat( "01.dat" ) );
    renderOp->setTransferFunctionDestination( Renderer::TF_RGBA );

    // Configure hardware mask.
    if( false )
    {
        renderOp->setHardwareMaskInputSource( Renderer::HM_SOURCE_SCALAR );
        renderOp->setHardwareMaskInput( "depth" );
        renderOp->setHardwareMaskOperator( Renderer::HM_OP_EQ );
        renderOp->setHardwareMaskReference( 0.5f );
        renderOp->setHardwareMaskEpsilon( 1.f );
    }
    else
    {
        renderOp->setHardwareMaskOperator( Renderer::HM_OP_OFF );
    }

    renderOp->setDB( dbBase );

    dsp->setRenderer( renderOp );
    dsp->setDB( dbBase );

    return( dsp );
}

DataSetPtr prepareDataSet( const std::string& csFile, const std::string& diskPath, const bool createDB, const bool nopage )
{
    DataSetPtr dataSet;

    DBBasePtr dbBase( DBBasePtr( (DBBase*)NULL ) );
#ifdef LFX_USE_CRUNCHSTORE
    if( !( csFile.empty() ) )
    {
        LFX_CRITICAL_STATIC( logstr, "Paging with DBCruchStore." );

        DBCrunchStorePtr cs( DBCrunchStorePtr( new DBCrunchStore() ) );

        crunchstore::DataManagerPtr manager( crunchstore::DataManagerPtr( new crunchstore::DataManager() ) );
        crunchstore::DataAbstractionLayerPtr cache( new crunchstore::NullCache );
        crunchstore::DataAbstractionLayerPtr buffer( new crunchstore::NullBuffer );
        manager->SetCache( cache );
        manager->SetBuffer( buffer );
        crunchstore::SQLiteStorePtr sqstore( new crunchstore::SQLiteStore );
        sqstore->SetStorePath( csFile );
        manager->AttachStore( sqstore, crunchstore::Store::BACKINGSTORE_ROLE );
        try
        {
            cs->setDataManager( manager );
        }
        catch( std::exception exc )
        {
            LFX_FATAL_STATIC( logstr, std::string( exc.what() ) );
            LFX_FATAL_STATIC( logstr, "Unable to set DataManager." );
            exit( 1 );
        }

        dbBase = ( DBBasePtr )cs;
    }
#endif
    if( csFile.empty() && !nopage )
    {
        LFX_CRITICAL_STATIC( logstr, "Paging with DBDisk." );

        DBDiskPtr disk( DBDiskPtr( new DBDisk() ) );
        disk->setRootPath( diskPath );
        dbBase = ( DBBasePtr )disk;
    }

    if( dbBase == NULL )
    {
        LFX_CRITICAL_STATIC( logstr, "Paging disabled." );
    }

    dataSet = prepareStreamline( dbBase );

    return( dataSet );
}


int main( int argc, char** argv )
{
    Log::instance()->setPriority( Log::PrioInfo, Log::Console );

    // Please document all options using Doxygen at the bottom of this file.
    LFX_INFO_STATIC( logstr, "Paging options:" );
    LFX_INFO_STATIC( logstr, "\t-dp <path> Page at runtime using DBDisk and this directory path." );
    LFX_INFO_STATIC( logstr, "\t-cs <dbFile> Write volume image data files using DBCrunchStore." );
    LFX_INFO_STATIC( logstr, "\t-nopage Do not page. Keep all data resident in the scene graph." );
    LFX_INFO_STATIC( logstr, "-dp <cwd> is the default if no other paging option is specified.\n" );

    osg::ArgumentParser arguments( &argc, argv );

    const bool nopage( arguments.find( "-nopage" ) > 0 );
    std::string csFile;
    std::string diskPath;
    if( !nopage )
    {
#ifdef LFX_USE_CRUNCHSTORE
        arguments.read( "-cs", csFile );
#endif
        arguments.read( "-dp", diskPath );
        if( !diskPath.empty() && !csFile.empty() )
        {
            LFX_WARNING_STATIC( logstr, "Can't use both CrunchStore and DBDisk. Using CrunchStore..." );
            diskPath.clear();
        }
    }

    bool createDB;
    if( !csFile.empty() )
    {
        Poco::File dbPocoFile( csFile );
        createDB = !( dbPocoFile.exists() );
    }
    else
    {
        Poco::File dbPocoFile( diskPath.empty() ? std::string( "." ) : diskPath );
        createDB = !( dbPocoFile.exists() );
    }

    // Create an example data set.
    osg::Group* root( new osg::Group );
    DataSetPtr dsp( prepareDataSet( csFile, diskPath, createDB, nopage ) );
    root->addChild( dsp->getSceneData() );

    // Adjust root state.
    {
        osg::StateSet* stateSet( root->getOrCreateStateSet() );

        // Add uniform to control transfer function min/max range.
        stateSet->addUniform( new osg::Uniform( "tfRange", osg::Vec2f( -3.f, 2.f ) ),
                              osg::StateAttribute::OVERRIDE );

        // Test hardware clip planes
        osg::ClipNode* cn( new osg::ClipNode() );
        cn->addClipPlane( new osg::ClipPlane( 0, 1., 0., 0., 3. ) );
        root->addChild( cn );
        stateSet->setMode( GL_CLIP_PLANE0, osg::StateAttribute::ON );
    }

    // Play the time series animation
    PlayControlPtr playControl( new PlayControl( dsp->getSceneData() ) );
    playControl->setTimeRange( dsp->getTimeRange() );

    osgViewer::Viewer viewer;
    viewer.setUpViewInWindow( 10, 30, 800, 440 );
    viewer.setCameraManipulator( new osgGA::TrackballManipulator() );
    viewer.addEventHandler( new osgViewer::StatsHandler() );
    viewer.setSceneData( root );


    double prevClockTime( 0. );
    while( !( viewer.done() ) )
    {
        const double clockTime( viewer.getFrameStamp()->getReferenceTime() );
        const double elapsed( clockTime - prevClockTime );
        prevClockTime = clockTime;
        playControl->elapsedClockTick( elapsed );

        /*
        // Test dynamic hardware mask range.
    	Renderer::setHardwareMaskRange(root->getStateSet(),
            (float)(2. * cos( clockTime * .7 )), 1.f + (float)(2. * sin( clockTime )) );
            */

        viewer.frame();
    }
    return( 0 );
}


/** \page TestStreamline streamline

TBD.

Uses the VectorRenderer to display point data as simple points,
spheres with variying radii, or vectors with varying direction.

If no style option is specified, simple points are used.

Paging options:
\li -dp <path> Page at runtime using DBDisk and this directory path.
\li -cs <dbFile> Write volume image data files using DBCrunchStore.
\li -nopage Do not page. Keep all data resident in the scene graph.

-dp <cwd> is the default if no other paging option is specified.
*/
