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
#include <latticefx/core/DBDisk.h>
#include <latticefx/core/VolumeRenderer.h>
#include <latticefx/core/ChannelDataOSGImage.h>
#include <latticefx/core/TransferFunctionUtils.h>
#include <latticefx/core/Log.h>
#include <latticefx/core/LogMacros.h>

#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>
#include <osg/ClipNode>
#include <osg/MatrixTransform>
#include <osg/PolygonMode>
#include <osg/NodeCallback>
#include <osgUtil/CullVisitor>
#include <osgUtil/RenderStage>
#include <osg/ComputeBoundsVisitor>
#include <osg/Texture2D>
#include <osg/Texture3D>

#include <osgwTools/MultiCameraProjectionMatrix.h>
#include <osgwTools/Shapes.h>

#include <boost/foreach.hpp>
#include <iostream>
#include <osg/io_utils>

#include <osgGA/GUIEventHandler>

class MyKeyHandler : public osgGA::GUIEventHandler
{
public:
    MyKeyHandler( osg::StateSet* stateSet )
      : _stateSet( stateSet ),
        _mode( false )
    {
        osg::Uniform* u = new osg::Uniform( "toggle", _mode );
        _stateSet->addUniform( u );
    }

    virtual bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa, osg::Object*, osg::NodeVisitor* )
    {
        bool handled( false );
        if( ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN )
        {
            if( ea.getKey() == 'a' )
            {
                osg::Uniform* u( _stateSet->getUniform( "toggle" ) );
                _mode = !_mode;
                u->set( _mode );
                handled = true;
            }
        }
        return( handled );
    }

protected:
    osg::ref_ptr< osg::StateSet > _stateSet;
    bool _mode;
};


using namespace lfx::core;



static osg::Vec2f winSize( 1200, 690 );
static osg::ref_ptr< osg::Uniform > windowSize;

static osg::ref_ptr< osg::Texture2D > colorTexA;
static osg::ref_ptr< osg::Texture2D > depthTexA;
static osg::ref_ptr< osg::Camera > splatCam;
static osg::ref_ptr< osg::Camera > rootCam;



/*
    The following code is an attempt to work around the known OSG
    issue with RenderStage::setCameraRequiresSetUp(). The code currently
    doesn't work, and should be investigated further. For now this
    code is disabled. See this commend:
       // TBD disable for now
    But the ResizeHandler remains in place to show to send window
    width and height to the ray traced volume shaders.
*/
class ResetCameraCallback : public osg::NodeCallback
{
public:
    ResetCameraCallback()
    {}

    virtual void operator()( osg::Node* node, osg::NodeVisitor* nv )
    {
        osgUtil::CullVisitor* cv( static_cast< osgUtil::CullVisitor* >( nv ) );
        DirtyMap::iterator it( _dirty.find( cv ) );
        if( it == _dirty.end() )
        {
            // TBD need to add thread safety mechanism.
            _dirty[ cv ] = true;
            it = _dirty.find( cv );
        }

        if( it->second )
        {
            cv->getRenderStage()->setCameraRequiresSetUp( true );
            it->second = false;
        }

        traverse( node, nv );
    }

    void setDirty( const bool dirty=true )
    {
        BOOST_FOREACH( DirtyMap::value_type& val, _dirty )
        {
            val.second = dirty;
        }
    }

protected:
    typedef std::map< osgUtil::CullVisitor*, bool > DirtyMap;
    DirtyMap _dirty;
};

void resetCamera( osg::Node* node )
{
    ResetCameraCallback* rcc( dynamic_cast< ResetCameraCallback* >( node->getCullCallback() ) );
    if( rcc == NULL )
    {
        rcc = new ResetCameraCallback();
        node->setCullCallback( rcc );
    }
    rcc->setDirty();
}

class ResizeHandler : public osgGA::GUIEventHandler
{
public:
    ResizeHandler( osg::Node* node )
      : _node( node )
    {
    }
    virtual bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa, osg::Object*, osg::NodeVisitor* )
    {
        if( ea.getEventType() != osgGA::GUIEventAdapter::RESIZE )
            return( false );

        winSize.x() = ea.getWindowWidth();
        winSize.y() = ea.getWindowHeight();
        windowSize->set( winSize );
        colorTexA->setTextureSize( winSize.x(), winSize.y() );
        depthTexA->setTextureSize( winSize.x(), winSize.y() );
        // TBD disable for now
        //colorTexA->dirtyTextureObject();
        //depthTexA->dirtyTextureObject();
        splatCam->setViewport( 0, 0, (int)winSize.x(), (int)winSize.y() );
        rootCam->setProjectionMatrixAsPerspective( 30., winSize.x()/winSize.y(), 1., 199. );

        resetCamera( _node.get() );
        return( false );
    }

