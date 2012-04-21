
#ifndef __LATTICEFX_ROOT_CALLBACK_H__
#define __LATTICEFX_ROOT_CALLBACK_H__ 1


#include <latticefx/Export.h>
#include <osg/NodeCallback>
#include <osg/Group>
#include <osg/Camera>
#include <latticefx/PageData.h>

#include <vector>


namespace lfx {


/** \class RootCallback RootCallback.h <latticefx/RootCallback.h>
\brief 
\details 
*/
class LATTICEFX_EXPORT RootCallback : public osg::NodeCallback
{
public:
    RootCallback();

    void addPageParent( osg::Group* parent );

    void setCamera( osg::Camera* camera );

    virtual void operator()( osg::Node* node, osg::NodeVisitor* nv );

protected:
    virtual ~RootCallback();

    double computePixelSize( const osg::BoundingSphere& bSphere, const osg::NodeVisitor* nv );
    static inline bool inRange( const double testValue, const PageData::RangeValues& range );

    osg::ref_ptr< osg::Camera > _camera;

    typedef std::vector< osg::ref_ptr< osg::Group > > GroupList;
    GroupList _pageParentList;
};


bool RootCallback::inRange( const double testValue, const PageData::RangeValues& range )
{
    return( ( testValue >= range.first ) &&
            ( testValue < range.second ) );
}



// lfx
}


// __LATTICEFX_DATA_SET_H__
#endif
