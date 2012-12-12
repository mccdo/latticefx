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

#include <latticefx/core/VolumeRenderer.h>
#include <latticefx/core/ChannelDataOSGArray.h>
#include <latticefx/core/ChannelDataOSGImage.h>
#include <latticefx/core/TextureUtils.h>
#include <latticefx/core/LogMacros.h>

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Texture1D>
#include <osg/Texture2D>
#include <osg/Texture3D>
#include <osg/BlendFunc>
#include <osg/Shader>
#include <osg/Program>
#include <osg/Uniform>
#include <osg/Depth>
#include <osg/CullFace>
#include <osgDB/FileUtils>


namespace lfx {
namespace core {


SpatialVolume::SpatialVolume()
  : _volumeDims( osg::Vec3( 1., 1., 1. ) ),
    _volumeOrigin( osg::Vec3( 0., 0., 0. ) )
{
}
SpatialVolume::SpatialVolume( const SpatialVolume& rhs )
  : _volumeDims( rhs._volumeDims ),
    _volumeOrigin( rhs._volumeOrigin )
{
}
SpatialVolume::~SpatialVolume()
{
}

void SpatialVolume::setVolumeDims( const osg::Vec3& volDims)
{
    _volumeDims = volDims;
}
osg::Vec3 SpatialVolume::getVolumeDims() const
{
    return( _volumeDims );
}

void SpatialVolume::setVolumeOrigin( const osg::Vec3& volOrigin)
{
    _volumeOrigin = volOrigin;
}
osg::Vec3 SpatialVolume::getVolumeOrigin() const
{
    return( _volumeOrigin );
}




VolumeRenderer::VolumeRenderer()
  : Renderer( "vol" ),
    _renderMode( SLICES ),
    _numPlanes( 100.f ),
    _maxSamples( 100.f )
{
    // Create and register uniform information, and initial/default values
    // (if we have them -- in some cases, we don't know the actual initial
    // values until scene graph creation).
    UniformInfo info;
    info = UniformInfo( "VolumeTexture", osg::Uniform::SAMPLER_3D, "Volume texture data sampler unit.", UniformInfo::PRIVATE );
    registerUniform( info );

    info = UniformInfo( "volumeDims", osg::Uniform::FLOAT_VEC3, "World coordinate volume texture dimensions.", UniformInfo::PRIVATE );
    registerUniform( info );

    info = UniformInfo( "volumeResolution", osg::Uniform::FLOAT_VEC3, "Volume texture resolution.", UniformInfo::PRIVATE );
    registerUniform( info );

    info = UniformInfo( "volumeCenter", osg::Uniform::FLOAT_VEC3, "Volume center location.", UniformInfo::PRIVATE );
    registerUniform( info );

    info = UniformInfo( "volumeNumPlanes", osg::Uniform::FLOAT, "Number of planes to render the volume." );
    registerUniform( info );

    info = UniformInfo( "volumeMaxSamples", osg::Uniform::FLOAT, "Max ray Samples for ray traced rendering." );
    registerUniform( info );

    info = UniformInfo( "volumeClipPlaneEnable0", osg::Uniform::INT, "Clip plane 0: 1=enabled, 0=disabled." );
    registerUniform( info );
    info = UniformInfo( "volumeClipPlaneEnable1", osg::Uniform::INT, "Clip plane 1: 1=enabled, 0=disabled." );
    registerUniform( info );
    info = UniformInfo( "volumeClipPlaneEnable2", osg::Uniform::INT, "Clip plane 2: 1=enabled, 0=disabled." );
    registerUniform( info );
    info = UniformInfo( "volumeClipPlaneEnable3", osg::Uniform::INT, "Clip plane 3: 1=enabled, 0=disabled." );
    registerUniform( info );
    info = UniformInfo( "volumeClipPlaneEnable4", osg::Uniform::INT, "Clip plane 4: 1=enabled, 0=disabled." );
    registerUniform( info );
    info = UniformInfo( "volumeClipPlaneEnable5", osg::Uniform::INT, "Clip plane 5: 1=enabled, 0=disabled." );
    registerUniform( info );
}
VolumeRenderer::VolumeRenderer( const VolumeRenderer& rhs )
  : Renderer( rhs ),
    _renderMode( rhs._renderMode ),
    _numPlanes( rhs._numPlanes ),
    _maxSamples( rhs._maxSamples )
{
}
VolumeRenderer::~VolumeRenderer()
{
}

// Used for SLICES mode.
osg::Geometry* VolumeRenderer::createDAIGeometry( int nInstances )
{
    osg::ref_ptr< osg::Geometry > geom( new osg::Geometry );
    geom->setUseDisplayList( false );
    geom->setUseVertexBufferObjects( true );

    const float halfDimX( .5 );
    const float halfDimZ( .5 );

    osg::Vec3Array* v = new osg::Vec3Array;
    v->resize( 4 );
    geom->setVertexArray( v );

    // Geometry for a single quad.
    (*v)[ 0 ] = osg::Vec3( -halfDimX, -halfDimZ, 0. );
    (*v)[ 1 ] = osg::Vec3( halfDimX, -halfDimZ, 0. );
    (*v)[ 2 ] = osg::Vec3( -halfDimX, halfDimZ, 0. );
    (*v)[ 3 ] = osg::Vec3( halfDimX, halfDimZ, 0. );

    // Use the DrawArraysInstanced PrimitiveSet and tell it to draw nInstances instances.
    geom->addPrimitiveSet( new osg::DrawArrays( GL_TRIANGLE_STRIP, 0, 4, nInstances ) );

    return( geom.release() );
}

// Used for RAY_TRACED mode.
osg::Geometry* VolumeRenderer::createCubeGeometry()
{
    osg::ref_ptr< osg::Geometry > geom( new osg::Geometry );
    geom->setUseDisplayList( false );
    geom->setUseVertexBufferObjects( true );

    const osg::Vec3 hd( _volumeDims * .5 );
    const osg::Vec3& c( _volumeOrigin );

    osg::Vec3Array* v( new osg::Vec3Array );
    v->resize( 8 );
    geom->setVertexArray( v );

    (*v)[0].set( -hd.x()+c.x(), -hd.y()+c.y(), -hd.z()+c.z() );
    (*v)[1].set( hd.x()+c.x(), -hd.y()+c.y(), -hd.z()+c.z() );
    (*v)[2].set( -hd.x()+c.x(), hd.y()+c.y(), -hd.z()+c.z() );
    (*v)[3].set( hd.x()+c.x(), hd.y()+c.y(), -hd.z()+c.z() );
    (*v)[4].set( -hd.x()+c.x(), -hd.y()+c.y(), hd.z()+c.z() );
    (*v)[5].set( hd.x()+c.x(), -hd.y()+c.y(), hd.z()+c.z() );
    (*v)[6].set( -hd.x()+c.x(), hd.y()+c.y(), hd.z()+c.z() );
    (*v)[7].set( hd.x()+c.x(), hd.y()+c.y(), hd.z()+c.z() );

    osg::Vec3Array* tc( new osg::Vec3Array );
    tc->resize( 8 );
    geom->setTexCoordArray( 0, tc );

    (*tc)[0].set( 0., 0., 0. );
    (*tc)[1].set( 1., 0., 0. );
    (*tc)[2].set( 0., 1., 0. );
    (*tc)[3].set( 1., 1., 0. );
    (*tc)[4].set( 0., 0., 1. );
    (*tc)[5].set( 1., 0., 1. );
    (*tc)[6].set( 0., 1., 1. );
    (*tc)[7].set( 1., 1., 1. );

    GLushort indices[] = {
        2, 0, 6, 6, 0, 4, // -x face
        1, 3, 5, 5, 3, 7,   // +x face
        0, 1, 4, 4, 1, 5, // -y face
        3, 2, 7, 7, 2, 6,   // +y face
        1, 0, 3, 3, 0, 2, // -z face
        4, 5, 6, 6, 5, 7    // +z face
    };

    osg::DrawElementsUShort* deus( new osg::DrawElementsUShort( GL_TRIANGLES, 36, indices ) );
    geom->addPrimitiveSet( deus );

    return( geom.release() );
}


osg::Node* VolumeRenderer::getSceneGraph( const ChannelDataPtr maskIn )
{
    osg::ref_ptr< osg::Geode > geode( new osg::Geode );

    osg::Geometry* geom;
    if( _renderMode == SLICES )
        geom = createDAIGeometry( _numPlanes );
    else
        geom = createCubeGeometry();
    geode->addDrawable( geom );

    // OSG has no clue where our vertex shader will place the geometric data,
    // so specify an initial bound to allow proper culling and near/far computation.
    osg::BoundingBox bb( (_volumeDims * -.5) + _volumeOrigin, (_volumeDims * .5) + _volumeOrigin);
    geom->setInitialBound( bb );


    osg::StateSet* stateSet( geode->getOrCreateStateSet() );

    ChannelDataPtr dataPtr( getInput( "volumedata" ) );
    if( dataPtr == NULL )
    {
        LFX_WARNING( "getSceneGraph(): Unable to find required volumedata ChannelData." );
        return( NULL );
    }
    ChannelDataOSGImage* dataImagePtr( static_cast<
        ChannelDataOSGImage* >( dataPtr.get() ) );


    // Create empty stub texture, to be paged in at run-time.
    const std::string imageFileName( dataImagePtr->getDBKey() );
    osg::Texture3D* volumeTexture( createStubTexture( imageFileName ) );
    volumeTexture->setDataVariance( osg::Object::DYNAMIC ); // for paging.

    stateSet->setTextureAttributeAndModes(
        getOrAssignTextureUnit( "volumeTex" ), volumeTexture );

    {
        UniformInfo& info( getUniform( "volumeResolution" ) );
        unsigned int x, y, z;
        dataImagePtr->getDimensions( x, y, z);
        osg::Vec3 res( x, y, z );
        if( res.length2() == 0. )
            // Must be a paged texture and we don't have the data yet.?
            // Add a bogus resolution, just so we have something.
            res.set( 100., 100., 100. );
        info._prototype->set( res );
        stateSet->addUniform( createUniform( info ), osg::StateAttribute::PROTECTED );
    }


    return( geode.release() );
}


osg::StateSet* VolumeRenderer::getRootState()
{
    osg::ref_ptr< osg::StateSet > stateSet( new osg::StateSet() );

    // position, direction, transfer function input, and hardware mask input texture
    // units are the same for all time steps, so set their sampler uniform unit
    // values in the root state.
    {
        UniformInfo& info( getUniform( "VolumeTexture" ) );
        info._prototype->set( getOrAssignTextureUnit( "volumeTex" ) );
        stateSet->addUniform( createUniform( info ), osg::StateAttribute::PROTECTED );
    }

    if( ( getTransferFunction() != NULL ) &&
        !( getTransferFunctionInput().empty() ) )
    {
        LFX_WARNING( "getRootState(): Transfer function input is not supported and will be ignored." );
    }

    // Set default values for volume shader uniforms.
    {
        UniformInfo& info( getUniform( "volumeDims" ) );
        info._prototype->set( osg::Vec3f( _volumeDims ) );
        stateSet->addUniform( createUniform( info ), osg::StateAttribute::PROTECTED );
    }
    {
        UniformInfo& info( getUniform( "volumeCenter" ) );
        info._prototype->set( osg::Vec3f( _volumeOrigin ) );
        stateSet->addUniform( createUniform( info ), osg::StateAttribute::PROTECTED );
    }
    if( _renderMode == SLICES )
    {
        {
            UniformInfo& info( getUniform( "volumeNumPlanes" ) );
            info._prototype->set( _numPlanes );
            stateSet->addUniform( createUniform( info ) );
        }
    }
    else
    {
        {
            UniformInfo& info( getUniform( "volumeMaxSamples" ) );
            info._prototype->set( _maxSamples );
            stateSet->addUniform( createUniform( info ) );
        }
    }

    {
        UniformInfo& info( getUniform( "volumeClipPlaneEnables" ) );
        for( int i=0; i<6; i++ )
            info._prototype->setElement( i, 0.f );
        stateSet->addUniform( createUniform( info ) );
    }

    if( _renderMode == SLICES )
    {
        // RAY_TRACED does blending internally with the frag shader.
        // SLICES must turn it on explicitly.
        osg::BlendFunc *fn = new osg::BlendFunc();
        fn->setFunction(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::ONE_MINUS_SRC_ALPHA);
        stateSet->setAttributeAndModes( fn, osg::StateAttribute::ON );

        // Do not need to write the depth buffer.
        osg::Depth* depth( new osg::Depth( osg::Depth::LESS, 0., 1., false ) );
        stateSet->setAttributeAndModes( depth );
    }
    else // RAY_TRACED
    {
        // For ray traced, disable depth test (the shader will take care
        // of that). and render back faces only (back half of volume).
        stateSet->setMode( GL_DEPTH_TEST, osg::StateAttribute::OFF );

        osg::CullFace* cf( new osg::CullFace( osg::CullFace::FRONT ) );
        stateSet->setAttributeAndModes( cf );
    }

    // Set base class transfer function and volume texture uniforms.
    addHardwareFeatureUniforms( stateSet.get() );

    osg::Program* program = new osg::Program();
    stateSet->setAttribute( program );
    if( _renderMode == SLICES )
    {
        program->addShader( loadShader( osg::Shader::VERTEX, "lfx-volumetricslice.vert" ) );
        program->addShader( loadShader( osg::Shader::FRAGMENT, "lfx-volumetricslice.frag" ) );
    }
    else
    {
        program->addShader( loadShader( osg::Shader::VERTEX, "lfx-volumeraytrace.vert" ) );
        program->addShader( loadShader( osg::Shader::FRAGMENT, "lfx-volumeraytrace.frag" ) );
    }

    return( stateSet.release() );
}

void VolumeRenderer::setNumPlanes( const float& numPlanes )
{
    if( numPlanes <= 0. )
    {
        LFX_WARNING( "setNumPlanes must be > 0.0. Using 100.0." );
        _numPlanes = 100.;
    }
    else
        _numPlanes = numPlanes;
}
float VolumeRenderer::getNumPlanes() const
{
    return( _numPlanes );
}

osg::Texture3D* VolumeRenderer::createStubTexture( const DBKey& key )
{
    // Create dummy Texture / Image as placeholder until real image data is paged in.
    osg::ref_ptr< osg::Texture3D > tex( new osg::Texture3D );
    tex->setResizeNonPowerOfTwoHint( false );
    tex->setFilter( osg::Texture::MIN_FILTER, osg::Texture::LINEAR );
    tex->setFilter( osg::Texture::MAG_FILTER, osg::Texture::LINEAR );
    tex->setWrap(osg::Texture2D::WRAP_R, osg::Texture2D::CLAMP);
    tex->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP);
    tex->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP);
    tex->setBorderColor( osg::Vec4d( 0., 0., 0., 0. ) );
    osg::Image* dummyImage( new osg::Image );
    dummyImage->setFileName( key );
    tex->setImage( dummyImage );

    return( tex.release() );
}


// core
}
// lfx
}
