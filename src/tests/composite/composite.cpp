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
#include <latticefx/core/PagingThread.h>
#include <latticefx/core/ChannelDataOSGArray.h>
#include <latticefx/core/ChannelDataLOD.h>
#include <latticefx/core/Preprocess.h>
#include <latticefx/core/Renderer.h>
#include <latticefx/core/PageData.h>
#include <latticefx/core/Log.h>
#include <latticefx/core/LogMacros.h>

#include <osgwTools/Shapes.h>
#include <osg/Geode>
#include <osg/Geometry>

#include <osgGA/TrackballManipulator>
#include <osgViewer/Viewer>


using namespace lfx::core;


class ColorProcess : public Preprocess
{
public:
    ColorProcess()
      : Preprocess()
    {
        setActionType( Preprocess::REPLACE_DATA );
    }

    virtual ChannelDataPtr operator()()
    {
        ChannelDataOSGArrayPtr input( boost::static_pointer_cast< ChannelDataOSGArray >( _inputs[ 0 ] ) );

        osg::Vec3Array* color( new osg::Vec3Array );
        color->push_back( osg::Vec3( 0., 1., 0. ) );
        ChannelDataOSGArrayPtr newData( new ChannelDataOSGArray( input->getName(), color ) );

        ChannelDataLODPtr cdLOD( new ChannelDataLOD( input->getName() ) );
        cdLOD->setRange( cdLOD->addChannel( input ),
            RangeValues( 0., 40000. ) );
        cdLOD->setRange( cdLOD->addChannel( newData ),
            RangeValues( 40000., FLT_MAX ) );
        return( cdLOD );
    }
};

class BoxRenderer : public Renderer
{
public:
    virtual osg::Node* getSceneGraph( const ChannelDataPtr maskIn )
    {
        ChannelDataOSGArray* cda( static_cast< ChannelDataOSGArray* >( _inputs[ 0 ].get() ) );
        osg::Vec3Array* c( static_cast< osg::Vec3Array* >( cda->asOSGArray() ) );

        osg::Vec4Array* color( new osg::Vec4Array );
        color->push_back( osg::Vec4( (*c)[ 0 ], 1.f ) );

        osg::Geode* geode( new osg::Geode() );

        osg::Geometry* geom( osgwTools::makeBox( osg::Vec3( .5, .5, .5 ) ) );
        geom->setColorArray( color );
        geom->setColorBinding( osg::Geometry::BIND_OVERALL );
        geode->addDrawable( geom );

        return( geode );
    }
};

DataSetPtr createDataSet()
{
    osg::Vec3Array* c( new osg::Vec3Array );
    c->push_back( osg::Vec3( 1., 0., 0. ) );
    ChannelDataOSGArrayPtr colorData( new ChannelDataOSGArray( "color", c ) );

    DataSetPtr dsp( new DataSet() );
    dsp->addChannel( colorData );

    ColorProcess* op( new ColorProcess );
    op->addInput( "color" );
    dsp->addPreprocess( PreprocessPtr( op ) );

    BoxRenderer* renderOp( new BoxRenderer );
    renderOp->addInput( "color" );
    dsp->setRenderer( RendererPtr( renderOp ) );

    return( dsp );
}


int main( int argc, char** argv )
{
    Log::instance()->setPriority( Log::PrioInfo, Log::Console );

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
