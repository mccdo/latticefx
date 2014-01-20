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
#include <latticefx/core/MiscUtils.h>

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Texture1D>
#include <osg/Texture2D>
#include <osg/Texture3D>
#include <osg/Shader>
#include <osg/Program>
#include <osg/Uniform>
#include <osgDB/ReadFile>
#include <osg/BlendFunc>
#include <osg/Depth>

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
    : Renderer( "stl", logName ),
      _numTraces( 1 ),
      _traceLengthPercent( .25f ),
      _traceSpeed( .2f ),
      _enableAnimation( true ),
      _imageScale( 1.f )
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

    info = UniformInfo( "tfInput", osg::Uniform::SAMPLER_3D, "Transfer function input data sampler unit.", UniformInfo::PRIVATE );
    registerUniform( info );

    info = UniformInfo( "hmInput", osg::Uniform::SAMPLER_3D, "Hardware mask input data sampler unit.", UniformInfo::PRIVATE );
    registerUniform( info );

    info = UniformInfo( "stlImage", osg::Uniform::SAMPLER_2D, "Image for individual streamline points.", UniformInfo::PRIVATE );
    registerUniform( info );

    info = UniformInfo( "numTraces", osg::Uniform::FLOAT, "Number of traces per vector data input." );
    registerUniform( info );

    info = UniformInfo( "traceLengthPercent", osg::Uniform::FLOAT, "Length of streamline trace as a percent of total data." );
    registerUniform( info );

    info = UniformInfo( "traceSpeed", osg::Uniform::FLOAT, "Percent of data to traverse per second during animation." );
    registerUniform( info );

    info = UniformInfo( "enableAnimation", osg::Uniform::BOOL, "Enable or disable animation." );
    registerUniform( info );

    info = UniformInfo( "imageScale", osg::Uniform::FLOAT, "Streamline diameter scale factor." );
    registerUniform( info );
}
////////////////////////////////////////////////////////////////////////////////
StreamlineRenderer::StreamlineRenderer( const StreamlineRenderer& rhs )
    : Renderer( rhs ),
      _numTraces( rhs._numTraces ),
      _traceLengthPercent( rhs._traceLengthPercent ),
      _traceSpeed( rhs._traceSpeed ),
      _enableAnimation( rhs._enableAnimation ),
      _imageScale( rhs._imageScale )
{
}
////////////////////////////////////////////////////////////////////////////////
StreamlineRenderer::~StreamlineRenderer()
{
}
////////////////////////////////////////////////////////////////////////////////
void StreamlineRenderer::setNumTraces( const int numTraces )
{
    _numTraces = numTraces;
}
////////////////////////////////////////////////////////////////////////////////
int StreamlineRenderer::getNumTraces() const
{
    return( _numTraces );
}
////////////////////////////////////////////////////////////////////////////////
void StreamlineRenderer::setTraceLengthPercent( float traceLengthPercent )
{
    if( traceLengthPercent > 1. )
        _traceLengthPercent = 1.;
    else if( traceLengthPercent < 0. )
        _traceLengthPercent = 0.;
    else
        _traceLengthPercent = traceLengthPercent;
}
////////////////////////////////////////////////////////////////////////////////
float StreamlineRenderer::getTraceLengthPercent() const
{
    return( _traceLengthPercent );
}
////////////////////////////////////////////////////////////////////////////////
void StreamlineRenderer::setTraceSpeed( const float traceSpeed )
{
    _traceSpeed = traceSpeed;
}
////////////////////////////////////////////////////////////////////////////////
float StreamlineRenderer::getTraceSpeed() const
{
    return( _traceSpeed );
}
////////////////////////////////////////////////////////////////////////////////
void StreamlineRenderer::setAnimationEnable( bool enable )
{
    _enableAnimation = enable;
}
////////////////////////////////////////////////////////////////////////////////
bool StreamlineRenderer::getAnimationEnable() const
{
    return( _enableAnimation );
}
////////////////////////////////////////////////////////////////////////////////
bool StreamlineRenderer::setImageScale( float scale )
{
	if( MiscUtils::isnot_close( _imageScale, scale, .001f ) )
	{
		_imageScale = scale;
		return true;
	}

	return false;
}
////////////////////////////////////////////////////////////////////////////////
float StreamlineRenderer::getImageScale() const
{ 
    return( _imageScale );
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

    const float dimX( 1.f ), dimY( 1.f );
    const float halfDimX( dimX * .5f );
    const float halfDimY( dimY * .5f );
    osg::Geometry* geom( osgwTools::makePlane( osg::Vec3( -halfDimX, -halfDimY, 0. ),
        osg::Vec3( dimX, 0., 0. ), osg::Vec3( 0., dimY, 0. ) ) );
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

    // position, transfer function input, and hardware mask input texture
    // units are the same for all time steps, so set their sampler uniform unit
    // values in the root state.
    {
        UniformInfo& info( getUniform( "texPos" ) );
        info._prototype->set( getOrAssignTextureUnit( "posTex" ) );
        stateSet->addUniform( createUniform( info ), osg::StateAttribute::PROTECTED );
    }

    // All streamline points have the same texture image.
    {
        UniformInfo& info( getUniform( "stlImage" ) );
        const int unit( getOrAssignTextureUnit( "stlImage" ) );
        info._prototype->set( unit );
        stateSet->addUniform( createUniform( info ), osg::StateAttribute::PROTECTED );

        osg::Texture2D *tex( new osg::Texture2D() );
        tex->setImage( osgDB::readImageFile( "splotch.png" ) );
        stateSet->setTextureAttributeAndModes( unit, tex, osg::StateAttribute::ON );
    }

    // Animation parameters
    {
        UniformInfo& info( getUniform( "numTraces" ) );
        info._prototype->set( (float)_numTraces );
        stateSet->addUniform( createUniform( info ) );
    }
    {
        UniformInfo& info( getUniform( "traceLengthPercent" ) );
        info._prototype->set( _traceLengthPercent );
        stateSet->addUniform( createUniform( info ) );
    }
    {
        UniformInfo& info( getUniform( "traceSpeed" ) );
        info._prototype->set( _traceSpeed );
        stateSet->addUniform( createUniform( info ) );
    }
    {
        UniformInfo& info( getUniform( "enableAnimation" ) );
        info._prototype->set( _enableAnimation );
        stateSet->addUniform( createUniform( info ) );
    }
    {
        UniformInfo& info( getUniform( "imageScale" ) );
        info._prototype->set( _imageScale );
        stateSet->addUniform( createUniform( info ) );
    }

    // Go in "transparent bin" (bin 10) so that streamlines render
    // after opaque objects in the scene.
    stateSet->setRenderBinDetails( 10, "RenderBin" );

    // Note:
    // It turns out that SRC_ALPHA, ONE_MINUS_SRC_ALPHA actually is
    // non-saturating. Give it a color just shy of full intensity white,
    // and the result will never saturate to white no matter how many
    // times it is overdrawn.
    osg::ref_ptr< osg::BlendFunc > bf( new osg::BlendFunc(
        GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ) );
    stateSet->setAttributeAndModes( bf.get() );

    // Note:
    // Leave the depth test enabled, but mask off depth writes (4th param is false).
    // This allows us to render the streamline points in any order, front to back
    // or back to front, and not lose any points by depth testing against themselves.
    osg::ref_ptr< osg::Depth > depth( new osg::Depth( osg::Depth::LESS, 0., 1., false ) );
    stateSet->setAttributeAndModes( depth.get() );


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
	json->insertObjValue( "numTraces", _numTraces );
	json->insertObjValue( "traceLengthPercent", _traceLengthPercent );
	json->insertObjValue( "traceSpeed", _traceSpeed );
	json->insertObjValue( "enableAnimation", _enableAnimation );
	json->insertObjValue( "imageScale", _imageScale );
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

	json->getValue( "numTraces", &_numTraces, _numTraces );
	json->getValue( "traceLengthPercent", &_traceLengthPercent, _traceLengthPercent );
	json->getValue( "traceSpeed", &_traceSpeed, _traceSpeed );
	json->getValue( "enableAnimation", &_enableAnimation, _enableAnimation );
	json->getValue( "imageScale", &_imageScale, _imageScale );

	json->popParent();
	return true;
}

////////////////////////////////////////////////////////////////////////////////
void StreamlineRenderer::dumpState( std::ostream &os )
{
	Renderer::dumpState( os );

	dumpStateStart( StreamlineRenderer::getClassName(), os );

    os << "_numTraces: " << _numTraces << std::endl;
    os << "_traceLengthPercent: " << _traceLengthPercent << std::endl;
    os << "_traceSpeed: " << _traceSpeed << std::endl;
    os << "_enableAnimation: " << _enableAnimation << std::endl;
    os << "_imageScale: " << _imageScale << std::endl;

    dumpStateEnd( StreamlineRenderer::getClassName(), os );
}

////////////////////////////////////////////////////////////////////////////////
// core
}
// lfx
}
