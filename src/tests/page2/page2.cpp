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

#include <latticefx/core/PagingCallback.h>
#include <latticefx/core/PagingThread.h>
#include <latticefx/core/DataSet.h>
#include <latticefx/core/VolumeRenderer.h>
#include <latticefx/core/ChannelDataOSGArray.h>
#include <latticefx/core/ChannelDataOSGImage.h>
#include <latticefx/core/ChannelDataLOD.h>
#include <latticefx/core/ChannelDataImageSet.h>
#include <latticefx/core/Preprocess.h>
#include <latticefx/core/Renderer.h>
#include <latticefx/core/PageData.h>

#include <osgwTools/Shapes.h>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Image>
#include <osg/Texture2D>

#include <osgViewer/Viewer>
#include <osgGA/TrackballManipulator>

#include <sstream>


class ImageProcess : public lfx::Preprocess
{
public:
    ImageProcess( unsigned int depth=3 )
      : lfx::Preprocess(),
        _depth( depth )
    {
        setActionType( lfx::Preprocess::REPLACE_DATA );
    }

    virtual lfx::ChannelDataPtr operator()()
    {
        lfx::ChannelDataOSGImagePtr input( boost::static_pointer_cast< lfx::ChannelDataOSGImage >( _inputs[ 0 ] ) );
        const std::string dataName( input->getName() );

        return( recurseBuildTree( 0, 0., 25000. ) );
    }

protected:
    lfx::ChannelDataOSGImagePtr generateImageData( const std::string& fileName, const unsigned int depth, const std::string& dataName )
    {
        std::ostringstream ostr;
        ostr << fileName << depth << ".png";

        osg::Image* image( new osg::Image );
        image->setFileName( ostr.str() );
        return( lfx::ChannelDataOSGImagePtr(
            new lfx::ChannelDataOSGImage( dataName, image ) ) );
    }

    lfx::ChannelDataPtr recurseBuildTree( unsigned int depth, const double minRange, const double maxRange )
    {
        const std::string baseName( "pagetex-near" );
        const std::string channelName( "texture" );

        if( depth == _depth )
            return( generateImageData( baseName, depth, channelName ) );


        lfx::ChannelDataLODPtr cdLOD( new lfx::ChannelDataLOD( channelName ) );
        cdLOD->setRange( cdLOD->addChannel( generateImageData( baseName, depth, channelName ) ),
            lfx::RangeValues( minRange, maxRange ) );

        const unsigned int nextDepth( depth + 1 );
        const double nextMin( maxRange );
        // If nextDepth == _depth, then we're at the end, so set nextMax to FLT_MAX.
        // Otherwise, increase maxRange by 4 because the area of a circle circumscribing
        // a box goes up by a factor of 4 when the box edge double in size.
        const double nextMax( ( nextDepth == _depth ) ? FLT_MAX : ( maxRange * 4. ) );

        lfx::ChannelDataImageSetPtr cdImageSet( new lfx::ChannelDataImageSet( channelName ) );

        lfx::ChannelDataPtr brick;            
        brick = recurseBuildTree( nextDepth, nextMin, nextMax );
        cdImageSet->setOffset( cdImageSet->addChannel( brick ),
            osg::Vec3( -1., -1., -1. ) );
        brick = recurseBuildTree( nextDepth, nextMin, nextMax );
        cdImageSet->setOffset( cdImageSet->addChannel( brick ),
            osg::Vec3( 1., -1., -1. ) );
        brick = recurseBuildTree( nextDepth, nextMin, nextMax );
        cdImageSet->setOffset( cdImageSet->addChannel( brick ),
            osg::Vec3( -1., 1., -1. ) );
        brick = recurseBuildTree( nextDepth, nextMin, nextMax );
        cdImageSet->setOffset( cdImageSet->addChannel( brick ),
            osg::Vec3( 1., 1., -1. ) );
        brick = recurseBuildTree( nextDepth, nextMin, nextMax );
        cdImageSet->setOffset( cdImageSet->addChannel( brick ),
            osg::Vec3( -1., -1., 1. ) );
        brick = recurseBuildTree( nextDepth, nextMin, nextMax );
        cdImageSet->setOffset( cdImageSet->addChannel( brick ),
            osg::Vec3( 1., -1., 1. ) );
        brick = recurseBuildTree( nextDepth, nextMin, nextMax );
        cdImageSet->setOffset( cdImageSet->addChannel( brick ),
            osg::Vec3( -1., 1., 1. ) );
        brick = recurseBuildTree( nextDepth, nextMin, nextMax );
        cdImageSet->setOffset( cdImageSet->addChannel( brick ),
            osg::Vec3( 1., 1., 1. ) );

        // Regardless of the depth level, there are two LODs. The first is displayed
        // for range (minRange, maxRange), and the second is displayed for range
        // (maxRange, FLT_MAX). In this case, the second LOD is a hierarchy of
        // LODs that are displayed at subranges of (maxRange, FLT_MAX).
        cdLOD->setRange( cdLOD->addChannel( cdImageSet ),
            lfx::RangeValues( maxRange, FLT_MAX ) );
        return( cdLOD );
    }

    unsigned int _depth;
};

class BoxRenderer : public lfx::Renderer, public lfx::SpatialVolume
{
public:
    virtual osg::Node* getSceneGraph( const lfx::ChannelDataPtr maskIn )
    {
        lfx::ChannelDataOSGImage* cdi( static_cast< lfx::ChannelDataOSGImage* >( _inputs[ 0 ].get() ) );
        osg::Image* image( cdi->getImage() );

        osg::Geode* geode( new osg::Geode() );
        osg::StateSet* stateSet( geode->getOrCreateStateSet() );
        stateSet->setTextureAttributeAndModes( 0, new osg::Texture2D( image ) );

        osg::Geometry* geom( osgwTools::makeBox( osg::Vec3( .5, .5, .5 ) ) );
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

lfx::DataSetPtr createDataSet()
{
    osg::Image* image( new osg::Image() );
    image->setFileName( "pagetex-near0.png" );
    lfx::ChannelDataOSGImagePtr imageData( new lfx::ChannelDataOSGImage( "texture", image ) );

    lfx::DataSetPtr dsp( new lfx::DataSet() );
    dsp->addChannel( imageData );

    ImageProcess* op( new ImageProcess );
    op->addInput( "texture" );
    dsp->addPreprocess( lfx::PreprocessPtr( op ) );

    BoxRenderer* renderOp( new BoxRenderer );
    renderOp->addInput( "texture" );
    dsp->setRenderer( lfx::RendererPtr( renderOp ) );

    return( dsp );
}


int main( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );

    lfx::DataSetPtr dsp( createDataSet() );

    osgViewer::Viewer viewer;
    viewer.setUpViewInWindow( 20, 30, 800, 460 );
    viewer.setCameraManipulator( new osgGA::TrackballManipulator() );

    viewer.setSceneData( dsp->getSceneData() );

    // Really we would need to change the projection matrix and viewport
    // in an event handler that catches window size changes. We're cheating.
    lfx::PagingThread* pageThread( lfx::PagingThread::instance() );
    const osg::Camera* cam( viewer.getCamera() );
    pageThread->setTransforms( cam->getProjectionMatrix(), cam->getViewport() );

    osg::Vec3d eye, center, up;
    while( !viewer.done() )
    {
        cam->getViewMatrixAsLookAt( eye, center, up );
        pageThread->setTransforms( osg::Vec3( eye ) );
        viewer.frame();
    }
    return( 0 );
}
