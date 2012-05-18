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

#ifndef __LATTICEFX_VOLUME_RENDERER_H__
#define __LATTICEFX_VOLUME_RENDERER_H__ 1


#include <latticefx/Export.h>
#include <latticefx/Renderer.h>

#include <boost/smart_ptr/shared_ptr.hpp>


namespace lfx {


/** \class VolumeRenderer VolumeRenderer.h <latticefx/VolumeRenderer.h>
\brief TBD
\details TBD
*/
class LATTICEFX_EXPORT VolumeRenderer : public lfx::Renderer
{
public:
    VolumeRenderer();
    VolumeRenderer( const VolumeRenderer& rhs );
    virtual ~VolumeRenderer();

    /** \brief Override base class lfx::Renderer
    \details Invoked by DataSet once per time step to obtain a scene graph for
    rendering volumetric data. */
    virtual osg::Node* getSceneGraph( const lfx::ChannelDataPtr maskIn );

    /** \brief Override base class lfx::Renderer
    \details Invoked by DataSet to obtain a global StateSet for all child
    scene graphs. */
    virtual osg::StateSet* getRootState();

protected:
};

typedef boost::shared_ptr< VolumeRenderer > VolumeRendererPtr;


// lfx
}


// __LATTICEFX_VOLUME_RENDERER_H__
#endif
