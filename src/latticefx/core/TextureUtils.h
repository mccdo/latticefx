/*************** <auto-copyright.rb BEGIN do not edit this line> **************
 *
 * Copyright 2012-2012 by Ames Laboratory
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 *************** <auto-copyright.rb END do not edit this line> ***************/

#ifndef __LFX_CORE_TEXTURE_UTILS_H__
#define __LFX_CORE_TEXTURE_UTILS_H__ 1


#include <latticefx/core/Export.h>
#include <latticefx/core/ChannelData.h>
#include <osg/Vec3>

namespace osg
{
class Image;
class Texture;
class Texture2D;
class Texture3D;
}


namespace lfx
{
namespace core
{


/** \defgroup TextureUtils Utilities for creating and working with texture-based data
*/
/**@{*/


/** \def LFX_TEXUTILS_USE_PO2
\brief Force texture dimensions to be powers of 2.
\details Control flag for computeTexture3DDimensions(). */
/** \def LFX_TEXUTILS_FORCE_UNIFORM
\brief Force texture dimensions to be uniform in s, t, and p.
\details Control flag for computeTexture3DDimensions(). */
#define LFX_TEXUTILS_USE_PO2        ( 0x1 << 0 )
#define LFX_TEXUTILS_FORCE_UNIFORM  ( 0x1 << 1 )

/** \brief Computes optimal  dimensions for a Texture3D
\details Useful for instanced rendering of \c numElements instances. Returns a Vec3
of dimensions for a texture large enough to hold \c numElements elements.

By default, the xyz return values are not necessarily powers of two, nor are they
necessarily uniform (equal to each other). However, these conditions can be forced
using the \c flags parameter.

It is assumed that shader code will convert the gl_InstanceIDARB into a texture
coordinate that will index into a texture with these dimensions. See
data/lfx-pointsphere.vert for code that does this. */
LATTICEFX_EXPORT osg::Vec3 computeTexture3DDimensions( const unsigned int numElements, const int flags = 0 );

/** \brief Create a Texture3D from ChannelData for instanced rendering.
\details Currently, \c source must be a ChannelDataOSGArray.

This function creates a new Texture3D with dimensions computed by
computeTexture3DDimensions(). Texture data is taken from \c source. It is expected
that the texture will be used for instanced rendering. */
LATTICEFX_EXPORT osg::Texture3D* createTexture3DForInstancedRenderer( const ChannelDataPtr source );

/** \brief Generate osg::Image for createTexture3DForInstancedRenderer().
\details */
LATTICEFX_EXPORT osg::Image* createImage3DForInstancedRenderer( const ChannelDataPtr source );


/**@}*/


// core
}
// lfx
}


// __LFX_CORE_TEXTURE_UTILS_H__
#endif
