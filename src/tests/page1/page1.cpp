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
#include <latticefx/core/ChannelDataOSGArray.h>
#include <latticefx/core/ChannelDataOSGImage.h>
#include <latticefx/core/ChannelDataLOD.h>
#include <latticefx/core/Preprocess.h>
#include <latticefx/core/Renderer.h>
#include <latticefx/core/PageData.h>
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


using namespace lfx::core;


class ImageProcess : public Preprocess
{
public:
    ImageProcess()
      : Preprocess()
    {
        setActionType( Preprocess::REPLACE_DATA );
    }

    virtual ChannelDataPtr operator()()
    {
        ChannelDataOSGImagePtr input( boost::static_pointer_cast< ChannelDataOSGImage >( _inputs[ 0 ] ) );

        const std::string farFileName( "pagetex-far.png" );
        osg::Image* farImage( osgDB::readImageFile( farFileName ) );
        farImage->setFileName( farFileName );
        ChannelDataOSGImagePtr newImage( new ChannelDataOSGImage( "texture", farImage ) );
        newImage->setDBKey( farFileName );
        newImage->reset();

        ChannelDataLODPtr cdLOD( new ChannelDataLOD( input->getName() ) );
        cdLOD->setRange( cdLOD->addChannel( newImage ),
            RangeValues( 0., 100000. ) );
        cdLOD->setRange( cdLOD->addChannel( input ),
            RangeValues( 100000., FLT_MAX ) );
        return( cdLOD );
    }
};

class BoxRenderer : public Renderer
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

        osg::Geometry* geom( osgwTools::makeBox( osg::Vec3( .5, .5, .5 ) ) );
        geom->setColorBinding( osg::Geometry::BIND_OVERALL );
        geode->addDrawable( geom );

        return( geode );
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
    renderOp->addInput( "texture" );
    dsp->setRenderer( RendererPtr( renderOp ) );

    return( dsp );
}


int main( int argc, char** argv )
{
    Log::instance()->setPriority( Log::PrioInfo, Log::Console );
    //Log::instance()->setPriority( Log::PrioTrace, "lfx.core.page" );

    osg::ArgumentParser arguments( &argc, argv );

    DataSetPtr dsp( createDataSet() );

    osgViewer::Viewer viewer;
    viewer.setUpViewInWindow( 20, 30, 800, 460 );
    viewer.setCameraManipulator( new osgGA::TrackballManipulator() );

    viewer.setSceneData( dsp->getSceneData() );

    // Really we would need to change the projection matrix and viewport
    // in an event handler that catches window size changes. We're cheating.
    PagingThread* pageThread( PagingThread::instance() );
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
