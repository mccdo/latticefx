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
#ifndef __LATTICEFX_CORE_VTK_ACTORRENDERER_H__
#define __LATTICEFX_CORE_VTK_ACTORRENDERER_H__ 1

#include <latticefx/core/vtk/Export.h>

#include <latticefx/core/ChannelData.h>
#include <latticefx/core/Renderer.h>

#include <string>

namespace osg {
    class Node;
}

namespace lfx {

namespace core {

namespace vtk {

/** \class VTKActorRenderer VTKActorRenderer.h <latticefx/core/vtk/VTKActorRenderer.h>
 \brief This class converts a vtkPolyData to OSG arrays for rendering.
 \details This class goes through a conversion process with vtkPolyData to enable
 the instance rendering tools to render a VTK vector field using GPU based instance
 rendering. */

class LATTICEFX_CORE_VTK_EXPORT VTKActorRenderer : public lfx::Renderer
{
public:
    ///Default constructor
    ///We are really a fancy lfx::VectorRenderer specific to VTK data
    VTKActorRenderer()
        :
        lfx::Renderer()
    {
        ;
    }
    
    ///Destructor
    virtual ~VTKActorRenderer()
    {
        ;
    }
    
    ///Set the active vector name to tell the render what to put in the textures
    ///\param activeVector The active vector name to use
    void SetActiveVector( const std::string& activeVector );
    
    ///Set the active scalar name to tell the render what to put in the textures
    ///\param activeScalar The active scalar name to use
    void SetActiveScalar( const std::string& activeScalar );
    
    ///We are overriding the lfx::VectorRenderer method and then calling it 
    ///once we have given it all of the data it needs.
    virtual osg::Node* getSceneGraph( const lfx::ChannelDataPtr maskIn );
    
protected:
    ///The active vector to set which vector to use for rendering
    std::string m_activeVector;
    ///The active scalar to set which scalar to use for rendering
    std::string m_activeScalar;
};

typedef boost::shared_ptr< VTKActorRenderer > VTKActorRendererPtr;

}
}
}

#endif