protected:
    osg::ref_ptr< osg::Node > _node;
};
/*
    End of code for working around RenderStage::setCameraRequiresSetUp().
*/



void prepareSceneCamera( osgViewer::Viewer& viewer )
{
    rootCam = viewer.getCamera();
    rootCam->setName( "rootCam" );

    // Viewer's Camera will render into there color and depth texture buffers:
    colorTexA = new osg::Texture2D;
    colorTexA->setTextureWidth( winSize.x() );
    colorTexA->setTextureHeight( winSize.y() );
    colorTexA->setInternalFormat( GL_RGBA );
    colorTexA->setBorderWidth( 0 );
    colorTexA->setFilter( osg::Texture::MIN_FILTER, osg::Texture::NEAREST );
    colorTexA->setFilter( osg::Texture::MAG_FILTER, osg::Texture::NEAREST );

    depthTexA = new osg::Texture2D;
    depthTexA->setTextureWidth( winSize.x() );
    depthTexA->setTextureHeight( winSize.y() );
    depthTexA->setInternalFormat( GL_DEPTH_COMPONENT );
    depthTexA->setBorderWidth( 0 );
    depthTexA->setFilter( osg::Texture::MIN_FILTER, osg::Texture::NEAREST );
    depthTexA->setFilter( osg::Texture::MAG_FILTER, osg::Texture::NEAREST );

    rootCam->attach( osg::Camera::COLOR_BUFFER0, colorTexA.get(), 0, 0 );
    rootCam->attach( osg::Camera::DEPTH_BUFFER, depthTexA.get(), 0, 0 );
    rootCam->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT, osg::Camera::FRAME_BUFFER );
}

osg::Camera* createDisplaySceneCamera()
{
    splatCam = new osg::Camera;
    splatCam->setName( "splatCam" );
    
    splatCam->setClearMask( 0 );
    splatCam->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER, osg::Camera::FRAME_BUFFER );

    splatCam->setReferenceFrame( osg::Camera::ABSOLUTE_RF );
    splatCam->setRenderOrder( osg::Camera::POST_RENDER );

    osg::Geode* geode( new osg::Geode );
    geode->addDrawable( osgwTools::makePlane(
        osg::Vec3( -1,-1,0 ), osg::Vec3( 2,0,0 ), osg::Vec3( 0,2,0 ) ) );
    geode->getOrCreateStateSet()->setTextureAttributeAndModes(
        0, colorTexA.get(), osg::StateAttribute::ON );
    geode->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    geode->getOrCreateStateSet()->setMode( GL_DEPTH_TEST, osg::StateAttribute::OFF );

    splatCam->addChild( geode );

    return( splatCam.get() );
}

osg::Node* createClipSubgraph()
{
    // Test hardware clip planes
    osg::ClipNode* cn( new osg::ClipNode() );
    osg::Vec3 n( .9, .8, 0. ); n.normalize();
    cn->addClipPlane( new osg::ClipPlane( 0, n[0], n[1], n[2], 7.75 ) );

    return( cn );
}

osg::Camera* createLfxCamera( osg::Node* node, const bool clip )
{
    osg::ref_ptr< osg::Camera > lfxCam( new osg::Camera );
    lfxCam->setName( "app lfxCam" );

    lfxCam->setClearMask( 0 );
    lfxCam->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER, osg::Camera::FRAME_BUFFER );

    lfxCam->setReferenceFrame( osg::Camera::RELATIVE_RF );
    lfxCam->setRenderOrder( osg::Camera::POST_RENDER );

    lfxCam->addChild( node );

    // Create a cull callback on the lfx Camera that will create a projection
    // matrix uniform with near & far planes that encompass both the lfx Camera
    // and the root Camera.
    osgwTools::MultiCameraProjectionMatrix* subCullCB( new osgwTools::MultiCameraProjectionMatrix() );
    lfxCam->setCullCallback( subCullCB );


    // Add uniforms required by Lfx ray traced volume rendering shaders,
    // which Lfx itself is unable to set. The application MUST specify
    // this uniforms.
    {
        osg::StateSet* stateSet( lfxCam->getOrCreateStateSet() );

        windowSize = new osg::Uniform( "windowSize", winSize );
        windowSize->setDataVariance( osg::Object::DYNAMIC );
        stateSet->addUniform( windowSize.get() );

        stateSet->setTextureAttributeAndModes( 1, depthTexA.get() );
        osg::Uniform* uniform = new osg::Uniform( osg::Uniform::SAMPLER_2D, "sceneDepth" ); uniform->set( 1 );
        stateSet->addUniform( uniform );

        if( clip )
        {
            lfxCam->addChild( createClipSubgraph() );

            // No built-in uniform to tell shaders that clip planes are enabled,
            // so send that info down as uniforms, one int uniform for each plane.
            osg::Uniform* clipPlaneEnables( new osg::Uniform( "volumeClipPlaneEnable0", 1 ) );
            stateSet->addUniform( clipPlaneEnables, osg::StateAttribute::OVERRIDE );
        }
    }

    return( lfxCam.release() );
}

