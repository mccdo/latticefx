
#include <latticefx/BoundUtils.h>
#include <osg/BoundingSphere>
#include <osg/BoundingBox>
#include <osg/Array>


namespace lfx {


osg::BoundingSphere getBound( const osg::Vec3Array& array, const double pad )
{
    osg::BoundingSphere bs;
    unsigned int idx;
    for( idx=0; idx<array.size(); ++idx )
        bs.expandBy( array[ idx ] );
    bs.radius() += pad;
    return( bs );
}
osg::BoundingBox getBound( const osg::Vec3Array& array, const osg::Vec3& pad )
{
    osg::BoundingBox bb;
    unsigned int idx;
    for( idx=0; idx<array.size(); ++idx )
        bb.expandBy( array[ idx ] );
    bb._min -= pad;
    bb._max += pad;
    return( bb );
}


// lfx
}
