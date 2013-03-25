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

#include <latticefx/core/VectorRenderer.h>
#include <latticefx/core/ChannelDataOSGArray.h>
#include <latticefx/core/TextureUtils.h>
#include <latticefx/core/BoundUtils.h>
#include <latticefx/core/LogMacros.h>

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Texture1D>
#include <osg/Texture2D>
#include <osg/Texture3D>
#include <osg/Shader>
#include <osg/Program>
#include <osg/Uniform>

#include <osgwTools/Shapes.h>
#include <string>



// When sending transfer function / hardware mask inputs as vertex attributes,
// specify these vertex attrib locations.
#define TF_INPUT_ATTRIB 14
#define HM_SOURCE_ATTRIB 15
// Note: GeForce 9800M supports only 0 .. 15.


namespace lfx
{
namespace core
{
////////////////////////////////////////////////////////////////////////////////
VectorRenderer::VectorRenderer()
    : Renderer( "vec" ),
      _pointStyle( SIMPLE_POINTS )
{
    // Specify default ChannelData name aliases for the required inputs.
    setInputNameAlias( POSITION, "positions" );
    setInputNameAlias( DIRECTION, "directions" );
    setInputNameAlias( RADIUS, "radii" );

    // Create and register uniform information, and initial/default values
    // (if we have them -- in some cases, we don't know the actual initial
    // values until scene graph creation).
    UniformInfo info;
    info = UniformInfo( "texDim", osg::Uniform::FLOAT_VEC3, "Texture dimensions for instanced rendering data.", UniformInfo::PRIVATE );
    registerUniform( info );

    info = UniformInfo( "texPos", osg::Uniform::SAMPLER_3D, "Position texture data sampler unit.", UniformInfo::PRIVATE );
    registerUniform( info );

    info = UniformInfo( "texRad", osg::Uniform::SAMPLER_3D, "Radius texture data sampler unit.", UniformInfo::PRIVATE );
    registerUniform( info );

    info = UniformInfo( "texDir", osg::Uniform::SAMPLER_3D, "Vector direction texture data sampler unit.", UniformInfo::PRIVATE );
    registerUniform( info );

    info = UniformInfo( "tfInput", osg::Uniform::SAMPLER_3D, "Transfer function input data sampler unit.", UniformInfo::PRIVATE );
    registerUniform( info );

    info = UniformInfo( "hmInput", osg::Uniform::SAMPLER_3D, "Hardware mask input data sampler unit.", UniformInfo::PRIVATE );
    registerUniform( info );
}
////////////////////////////////////////////////////////////////////////////////
VectorRenderer::VectorRenderer( const VectorRenderer& rhs )
    : Renderer( rhs ),
      _pointStyle( rhs._pointStyle )
{
}
////////////////////////////////////////////////////////////////////////////////
VectorRenderer::~VectorRenderer()
{
}
////////////////////////////////////////////////////////////////////////////////
osg::Node* VectorRenderer::getSceneGraph( const ChannelDataPtr maskIn )
{
    osg::ref_ptr< osg::Geode > geode( new osg::Geode );

    // Get the input position data as an osg::Vec3Array. Position data is required
    // for all rendering styles.
    // Get the position data using the input type alias for the POSITION input type.
    ChannelDataPtr posAlias( getInput( getInputNameAlias( POSITION ) ) );
    if( posAlias == NULL )
    {
        LFX_WARNING( "getSceneGraph(): Unable to find required POSITION ChannelData." );
        return( NULL );
    }
    ChannelDataPtr posChannel( posAlias->getMaskedChannel( maskIn ) );
    osg::Array* sourceArray( posChannel->asOSGArray() );
    const unsigned int numElements( sourceArray->getNumElements() );
    osg::Vec3Array* positions( static_cast< osg::Vec3Array* >( sourceArray ) );

    switch( _pointStyle )
    {
    default:
    case SIMPLE_POINTS:
    {
        osg::Geometry* geom( new osg::Geometry() );
        geom->setUseDisplayList( false );
        geom->setUseVertexBufferObjects( true );

        // Set vertex array from our osg::Vec3Array of position data.
        geom->setVertexArray( positions );

        // Note that the transfer function input must be specified in the
        // Renderer-derived class rather than Renderer::addHardwareFeatureUniforms()
        // because the derived class might require the input data in a specific
        // format. SPHERES and DIRECTION_VECTORS, which use instanced rendering, require
        // a 3D texture. But SIMPLE_POINTS requires a generic vertex attrib.
        if( getTransferFunction() != NULL )
        {
            // If using a transfer function, we must have a valid input.
            const ChannelDataPtr tfInputByName( getInput( getTransferFunctionInput() ) );
            if( tfInputByName == NULL )
            {
                LFX_WARNING( "getSceneGraph(): Unable to find input \"" +
                             getTransferFunctionInput() + "\"." );
                return( NULL );
            }
            const ChannelDataPtr tfInputChannel( tfInputByName->getMaskedChannel( maskIn ) );
            osg::Array* tfInputArray( tfInputChannel->asOSGArray() );
            // simplepoints shader supports only vec3 tf input. Convert the tf input data to a vec3 array.
            osg::Vec3Array* tfInputArray3( ChannelDataOSGArray::convertToVec3Array( tfInputArray ) );
            geom->setVertexAttribArray( TF_INPUT_ATTRIB, tfInputArray3 );
            geom->setVertexAttribBinding( TF_INPUT_ATTRIB, osg::Geometry::BIND_PER_VERTEX );
        }

        // As above, hardware mask input not handled by Renderer::addHardwareFeatureUniforms().
        if( getHardwareMaskInputSource() == HM_SOURCE_SCALAR )
        {
            // Hardware mask input is a scalar. Find the ChannelData and return an error
            // if it's not present.
            const ChannelDataPtr hmInputByName( getInput( getHardwareMaskInput() ) );
            if( hmInputByName == NULL )
            {
                LFX_WARNING( "getSceneGraph(): Unable to find input \"" +
                             getHardwareMaskInput() + "\"." );
                return( NULL );
            }
            const ChannelDataPtr hmInputChannel( hmInputByName->getMaskedChannel( maskIn ) );
            osg::Array* hmInputArray( hmInputChannel->asOSGArray() );
            geom->setVertexAttribArray( HM_SOURCE_ATTRIB, hmInputArray );
            geom->setVertexAttribBinding( HM_SOURCE_ATTRIB, osg::Geometry::BIND_PER_VERTEX );
        }

        // Draw points.
        geom->addPrimitiveSet( new osg::DrawArrays( GL_POINTS, 0, numElements ) );
        geode->addDrawable( geom );
        break;
    }
    case POINT_SPRITES:
    {
        LFX_CRITICAL( "getSceneGraph(): POINT_SPRITES style is not yet implemented." );
        break;
    }
    case SPHERES:
    {
        // Geodesic sphere with subdivision=1 produces 20 sides. sub=2 produces 80 sides.
        osg::Geometry* geom( osgwTools::makeGeodesicSphere( 1., 1 ) );
        geom->setUseDisplayList( false );
        geom->setUseVertexBufferObjects( true );
        // TBD bound pad needs to be settable.
        geom->setInitialBound( lfx::core::getBound( *positions, osg::Vec3( 1., 1., 1. ) ) );
        geode->addDrawable( geom );

        // Set the number of instances.
        unsigned int idx;
        for( idx = 0; idx < geom->getNumPrimitiveSets(); ++idx )
        {
            geom->getPrimitiveSet( idx )->setNumInstances( numElements );
        }

        osg::StateSet* stateSet( geode->getOrCreateStateSet() );

        // We send down instanced data in a 3D texture. We use the instance ID to generate
        // a 3D texture coordinate and retrieve the data from the array. Get the dimensions,
        // and pass it to the shader (required so that it can compute the texture coordinates).
        //
        // DO NOT specify texDim uniform in getRootState(). The number of elements in the
        // input ChannelData could vary from one time step to the next, which affects the
        // size of the data textures and therefore the texDim uniform values. It must be
        // specified per time step.
        const osg::Vec3f dimensions( lfx::core::computeTexture3DDimensions( numElements ) );
        {
            UniformInfo& info( getUniform( "texDim" ) );
            info._prototype->set( dimensions );
            stateSet->addUniform( createUniform( info ), osg::StateAttribute::PROTECTED );
        }

        // Create image data and store in DB.
        osg::Texture3D* posTex( createTexture( posChannel ) );

        const unsigned int posTexUnit( getOrAssignTextureUnit( "posTex" ) );
        stateSet->setTextureAttributeAndModes( posTexUnit, posTex, osg::StateAttribute::OFF );

        // Get the radius data using the input type alias for the RADIUS input type.
        ChannelDataPtr radAlias( getInput( getInputNameAlias( RADIUS ) ) );
        if( radAlias == NULL )
        {
            LFX_WARNING( "getSceneGraph(): Unable to find required RADIUS ChannelData." );
            return( NULL );
        }
        const ChannelDataPtr radChannel( radAlias->getMaskedChannel( maskIn ) );

        // Create image data and store in DB.
        osg::Texture3D* radTex( createTexture( radChannel ) );

        const unsigned int radTexUnit( getOrAssignTextureUnit( "radTex" ) );
        stateSet->setTextureAttributeAndModes( radTexUnit, radTex, osg::StateAttribute::OFF );

        // Note that the transfer function input must be specified in the
        // Renderer-derived class rather than Renderer::addHardwareFeatureUniforms()
        // because the derived class might require the input data in a specific
        // format. SPHERES and DIRECTION_VECTORS, which use instanced rendering, require
        // a 3D texture. But SIMPLE_POINTS requires a generic vertex attrib.
        if( getTransferFunction() != NULL )
        {
            const ChannelDataPtr tfInputByName( getInput( getTransferFunctionInput() ) );
            if( tfInputByName == NULL )
            {
                LFX_WARNING( "getSceneGraph(): Unable to find input \"" +
                             getTransferFunctionInput() + "\"." );
                return( NULL );
            }
            const ChannelDataPtr tfInputChannel( tfInputByName->getMaskedChannel( maskIn ) );

            osg::Texture3D* tfInputTex( createTexture( tfInputChannel ) );

            const unsigned int tfInputUnit( getOrAssignTextureUnit( "tfInput" ) );
            stateSet->setTextureAttributeAndModes( tfInputUnit, tfInputTex, osg::StateAttribute::OFF );
        }

        // As above, hardware mask input not handled by Renderer::addHardwareFeatureUniforms().
        if( getHardwareMaskInputSource() == HM_SOURCE_SCALAR )
        {
            // Hardware mask input is a scalar. Find the ChannelData and return an error
            // if it's not present.
            const ChannelDataPtr hmInputByName( getInput( getHardwareMaskInput() ) );
            if( hmInputByName == NULL )
            {
                LFX_WARNING( "getSceneGraph(): Unable to find input \"" +
                             getHardwareMaskInput() + "\"." );
                return( NULL );
            }
            const ChannelDataPtr hmInputChannel( hmInputByName->getMaskedChannel( maskIn ) );

            osg::Texture3D* hmInputTex( createTexture( hmInputChannel ) );

            const unsigned int hmInputUnit( getOrAssignTextureUnit( "hmInput" ) );
            stateSet->setTextureAttributeAndModes( hmInputUnit, hmInputTex, osg::StateAttribute::OFF );
        }
        break;
    }
    case DIRECTION_VECTORS:
    {
        osg::Geometry* geom( osgwTools::makeArrow() );
        geom->setUseDisplayList( false );
        geom->setUseVertexBufferObjects( true );
        // TBD bound pad needs to be settable.
        geom->setInitialBound( lfx::core::getBound( *positions, osg::Vec3( 1., 1., 1. ) ) );
        geode->addDrawable( geom );

        // Set the number of instances.
        unsigned int idx;
        for( idx = 0; idx < geom->getNumPrimitiveSets(); ++idx )
        {
            geom->getPrimitiveSet( idx )->setNumInstances( numElements );
        }

        osg::StateSet* stateSet( geode->getOrCreateStateSet() );

        // We send down instanced data in a 3D texture. We use the instance ID to generate
        // a 3D texture coordinate and retrieve the data from the array. Get the dimensions,
        // and pass it to the shader (required so that it can compute the texture coordinates).
        //
        // DO NOT specify texDim uniform in getRootState(). The number of elements in the
        // input ChannelData could vary from one time step to the next, which affects the
        // size of the data textures and therefore the texDim uniform values. It must be
        // specified per time step.
        const osg::Vec3f dimensions( lfx::core::computeTexture3DDimensions( numElements ) );
        {
            UniformInfo& info( getUniform( "texDim" ) );
            info._prototype->set( dimensions );
            stateSet->addUniform( createUniform( info ), osg::StateAttribute::PROTECTED );
        }

        osg::Texture3D* posTex( createTexture( posChannel ) );

        const unsigned int posUnit( getOrAssignTextureUnit( "posTex" ) );
        stateSet->setTextureAttributeAndModes( posUnit, posTex, osg::StateAttribute::OFF );

        // Get the direction data using the input type alias for the DIRECTION input type.
        ChannelDataPtr dirAlias( getInput( getInputNameAlias( DIRECTION ) ) );
        if( dirAlias == NULL )
        {
            LFX_WARNING( "getSceneGraph(): Unable to find required DIRECTION ChannelData." );
            return( NULL );
        }
        const ChannelDataPtr dirChannel( dirAlias->getMaskedChannel( maskIn ) );
        osg::Texture3D* dirTex( createTexture( dirChannel ) );
        const unsigned int dirUnit( getOrAssignTextureUnit( "dirTex" ) );
        stateSet->setTextureAttributeAndModes( dirUnit, dirTex, osg::StateAttribute::OFF );

        // Note that the transfer function input must be specified in the
        // Renderer-derived class rather than Renderer::addHardwareFeatureUniforms()
        // because the derived class might require the input data in a specific
        // format. SPHERES and DIRECTION_VECTORS, which use instanced rendering, require
        // a 3D texture. But SIMPLE_POINTS requires a generic vertex attrib.
        if( getTransferFunction() != NULL )
        {
            const ChannelDataPtr tfInputByName( getInput( getTransferFunctionInput() ) );
            if( tfInputByName == NULL )
            {
                LFX_WARNING( "getSceneGraph(): Unable to find input \"" +
                             getTransferFunctionInput() + "\"." );
                return( NULL );
            }
            const ChannelDataPtr tfInputChannel( tfInputByName->getMaskedChannel( maskIn ) );
            osg::Texture3D* tfInputTex( createTexture( tfInputChannel ) );
            const unsigned int tfInputUnit( getOrAssignTextureUnit( "tfInput" ) );
            stateSet->setTextureAttributeAndModes( tfInputUnit, tfInputTex, osg::StateAttribute::OFF );
        }

        // As above, hardware mask input not handled by Renderer::addHardwareFeatureUniforms().
        if( getHardwareMaskInputSource() == HM_SOURCE_SCALAR )
        {
            // Hardware mask input is a scalar. Find the ChannelData and return an error
            // if it's not present.
            const ChannelDataPtr hmInputByName( getInput( getHardwareMaskInput() ) );
            if( hmInputByName == NULL )
            {
                LFX_WARNING( "getSceneGraph(): Unable to find input \"" +
                             getHardwareMaskInput() + "\"." );
                return( NULL );
            }
            const ChannelDataPtr hmInputChannel( hmInputByName->getMaskedChannel( maskIn ) );
            osg::Texture3D* hmInputTex( createTexture( hmInputChannel ) );
            const unsigned int hmInputUnit( getOrAssignTextureUnit( "hmInput" ) );
            stateSet->setTextureAttributeAndModes( hmInputUnit, hmInputTex, osg::StateAttribute::OFF );
        }
        break;
    }
    }

    return( geode.release() );
}
////////////////////////////////////////////////////////////////////////////////
osg::StateSet* VectorRenderer::getRootState()
{
    osg::ref_ptr< osg::StateSet > stateSet( new osg::StateSet() );

    switch( _pointStyle )
    {
    default:
    case SIMPLE_POINTS:
    {
        // Set base class transfer function and hardware mask uniforms.
        addHardwareFeatureUniforms( stateSet.get() );

        osg::Program* program = new osg::Program();
        program->addBindAttribLocation( "tfInput", TF_INPUT_ATTRIB );
        stateSet->setAttribute( program );
        program->addShader( loadShader( osg::Shader::VERTEX, "lfx-simplepoints.vert" ) );
        program->addShader( loadShader( osg::Shader::FRAGMENT, "lfx-simplepoints.frag" ) );
        break;
    }
    case POINT_SPRITES:
    {
        LFX_CRITICAL( "getRootState(): POINT_SPRITES style is not yet implemented." );
        break;
    }
    case SPHERES:
    {
        // Required for correct lighting of clipped spheres.
        stateSet->setMode( GL_VERTEX_PROGRAM_TWO_SIDE, osg::StateAttribute::ON );

        // position, radius, transfer function input, and hardware mask input texture
        // units are the same for all time steps, so set their sampler uniform unit
        // values in the root state.
        {
            UniformInfo& info( getUniform( "texPos" ) );
            info._prototype->set( getOrAssignTextureUnit( "posTex" ) );
            stateSet->addUniform( createUniform( info ), osg::StateAttribute::PROTECTED );
        }

        {
            UniformInfo& info( getUniform( "texRad" ) );
            info._prototype->set( getOrAssignTextureUnit( "radTex" ) );
            stateSet->addUniform( createUniform( info ), osg::StateAttribute::PROTECTED );
        }

        if( getTransferFunction() != NULL )
        {
            UniformInfo& info( getUniform( "tfInput" ) );
            info._prototype->set( getOrAssignTextureUnit( "tfInput" ) );
            stateSet->addUniform( createUniform( info ), osg::StateAttribute::PROTECTED );
        }

        if( getHardwareMaskInputSource() == HM_SOURCE_SCALAR )
        {
            UniformInfo& info( getUniform( "hmInput" ) );
            info._prototype->set( getOrAssignTextureUnit( "hmInput" ) );
            stateSet->addUniform( createUniform( info ), osg::StateAttribute::PROTECTED );
        }

        // Set base class transfer function and hardware mask uniforms.
        addHardwareFeatureUniforms( stateSet.get() );


        osg::Program* program = new osg::Program();
        stateSet->setAttribute( program );
        program->addShader( loadShader( osg::Shader::VERTEX, "lfx-pointspheres.vert" ) );
        program->addShader( loadShader( osg::Shader::FRAGMENT, "lfx-pointspheres.frag" ) );
        break;
    }
    case DIRECTION_VECTORS:
    {
        // Required for correct lighting of clipped arrows.
        stateSet->setMode( GL_VERTEX_PROGRAM_TWO_SIDE, osg::StateAttribute::ON );

        // position, direction, transfer function input, and hardware mask input texture
        // units are the same for all time steps, so set their sampler uniform unit
        // values in the root state.
        {
            UniformInfo& info( getUniform( "texPos" ) );
            info._prototype->set( getOrAssignTextureUnit( "posTex" ) );
            stateSet->addUniform( createUniform( info ), osg::StateAttribute::PROTECTED );
        }

        {
            UniformInfo& info( getUniform( "texDir" ) );
            info._prototype->set( getOrAssignTextureUnit( "dirTex" ) );
            stateSet->addUniform( createUniform( info ), osg::StateAttribute::PROTECTED );
        }

        if( getTransferFunction() != NULL )
        {
            const unsigned int tfInputUnit( getOrAssignTextureUnit( "tfInput" ) );
            osg::Uniform* tfInputUni( new osg::Uniform( osg::Uniform::SAMPLER_3D, "tfInput" ) );
            tfInputUni->set( ( int )tfInputUnit );
            stateSet->addUniform( tfInputUni );
        }

        if( getHardwareMaskInputSource() == HM_SOURCE_SCALAR )
        {
            UniformInfo& info( getUniform( "hmInput" ) );
            info._prototype->set( getOrAssignTextureUnit( "hmInput" ) );
            stateSet->addUniform( createUniform( info ), osg::StateAttribute::PROTECTED );
        }

        // Set base class transfer function and hardware mask uniforms.
        addHardwareFeatureUniforms( stateSet.get() );


        osg::Program* program = new osg::Program();
        stateSet->setAttribute( program );
        program->addShader( loadShader( osg::Shader::VERTEX, "lfx-vectorfield.vert" ) );
        program->addShader( loadShader( osg::Shader::FRAGMENT, "lfx-vectorfield.frag" ) );
        break;
    }
    }

    return( stateSet.release() );
}
////////////////////////////////////////////////////////////////////////////////
void VectorRenderer::setPointStyle( const PointStyle& pointStyle )
{
    _pointStyle = pointStyle;
}
////////////////////////////////////////////////////////////////////////////////
VectorRenderer::PointStyle VectorRenderer::getPointStyle() const
{
    return( _pointStyle );
}
////////////////////////////////////////////////////////////////////////////////
osg::Texture3D* VectorRenderer::createTexture( ChannelDataPtr data )
{
    osg::ref_ptr< osg::Texture3D > tex( new osg::Texture3D );
    tex->setResizeNonPowerOfTwoHint( false );
    tex->setFilter( osg::Texture::MIN_FILTER, osg::Texture::NEAREST );
    tex->setFilter( osg::Texture::MAG_FILTER, osg::Texture::NEAREST );
    tex->setUseHardwareMipMapGeneration( false );

    // Create Image representation of the data using special
    // createImage3DForInstancedRenderer() tool.
    osg::ref_ptr< osg::Image > image( createImage3DForInstancedRenderer( data ) );
    image->setFileName( data->getName() );

    if( _db != NULL )
    {
        // Store the actual data in the DB.
        const DBKey key( _db->generateDBKey() );
        image->setFileName( key );
        _db->storeImage( image.get(), key );

        // Now create a dummy image containing no data and stick this in the Texture3D.
        // The actual data will be paged in at runtime.
        osg::Image* dummyImage( new osg::Image );
        dummyImage->setFileName( image->getFileName() );
        tex->setImage( dummyImage );
    }
    else
    {
        // Store the actual data in the scene graph.
        tex->setImage( image.get() );
        tex->setName( "donotpage" );
    }

    return( tex.release() );
}
////////////////////////////////////////////////////////////////////////////////
// core
}
// lfx
}
