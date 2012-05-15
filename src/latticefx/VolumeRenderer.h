
#ifndef __LATTICEFX_VOLUME_RENDERER_H__
#define __LATTICEFX_VOLUME_RENDERER_H__ 1


#include <latticefx/Export.h>
#include <latticefx/Renderer.h>

#include <boost/smart_ptr/shared_ptr.hpp>


namespace lfx {


/** \class VolumeRenderer VolumeRenderer.h <latticefx/VolumeRenderer.h>
\brief TBD
\details TBD
*/
class LATTICEFX_EXPORT VolumeRenderer : public lfx::Renderer
{
public:
    VolumeRenderer();
    VolumeRenderer( const VolumeRenderer& rhs );
    virtual ~VolumeRenderer();

    /** \brief Override base class lfx::Renderer
    \details Invoked by DataSet once per time step to obtain a scene graph for
    rendering volumetric data. */
    virtual osg::Node* getSceneGraph( const lfx::ChannelDataPtr maskIn );

    /** \brief Override base class lfx::Renderer
    \details Invoked by DataSet to obtain a global StateSet for all child
    scene graphs. */
    virtual osg::StateSet* getRootState();

protected:
};

typedef boost::shared_ptr< VolumeRenderer > VolumeRendererPtr;


// lfx
}


// __LATTICEFX_VOLUME_RENDERER_H__
#endif
