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


namespace lfx {


SurfaceRenderer::SurfaceRenderer()
{
}
SurfaceRenderer::SurfaceRenderer( const SurfaceRenderer& rhs )
{
}
SurfaceRenderer::~SurfaceRenderer()
{
}


osg::Node* SurfaceRenderer::getSceneGraph( const lfx::ChannelDataPtr maskIn )
{
    return( NULL );
}

osg::StateSet* SurfaceRenderer::getRootState()
{
    return( NULL );
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


// lfx
}
