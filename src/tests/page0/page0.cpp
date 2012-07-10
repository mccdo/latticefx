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
#include <latticefx/core/PageData.h>
#include <latticefx/core/Log.h>
#include <latticefx/core/LogMacros.h>

#include <osg/Geode>
#include <osg/Geometry>
#include <osgwTools/Shapes.h>

#include <osgViewer/Viewer>
#include <osgGA/TrackballManipulator>

#include <string>



void generateDataFiles( osg::Group* root )
{
    osg::Vec4Array* color0( new osg::Vec4Array );
    color0->push_back( osg::Vec4( 1., 1., 1., 1. ) );

    osg::Geode* geode( new osg::Geode );
    osg::Geometry* geom( osgwTools::makePlane( osg::Vec3( -1., 0., -1. ),
        osg::Vec3( 2., 0., 0. ), osg::Vec3( 0., 0., 2. ) ) );
    geode->addDrawable( geom );
    geom->setColorArray( color0 );
    geom->setColorBinding( osg::Geometry::BIND_OVERALL );

    root->addChild( geode );


    osg::Vec4Array* color1( new osg::Vec4Array );
    color1->push_back( osg::Vec4( 1., 0., 0., 1. ) );

    geode = new osg::Geode;
    geom = osgwTools::makePlane( osg::Vec3( -1., 0., -1. ),
        osg::Vec3( 2., 0., 0. ), osg::Vec3( 0., 0., 2. ) );
    geode->addDrawable( geom );
    geom->setColorArray( color1 );
    geom->setColorBinding( osg::Geometry::BIND_OVERALL );

    root->addChild( geode );
}

int main( int argc, char** argv )
{
    lfx::Log::instance()->setPriority( lfx::Log::PrioInfo, lfx::Log::Console );

    osg::ref_ptr< osg::Group > root( new osg::Group );
    root->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    generateDataFiles( root.get() );

    lfx::PageData* pageData( new lfx::PageData );
    pageData->setRangeMode( lfx::PageData::PIXEL_SIZE_RANGE );
    pageData->setParent( root.get() );
    pageData->setRangeData( 0, lfx::PageData::RangeData( 0, 50000, "page-white.osg" ) );
    pageData->setRangeData( 1, lfx::PageData::RangeData( 50000, FLT_MAX, "page-red.osg" ) );
    root->setUserData( pageData );

    root->setUpdateCallback( new lfx::PagingCallback() );

    root->addChild( new osg::Group );
    root->addChild( new osg::Group );


    osgViewer::Viewer viewer;
    viewer.setUpViewInWindow( 20, 30, 800, 450 );
    viewer.setCameraManipulator( new osgGA::TrackballManipulator() );

    viewer.setSceneData( root.get() );

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
