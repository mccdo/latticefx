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

#ifndef __LFX_CORE_VTK_VOLUME_BRICK_DATA_H__
#define __LFX_CORE_VTK_VOLUME_BRICK_DATA_H__ 1

#include <latticefx/core/vtk/Export.h>
#include <latticefx/core/HierarchyUtils.h>



namespace lfx {
namespace core {
namespace vtk {


/** \class VTKVolumeBrickData VTKVolumeBrickData.h <latticefx/core/vtk/VTKVolumeBrickData.h>
\brief To be done
\details To be done
*/
class LATTICEFX_CORE_VTK_EXPORT VTKVolumeBrickData : public lfx::core::VolumeBrickData
{
public:

    /** To be done. Override this. */
    virtual osg::Image* getBrick( const osg::Vec3s& brickNum ) const;
};


// vtk
}
// core
}
// lfx
}


// __LFX_CORE_VTK_VOLUME_BRICK_DATA_H__
#endif
