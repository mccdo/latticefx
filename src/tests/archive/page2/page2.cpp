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

#include <latticefx/core/PagingThread.h>
#include <latticefx/core/DataSet.h>
#include <latticefx/core/DBDisk.h>
#include <latticefx/core/VolumeRenderer.h>
#include <latticefx/core/ChannelDataOSGArray.h>
#include <latticefx/core/ChannelDataOSGImage.h>
#include <latticefx/core/ChannelDataLOD.h>
#include <latticefx/core/ChannelDataImageSet.h>
#include <latticefx/core/Preprocess.h>
#include <latticefx/core/Renderer.h>
#include <latticefx/core/HierarchyUtils.h>
#include <latticefx/core/Log.h>
#include <latticefx/core/LogMacros.h>

#include <osgwTools/Shapes.h>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Image>
#include <osg/Texture2D>

#include <osgViewer/Viewer>
#include <osgGA/TrackballManipulator>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include <sstream>


using namespace lfx::core;


class ImageProcess : public Preprocess
{
public:
    ImageProcess( unsigned int depth=3 )
      : Preprocess(),
        _depth( depth )
    {
        setActionType( Preprocess::REPLACE_DATA );
    }

    virtual ChannelDataPtr operator()()
    {
        ChannelDataOSGImagePtr input( boost::static_pointer_cast< ChannelDataOSGImage >( _inputs[ 0 ] ) );
        const std::string dataName( input->getName() );

        return( recurseBuildTree( 0, 0., 25000. ) );
    }

protected:
    ChannelDataOSGImagePtr generateImageData( const std::string& fileName, const unsigned int depth, const std::string& dataName )
    {
        std::ostringstream ostr;
        ostr << fileName << depth << ".png";

        const std::string imageName( ostr.str() );
        osg::Image* localImage( osgDB::readImageFile( imageName ) );
        localImage->setFileName( imageName );
        ChannelDataOSGImagePtr cdip( new ChannelDataOSGImage( dataName, localImage ) );
        cdip->setDBKey( imageName );
        cdip->reset();
        return( cdip );
    }

    ChannelDataPtr recurseBuildTree( unsigned int depth, const double minRange, const double maxRange )
    {
        const std::string baseName( "pagetex-near" );
        const std::string channelName( "texture" );

        if( depth == _depth )
            return( generateImageData( baseName, depth, channelName ) );


        ChannelDataLODPtr cdLOD( new ChannelDataLOD( channelName ) );
        unsigned int channelIdx( cdLOD->addChannel( generateImageData( baseName, depth, channelName ) ) );
        cdLOD->setRange( channelIdx, RangeValues( minRange, maxRange ) );

        const unsigned int nextDepth( depth + 1 );
        // If nextDepth == _depth, then we're at the end, so set nextMax to FLT_MAX.
        // Otherwise, increase maxRange by 4 because the area of a circle circumscribing
        // a box goes up by a factor of 4 when the box edge double in size.
        const double nextMax( ( nextDepth == _depth ) ? FLT_MAX : ( maxRange * 4. ) );

        ChannelDataImageSetPtr cdImageSet( new ChannelDataImageSet( channelName ) );

        ChannelDataPtr brick;            
        brick = recurseBuildTree( nextDepth, 0., nextMax );
        channelIdx = cdImageSet->addChannel( brick );
        cdImageSet->setOffset( channelIdx, osg::Vec3( -1., -1., -1. ) );

        brick = recurseBuildTree( nextDepth, 0., nextMax );
        channelIdx = cdImageSet->addChannel( brick );
        cdImageSet->setOffset( channelIdx, osg::Vec3( 1., -1., -1. ) );

        brick = recurseBuildTree( nextDepth, 0., nextMax );
        channelIdx = cdImageSet->addChannel( brick );
        cdImageSet->setOffset( channelIdx, osg::Vec3( -1., 1., -1. ) );

        brick = recurseBuildTree( nextDepth, 0., nextMax );
        channelIdx = cdImageSet->addChannel( brick );
        cdImageSet->setOffset( channelIdx, osg::Vec3( 1., 1., -1. ) );

        brick = recurseBuildTree( nextDepth, 0., nextMax );
        channelIdx = cdImageSet->addChannel( brick );
        cdImageSet->setOffset( channelIdx, osg::Vec3( -1., -1., 1. ) );

        brick = recurseBuildTree( nextDepth, 0., nextMax );
        channelIdx = cdImageSet->addChannel( brick );
        cdImageSet->setOffset( channelIdx, osg::Vec3( 1., -1., 1. ) );

        brick = recurseBuildTree( nextDepth, 0., nextMax );
        channelIdx = cdImageSet->addChannel( brick );
        cdImageSet->setOffset( channelIdx, osg::Vec3( -1., 1., 1. ) );

        brick = recurseBuildTree( nextDepth, 0., nextMax );
        channelIdx = cdImageSet->addChannel( brick );
        cdImageSet->setOffset( channelIdx, osg::Vec3( 1., 1., 1. ) );

        // Regardless of the depth level, there are two LODs. The first is displayed
        // for range (0., maxRange), and the second is displayed for range
        // (maxRange, FLT_MAX). In this case, the second LOD is a hierarchy of
        // LODs that are displayed at subranges of (maxRange, FLT_MAX).
        channelIdx = cdLOD->addChannel( cdImageSet );
        cdLOD->setRange( channelIdx, RangeValues( maxRange, FLT_MAX ) );
        return( cdLOD );
    }

    unsigned int _depth;
};

