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
#ifndef __LATTICEFX_CORE_VTK_SURFACERENDERER_H__
#define __LATTICEFX_CORE_VTK_SURFACERENDERER_H__ 1

#include <latticefx/core/SurfaceRenderer.h>

#include <latticefx/core/vtk/Export.h>

class vtkPolyData;

namespace lfx {

namespace core {

namespace vtk {

/** \class VTKSurfaceRenderer VTKSurfaceRenderer.h <latticefx/core/vtk/VTKSurfaceRenderer.h>
 \brief This class converts a vtkPolyData to OSG arrays for rendering.
 \details This class goes through a conversion process with vtkPolyData to enable
 the instance rendering tools to render a VTK vector field using GPU based instance
 rendering. */

class LATTICEFX_CORE_VTK_EXPORT VTKSurfaceRenderer : public lfx::core::SurfaceRenderer
{
public:
    ///Default constructor
    ///We are really a fancy lfx::core::VectorRenderer specific to VTK data
    VTKSurfaceRenderer() 
        : 
        lfx::core::SurfaceRenderer(),
        m_pd( 0 )
    {
        ;
    }
    
    ///Destructor
    virtual ~VTKSurfaceRenderer()
    {
        ;
    }
    
    ///Set the active vector name to tell the render what to put in the textures
    ///\param activeVector The active vector name to use
    void SetActiveVector( const std::string& activeVector );
    
    ///Set the active scalar name to tell the render what to put in the textures
    ///\param activeScalar The active scalar name to use
    void SetActiveScalar( const std::string& activeScalar );
    
    ///We are overriding the lfx::core::VectorRenderer method and then calling it 
    ///once we have given it all of the data it needs.
    virtual osg::Node* getSceneGraph( const lfx::core::ChannelDataPtr maskIn );
    
protected:
    void ExtractVTKPrimitives();

    ///The active vector to set which vector to use for rendering
    std::string m_activeVector;
    ///The active scalar to set which scalar to use for rendering
    std::string m_activeScalar;
    
    vtkPolyData* m_pd;
};

typedef boost::shared_ptr< VTKSurfaceRenderer > VTKSurfaceRendererPtr;

}
}
}

#endif //__LATTICEFX_CORE_VTK_SURFACERENDERER_H__
