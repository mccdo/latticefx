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

#include <latticefx/VolumeRenderer.h>
#include <latticefx/ChannelDataOSGArray.h>
#include <latticefx/TextureUtils.h>

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Texture1D>
#include <osg/Texture2D>
#include <osg/Texture3D>
#include <osg/Shader>
#include <osg/Program>
#include <osg/Uniform>
#include <osgDB/FileUtils>



namespace lfx {


VolumeRenderer::VolumeRenderer()
{
}
VolumeRenderer::VolumeRenderer( const VolumeRenderer& rhs )
  : lfx::Renderer( rhs )
{
}
VolumeRenderer::~VolumeRenderer()
{
}

osg::Node* VolumeRenderer::getSceneGraph( const lfx::ChannelDataPtr maskIn )
{
    OSG_NOTICE << "VolumeRenderer::getSceneGraph(): Not yet implemented." << std::endl;
    return( NULL );
}

osg::StateSet* VolumeRenderer::getRootState()
{
    OSG_NOTICE << "VolumeRenderer::getRootState(): Not yet implemented." << std::endl;
    return( NULL );
}


// lfx
}
