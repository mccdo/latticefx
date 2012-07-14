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

#include <latticefx/core/VectorRenderer.h>
#include <latticefx/core/ChannelDataOSGArray.h>
#include <latticefx/core/TextureUtils.h>
#include <latticefx/core/BoundUtils.h>
#include <latticefx/core/DBUtils.h>
#include <latticefx/core/LogMacros.h>

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Texture1D>
#include <osg/Texture2D>
#include <osg/Texture3D>
#include <osg/Shader>
#include <osg/Program>
#include <osg/Uniform>
#include <osgDB/FileUtils>

#include <osgwTools/Shapes.h>
#include <string>



// When sending transfer function / hardware mask inputs as vertex attributes,
// specify these vertex attrib locations.
#define TF_INPUT_ATTRIB 14
#define HM_SOURCE_ATTRIB 15
// Note: GeForce 9800M supports only 0 .. 15.


namespace lfx {
namespace core {


VectorRenderer::VectorRenderer()
  : Renderer( "vec" ),
    _pointStyle( SIMPLE_POINTS )
{
    // Specify default ChannelData name aliases for the required inputs.
    setInputNameAlias( POSITION, "positions" );
    setInputNameAlias( DIRECTION, "directions" );
    setInputNameAlias( RADIUS, "radii" );
}
VectorRenderer::VectorRenderer( const VectorRenderer& rhs )
  : Renderer( rhs ),
    _pointStyle( rhs._pointStyle ),
    _inputTypeMap( rhs._inputTypeMap )
{
}
VectorRenderer::~VectorRenderer()
{
}

osg::Node* VectorRenderer::getSceneGraph( const ChannelDataPtr maskIn )
{
    osg::ref_ptr< osg::Geode > geode( new osg::Geode );

    // Get the input position data as an osg::Vec3Array. Position data is required
    // for all rendering styles.
    // Get the position data using the input type alias for the POSITION input type.
    ChannelDataPtr posAlias( getInput( getInputTypeAlias( POSITION ) ) );
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
            geom->setVertexAttribArray( TF_INPUT_ATTRIB, tfInputArray );
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
        for( idx=0; idx < geom->getNumPrimitiveSets(); ++idx )
            geom->getPrimitiveSet( idx )->setNumInstances( numElements );

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
        osg::Uniform* texDim( new osg::Uniform( "texDim", dimensions ) );
        stateSet->addUniform( texDim );

        // Create image data and store in DB.
        osg::Texture3D* posTex( createDummyDBTexture( posChannel ) );

        const unsigned int posTexUnit( getOrAssignTextureUnit( "posTex" ) );
        stateSet->setTextureAttributeAndModes( posTexUnit, posTex, osg::StateAttribute::OFF );

        // Get the radius data using the input type alias for the RADIUS input type.
        ChannelDataPtr radAlias( getInput( getInputTypeAlias( RADIUS ) ) );
        if( radAlias == NULL )
        {
            LFX_WARNING( "getSceneGraph(): Unable to find required RADIUS ChannelData." );
            return( NULL );
        }
        const ChannelDataPtr radChannel( radAlias->getMaskedChannel( maskIn ) );

        // Create image data and store in DB.
        osg::Texture3D* radTex( createDummyDBTexture( radChannel ) );

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

            osg::Texture3D* tfInputTex( createDummyDBTexture( tfInputChannel ) );

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

            osg::Texture3D* hmInputTex( createDummyDBTexture( hmInputChannel ) );

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
        for( idx=0; idx < geom->getNumPrimitiveSets(); ++idx )
            geom->getPrimitiveSet( idx )->setNumInstances( numElements );

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
        osg::Uniform* texDim( new osg::Uniform( "texDim", dimensions ) );
        stateSet->addUniform( texDim );

        osg::Texture3D* posTex( createDummyDBTexture( posChannel ) );

        const unsigned int posUnit( getOrAssignTextureUnit( "posTex" ) );
        stateSet->setTextureAttributeAndModes( posUnit, posTex, osg::StateAttribute::OFF );

        // Get the direction data using the input type alias for the DIRECTION input type.
        ChannelDataPtr dirAlias( getInput( getInputTypeAlias( DIRECTION ) ) );
        if( dirAlias == NULL )
        {
            LFX_WARNING( "getSceneGraph(): Unable to find required DIRECTION ChannelData." );
            return( NULL );
        }
        const ChannelDataPtr dirChannel( dirAlias->getMaskedChannel( maskIn ) );
        osg::Texture3D* dirTex( createDummyDBTexture( dirChannel ) );
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
            osg::Texture3D* tfInputTex( createDummyDBTexture( tfInputChannel ) );
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
            osg::Texture3D* hmInputTex( createDummyDBTexture( hmInputChannel ) );
            const unsigned int hmInputUnit( getOrAssignTextureUnit( "hmInput" ) );
            stateSet->setTextureAttributeAndModes( hmInputUnit, hmInputTex, osg::StateAttribute::OFF );
        }
        break;
    }
    }

    return( geode.release() );
}

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
        program->addShader( loadShader( osg::Shader::VERTEX, "lfx-simplepoints.vs" ) );
        program->addShader( loadShader( osg::Shader::FRAGMENT, "lfx-simplepoints.fs" ) );
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
        const unsigned int posTexUnit( getOrAssignTextureUnit( "posTex" ) );
        osg::Uniform* posUni( new osg::Uniform( osg::Uniform::SAMPLER_3D, "texPos" ) ); posUni->set( (int)posTexUnit );
        stateSet->addUniform( posUni );

        const unsigned int radTexUnit( getOrAssignTextureUnit( "radTex" ) );
        osg::Uniform* radUni( new osg::Uniform( osg::Uniform::SAMPLER_3D, "texRad" ) ); radUni->set( (int)radTexUnit );
        stateSet->addUniform( radUni );

        if( getTransferFunction() != NULL )
        {
            const unsigned int tfInputUnit( getOrAssignTextureUnit( "tfInput" ) );
            osg::Uniform* tfInputUni( new osg::Uniform( osg::Uniform::SAMPLER_3D, "tfInput" ) ); tfInputUni->set( (int)tfInputUnit );
            stateSet->addUniform( tfInputUni );
        }

        if( getHardwareMaskInputSource() == HM_SOURCE_SCALAR )
        {
            const unsigned int hmInputUnit( getOrAssignTextureUnit( "hmInput" ) );
            osg::Uniform* hmInputUni( new osg::Uniform( osg::Uniform::SAMPLER_3D, "hmInput" ) ); hmInputUni->set( (int)hmInputUnit );
            stateSet->addUniform( hmInputUni );
        }

        // Set base class transfer function and hardware mask uniforms.
        addHardwareFeatureUniforms( stateSet.get() );


        osg::Program* program = new osg::Program();
        stateSet->setAttribute( program );
        program->addShader( loadShader( osg::Shader::VERTEX, "lfx-pointspheres.vs" ) );
        program->addShader( loadShader( osg::Shader::FRAGMENT, "lfx-pointspheres.fs" ) );
        break;
    }
    case DIRECTION_VECTORS:
    {
        // Required for correct lighting of clipped arrows.
        stateSet->setMode( GL_VERTEX_PROGRAM_TWO_SIDE, osg::StateAttribute::ON );

        // position, direction, transfer function input, and hardware mask input texture
        // units are the same for all time steps, so set their sampler uniform unit
        // values in the root state.
        const unsigned int posTexUnit( getOrAssignTextureUnit( "posTex" ) );
        osg::Uniform* posUni( new osg::Uniform( osg::Uniform::SAMPLER_3D, "texPos" ) ); posUni->set( (int)posTexUnit );
        stateSet->addUniform( posUni );

        const unsigned int dirTexUnit( getOrAssignTextureUnit( "dirTex" ) );
        osg::Uniform* dirUni( new osg::Uniform( osg::Uniform::SAMPLER_3D, "texDir" ) ); dirUni->set( (int)dirTexUnit );
        stateSet->addUniform( dirUni );

        if( getTransferFunction() != NULL )
        {
            const unsigned int tfInputUnit( getOrAssignTextureUnit( "tfInput" ) );
            osg::Uniform* tfInputUni( new osg::Uniform( osg::Uniform::SAMPLER_3D, "tfInput" ) ); tfInputUni->set( (int)tfInputUnit );
            stateSet->addUniform( tfInputUni );
        }

        if( getHardwareMaskInputSource() == HM_SOURCE_SCALAR )
        {
            const unsigned int hmInputUnit( getOrAssignTextureUnit( "hmInput" ) );
            osg::Uniform* hmInputUni( new osg::Uniform( osg::Uniform::SAMPLER_3D, "hmInput" ) ); hmInputUni->set( (int)hmInputUnit );
            stateSet->addUniform( hmInputUni );
        }

        // Set base class transfer function and hardware mask uniforms.
        addHardwareFeatureUniforms( stateSet.get() );


        osg::Program* program = new osg::Program();
        stateSet->setAttribute( program );
        program->addShader( loadShader( osg::Shader::VERTEX, "lfx-vectorfield.vs" ) );
        program->addShader( loadShader( osg::Shader::FRAGMENT, "lfx-vectorfield.fs" ) );
        break;
    }
    }

    return( stateSet.release() );
}


