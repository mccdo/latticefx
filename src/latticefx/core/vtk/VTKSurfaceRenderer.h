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
#ifndef __LATTICEFX_CORE_VTK_SURFACERENDERER_H__
#define __LATTICEFX_CORE_VTK_SURFACERENDERER_H__ 1

#include <latticefx/core/SurfaceRenderer.h>

#include <latticefx/core/vtk/ChannelDatavtkDataObject.h>

#include <latticefx/core/vtk/Export.h>

class vtkPolyData;

namespace lfx
{

namespace core
{

namespace vtk
{

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
    VTKSurfaceRenderer( const std::string& logName = std::string( "" ) )
        :
        lfx::core::SurfaceRenderer( logName ),
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

    ///Set the color by scalar
    ///\note Used in pipelines where the active scalar is used to make a surface
    ///or some other feature and a second scalar is used for color.
    void SetColorByScalar( std::string const scalarName );

    ///We are overriding the lfx::core::VectorRenderer method and then calling it
    ///once we have given it all of the data it needs.
    virtual osg::Node* getSceneGraph( const lfx::core::ChannelDataPtr maskIn );

protected:
    ///Create the raw OSG primitives from the VTK data
    void ExtractVTKPrimitives();

    ///Setup the normal and vertex osg arrays
    void SetupNormalAndVertexArrays( vtkPolyData* pd );
    
    ///Setup the osg color arrays
    void SetupColorArrays( vtkPolyData* pd );

    ///The active vector to set which vector to use for rendering
    std::string m_activeVector;
    ///The active scalar to set which scalar to use for rendering
    std::string m_activeScalar;
    ///The color by scalar
    std::string m_colorByScalar;
    ///The raw VTK data to render
    vtkPolyData* m_pd;
    ///Scalar channel data
    std::map< std::string, lfx::core::ChannelDataPtr > m_scalarChannels;
    ///The raw dataset object
    lfx::core::vtk::ChannelDatavtkDataObjectPtr m_dataObject;
};

typedef boost::shared_ptr< VTKSurfaceRenderer > VTKSurfaceRendererPtr;

}
}
}

#endif //__LATTICEFX_CORE_VTK_SURFACERENDERER_H__
