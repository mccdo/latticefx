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

#ifndef __LFX_CORE_VOLUME_RENDERER_H__
#define __LFX_CORE_VOLUME_RENDERER_H__ 1


#include <latticefx/core/Export.h>
#include <latticefx/core/Renderer.h>
#include <latticefx/core/DBUtils.h>

#include <boost/smart_ptr/shared_ptr.hpp>


// Forward
namespace osg {
    class Texture3D;
}

namespace lfx {
namespace core {


/** \class SpatialVolume VolumeRenderer.h <latticefx/core/VolumeRenderer.h>
\brief Container class for spatial information.
\details VolumeRenderer and other Renderer-derived classes that require
spatial information should derive from this class. DataSet accesses
SpatialVolume parameters to set the transforms of subgraphs when creating
nested volume tree scene graphs. */
class LATTICEFX_EXPORT SpatialVolume
{
public:
    SpatialVolume();
    SpatialVolume( const SpatialVolume& rhs );
    virtual ~SpatialVolume();

    // volume dims and origin
    /** \brief Set dimensions (in world units) of the volume box.
    \details This can be varied on every draw. */
    void setVolumeDims( const osg::Vec3& volDims);
    /** \brief Get dimensions (in world units) of the volume box. */
    osg::Vec3 getVolumeDims() const;

    /** \brief Set location (in world units) of the center of the volume box.
    \details This can be varied on every draw. */
    void setVolumeOrigin( const osg::Vec3& volOrigin);
    /** \brief Get the location (in world units) of the center of the volume box. */
    osg::Vec3 getVolumeOrigin() const;

protected:
    osg::Vec3 _volumeDims, _volumeOrigin;
};

typedef boost::shared_ptr< SpatialVolume > SpatialVolumePtr;



/** \class VolumeRenderer VolumeRenderer.h <latticefx/core/VolumeRenderer.h>
\brief TBD
\details TBD
*/
class LATTICEFX_EXPORT VolumeRenderer : public Renderer, public SpatialVolume
{
public:
    VolumeRenderer();
    VolumeRenderer( const VolumeRenderer& rhs );
    virtual ~VolumeRenderer();

    /** \brief Override base class Renderer
    \details Invoked by DataSet once per time step to obtain a scene graph for
    rendering volumetric data. */
    virtual osg::Node* getSceneGraph( const ChannelDataPtr maskIn );

    /** \brief Override base class Renderer
    \details Invoked by DataSet to obtain a global StateSet for all child
    scene graphs. */
    virtual osg::StateSet* getRootState();

    /** \brief Set the maximum number of planes used to slice the volume.
    \details The default is 1024. This must be set prior to calling getSceneGraph. */
    void setMaxSlices( const unsigned int& maxSlices );
    /** \brief Get the maximum number of planes used to slice the volume. */
    unsigned int getMaxSlices() const;

    /** \brief Set the number of planes to use to render the volume.
    \details The default is 100.0f planes. */
    void setNumPlanes( const float& numPlanes );
    /** \brief Get the number of planes to use to render the volume. */
    float getNumPlanes() const;

protected:
    static osg::Texture3D* createStubTexture( const DBKey& key );

    unsigned int _maxSlices;
    float _planeSpacing;
    float _numPlanes;
};

typedef boost::shared_ptr< VolumeRenderer > VolumeRendererPtr;


// core
}
// lfx
}


// __LFX_CORE_VOLUME_RENDERER_H__
#endif