DataSetPtr prepareVolume( const std::string& fileName, const osg::Vec3& dims,
        const bool useIso, const float isoVal )
{
    osg::ref_ptr< osg::Image > image( new osg::Image() );
    image->setFileName( fileName );

    ChannelDataOSGImagePtr volumeData( new ChannelDataOSGImage( "volumedata", image.get() ) );
    volumeData->setDBKey( fileName );

    DataSetPtr dsp( new DataSet() );
    DBDiskPtr dbDisk( DBDiskPtr( new DBDisk() ) );
    dsp->setDB( dbDisk );

    dsp->addChannel( volumeData );

    VolumeRendererPtr renderOp( new VolumeRenderer() );
    renderOp->setVolumeDims( dims );
    renderOp->setRenderMode( VolumeRenderer::RAY_TRACED );
    renderOp->setMaxSamples( 400.f );
    renderOp->setTransparency( 1.f );
    renderOp->setTransparencyEnable( useIso ? false : true );

    renderOp->addInput( "volumedata" );
    dsp->setRenderer( renderOp );

    renderOp->setTransferFunction( lfx::core::loadImageFromDat( "01.dat", LFX_ALPHA_RAMP_0_TO_1 ) );
    renderOp->setTransferFunctionDestination( Renderer::TF_RGBA );

    // Render when alpha values are greater than 0.15.
    renderOp->setHardwareMaskInputSource( Renderer::HM_SOURCE_ALPHA );
    renderOp->setHardwareMaskOperator( useIso ? Renderer::HM_OP_EQ : Renderer::HM_OP_GT );
    renderOp->setHardwareMaskReference( isoVal );
    if( useIso )
        renderOp->setHardwareMaskEpsilon( 0.1 );

    return( dsp );
}

osg::Node* createScene( const bool clip, const osg::Vec3& dims=osg::Vec3(0.,0.,0.) )
{
    osg::Geometry* geom( osgwTools::makeClosedCylinder(
        osg::Matrix::translate( 0., 0., -30. ), 60., 8., 8., true, true, osg::Vec2s(1,16) ) );
    osg::Vec4Array* c( new osg::Vec4Array() );
    c->push_back( osg::Vec4( 1., .5, 0., 1. ) );
    geom->setColorArray( c );
    geom->setColorBinding( osg::Geometry::BIND_OVERALL );

    osg::Geode* geode( new osg::Geode() );
    geode->addDrawable( geom );

    osg::Group* root( new osg::Group );
    root->addChild( geode );

    if( dims.length2() > 0. )
    {
        geom = osgwTools::makeWireBox( dims * .5 );
        geode = new osg::Geode;
        geode->addDrawable( geom );
        root->addChild( geode );
    }

    if( clip )
    {
        root->addChild( createClipSubgraph() );

        osg::StateSet* stateSet( root->getOrCreateStateSet() );
        stateSet->setMode( GL_CLIP_PLANE0, osg::StateAttribute::ON );
    }

    return( root );
}



