
#ifndef __LATTICEFX_ROOT_CALLBACK_H__
#define __LATTICEFX_ROOT_CALLBACK_H__ 1


#include <latticefx/Export.h>
#include <osg/NodeCallback>


namespace lfx {


/** \class RootCallback RootCallback.h <latticefx/RootCallback.h>
\brief 
\details 
*/
class LATTICEFX_EXPORT RootCallback : public osg::NodeCallback
{
public:
    RootCallback();

    virtual void operator()( osg::Node* node, osg::NodeVisitor* nv );

protected:
    virtual ~RootCallback();

    bool _pagingActive;
};


// lfx
}


// __LATTICEFX_DATA_SET_H__
#endif