class BoxRenderer : public Renderer, public SpatialVolume
{
public:
    virtual osg::Node* getSceneGraph( const ChannelDataPtr maskIn )
    {
        ChannelDataOSGImage* cdi( static_cast< ChannelDataOSGImage* >( _inputs[ 0 ].get() ) );
        osg::ref_ptr< osg::Image > image( cdi->getImage() );
        osg::Image* stubImage( new osg::Image() );
        stubImage->setImage( image->s(), image->t(), image->r(),
            image->getInternalTextureFormat(), image->getPixelFormat(),
            image->getDataType(),
            (unsigned char*) NULL,
            osg::Image::NO_DELETE, image->getPacking() );
        stubImage->setFileName( image->getFileName() );

        osg::Geode* geode( new osg::Geode() );
        osg::StateSet* stateSet( geode->getOrCreateStateSet() );
        stateSet->setTextureAttributeAndModes( 0, new osg::Texture2D( stubImage ) );

        osg::Geometry* geom( osgwTools::makeBox( osg::Matrix::translate( getVolumeOrigin() ),
            getVolumeDims() * .5 ) );
        geom->setColorBinding( osg::Geometry::BIND_OVERALL );
        geode->addDrawable( geom );

        return( geode );
    }

    virtual osg::StateSet* getRootState()
    {
        osg::StateSet* stateSet( new osg::StateSet );
        stateSet->setMode( GL_NORMALIZE, osg::StateAttribute::ON );
        return( stateSet );
    }
};

DataSetPtr createDataSet()
{
    const std::string baseFileName( "pagetex-near0.png" );
    osg::Image* image( osgDB::readImageFile( baseFileName ) );
    image->setFileName( baseFileName );
    ChannelDataOSGImagePtr imageData( new ChannelDataOSGImage( "texture", image ) );
    imageData->setDBKey( baseFileName );

    DataSetPtr dsp( new DataSet() );
    dsp->setDB( lfx::core::DBBasePtr( new lfx::core::DBDisk() ) );

    dsp->addChannel( imageData );

    ImageProcess* op( new ImageProcess );
    op->addInput( "texture" );
    dsp->addPreprocess( PreprocessPtr( op ) );

    BoxRenderer* renderOp( new BoxRenderer );
    renderOp->setVolumeDims( osg::Vec3( 2., 1., 1. ) );
    renderOp->setVolumeOrigin( osg::Vec3( 2., 0.5, 1.5 ) );
    renderOp->addInput( "texture" );
    dsp->setRenderer( RendererPtr( renderOp ) );

    return( dsp );
}


class MyHierarchyDB : public TraverseHierarchy::HierarchyCallback
{
public:
    virtual void operator()( ChannelDataPtr cdp, const std::string& hierarchyNameString )
    {
        if( cdp == NULL )
        {
            LFX_CRITICAL_STATIC( "lfx.demo", "NULL: " + hierarchyNameString );
        }
        else
        {
            LFX_CRITICAL_STATIC( "lfx.demo", cdp->getName() + ": " + hierarchyNameString );
        }
    }
};

std::string intToString( const unsigned int n )
{
    std::ostringstream ostr;
    ostr << n;
    return( ostr.str() );
}

void assemble()
{
    unsigned int counter( 0 );

    const unsigned int depth( 3 );
    AssembleHierarchy ah( depth, 25000. );

    ah.addChannelData( ChannelDataPtr( new ChannelData( intToString( counter++ ) ) ), "" );
    for( unsigned int mIdx=0; mIdx<8; ++mIdx )
    {
        std::string string1( 1, '0' + mIdx );
        ah.addChannelData( ChannelDataPtr( new ChannelData( intToString( counter++ ) ) ), string1 );

        if( depth >= 3 )
        {
            for( unsigned int nIdx=0; nIdx<8; ++nIdx )
            {
                std::string string2( 1, '0' + nIdx );
                ah.addChannelData( ChannelDataPtr( new ChannelData( intToString( counter++ ) ) ), string1 + string2 );

                if( depth >= 4 )
                {
                    for( unsigned int oIdx=0; oIdx<8; ++oIdx )
                    {
                        std::string string3( 1, '0' + oIdx );
                        ah.addChannelData( ChannelDataPtr( new ChannelData( intToString( counter++ ) ) ), string1 + string2 + string3 );
                    }
                }
            }
        }
    }

    MyHierarchyDB myDB;
    TraverseHierarchy th( ah.getRoot(), myDB );
    LFX_CRITICAL_STATIC( "lfx.demo", "---------------------------" );
}

int main( int argc, char** argv )
{
    Log::instance()->setPriority( Log::PrioInfo, Log::Console );
    //Log::instance()->setPriority( Log::PrioTrace, "lfx.core.page" );

    // Test hierarchy utils
    assemble();

    osg::ArgumentParser arguments( &argc, argv );

    DataSetPtr dsp( createDataSet() );

    osgViewer::Viewer viewer;
    viewer.setUpViewInWindow( 20, 30, 800, 460 );
    osgGA::TrackballManipulator* tbm( new osgGA::TrackballManipulator() );
    viewer.setCameraManipulator( tbm );

    osg::Group* root( new osg::Group );
    root->addChild( dsp->getSceneData() );
    //osgDB::writeNodeFile( *(dsp->getSceneData()), "out.osg" );
    root->addChild( osgDB::readNodeFile( "axes.osg" ) );
    viewer.setSceneData( root );
    tbm->home( 0. );

    // Test hierarchy utils
    MyHierarchyDB myHierDB;
    TraverseHierarchy( dsp->getChannel( "texture" ), myHierDB );

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
