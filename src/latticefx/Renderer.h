
#ifndef __LATTICEFX_RENDERER_H__
#define __LATTICEFX_RENDERER_H__ 1


#include <latticefx/Export.h>
#include <latticefx/OperationBase.h>

#include <boost/smart_ptr/shared_ptr.hpp>

#include <list>



namespace lfx {


/** \class Renderer Renderer.h <latticefx/Renderer.h>
\brief Base class for Rendering Framework operations.
\details TBD
*/
class LATTICEFX_EXPORT Renderer : public lfx::OperationBase
{
public:
    Renderer();
    virtual ~Renderer();

    /** \brief TBD
    \details TBD */
    virtual osg::Node* getSceneGraph( const lfx::ChannelDataPtr maskIn ) = 0;

protected:
};

typedef boost::shared_ptr< Renderer > RendererPtr;
typedef std::list< RendererPtr > RendererList;


// lfx
}


// __LATTICEFX_RENDERER_H__
#endif
