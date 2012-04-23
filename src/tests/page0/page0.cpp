
#include <latticefx/RootCallback.h>
#include <latticefx/PagingThread.h>
#include <latticefx/PageData.h>

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/BoundingSphere>
#include <osgwTools/Shapes.h>
#include <osgViewer/Viewer>
#include <osgDB/WriteFile>

#include <string>



void generateDataFiles()
{
    osg::Vec4Array* color0( new osg::Vec4Array );
    color0->push_back( osg::Vec4( 1., 1., 1., 1. ) );

    osg::Geode* geode( new osg::Geode );
    osg::Geometry* geom( osgwTools::makePlane( osg::Vec3( -1., 0., -1. ),
        osg::Vec3( 2., 0., 0. ), osg::Vec3( 0., 0., 2. ) ) );
    geode->addDrawable( geom );
    geom->setColorArray( color0 );
    geom->setColorBinding( osg::Geometry::BIND_OVERALL );

    osgDB::writeNodeFile( *geode, "page-white.osg" );


    osg::Vec4Array* color1( new osg::Vec4Array );
    color1->push_back( osg::Vec4( 1., 0., 0., 1. ) );

    geode = new osg::Geode;
    geom = osgwTools::makePlane( osg::Vec3( -1., 0., -1. ),
        osg::Vec3( 2., 0., 0. ), osg::Vec3( 0., 0., 2. ) );
    geode->addDrawable( geom );
    geom->setColorArray( color1 );
    geom->setColorBinding( osg::Geometry::BIND_OVERALL );

    osgDB::writeNodeFile( *geode, "page-red.osg" );
}

int main( int argc, char** argv )
{
    // To generate the test data, page-red.osg and page-white.osg:
    //generateDataFiles();

    osg::ref_ptr< osg::Group > root( new osg::Group );
    root->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );

    lfx::PageData* pageData( new lfx::PageData );
    pageData->setRangeMode( lfx::PageData::PIXEL_SIZE_RANGE );
    pageData->setParent( root.get() );
    pageData->setRangeData( 0, lfx::PageData::RangeData( 0, 50000, "page-white.osg" ) );
    pageData->setRangeData( 1, lfx::PageData::RangeData( 50000, FLT_MAX, "page-red.osg" ) );
    pageData->setBound( osg::BoundingSphere( osg::Vec3( 0., 0., 0. ), 1.5f ) );
    root->setUserData( pageData );

    lfx::RootCallback* rootCallback( new lfx::RootCallback );
    rootCallback->addPageParent( root.get() );
    root->setUpdateCallback( rootCallback );

    root->addChild( new osg::Group );
    root->addChild( new osg::Group );


    osgViewer::Viewer viewer;
    viewer.setUpViewInWindow( 20, 30, 800, 450 );
    viewer.setSceneData( root.get() );

    rootCallback->setCamera( viewer.getCamera() );

    return( viewer.run() );
}
