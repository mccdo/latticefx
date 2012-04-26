
#ifndef __LATTICEFX_BOUND_UTILS_H__
#define __LATTICEFX_BOUND_UTILS_H__ 1


#include <latticefx/Export.h>
#include <osg/BoundingSphere>
#include <osg/BoundingBox>
#include <osg/Array>



namespace lfx {


/** \defgroup BoundUtils Utilities for fitting OSG bounding volumes
*/
/**@{*/


/** \brief Return a sphere that contains all points in the specified array.
\details Encloses all points, then adds the optional \c pad to the sphere radius. */
LATTICEFX_EXPORT osg::BoundingSphere getBound( const osg::Vec3Array& array, const double pad=0. );

/** \brief Return a box that contains all points in the specified array.
\details Encloses all points, then adds the optional \c pad to the box max extents and
subtracts it from the box min extents. */
LATTICEFX_EXPORT osg::BoundingBox getBound( const osg::Vec3Array& array, const osg::Vec3& pad=osg::Vec3( 0., 0., 0. ) );


/**@}*/


// lfx
}


// __LATTICEFX_BOUND_UTILS_H__
#endif
