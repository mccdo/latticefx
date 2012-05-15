
#include <latticefx/VolumeRenderer.h>
#include <latticefx/ChannelDataOSGArray.h>
#include <latticefx/TextureUtils.h>

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Texture1D>
#include <osg/Texture2D>
#include <osg/Texture3D>
#include <osg/Shader>
#include <osg/Program>
#include <osg/Uniform>
#include <osgDB/FileUtils>



namespace lfx {


VolumeRenderer::VolumeRenderer()
{
}
VolumeRenderer::VolumeRenderer( const VolumeRenderer& rhs )
  : lfx::Renderer( rhs )
{
}
VolumeRenderer::~VolumeRenderer()
{
}

osg::Node* VolumeRenderer::getSceneGraph( const lfx::ChannelDataPtr maskIn )
{
    OSG_NOTICE << "VolumeRenderer::getSceneGraph(): Not yet implemented." << std::endl;
    return( NULL );
}

osg::StateSet* VolumeRenderer::getRootState()
{
    OSG_NOTICE << "VolumeRenderer::getRootState(): Not yet implemented." << std::endl;
    return( NULL );
}


// lfx
}
