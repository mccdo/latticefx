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

#ifndef __LFX_CORE_PAGE_DATA_H__
#define __LFX_CORE_PAGE_DATA_H__ 1


#include <latticefx/core/Export.h>
#include <latticefx/core/types.h>

#include <osg/observer_ptr>
#include <osg/Object>
#include <osg/Group>

#include <string>
#include <map>
#include <vector>


namespace lfx {
namespace core {


/** \addtogroup PagingSupport */
/**@{*/


/** \brief Stores values used to determine whether a child should be paged in or out.
\details For PIXEL_SIZE_RANGE, \c first must be the minimum pixel size and \c second must be
the maximum pixel size. for TIME_TANGE, \c first is the time, and \c second is ignored. */
typedef std::pair< double, double > RangeValues;
typedef std::vector< RangeValues > RangeValueList;


/** \class PageData PageData.h <latticefx/core/PageData.h>
\brief Data for an osg::Group parent of pageable children.
\details This class allows an osg::Group to have pageable children. An instance of
PageData should be attached to the Group as UserData. The Group then becomes analogous
to the osg::PagedLOD node, but supports the paging requirements of LatticeFX (paging
based on time as well as pixel size, interaction with PagingThread and PagingCallback, etc).
*/
class LATTICEFX_EXPORT PageData : public osg::Object
{
public:
    /** \brief Enum for the mode of paging operation.
    \details All children must page the same way, either based on their pixel size (derived
    from transform matrices and viewport) or their current time in a time series data set.
    */
    typedef enum {
        UNSPECIFIED_RANGE,
        PIXEL_SIZE_RANGE,
        TIME_RANGE
    } RangeMode;

    PageData( const RangeMode rangeMode=UNSPECIFIED_RANGE );
    PageData( const PageData& rhs, const osg::CopyOp& copyOp=osg::CopyOp::SHALLOW_COPY );
    META_Object( lfx, PageData );

    /** \brief set the RangeMode for the owning Group parent.
    \details Default is UNSPECIFIED_RANGE resulting in undefined behavior. Apps should
    change this default to either PIXEL_SIZE_RANGE or TIME_RANGE. */
    void setRangeMode( const RangeMode rangeMode );
    /** \brief Retrieve the RangeMode. */
    RangeMode getRangeMode() const;

    /** \brief For TIME_RANGE, the min and max time values of the entire time series.
    \details Called by DataSet during scene graph creation. */
    void setMinMaxTime( const TimeValue minTime, const TimeValue maxTime );
    void getMinMaxTime( TimeValue& minTime, TimeValue& maxTime );

    /** \brief Data for each pageable child.
    \details Contains information required for paging. Client code should add one
    of these per pageable child using PageData::setRangeData().
    
    Stored information includes the child index, range values for which this child is
    valie, and a database key to load the child. Also contains an enum that describes
    the current page status of the child. */
    struct LATTICEFX_EXPORT RangeData {
        RangeData();
        RangeData( RangeValues rangeValues, const DBKey& dbKey=DBKey( "" ) );
        RangeData( double minVal, double maxVal, const DBKey& dbKey=DBKey( "" ) );

        RangeValues _rangeValues;

        /** \brief Enum for current page status.
        \li UNLOADED        Child is en empty stub Group.
        \li LOAD_REQUESTED  PagingTHread has been asked to load this child.
        \li LOADED,         The child has been loaded and attached.
        \li ACTIVE          The loaded child is in use and has not expired.
        The paging status moves sequentially through these enums, with the exception
        of a cancellation or child expiration, in which case LOAD_REQUESTED and ACTIVE
        can change to UNLOADED. (LOADED never changes directly to UNLOADED, it always
        goes to ACTIVE first.) */
        typedef enum {
            UNLOADED,
            LOAD_REQUESTED,
            LOADED,
            ACTIVE
        } StatusType;
        StatusType _status;
    };

    /** \brief Add data regarding a pageable child.
    \details Client code should add one of these for each pageable child. Not all children
    need to be pageable. */
    void setRangeData( const unsigned int childIndex, const RangeData& rangeData );
    /** \brief Retrieve data ragarding a pageable child. */
    RangeData* getRangeData( const unsigned int childIndex );
    /** \overload */
    const RangeData* getRangeData( const unsigned int childIndex ) const;

    /** \brief Maps of child index numbers to RangeData structs. */
    typedef std::map< unsigned int, RangeData > RangeDataMap;
    /** \brief Obtain the RangeDataMap.
    \details Used by PagingCallback to iterate over the RangeData for each pageable child
    during the update traversal. */
    RangeDataMap& getRangeDataMap();

    /** \brief Specify the osg::Group that owns this PageData instance. */
    void setParent( osg::Group* parent );
    /** \brief Obtain the osgt::Group that owns this PageData instance.
    \details PagingCallback calls this function so that it can add or remove children. */
    osg::Group* getParent();

protected:
    virtual ~PageData();

    RangeMode _rangeMode;
    TimeValue _minTime, _maxTime;

    RangeDataMap _rangeDataMap;
    osg::observer_ptr< osg::Group > _parent;
};


/**@}*/


// core
}
// lfx
}


// __LFX_CORE_PAGE_DATA_H__
#endif
