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

#ifndef __LFX_CORE_BOUND_UTILS_H__
#define __LFX_CORE_BOUND_UTILS_H__ 1


#include <latticefx/core/Export.h>
#include <osg/BoundingSphere>
#include <osg/BoundingBox>
#include <osg/Array>



namespace lfx
{
namespace core
{


/** \defgroup BoundUtils Utilities for fitting OSG bounding volumes
*/
/**@{*/


/** \brief Return a sphere that contains all points in the specified array.
\details Encloses all points, then adds the optional \c pad to the sphere radius. */
LATTICEFX_EXPORT osg::BoundingSphere getBound( const osg::Vec3Array& array, const double pad = 0. );

/** \brief Return a box that contains all points in the specified array.
\details Encloses all points, then adds the optional \c pad to the box max extents and
subtracts it from the box min extents. */
LATTICEFX_EXPORT osg::BoundingBox getBound( const osg::Vec3Array& array, const osg::Vec3& pad = osg::Vec3( 0., 0., 0. ) );


/**@}*/


// core
}
// lfx
}


// __LFX_CORE_BOUND_UTILS_H__
#endif
