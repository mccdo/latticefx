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

#ifndef __LFX_CORE_HIERARCHY_UTILS_H__
#define __LFX_CORE_HIERARCHY_UTILS_H__ 1

#include <latticefx/core/Export.h>
#include <latticefx/core/ChannelData.h>
#include <latticefx/core/DBBase.h>
#include <latticefx/core/Preprocess.h>

#include <osg/Vec3>
#include <osg/Vec3s>
#include <osg/ref_ptr>
#include <osg/Image>

#include <vector>



namespace lfx {
namespace core {


/** \class TraverseHierarchy HierarchyUtils <latticefx/core/HierarchyUtils.h>
\brief A utility class for traversing hierarchies of ChannelData.
\details
Pass the root of the ChannelData hierarchy to the constructor, along
with an instance of a HierarchyCallback. The constructor traverses
the hierarchy and call into the callback for each ChannelData in the
hierarchy. */
class LATTICEFX_EXPORT TraverseHierarchy
{
public:
    class HierarchyCallback
    {
    public:
        HierarchyCallback() {}
        HierarchyCallback( const HierarchyCallback& rhs ) {}
        virtual ~HierarchyCallback() {}

        virtual void operator()( ChannelDataPtr cdp, const std::string& hierarchyNameString ) {}

    protected:
    };

    /** Constructor */
    TraverseHierarchy( ChannelDataPtr root=ChannelDataPtr((ChannelData*)NULL) );
    TraverseHierarchy( ChannelDataPtr root, HierarchyCallback& cb );

    void setRoot( ChannelDataPtr root ) { _root = root; }
    void setCallback( HierarchyCallback& cb ) { _cb = cb; }
    void execute();

protected:
    virtual void traverse( ChannelDataPtr cdp );

    typedef std::list< int > NameList;

    static std::string toString( const NameList& nameList );

    ChannelDataPtr _root;
    NameList _name;
    HierarchyCallback _cb;
};



/** \class PruneHierarchy HierarchyUtils <latticefx/core/HierarchyUtils.h>
\brief Prune empty branches from a ChannelData hierarchy.
\details Removes all ChannelData objects that are ChannelDataComposite
and contain no subordinate ChannelData.

See AssembleHierarchy::prune(). */
class LATTICEFX_EXPORT PruneHierarchy
{
public:
    PruneHierarchy( ChannelDataPtr root );

protected:
    virtual int traverse( ChannelDataPtr cdp );
};


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

After you have added all of your ChannelData bricks, you can optionally call
prune() to remove empty branches from the octree data structure. Finally, call
getRoot() to obtain the top-level ChannelData of the hierarchy.
*/
class LATTICEFX_EXPORT AssembleHierarchy
{
public:
    typedef std::vector< double > RangeVec;

    /** \brief Constructor
    \details Creates a ChannelData hierarchy.
    Uses the explicit list of LOD ranges as switch points.
    */
    AssembleHierarchy( RangeVec ranges );
    /** \brief Constructor
    \details Creates a ChannelData hierarchy.
    Creates an implicit list of LOD range switch points based
    on the given parameters.
    \param maxDepth The maximum depth achieved by the bricked textures
    \param baseRange The switch distance between the lowest LOD and one detail level higher.
    */
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

    /** \brief Prune empty branches from the hierarchy.
    \detauls Call this function after adding all ChannelData objects, and just
    before calling getRoot(). This function will remove any empty branches from
    the octree, resulting in a minimal data structure. */
    void prune();

    /** \brief Retrieve the created hierarchy.
    \details Call this function after adding all ChannelDraw objects. */
    ChannelDataPtr getRoot() const { return( _root ); }

protected:
    void recurseInit( ChannelDataPtr cdp, unsigned int depth );

    ChannelDataPtr _root;
    RangeVec _ranges;

    ChannelDataPtr _iterator;
};


/** \class NameStringGenerator HierarchyUtils <latticefx/core/HierarchyUtils.h>
\brief Generate hierarchy name strings for a subvolume.
\details Given the full texel extents of a volume, this class provides
name strings and octant offsets for a subvolume of specified extents at
a given location within that volume.
*/
class LATTICEFX_EXPORT NameStringGenerator
{
public:
    /** Constructor. Returned name strings and offsets are valid for
    a volume with the specified dimensions. */
    NameStringGenerator( const osg::Vec3s& dimensions );

    /** \brief Get a name string.
    \details */
    std::string getNameString( const osg::Vec3s& subDims, const osg::Vec3s& subMinCorner );

    /** \brief Get a name string and an octant offset.
    \details */
    std::string getNameString( osg::Vec3s& offset, const osg::Vec3s& subDims, const osg::Vec3s& subMinCorner );

protected:
    osg::Vec3s _dims;
};



/** \class VolumeBrickData HierarchyUtils <latticefx/core/HierarchyUtils.h>
\brief A collection of volume bricks for a single LOD.
\details TBD
*/
class LATTICEFX_EXPORT VolumeBrickData
{
public:
    VolumeBrickData( const bool prune=false );
    ~VolumeBrickData();

