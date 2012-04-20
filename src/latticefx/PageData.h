
#ifndef __LATTICEFX_PAGE_DATA_H__
#define __LATTICEFX_PAGE_DATA_H__ 1


#include <latticefx/Export.h>
#include <osg/observer_ptr>
#include <osg/Object>
#include <osg/Group>

#include <map>
#include <string>


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

    typedef std::pair< double, double > RangeValues;

    struct LATTICEFX_EXPORT RangeData {
        RangeData();
        RangeData( double minVal, double maxVal, const std::string& fileName=std::string( "" ) );

        unsigned int _childIndex;
        RangeValues _rangeValues;
        std::string _fileName;

        typedef enum {
            UNLOADED,
            LOAD_REQUESTED,
            LOADED,
            UNLOAD_REQUESTED
        } StatusType;
        StatusType _status;
    };

    void setRangeData( const unsigned int childIndex, const RangeData& rangeData );
    RangeData* getRangeData( const unsigned int childIndex );
    const RangeData* getRangeData( const unsigned int childIndex ) const;

    typedef std::map< unsigned int, RangeData > RangeDataMap;
    RangeDataMap& getRangeDataMap();


    void setBound( const osg::BoundingSphere& bound );
    osg::BoundingSphere getBound() const;

    void setParent( osg::Group* parent );
    osg::Group* getParent();

protected:
    virtual ~PageData();

    RangeMode _rangeMode;

    RangeDataMap _rangeDataMap;

    osg::BoundingSphere _bound;

    osg::observer_ptr< osg::Group > _parent;
};


// lfx
}


// __LATTICEFX_PAGE_DATA_H__
#endif
