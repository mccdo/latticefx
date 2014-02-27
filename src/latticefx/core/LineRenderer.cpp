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

#include <latticefx/core/LineRenderer.h>
#include <latticefx/core/ChannelDataOSGArray.h>
#include <latticefx/core/BoundUtils.h>
#include <latticefx/core/LogMacros.h>
#include <latticefx/core/JsonSerializer.h>

#include <osg/Geode>
#include <osg/Geometry>



// When sending transfer function array, specify this vertex attrib locations.
#define TF_INPUT_ATTRIB 15
// Note: GeForce 9800M supports only 0 .. 15.


namespace lfx
{
namespace core
{


LineRenderer::LineRenderer( const std::string& logName )
    : Renderer( "line", logName )
{
    // Specify default ChannelData name aliases for the required inputs.
    setInputNameAlias( VERTEX, "positions" );

    // Create and register uniform information, and initial/default values
    // (if we have them -- in some cases, we don't know the actual initial
    // values until scene graph creation).
    UniformInfo info;
    info = UniformInfo( "warpScale", osg::Uniform::FLOAT, "Vertex warp scale value." );
    info._prototype->set( 0.f );
    registerUniform( info );

    info = UniformInfo( "warpEnabled", osg::Uniform::BOOL, "Vertex warp enable toggle." );
    info._prototype->set( false );
    registerUniform( info );
}
LineRenderer::LineRenderer( const LineRenderer& rhs )
    : Renderer( rhs )
{
}
LineRenderer::~LineRenderer()
{
}


osg::Node* LineRenderer::getSceneGraph( const ChannelDataPtr maskIn )
{
    osg::ref_ptr< osg::Geode > geode( new osg::Geode );

    ChannelDataPtr posAlias( getInput( getInputNameAlias( VERTEX ) ) );
    if( posAlias == NULL )
    {
        LFX_WARNING( "getSceneGraph(): Unable to find required POSITION ChannelData." );
        return( NULL );
    }
    ChannelDataPtr posChannel( posAlias->getMaskedChannel( maskIn ) );
    osg::Array* sourceArray( posChannel->asOSGArray() );
    osg::Vec3Array* verts( static_cast< osg::Vec3Array* >( sourceArray ) );

    ChannelDataPtr normAlias( getInput( getInputNameAlias( NORMAL ) ) );
    if( normAlias == NULL )
    {
        LFX_WARNING( "getSceneGraph(): Unable to find required NORMAL ChannelData." );
        return( NULL );
    }
    ChannelDataPtr normChannel( normAlias->getMaskedChannel( maskIn ) );
    sourceArray = normChannel->asOSGArray();
    osg::Vec3Array* norms( static_cast< osg::Vec3Array* >( sourceArray ) );

    osg::ref_ptr< osg::Geometry > geom( new osg::Geometry );
    geom->setUseDisplayList( false );
    geom->setUseVertexBufferObjects( true );

    geom->setVertexArray( verts );
    geom->setNormalArray( norms );
    geom->setNormalBinding( osg::Geometry::BIND_PER_VERTEX );

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
        // Line shader supports only vec3 tf input. Convert the tf input data to a vec3 array.
        osg::Vec3Array* tfInputArray3( ChannelDataOSGArray::convertToVec3Array( tfInputArray ) );
        geom->setVertexAttribArray( TF_INPUT_ATTRIB, tfInputArray3 );
        geom->setVertexAttribBinding( TF_INPUT_ATTRIB, osg::Geometry::BIND_PER_VERTEX );
    }

    if( geom->getNumPrimitiveSets() > 0 )
    {
        geode->addDrawable( geom.get() );
    }
    return( geode.release() );
}

osg::StateSet* LineRenderer::getRootState()
{
    osg::ref_ptr< osg::StateSet > stateSet( new osg::StateSet );

    addHardwareFeatureUniforms( stateSet.get() );

    ChannelDataPtr warpVAlias( getInput( getInputNameAlias( WARP_VERTEX ) ) );
    ChannelDataPtr warpNAlias( getInput( getInputNameAlias( WARP_NORMAL ) ) );

    const bool warpEnabled( ( warpVAlias != NULL ) && ( warpNAlias != NULL ) );
    UniformInfo& info( getUniform( "warpEnabled" ) );
    info._prototype->set( warpEnabled );
    stateSet->addUniform( createUniform( getUniform( "warpEnabled" ) ) );

    osg::Program* program( new osg::Program() );
    program->addBindAttribLocation( "tfInput", TF_INPUT_ATTRIB );

    program->addShader( loadShader( osg::Shader::VERTEX, "lfx-line.vert" ) );
    program->addShader( loadShader( osg::Shader::FRAGMENT, "lfx-line.frag" ) );
    stateSet->setAttribute( program );

    if( warpEnabled )
    {
        stateSet->addUniform( createUniform( getUniform( "warpScale" ) ) );
    }

    return( stateSet.release() );
}



void LineRenderer::serializeData( JsonSerializer *json ) const
{
	// let the parent write its data
	Renderer::serializeData( json );

	json->insertObj( LineRenderer::getClassName(), true );
	json->popParent();
}

bool LineRenderer::loadData( JsonSerializer *json, IObjFactory *pfactory, std::string *perr )
{
	// let the parent load its data
	if ( !Renderer::loadData( json, pfactory, perr )) return false;

	// get to this classes data
	if ( !json->getObj( LineRenderer::getClassName() ) )
	{
		if (perr) *perr = "Json: Failed to get LineRenderer data";
		return false;
	}

	json->popParent();
	return true;
}


////////////////////////////////////////////////////////////////////////////////
void LineRenderer::dumpState( std::ostream &os )
{
	Renderer::dumpState( os );

	dumpStateStart( LineRenderer::getClassName(), os );
	// TODO:
	//_primitiveSetGenerator
	dumpStateEnd( LineRenderer::getClassName(), os );
}

// core
}
// lfx
}
