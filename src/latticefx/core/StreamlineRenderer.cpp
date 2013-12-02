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

#include <latticefx/core/StreamlineRenderer.h>
#include <latticefx/core/ChannelDataOSGArray.h>
#include <latticefx/core/TextureUtils.h>
#include <latticefx/core/BoundUtils.h>
#include <latticefx/core/LogMacros.h>
#include <latticefx/core/JsonSerializer.h>

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
#define HM_INPUT_ATTRIB 15
// Note: GeForce 9800M supports only 0 .. 15.


namespace lfx
{
namespace core
{
////////////////////////////////////////////////////////////////////////////////
StreamlineRenderer::StreamlineRenderer( const std::string& logName )
    : Renderer( "stl", logName )
{
    // Specify default ChannelData name aliases for the required inputs.
    setInputNameAlias( POSITION, "positions" );

    // Create and register uniform information, and initial/default values
    // (if we have them -- in some cases, we don't know the actual initial
    // values until scene graph creation).
    UniformInfo info;
    info = UniformInfo( "texDim", osg::Uniform::FLOAT_VEC3, "Texture dimensions for instanced rendering data.", UniformInfo::PRIVATE );
    registerUniform( info );

    info = UniformInfo( "texPos", osg::Uniform::SAMPLER_3D, "Position texture data sampler unit.", UniformInfo::PRIVATE );
    registerUniform( info );

    info = UniformInfo( "texDir", osg::Uniform::SAMPLER_3D, "Vector direction texture data sampler unit.", UniformInfo::PRIVATE );
    registerUniform( info );

    info = UniformInfo( "tfInput", osg::Uniform::SAMPLER_3D, "Transfer function input data sampler unit.", UniformInfo::PRIVATE );
    registerUniform( info );

    info = UniformInfo( "hmInput", osg::Uniform::SAMPLER_3D, "Hardware mask input data sampler unit.", UniformInfo::PRIVATE );
    registerUniform( info );
}
////////////////////////////////////////////////////////////////////////////////
StreamlineRenderer::StreamlineRenderer( const StreamlineRenderer& rhs )
    : Renderer( rhs )
{
}
////////////////////////////////////////////////////////////////////////////////
StreamlineRenderer::~StreamlineRenderer()
{
}
////////////////////////////////////////////////////////////////////////////////
osg::Node* StreamlineRenderer::getSceneGraph( const ChannelDataPtr maskIn )
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

    return( geode.release() );
}
////////////////////////////////////////////////////////////////////////////////
osg::StateSet* StreamlineRenderer::getRootState()
{
    osg::ref_ptr< osg::StateSet > stateSet( new osg::StateSet() );

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
    program->addShader( loadShader( osg::Shader::VERTEX, "lfx-streamline.vert" ) );
    program->addShader( loadShader( osg::Shader::FRAGMENT, "lfx-streamline.frag" ) );

    return( stateSet.release() );
}


////////////////////////////////////////////////////////////////////////////////
osg::Texture3D* StreamlineRenderer::createTexture( ChannelDataPtr data )
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
void StreamlineRenderer::serializeData( JsonSerializer *json ) const
{
	// let the parent write its data
	Renderer::serializeData( json );

	json->insertObj( StreamlineRenderer::getClassName(), true );
	json->popParent();
}

////////////////////////////////////////////////////////////////////////////////
bool StreamlineRenderer::loadData( JsonSerializer *json, IObjFactory *pfactory, std::string *perr )
{
	// let the parent load its data
	if ( !Renderer::loadData( json, pfactory, perr )) return false;

	// get to this classes data
	if ( !json->getObj( StreamlineRenderer::getClassName() ) )
	{
		if (perr) *perr = "Json: Failed to get StreamlineRenderer data";
		return false;
	}

	json->popParent();
	return true;
}

////////////////////////////////////////////////////////////////////////////////
void StreamlineRenderer::dumpState( std::ostream &os )
{
	Renderer::dumpState( os );

	dumpStateStart( StreamlineRenderer::getClassName(), os );
	dumpStateEnd( StreamlineRenderer::getClassName(), os );
}

////////////////////////////////////////////////////////////////////////////////
// core
}
// lfx
}
