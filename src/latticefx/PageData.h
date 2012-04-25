
#ifndef __LATTICEFX_PAGE_DATA_H__
#define __LATTICEFX_PAGE_DATA_H__ 1


#include <latticefx/Export.h>
#include <osg/observer_ptr>
#include <osg/Object>
#include <osg/Group>

#include <map>
#include <string>


namespace lfx {


/** \addtogroup PagingSupport */
/**@{*/


/** \class PageData PageData.h <latticefx/PageData.h>
\brief Data for an osg::Group parent of pageable children.
\details This class allows an osg::Group to have pageable children. An instance of
PageData should be attached to the Group as UserData. The Group then becomes analogous
to the osg::PagedLOD node, but supports the paging requirements of LatticeFX (paging
based on time as well as pixel size, interaction with PagingThread and RootCallback, etc).
*/
class LATTICEFX_EXPORT PageData : public osg::Object
{
public:
    PageData();
    PageData( const PageData& rhs, const osg::CopyOp& copyOp=osg::CopyOp::SHALLOW_COPY );
    META_Object( lfx, PageData );

    /** \brief Enum for the mode of paging operation.
    \details All children must page the same way, either based on their pixel size (derived
    from transform matrices and viewport) or their current time in a time series data set.
    */
    typedef enum {
        UNSPECIFIED_RANGE,
        PIXEL_SIZE_RANGE,
        TIME_RANGE
    } RangeMode;
    /** \brief set the RangeMode for the owning Group parent.
    \details Default is UNSPECIFIED_RANGE resulting in undefined behavior. Apps should
    change this default to either PIXEL_SIZE_RANGE or TIME_RANGE. */
    void setRangeMode( const RangeMode rangeMode );
    /** \brief Retrieve the RangeMode. */
    RangeMode getRangeMode() const;

    /** \brief Stores values used to determine whether a child should be paged in or out.
    \details For PIXEL_SIZE_RANGE, \c first must be the minimum pixel size and \c second must be
    the maximum pixel size. for TIME_TANGE, \c first is the time, and \c second is ignored. */
    typedef std::pair< double, double > RangeValues;

    /** \brief Data for each pageable child.
    \details Contains information required for paging. Client code should add one
    of these per pageable child using PageData::setRangeData().
    
    Stored information includes the child index, range values for which this child is
    valie, and a file name to page in when needed. Also contains an enum that describes
    the current page status of the child.
    
    TBD. In the future, we should probably also support paging in a child from a database?
    If so, we need to store a DB key in this struct. */
    struct LATTICEFX_EXPORT RangeData {
        RangeData();
        RangeData( double minVal, double maxVal, const std::string& fileName=std::string( "" ) );

        unsigned int _childIndex;
        RangeValues _rangeValues;
        std::string _fileName;

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
    \details Used by RootCallback to iterate over the RangeData for each pageable child
    during the update traversal. */
    RangeDataMap& getRangeDataMap();

    /** \brief Specify the osg::Group that owns this PageData instance. */
    void setParent( osg::Group* parent );
    /** \brief Obtain the osgt::Group that owns this PageData instance.
    \details RootCallback calls this function so that it can add or remove children. */
    osg::Group* getParent();

protected:
    virtual ~PageData();

    RangeMode _rangeMode;
    RangeDataMap _rangeDataMap;
    osg::observer_ptr< osg::Group > _parent;
};


/**@}*/


// lfx
}


// __LATTICEFX_PAGE_DATA_H__
#endif
