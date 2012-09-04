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

#ifndef __LFX_CORE_HEIRARCHY_UTILS_H__
#define __LFX_CORE_HEIRARCHY_UTILS_H__ 1

#include <latticefx/core/Export.h>
#include <latticefx/core/ChannelData.h>

#include <osg/Vec3>

#include <vector>



namespace lfx {
namespace core {


class HierarchyCallback
{
public:
    HierarchyCallback() {}
    HierarchyCallback( const HierarchyCallback& rhs ) {}
    virtual ~HierarchyCallback() {}

    virtual void operator()( ChannelDataPtr cdp, const std::string& hierarchyNameString ) {}

protected:
};

LATTICEFX_EXPORT void traverseHeirarchy( ChannelDataPtr root, HierarchyCallback& cb );


/** \class AssembleHierarchy HierarchyUtils <latticefx/core/HierarchyUtils.h>
\brief A utility class for creating hierarchies of volume data.
\details
Basically you make an instance of an AssembleHierarchy class. You must pass the
LOD range info to the constructor. You can pass an explicit vector of values, or
you can have the class create that implicitly by providing a max depth and the
switch size for the lowest LOD. If your hierarchy has 4 levels of detail, then
your maxDepth is 4 and there would be three range values (one for each switch
point).

The constructor takes the range values and creates the skeleton of the hierarchy,
composed of ChannelDataLOD and ChannelDataImageSet objects, but no data. The
class fills in the ranges for the LODs at this point, but does not fill in the
offsets for the ImageSets.

Next, you call addChannelData for each texture brick you want to add. The second
param is the name string as I described previously. The third parameter is an
optional offset vector, the normalized offset for this brick within its current
ImageSet. (Do not pass the fourth (depth) parameter; it's for internal use during
recursion.)

If you don't pass in the offset, an offset is inferred from the last digit of the
name string.

addChannelData() recurses / calls itself as it traverses down the skeleton created
by the constructor, until it finds the appropriate insertion point for the data as
determined by the name string.

After you have added all of your ChannelData bricks, call getRoot() to obtain the
top-level ChannelData of the hierarchy. 
*/
class LATTICEFX_EXPORT AssembleHierarchy
{
public:
    typedef std::vector< double > RangeVec;

    ///Constructor
    AssembleHierarchy( RangeVec ranges );
    ///Constructor
    ///\param maxDepth The maximum depth achieved by the bricked textures
    AssembleHierarchy( unsigned int maxDepth, double baseRange=25000. );
    ///Copy constructor
    AssembleHierarchy( const AssembleHierarchy& rhs );
    ///Destructor
    virtual ~AssembleHierarchy();

    ///In this method if an \ref offset  is not provided the name string will be
    ///used to decide which octant a channel belongs to. This is done as follows:
    /// 0 = -1., -1., -1.
    /// 1 =  1., -1., -1. 
    /// 2 = -1.,  1., -1.
    /// 3 =  1.,  1., -1.
    /// 4 = -1., -1.,  1.
    /// 5 =  1., -1.,  1.
    /// 6 = -1.,  1.,  1.
    /// 7 =  1.,  1.,  1.
    ///\param cdp The channel data representing the octant
    ///\param nameString The integer value encoded as a string that contains
    ///the location of the octant relative to the whole. It may look like:
    /// - 000
    /// - 017
    /// - 701
    ///\param offset The offset for a brick in normalized coordinate space. An 
    ///example of these values is listed above.
    ///\param depth Do not use this value. It is for internal processing only.
    void addChannelData( ChannelDataPtr cdp, const std::string nameString,
            const osg::Vec3& offset=osg::Vec3( 0., 0., 0. ), const unsigned int depth=0 );

    ChannelDataPtr getRoot() const;

protected:
    void recurseInit( ChannelDataPtr cdp, unsigned int depth );

    ChannelDataPtr _root;
    RangeVec _ranges;

    ChannelDataPtr _iterator;
};


// core
}
// lfx
}


// __LFX_CORE_HEIRARCHY_UTILS_H__
#endif
