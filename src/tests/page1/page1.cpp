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

#include "RootCallback.h"
#include <latticefx/DataSet.h>
#include <latticefx/ChannelDataOSGArray.h>
#include <latticefx/ChannelDataOSGImage.h>
#include <latticefx/ChannelDataLOD.h>
#include <latticefx/Preprocess.h>
#include <latticefx/Renderer.h>
#include <latticefx/PageData.h>

#include <osgwTools/Shapes.h>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Image>
#include <osg/Texture2D>

#include <osgDB/WriteFile>
#include <osgViewer/Viewer>


class ImageProcess : public lfx::Preprocess
{
public:
    ImageProcess()
      : lfx::Preprocess()
    {
        setActionType( lfx::Preprocess::REPLACE_DATA );
    }

    virtual lfx::ChannelDataPtr operator()()
    {
        lfx::ChannelDataOSGImagePtr input( boost::static_pointer_cast< lfx::ChannelDataOSGImage >( _inputs[ 0 ] ) );

        osg::Image* farImage( new osg::Image );
        farImage->setFileName( "pagetex-far.png" );
        lfx::ChannelDataOSGImagePtr newImage( new lfx::ChannelDataOSGImage( "texture", farImage ) );

        lfx::ChannelDataLODPtr cdLOD( new lfx::ChannelDataLOD( input->getName() ) );
        cdLOD->setRange( cdLOD->addChannel( input ),
            lfx::RangeValues( 0., 20000. ) );
        cdLOD->setRange( cdLOD->addChannel( newImage ),
            lfx::RangeValues( 20000., FLT_MAX ) );
        return( cdLOD );
    }
};

class BoxRenderer : public lfx::Renderer
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
};

lfx::DataSetPtr createDataSet()
{
    osg::Image* image( new osg::Image() );
    image->setFileName( "pagetex-near0.png" );
    lfx::ChannelDataOSGImagePtr imageData( new lfx::ChannelDataOSGImage( "texture", image ) );

    lfx::DataSetPtr dsp( new lfx::DataSet() );
    dsp->setSceneGraphPagesTexturesOnly();
    dsp->useCustomRootCallback( new lfxdev::RootCallback() );
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
    osg::Node* root( dsp->getSceneData() );
    osgDB::writeNodeFile( *root, "out.osg" );
    viewer.setSceneData( root );

    lfx::RootCallback* rootCallback( static_cast< lfx::RootCallback* >( root->getUpdateCallback() ) );
    rootCallback->setCamera( viewer.getCamera() );

    return( viewer.run() );
}
