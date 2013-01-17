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

#include <boost/serialization/access.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/base_object.hpp>
#include <osgwTools/SerializerSupport.h>
#include <boost/serialization/nvp.hpp>

#include <boost/smart_ptr/shared_ptr.hpp>


// Forward
namespace osg {
    class Geometry;
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
    void setVolumeDims( const osg::Vec3f& volDims);
    /** \brief Get dimensions (in world units) of the volume box. */
    osg::Vec3f getVolumeDims() const;

    /** \brief Set location (in world units) of the center of the volume box.
    \details This can be varied on every draw. */
    void setVolumeOrigin( const osg::Vec3f& volOrigin);
    /** \brief Get the location (in world units) of the center of the volume box. */
    osg::Vec3f getVolumeOrigin() const;

protected:
    osg::Vec3f _volumeDims, _volumeOrigin;

private:
    friend class boost::serialization::access;

    template< class Archive >
    void serialize( Archive& ar, const unsigned int version )
    {
        ar & BOOST_SERIALIZATION_NVP( _volumeDims );
        ar & BOOST_SERIALIZATION_NVP( _volumeOrigin );
    }
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


    /** \brief Input aliases; use with OperationBase::set/getInputNameAlias to allow
    attaching input ChannelData with arbitrary names. */
    typedef enum {
        VOLUME_DATA,  /**< "volumedata" */
    } InputType;


    /** \brief Override base class Renderer
    \details Invoked by DataSet once per time step to obtain a scene graph for
    rendering volumetric data. */
    virtual osg::Node* getSceneGraph( const ChannelDataPtr maskIn );

    /** \brief Override base class Renderer
    \details Invoked by DataSet to obtain a global StateSet for all child
    scene graphs. */
    virtual osg::StateSet* getRootState();


    /** \brief Volume rendering algorithm
    \details Enum for supported rendering algorithms. */
    typedef enum {
        SLICES,
        RAY_TRACED,
    } RenderMode;

    /** \brief Set the rendering style.
    \details The default is SLICES.
    
    RAY_TRACED mode requies special configuration steps on the part of the
    application. See the test volume-rt for an example.

    - The volume ray tracer needs the following inputs:
      - The opaque scene depth buffer as a 2D texture, and the associated uniform named "sceneDepth".
      .
      This means you'll render the opaque scene to an FBO with two texture attached, one for color and one for depth.
      Furthermore, the textures must be the same width and height as the final window.
      - The width and height of the above two textures / window, as a uniform vec2 named "windowSize".
    - You'll need to configure your scene graph in a special way:
      - A top-level Camera to render your opaque scene to FBO.
        - 1st child: the opaque scene subgraph.
        - 2nd child: a special "stub lfx scene" (described below).
        - 3rd child: a post-render Camera and full-screen quad to splat your color buffer into the window.
        - 4th child: the actual Lfx scene graph, to render the ray traced volume data into the window.
      - "Stub lfx scene": The ray tracing shader compares computed depth values against the depth buffer from your opaque scene. This means the 1st child and 4th child need to use the same projection matrix so that the depth values are compatible. But this can be problematic when using auto-compute near & far. To make a long story short, the "stub lfx scene" ensures that the auto-compute takes the lfx scene into account when computing the near & far planes for your opaque scene.
    */
    void setRenderMode( const RenderMode& renderMode ) { _renderMode = renderMode; }
    /** \brief Get the rendering algorithm. */
    RenderMode getRenderMode() const { return( _renderMode ); }


    /** \brief Specify the number of slices for RenderMode SLICES.
    \details Ignored if RenderMode != SLICES. The default is 100.0f planes. */
    void setNumPlanes( const float& numPlanes );
    /** \brief Get the number of slices for RenderMode SLICES. */
    float getNumPlanes() const;

    /** \brief Specify the max ray samples, used when RenderMode is RAY_TRACED.
    \detauls Ignored if RenderMode != RAY_TRACED.
    Analogous to setNumPlanes (for RenderMode SLICES). This is the number of
    volume sample points per pixel for a worst-case rendering (view vector
    aligned with volume diagonal). The default is 100.f samples. */
    void setMaxSamples( const float maxSamples ) { _maxSamples = maxSamples; }
    /** \brief Get the max ray samples for RenderMode RAY_TRACED. */
    float getMaxSamples() const { return( _maxSamples ); }

    /** \brief Transparancy scale value.
    \details After the shader computes an alpha value for the RGBA color,
    it multiplies the alpha by this value. Smaller values reduce the alpha
    resulting in greater transparency. Larger values cause the volume to
    appear more opaque.

    For correct transparency, blending should be enabled. The VolumeRenderer
    scene graph root state enables blending by default.

    The default transparency scalar is 1.0. */
    void setTransparency( const float trans ) { _transparencyScalar = trans; }
    /** \brief Get the transparency scalar. */
    float getTransparency() const { return( _transparencyScalar ); }

    /** \brief Enable or disable transparency.
    \detauls Transparency is enabled by default. To disable it, set this
    value to false. For maximum performance, the app should also disable GL_BLENDING
    above the latticefx scene graph using the OVERRIDE bit. */
    void setTransparencyEnable( const bool enable=true ) { _transparencyEnable = enable; }
    /** \brief Get the transparency enable flag. */
    bool getTransparencyEnable() const { return( _transparencyEnable ); }

protected:
    static osg::Texture3D* createStubTexture( const DBKey& key );
    virtual bool validInputs() const;

    osg::Geometry* createDAIGeometry( int nInstances );
    osg::Geometry* createCubeGeometry();


    RenderMode _renderMode;

    float _numPlanes;
    float _maxSamples;

    float _transparencyScalar;
    bool _transparencyEnable;

private:
    friend class boost::serialization::access;

    template< class Archive >
    void serialize( Archive& ar, const unsigned int version )
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP( Renderer );
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP( SpatialVolume );
        ar & BOOST_SERIALIZATION_NVP( _renderMode );
        ar & BOOST_SERIALIZATION_NVP( _numPlanes );
        ar & BOOST_SERIALIZATION_NVP( _maxSamples );
        ar & BOOST_SERIALIZATION_NVP( _transparencyScalar );
        ar & BOOST_SERIALIZATION_NVP( _transparencyEnable );
    }
};


typedef boost::shared_ptr< VolumeRenderer > VolumeRendererPtr;


// core
}
// lfx
}


BOOST_CLASS_VERSION( lfx::core::SpatialVolume, 0 );
BOOST_CLASS_VERSION( lfx::core::VolumeRenderer, 0 );


// __LFX_CORE_VOLUME_RENDERER_H__
#endif
