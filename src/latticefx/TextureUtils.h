
#ifndef __LATTICEFX_TEXTURE_UTILS_H__
#define __LATTICEFX_TEXTURE_UTILS_H__ 1


#include <latticefx/Export.h>
#include <latticefx/ChannelData.h>
#include <osg/Vec3>

namespace osg {
    class Texture;
    class Texture2D;
    class Texture3D;
}


namespace lfx {


/** \defgroup TextureUtils Utilities for creating and working with texture-based data
*/
/**@{*/


/** \def LFX_TEXUTILS_USE_PO2
\brief Force texture dimensions to be powers of 2.
\details Control flag for computeTexture3DDimensions(). */
/** \def LFX_TEXUTILS_ALLOW_NONUNIFORM
\brief Force texture dimensions to be uniform in s, t, and p.
\details Control flag for computeTexture3DDimensions(). */
#define LFX_TEXUTILS_USE_PO2        ( 0x1 << 0 )
#define LFX_TEXUTILS_FORCE_UNIFORM  ( 0x1 << 1 )
    
/** \brief Computes optimal  dimensions for a Texture3D
\details
*/
LATTICEFX_EXPORT osg::Vec3 computeTexture3DDimensions( const unsigned int numElements, const int flags=0 );

/** \brief Create a Texture3D from ChannelData for instanced rendering.
\details EBD */
LATTICEFX_EXPORT osg::Texture3D* createTexture3DForInstancedRenderer( const ChannelDataPtr source );


/**@}*/


// lfx
}


// __LATTICEFX_TEXTURE_UTILS_H__
#endif
