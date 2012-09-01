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
#include <latticefx/core/VolumeRenderer.h>
#include <latticefx/core/ChannelDataOSGImage.h>
#include <latticefx/core/HierarchyUtils.h>
#include <latticefx/core/PagingThread.h>
#include <latticefx/core/TransferFunctionUtils.h>
#include <latticefx/core/Log.h>
#include <latticefx/core/LogMacros.h>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osg/Group>
#include <osg/ArgumentParser>
#include <osgGA/TrackballManipulator>

#include <Poco/Path.h>
#include <Poco/Glob.h>
#include <boost/foreach.hpp>

#include <string>


static std::string logstr( "lfx.demo" );

using namespace lfx::core;


class ImageHierarchyLoader : public Preprocess
{
public:
    ImageHierarchyLoader( const std::string fileName )
      : _fileName( fileName )
    {
        setActionType( Preprocess::ADD_DATA );
    }
    ~ImageHierarchyLoader() {}

    virtual ChannelDataPtr operator()()
    {
        const std::string fullName( osgDB::findDataFile( _fileName ) );
        if( fullName.empty() )
        {
            LFX_WARNING_STATIC( logstr, "Can't find \"" + _fileName + "\"." );
            return( ChannelDataPtr( (ChannelData*)NULL ) );
        }
        const std::string pathOnly( osgDB::getFilePath( fullName ) );

        // Create a glob pattern from the given _fileName.
        const std::string nameOnly( osgDB::getSimpleFileName( fullName ) );
        const size_t firstDash( nameOnly.find_first_of( "-" ) );
        const size_t lastDash( nameOnly.find_last_of( "-" ) );
        if( ( firstDash == lastDash ) || ( firstDash == std::string::npos ) ||
            ( lastDash == std::string::npos ) )
        {
            LFX_WARNING_STATIC( logstr, "\"" + _fileName + "\" has invalid name pattern." );
            return( ChannelDataPtr( (ChannelData*)NULL ) );
        }
        const std::string globPattern( nameOnly.substr( 0, firstDash+1 ) + "*" +
            nameOnly.substr( lastDash ) );
        LFX_DEBUG_STATIC( logstr, pathOnly );
        LFX_DEBUG_STATIC( logstr, globPattern );

        // Find all files matching pattern.
        Poco::Path globPath( osgDB::concatPaths( pathOnly, globPattern ) );
        std::set< std::string > results;
        Poco::Glob::glob( globPath, results );
        if( results.empty() )
        {
            LFX_WARNING_STATIC( logstr, "No files found matching pattern: \"" + globPattern + "\"." );
            return( ChannelDataPtr( (ChannelData*)NULL ) );
        }

        // Determine the hierarchy maxDepth from the longest hierarchy name.
        unsigned int maxDepth( 1 );
        BOOST_FOREACH( const std::string& fName, results )
        {
            Poco::Path fullName( fName );
            const std::string& actualName( fullName.getFileName() );
            size_t depth( actualName.find_last_of( "-" ) - actualName.find_first_of( "-" ) );
            if( depth > maxDepth )
                maxDepth = depth;
        }

        // Create the ChannelData hierarchy.
        AssembleHierarchy ah( maxDepth, 30000. );
        BOOST_FOREACH( const std::string& fName, results )
        {
            // Create this ChannelData.
            ChannelDataOSGImagePtr cdImage( new ChannelDataOSGImage() );
            cdImage->setStorageModeHint( ChannelData::STORE_IN_DB );
            cdImage->setDBKey( DBKey( fName ) );

            // Get the hierarchy name string.
            Poco::Path fullName( fName );
            const std::string& actualName( fullName.getFileName() );
            const size_t firstLoc( actualName.find_first_of( "-" ) + 1 );
            const size_t lastLoc( actualName.find_last_of( "-" ) );
            const std::string hierarchyName( actualName.substr( firstLoc, lastLoc-firstLoc ) );

            LFX_DEBUG_STATIC( logstr, "Adding " + fName + ": " + hierarchyName );
            cdImage->setName( "volumedata" );
            ah.addChannelData( cdImage, hierarchyName );
        }

        // Return the hierarchy root.
        ChannelDataPtr cdp( ah.getRoot() );
        cdp->setName( "volumedata" );
        return( cdp );
    }

protected:
    std::string _fileName;
};


DataSetPtr prepareVolume( const std::string& fileName, const osg::Vec3& dims )
{
    DataSetPtr dsp( new DataSet() );

    ImageHierarchyLoader* ihl = new ImageHierarchyLoader( fileName );
    dsp->addPreprocess( PreprocessPtr( (Preprocess*)ihl ) );

    VolumeRendererPtr renderOp( new VolumeRenderer() );
    renderOp->setVolumeDims( dims );
    renderOp->setPlaneSpacing( .3f );

    renderOp->addInput( "volumedata" );
    dsp->setRenderer( renderOp );

    renderOp->setTransferFunction( lfx::core::loadImageFromDat( "01.dat", LFX_ALPHA_RAMP_0_TO_1 ) );
    renderOp->setTransferFunctionDestination( Renderer::TF_RGBA );

    // Render when alpha values are greater than 0.15.
    renderOp->setHardwareMaskInputSource( Renderer::HM_SOURCE_ALPHA );
    renderOp->setHardwareMaskOperator( Renderer::HM_OP_GT );
    renderOp->setHardwareMaskReference( .15f );

    return( dsp );
}

int main( int argc, char** argv )
{
    Log::instance()->setPriority( Log::PrioInfo, Log::Console );
    //Log::instance()->setPriority( Log::PrioTrace, "lfx.core.page" );

    osg::ArgumentParser arguments( &argc, argv );

    std::string fileName;
    arguments.read( "-f", fileName );
    if( fileName.empty() )
    {
        LFX_FATAL_STATIC( logstr, "Must specify \"-f <filename>\" on command line." );
        return( 1 );
    }

    osg::Vec3 dims( 50., 50., 50. );
    arguments.read( "-d", dims[0],dims[1],dims[2] );

    // Create the lfx data set.
    osg::Group* root (new osg::Group);

    DataSetPtr dsp( prepareVolume( fileName, dims ) );
    root->addChild( dsp->getSceneData() );

    osgViewer::Viewer viewer;
    viewer.getCamera()->setClearColor( osg::Vec4( 0., 0., 0., 1. ) );
    viewer.setUpViewInWindow( 20, 30, 800, 460 );
    viewer.addEventHandler( new osgViewer::StatsHandler() );
    osgGA::TrackballManipulator* tbm( new osgGA::TrackballManipulator() );
    viewer.setCameraManipulator( tbm );
    viewer.setSceneData( root );


    // Really we would need to change the projection matrix and viewport
    // in an event handler that catches window size changes. We're cheating.
    PagingThread* pageThread( PagingThread::instance() );
    const osg::Camera* cam( viewer.getCamera() );
    pageThread->setTransforms( cam->getProjectionMatrix(), cam->getViewport() );

    osg::Vec3d eye, center, up;
    while( !viewer.done() )
    {
        tbm->getInverseMatrix().getLookAt( eye, center, up );
        pageThread->setTransforms( osg::Vec3( eye ) );
        viewer.frame();
    }
    return( 0 );
}