int main( int argc, char** argv )
{
    Log::instance()->setPriority( Log::PrioInfo, Log::Console );
    Log::instance()->setPriority( Log::PrioTrace, "lfx.core.page" );

    osg::ArgumentParser arguments( &argc, argv );

    // Please document all options using Doxygen at the bottom of this file.
    std::cout << "-f <file>\tSpecify the 3D image file to load. Default is HeadVolume.dds." << std::endl;
    std::cout << "-d <x> <y> <z>\tDefault is 50 50 50." << std::endl;
    std::cout << "-cyl\tDisplay a polygonal cylinder." << std::endl;
    std::cout << "-iso <x>\tDisplay as an isosurface." << std::endl;
    std::cout << "-clip\tTest clip plane." << std::endl;
    std::cout << "-mt\tTest with parent MatrixTransforms." << std::endl;

    std::string fileName( "HeadVolume.dds" );
    arguments.read( "-f", fileName );

    osg::Vec3 dims( 50., 50., 50. );
    arguments.read( "-d", dims[0],dims[1],dims[2] );

    const bool clip( arguments.find( "-clip" ) > 0 );
    const bool cyl( arguments.find( "-cyl" ) > 0 );
    const bool mt( arguments.find( "-mt" ) > 0 );

    float isoVal( 0.15 );
    const bool useIso( arguments.read( "-iso", isoVal ) );


    // Create an example data set.
    osg::Group* volume( new osg::Group );

    DataSetPtr dsp( prepareVolume( fileName, dims, useIso, isoVal ) );

    if( mt )
    {
        // the translate will occur in the unscaled units, and the scaling will occur around the new origin.
        // A is just translate and nominal scale
        const osg::Matrix transformA = osg::Matrixd::scale( .5, .5, .5 ) *
                osg::Matrixd::translate(-75.0, -75.0, -75.0);
        osg::MatrixTransform* mtA( new osg::MatrixTransform( transformA ) );
        mtA->addChild( dsp->getSceneData() );
        volume->addChild( mtA );

        // B: scale and rotate but no translate
        const osg::Matrix transformB = osg::Matrixd::rotate( osg::DegreesToRadians( 45.0 ), 0.0, 1.0, 0.0 ) *
                osg::Matrixd::scale( 2.0, 2.0, 2.0 );
        osg::MatrixTransform* mtB( new osg::MatrixTransform( transformB ) );
        mtB->addChild( dsp->getSceneData() );
        volume->addChild( mtB );

        // C: translate, scale AND rotate
        // Note this is a non-uniform scale, performed after the rotation,
        // so the volume will be skewed.
        const osg::Matrix transformC = osg::Matrixd::rotate( osg::DegreesToRadians( 45.0 ), 1.0, 0.0, 0.0 ) *
                osg::Matrixd::scale( 2.0, 2.0, 1.0 ) *
                osg::Matrixd::translate( 100.0, 0.0, 0.0 );
        osg::MatrixTransform* mtC( new osg::MatrixTransform( transformC ) );
        mtC->addChild( dsp->getSceneData() );
        volume->addChild( mtC );
        // Put wireframe boxes around volumes to test rotation and scaling of texture versus osg object
        // This requires a pre-built cube object of unit size.
        if( true )
        {
            osg::Geode* cubeNode( new osg::Geode() );
            cubeNode->addDrawable( osgwTools::makeWireBox( osg::Matrixd::scale( dims ), osg::Vec3( .5, .5, .5 ) ) );
            mtA->addChild( cubeNode );
            mtB->addChild( cubeNode );
            mtC->addChild( cubeNode );
        }

        volume->getOrCreateStateSet()->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
    }
    else
        volume->addChild( dsp->getSceneData() );

    
    osgViewer::Viewer viewer;
    viewer.setThreadingModel( osgViewer::ViewerBase::CullDrawThreadPerContext );
    viewer.getCamera()->setClearColor( osg::Vec4( 0., 0., 0., 1. ) );
    viewer.setUpViewInWindow( 10, 30, winSize.x(), winSize.y() );
    viewer.setCameraManipulator( new osgGA::TrackballManipulator() );
    viewer.addEventHandler( new osgViewer::StatsHandler() );


    osg::Node* scene( NULL );
    if( cyl )
        scene = createScene( clip, dims );

    // Assemble camera RTT and scene hierarchy.
    // viewerCamera (renders to both color and depth textures)
    //   |-> sceneRoot, consisting of a cylinder
    //   |-> splatCam (to display the color texture to the window)
    //    \> volume, consisting of an lfxCam parent and the volume data
    // viewerCamera renders 'scene' to colorTexA and depthTexA.
    // splatCam uses colorTexA as input.
    // lfxCam uses colorTexA and depthTexA as input. Lfx volume shaders
    //   use colorTexA for blending, and depthTexA for correct depth
    //   testing while stepping along the ray.
    prepareSceneCamera( viewer );
    osg::Group* rootGroup( new osg::Group );
    if( scene != NULL )
        rootGroup->addChild( scene );
    rootGroup->addChild( createDisplaySceneCamera() );
    rootGroup->addChild( createLfxCamera( volume, clip ) );
    viewer.setSceneData( rootGroup );
    viewer.addEventHandler( new ResizeHandler( scene ) );

    viewer.addEventHandler( new MyKeyHandler( rootGroup->getOrCreateStateSet() ) );

    while( !( viewer.done() ) )
    {
        viewer.frame();
    }
    return( 0 );
}



/** \page TestVolumeRT Test volume-rt

Display a single volume data image using RAY_TRACED mode.

Specify a file with the -f option. If you do not specify this option,
volume-rt attempts to use a file called HeadVOlume.dds.
\li -f <file> Specify the 3D image file to load.

Other options:
\li -d <x> <y> <z> Default is 50 50 50.
\li -cyl Display a polygonal cylinder.
\li -iso <x> Display as an isosurface.
\li -clip Test clip plane.
\li -mt Test with parent MatrixTransforms.

If \c -iso is not specified, the test renders using the hardware mask
condifigured to display alpha values greater than 0.15. If \c -iso is
specified, the test renders alpha values equal to the specified value.

*/
