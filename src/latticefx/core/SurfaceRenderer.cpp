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

#include <latticefx/core/SurfaceRenderer.h>

#include <osg/Geode>
#include <osg/Geometry>


namespace lfx {


SurfaceRenderer::SurfaceRenderer()
  : lfx::Renderer()
{
    // Specify default ChannelData name aliases for the required inputs.
    setInputNameAlias( POSITION, "positions" );
    setInputNameAlias( NORMAL, "normals" );
    setInputNameAlias( WARP_DIRECTION, "warp_directions" );
}
SurfaceRenderer::SurfaceRenderer( const SurfaceRenderer& rhs )
  : lfx::Renderer( rhs ),
    _inputTypeMap( rhs._inputTypeMap )
{
}
SurfaceRenderer::~SurfaceRenderer()
{
}


osg::Node* SurfaceRenderer::getSceneGraph( const lfx::ChannelDataPtr maskIn )
{
    osg::ref_ptr< osg::Geode > geode( new osg::Geode );

    ChannelDataPtr posAlias( getInput( getInputTypeAlias( POSITION ) ) );
    if( posAlias == NULL )
    {
        OSG_WARN << "SurfaceRenderer::getSceneGraph(): Unable to find required POSITION ChannelData." << std::endl;
        return( NULL );
    }
    ChannelDataPtr posChannel( posAlias->getMaskedChannel( maskIn ) );
    osg::Array* sourceArray( posChannel->asOSGArray() );
    const unsigned int numElements( sourceArray->getNumElements() );
    osg::Vec3Array* verts( static_cast< osg::Vec3Array* >( sourceArray ) );

    ChannelDataPtr normAlias( getInput( getInputTypeAlias( NORMAL ) ) );
    if( normAlias == NULL )
    {
        OSG_WARN << "SurfaceRenderer::getSceneGraph(): Unable to find required NORMAL ChannelData." << std::endl;
        return( NULL );
    }
    ChannelDataPtr normChannel( normAlias->getMaskedChannel( maskIn ) );
    sourceArray = normChannel->asOSGArray();
    osg::Vec3Array* norms( static_cast< osg::Vec3Array* >( sourceArray ) );

    osg::Vec3Array* warps( NULL );
    ChannelDataPtr warpAlias( getInput( getInputTypeAlias( WARP_DIRECTION ) ) );
    if( warpAlias != NULL )
    {
        ChannelDataPtr warpChannel( warpAlias->getMaskedChannel( maskIn ) );
        sourceArray = warpChannel->asOSGArray();
        warps = static_cast< osg::Vec3Array* >( sourceArray );
    }

    osg::ref_ptr< osg::Geometry > geom( new osg::Geometry );
    geom->setUseDisplayList( false );
    geom->setUseVertexBufferObjects( true );

    geom->setVertexArray( verts );
    geom->setNormalArray( norms );
    geom->setNormalBinding( osg::Geometry::BIND_PER_VERTEX );

    if( _primitiveSetGenerator == NULL )
        _primitiveSetGenerator = PrimitiveSetGeneratorPtr( new SimpleTrianglePrimitiveSetGenerator() );
    (*_primitiveSetGenerator)( geom.get(), numElements );

    geode->addDrawable( geom.get() );
    return( geode.release() );
}

osg::StateSet* SurfaceRenderer::getRootState()
{
    return( NULL );
}


void SurfaceRenderer::setPrimitiveSetGenerator( PrimitiveSetGeneratorPtr primitiveSetGenerator )
{
    _primitiveSetGenerator = primitiveSetGenerator;
}
PrimitiveSetGeneratorPtr SurfaceRenderer::getPrimitiveSetGenerator()
{
    return( _primitiveSetGenerator );
}


void SurfaceRenderer::setInputNameAlias( const InputType& inputType, const std::string& alias )
{
    _inputTypeMap[ inputType ] = alias;
}
std::string SurfaceRenderer::getInputTypeAlias( const InputType& inputType ) const
{
    InputTypeMap::const_iterator it( _inputTypeMap.find( inputType ) );
    if( it != _inputTypeMap.end() )
        // Found it.
        return( it->second );
    else
        // Should never happen, as the constructor assigns defaults.
        return( "" );
}





PrimitiveSetGenerator::PrimitiveSetGenerator()
{
}
PrimitiveSetGenerator::PrimitiveSetGenerator( const PrimitiveSetGenerator& rhs )
{
}
PrimitiveSetGenerator::~PrimitiveSetGenerator()
{
}

SimpleTrianglePrimitiveSetGenerator::SimpleTrianglePrimitiveSetGenerator()
  : PrimitiveSetGenerator()
{
}
SimpleTrianglePrimitiveSetGenerator::SimpleTrianglePrimitiveSetGenerator( const SimpleTrianglePrimitiveSetGenerator& rhs )
  : PrimitiveSetGenerator( rhs )
{
}
SimpleTrianglePrimitiveSetGenerator::~SimpleTrianglePrimitiveSetGenerator()
{
}

void SimpleTrianglePrimitiveSetGenerator::operator()( osg::Geometry* geom, unsigned int numElements )
{
    osg::DrawArrays* da( new osg::DrawArrays( GL_TRIANGLES, 0, numElements ) );
    geom->addPrimitiveSet( da );
}


// lfx
}
