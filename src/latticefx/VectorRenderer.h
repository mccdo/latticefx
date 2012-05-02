
#ifndef __LATTICEFX_VECTOR_RENDERER_H__
#define __LATTICEFX_VECTOR_RENDERER_H__ 1


#include <latticefx/Export.h>
#include <latticefx/Renderer.h>

#include <boost/smart_ptr/shared_ptr.hpp>



namespace lfx {


/** \class VectorRenderer VectorRenderer.h <latticefx/VectorRenderer.h>
\brief TBD
\details TBD
*/
class LATTICEFX_EXPORT VectorRenderer : public lfx::Renderer
{
public:
    VectorRenderer();
    VectorRenderer( const VectorRenderer& rhs );
    virtual ~VectorRenderer();

    virtual osg::Node* getSceneGraph( const lfx::ChannelDataPtr maskIn );

protected:
};

typedef boost::shared_ptr< VectorRenderer > VectorRendererPtr;


// lfx
}


// __LATTICEFX_VECTOR_RENDERER_H__
#endif
