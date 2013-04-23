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
#include <latticefx/core/VolumeBrickDataPtr.h>

#include <osg/Vec3>
#include <osg/Vec3s>
#include <osg/ref_ptr>
#include <osg/Image>

#include <vector>



namespace lfx
{
namespace core
{


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
    TraverseHierarchy( ChannelDataPtr root = ChannelDataPtr( ( ChannelData* )NULL ) );
    TraverseHierarchy( ChannelDataPtr root, HierarchyCallback& cb );

    void setRoot( ChannelDataPtr root )
    {
        _root = root;
    }
    void setCallback( HierarchyCallback& cb )
    {
        _cb = cb;
    }
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
    AssembleHierarchy( unsigned int maxDepth, double baseRange = 25000. );
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
                         const osg::Vec3& offset = osg::Vec3( 0., 0., 0. ), const unsigned int depth = 0 );

    /** \brief Prune empty branches from the hierarchy.
    \detauls Call this function after adding all ChannelData objects, and just
    before calling getRoot(). This function will remove any empty branches from
    the octree, resulting in a minimal data structure. */
    void prune();

    /** \brief Retrieve the created hierarchy.
    \details Call this function after adding all ChannelDraw objects. */
    ChannelDataPtr getRoot() const
    {
        return( _root );
    }

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
\details In typical use, an application derives from VolumeBrickData, and the
derived class overrides the getBrick(const osg::Vec3s&) method to allow access
to bricks representing the volume. See the shapeCreator application for an
example of this usage (for procedurally generated volume data 00 However, a
derived class might simply generate and store all bricks, or page them in from
disk).

Note that it is up to the derived class to support the \c prune contructor
parameter (and \c _prune member variable). When exabled, getBrick() should
return NULL. (However, see Downsampler for implementation details.) */
class LATTICEFX_EXPORT VolumeBrickData
{
public:
    VolumeBrickData( const bool prune = false );
    virtual ~VolumeBrickData();


    virtual void setDepth( const unsigned int depth );
    unsigned int getDepth() const;
    osg::Vec3s getNumBricks() const;

    /** \brief Add a brick to the \c _images vector of bricks.
    \details This is used
    by the Downsample class to create lower LODs. It can also be used
    by an application or any derived class. However, for derived classes
    that load bricks from secondary storage, or generate bricks
    procedurally, this function is generally not used.

    Note that the added brick does not account for brick overlap. */
    void addBrick( const osg::Vec3s& brickNum, osg::Image* image );

    /** \brief Return the apecified brick by xyz index into the volume.
    \details Indices are in the range (0,0,0) to getNumBricks(). The
    return value could be NULL if \c brickNum is out of range, or if
    pruning is enabled.

    The returned brick does not account for brick overlap. */
    virtual osg::Image* getBrick( const osg::Vec3s& brickNum ) const;
    /** \brief Hierarchy name interface to brick access.
    \details The returned brick does not account for brick overlap. */
    osg::Image* getBrick( const std::string& brickName ) const;

    osg::Image* getSeamlessBrick( const osg::Vec3s& brickNum ) const;
    osg::Image* getSeamlessBrick( const std::string& brickName ) const;

protected:
    int brickIndex( const osg::Vec3s& brickNum ) const;
    osg::Vec3s nameToBrickNum( const std::string& name ) const;

    /** \brief Copy texels for proper brick overlap.
    \details Copy texels from \c source to \c dest to eliminate seams between bricks.
    \c index indicates which of eight \c source images are being executed: 0 indicates
    the primary brick, 1 is adjacane in +s, 2 in +t, 3 in +s and +t, 4 in +r, 5 in +s
    and +r, 6 in +t and +r, and 7 in +s, +t, and +r.

    Derives classes can optionally override this method to provide their own overlapping
    sceheme. */
    virtual void overlap( osg::Image* dest, const osg::Image* source, const unsigned int index ) const;

    /** \brief Create a new brick for overlapping.
    \details Using \c proto as a base Image, this function creates a new image with
    dimensions equal to \c proto plus \c overlap. The new image has the same format and
    type as \c proto. The function allocates pixel data initialized to 0. */
    osg::Image* newBrick( const osg::Image* proto, const osg::Vec3s& overlap = osg::Vec3s( 1, 1, 1 ) ) const;

    osg::Vec3s _numBricks;
    unsigned int _depth;
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
*/
class LATTICEFX_EXPORT Downsampler
{
public:
    Downsampler( const VolumeBrickDataPtr hiRes );
    virtual ~Downsampler();

    /** \brief TBD
    \details
    Note that calling code is responsible for calling delete
    on the return value. */
    VolumeBrickDataPtr getLow() const;

protected:
    osg::Image* sample( const osg::Image* i0, const osg::Image* i1, const osg::Image* i2, const osg::Image* i3,
                        const osg::Image* i4, const osg::Image* i5, const osg::Image* i6, const osg::Image* i7 ) const;

    const VolumeBrickDataPtr _hi;
    mutable VolumeBrickDataPtr _low;
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

save() iteratively creates a new instance of Downsampler to resample
each successive LOD into a new VolumeBrickData, storeing each lower LOD
as osg::Image objects in memory. This is a potential problem if the data
set is so large that all lower LODs do not fit in available RAM.

After all LODs have been created, save() obtains bricks from each LOD
using VolumeBrickData::getSeamlessBrick(), and stores them to the
\c db database. */
class LATTICEFX_EXPORT SaveHierarchy
{
public:
	typedef std::vector< VolumeBrickDataPtr > LODVector;

public:
    SaveHierarchy( const std::string baseName );
    virtual ~SaveHierarchy();

	static unsigned int computeLevel( unsigned short numbricksX );
	void addAllLevels( LODVector &allLevels );
	void addLevel( unsigned int level, VolumeBrickDataPtr base, bool addSubLevels=true );
	void addLevel( VolumeBrickDataPtr base );

	bool save( DBBasePtr db, VolumeBrickDataPtr base );
    bool save( DBBasePtr db );

protected:
    void recurseSaveBricks( DBBasePtr db, const std::string brickName );

    unsigned int _depth;

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
    virtual ~LoadHierarchy();

    virtual ChannelDataPtr operator()();

    /** \brief Set whether to actually load the data into memory.
    \details By default, LoadHierarchy creates a ChannelData hierarchy that
    contains only DB keys but no actual data. Call setLoadData(true) to
    change this behavior so that LoadHierarchy loads the actual data from
    the DB, and the resulting ChannelData hierarchy contains all the loaded
    data in memory. */
    void setLoadData( const bool load );
    /** \brief Get whether to load the data into memory. */
    bool getLoadData() const;

protected:
    static bool valid( const std::string& fileName );

    bool _load;
};

typedef boost::shared_ptr< LoadHierarchy > LoadHierarchyPtr;


// core
}
// lfx
}


// __LFX_CORE_HIERARCHY_UTILS_H__
#endif