    void setNumBricks( const osg::Vec3s& numBricks );
    osg::Vec3s getNumBricks() const;

    void addBrick( const osg::Vec3s& brickNum, osg::Image* image );
    virtual osg::Image* getBrick( const osg::Vec3s& brickNum ) const;
    osg::Image* getBrick( const std::string& brickName ) const;

protected:
    int brickIndex( const osg::Vec3s& brickNum ) const;

    osg::Vec3s _numBricks;
    bool _prune;

    typedef std::vector< osg::ref_ptr< osg::Image > > ImageVector;
    ImageVector _images;
};



/** \class Downsampler HierarchyUtils <latticefx/core/HierarchyUtils.h>
\brief Resamples a VolumeBrickData into a lower LOD.
\details To create a lower LOD version of a VolumeBrickData, pass the base LOD
as the \c hiRes constructor parameter, then call getLow(). getLow() resamples
\c hiRes to create a new VolumeBrickData containing 1/8th as many bricks as
\c hiRes.

The number of bricks in \c hiRes in each dimension must be a power of 2. The
osg::Image dimensions of each brick in \c hiRes must be a power of 2.

It is the application's responsibility to delete the VolumeBrickData returned
by getLow(). */
class LATTICEFX_EXPORT Downsampler
{
public:
    Downsampler( const VolumeBrickData* hiRes );
    ~Downsampler();

    /** \brief TBD
    \details
    Note that calling code is responsible for calling delete
    on the return value. */
    VolumeBrickData* getLow() const;

protected:
    osg::Image* sample( const osg::Image* i0, const osg::Image* i1, const osg::Image* i2, const osg::Image* i3,
        const osg::Image* i4, const osg::Image* i5, const osg::Image* i6, const osg::Image* i7 ) const;

    const VolumeBrickData* _hi;
    mutable VolumeBrickData* _low;
};



/** \class SaveHierarchy HierarchyUtils <latticefx/core/HierarchyUtils.h>
\brief Create a volume LOD set and save it to a DB.
\details Create an instance of a VolumeBrickData containing the highest
LOD of your volume data set in bricked form. Pass this to the constructor,
along with a baseName to use for the database keys. Then call save() with
the database you wish to save the hierarchy into.

The \c base constructor parameter must contain (or provide access to) the
highest LOD of the volume data in brick form. If the data set is relatively
small, \c base can be an instance of VolumeBrickData containing each brick
as an osg::Image. If the data set is large and doesn't fit into memory, then
\c base should be a derivation of VolumeBrickData that loads bricks from
secondary storage or generates bricks procedurally.

The hierarchy depth is computed as the log base 2 of a \c base brick's x
dimension. For this reason, all bricks in \c base must have the same size,
and the bricks should have square dimensions. Furthermore, the dimensions
should be a power of 2, and the number of bricks in \c base should also be
a power of 2. These restrictions could be lifted as the result of future
development.

save() interatively creates a new instance of Downsampler to resample
each successive LOD into a new VolumeBrickData, storeing each lower LOD
as osg::Image objects in memory. This is a potential problem if the data
set is so large that all lower LODs do not fit in available RAM.

After all LODs have been created, save() stores all bricks from all LODs
into the \c db database. */
class LATTICEFX_EXPORT SaveHierarchy
{
public:
    SaveHierarchy( VolumeBrickData* base, const std::string baseName );
    ~SaveHierarchy();

    void save( DBBasePtr db );

protected:
    void recurseSaveBricks( DBBasePtr db, std::string& brickName );

    unsigned int _depth;

    typedef std::vector< VolumeBrickData* > LODVector;
    LODVector _lodVec;

    std::string _baseName;
};



/** \class LoadHierarchy HierarchyUtils <latticefx/core/HierarchyUtils.h>
\brief Loads a volume hierarchy from a DB.
\details Create an instance of this class and specify the database
containing the volume hierarchy with Preprocess::setDB(). Attach the
LoadHierarchy instance to the DataSet. LoadHierarchy::operator()()
creates a hierarchy of ChannelDataLOD and ChannelDataImageSet objects,
with ChannelDataImage objects holding references to images in the
database.

All keys in the database that are valid (of the form "<text>-<numbers>-<text>")
are added, so it is recommended that apps store one volume data set per DB
(do not store multiple volume data sets in the DB, otherwise all valid keys
from all volume data sets will be added to the ChannelData hierarchy). */
class LATTICEFX_EXPORT LoadHierarchy : public Preprocess
{
public:
    LoadHierarchy();
    ~LoadHierarchy();

    virtual ChannelDataPtr operator()();

protected:
    static bool valid( const std::string& fileName );
};


// core
}
// lfx
}


// __LFX_CORE_HIERARCHY_UTILS_H__
#endif
