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
#ifndef __LATTICEFX_CORE_VTK_BASE_RTP_OPERATION_H__
#define __LATTICEFX_CORE_VTK_BASE_RTP_OPERATION_H__ 1

#include <latticefx/core/SurfaceRenderer.h>

#include <latticefx/core/vtk/Export.h>

class vtkCellArray;

namespace lfx {

namespace core {

namespace vtk {

/** \class VTKPrimitiveSetGenerator VTKPrimitiveSetGenerator.h <latticefx/core/vtk/VTKPrimitiveSetGenerator.h>
 \brief Base class for VTK based operations to create primitives from VTK primitives.
 \details This class holds the call operator to enable VTK RTP pipelines
 to convert VTK primitives to OSG primitives. */

class LATTICEFX_CORE_VTK_EXPORT VTKPrimitiveSetGenerator : public lfx::core::PrimitiveSetGenerator
{
public:
    
    ///Default constructor
    VTKPrimitiveSetGenerator( vtkCellArray* const strips )
        : 
        lfx::core::PrimitiveSetGenerator(),
        m_triStrips( strips )
    {
        ;
    }
    
    ///Destructor
    virtual ~VTKPrimitiveSetGenerator()
    {
        //m_triStrips->Delete();
    }

    ///Copy constructor
    VTKPrimitiveSetGenerator( const VTKPrimitiveSetGenerator& rhs );

    virtual void operator()( const SurfaceRenderer* /* surfaceRenderer */, osg::Geometry* geom );

protected:    
    ///The triangle strips from vtkPolyData
    vtkCellArray* m_triStrips;
};

typedef boost::shared_ptr< VTKPrimitiveSetGenerator > VTKPrimitiveSetGeneratorPtr;

}
}
}
// __LATTICEFX_CORE_VTK_ISOSURFACE_RTP_OPERATION_H__
#endif