void VectorRenderer::setPointStyle( const PointStyle& pointStyle )
{
    _pointStyle = pointStyle;
}
VectorRenderer::PointStyle VectorRenderer::getPointStyle() const
{
    return( _pointStyle );
}

void VectorRenderer::setInputNameAlias( const InputType& inputType, const std::string& alias )
{
    _inputTypeMap[ inputType ] = alias;
}
std::string VectorRenderer::getInputTypeAlias( const InputType& inputType ) const
{
    InputTypeMap::const_iterator it( _inputTypeMap.find( inputType ) );
    if( it != _inputTypeMap.end() )
        // Found it.
        return( it->second );
    else
        // Should never happen, as the constructor assigns defaults.
        return( "" );
}


osg::Texture3D* VectorRenderer::createDummyDBTexture( ChannelDataPtr data )
{
    osg::ref_ptr< osg::Image > image( createImage3DForInstancedRenderer( data ) );
    const DBKey key( generateDBKey() );
    image->setFileName( key );
    storeImage( image.get(), key );

    // Create dummy Texture / Image as placeholder until real image data is paged in.
    osg::ref_ptr< osg::Texture3D > tex( new osg::Texture3D );
    tex->setResizeNonPowerOfTwoHint( false );
    tex->setFilter( osg::Texture::MIN_FILTER, osg::Texture::NEAREST );
    tex->setFilter( osg::Texture::MAG_FILTER, osg::Texture::NEAREST );
    osg::Image* dummyImage( new osg::Image );
    dummyImage->setFileName( key );
    tex->setImage( dummyImage );

    return( tex.release() );
}


// core
}
// lfx
}
