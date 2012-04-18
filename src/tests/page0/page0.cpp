
#include <latticefx/RootCallback.h>
#include <latticefx/PagingThread.h>
#include <latticefx/PageData.h>

#include <osg/Geode>
#include <osg/Geometry>
#include <osgwTools/Shapes.h>
#include <osgViewer/Viewer>



int main( int argc, char** argv )
{
    osg::ref_ptr< osg::Group > root( new osg::Group );
    root->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );

    lfx::PageData* pageData( new lfx::PageData );
    pageData->setRangeMode( lfx::PageData::PIXEL_SIZE_RANGE );
    pageData->setParent( root.get() );
    root->setUserData( pageData );

    lfx::RootCallback* rootCallback( new lfx::RootCallback );
    root->setUpdateCallback( rootCallback );


    osg::Vec4Array* color0( new osg::Vec4Array );
    color0->push_back( osg::Vec4( 1., 1., 1., 1. ) );

    osg::Geode* geode( new osg::Geode );
    root->addChild( geode );
    pageData->setRangeData( root->getChildIndex( geode ),
        lfx::PageData::RangeData( 0, 500 ) );
    osg::Geometry* geom( osgwTools::makePlane( osg::Vec3( -1., 0., -1. ),
        osg::Vec3( 2., 0., 0. ), osg::Vec3( 0., 0., 2. ) ) );
    geode->addDrawable( geom );
    geom->setColorArray( color0 );
    geom->setColorBinding( osg::Geometry::BIND_OVERALL );


    osg::Vec4Array* color1( new osg::Vec4Array );
    color1->push_back( osg::Vec4( 1., 0., 0., 1. ) );

    geode = new osg::Geode;
    root->addChild( geode );
    pageData->setRangeData( root->getChildIndex( geode ),
        lfx::PageData::RangeData( 500, FLT_MAX ) );
    geom = osgwTools::makePlane( osg::Vec3( -1., 0., -1. ),
        osg::Vec3( 2., 0., 0. ), osg::Vec3( 0., 0., 2. ) );
    geode->addDrawable( geom );
    geom->setColorArray( color1 );
    geom->setColorBinding( osg::Geometry::BIND_OVERALL );



    osgViewer::Viewer viewer;
    viewer.setUpViewInWindow( 20, 30, 800, 450 );
    viewer.setSceneData( root.get() );
    return( viewer.run() );
}
