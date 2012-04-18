
#ifndef __LATTICEFX_PAGE_DATA_H__
#define __LATTICEFX_PAGE_DATA_H__ 1


#include <latticefx/Export.h>
#include <osg/observer_ptr>
#include <osg/Object>
#include <osg/Group>

#include <vector>


namespace lfx {


/** \class PageData PageData.h <latticefx/PageData.h>
\brief 
\details 
*/
class LATTICEFX_EXPORT PageData : public osg::Object
{
public:
    PageData();
    PageData( const PageData& rhs, const osg::CopyOp& copyOp=osg::CopyOp::SHALLOW_COPY );
    META_Object( lfx, PageData );

    typedef enum {
        UNSPECIFIED_RANGE,
        PIXEL_SIZE_RANGE,
        TIME_RANGE
    } RangeMode;
    void setRangeMode( const RangeMode rangeMode );
    RangeMode getRangeMode() const;

    typedef std::pair< double, double > RangeData;
    void setRangeData( const unsigned int childIndex, const RangeData rangeData );
    RangeData getRangeData( const unsigned int childIndex ) const;

    void setParent( osg::Group* parent );
    osg::Group* getParent();

protected:
    virtual ~PageData();

    RangeMode _rangeMode;

    typedef std::vector< RangeData > RangeDataVec;
    RangeDataVec _rangeDataVec;

    osg::observer_ptr< osg::Group > _parent;
};


// lfx
}


// __LATTICEFX_PAGE_DATA_H__
#endif
