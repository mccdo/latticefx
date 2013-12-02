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

#ifndef __LFX_CORE_STREAMLINE_RENDERER_H__
#define __LFX_CORE_STREAMLINE_RENDERER_H__ 1


#include <latticefx/core/Export.h>
#include <latticefx/core/Renderer.h>

#include <boost/serialization/access.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/nvp.hpp>

#include <boost/smart_ptr/shared_ptr.hpp>
#include <string>
#include <map>


// Forward
namespace osg
{
class Texture3D;
}


namespace lfx
{
namespace core
{


/** \class StreamlineRenderer StreamlineRenderer.h <latticefx/core/StreamlineRenderer.h>
\brief Renders vector data in a variety of styles from ChannelDataOSGArray inputs.
\details Attach the VectorRenderer to a DataSet as follows:

\code
    DataSetPtr dsp( new DataSet() );
    VectorRendererPtr renderOp( new VectorRenderer() );
    dsp->setRenderer( renderOp );
\endcode

Supported rendering style:
- SIMPLE_POINTS
- POINT_SPRITES (not currently implemented)
- SPHERES
- DIRECTION_VECTORS

SIMPLE_POINTS renders an array of vertex data using the GL_POINTS primitive mode.
SPHERES renders a sphere at each position location using instanced rendering.
DIRECTION_VECTORS renders a vector arrow at each position location using instanced rendering.

Required ChannelData inputs:
<table border="0">
  <tr>
    <td><b>InputType enum</b></td>
    <td><b>ChannelData data type</b></td>
    <td><b>Required by</b></td>
    <td><b>Notes</b></td>
  </tr>
  <tr>
    <td>POSITION</td>
    <td>ChannelDataOSGArray (osg::Vec3Array)</td>
    <td>All rendering syles</td>
    <td>xyz position data.</td>
  </tr>
  <tr>
    <td>DIRECTION</td>
    <td>ChannelDataOSGArray (osg::Vec3Array)</td>
    <td>DIRECTION_VECTORS</td>
    <td>Length determines scale factor.</td>
  </tr>
  <tr>
    <td>RADIUS</td>
    <td>ChannelDataOSGArray (osg::FloatArray)</td>
    <td>POINT_SPRITES, SPHERES</td>
    <td>WC radius of the sprite or sphere.</td>
  </tr>
</table>

Required name-value pair inputs (see OperationBase): None.

VectorRenderer stores point and related rendering data in a specially
formatted 3D texture data block, and extracts that data at runtime using
the OpenGL instanced rendering feature.

If the DB is non-NULL (see OperationBase::setDB() ), the 3D texture
instanced rendering data is store in a DB during execution of
getSceneGraph(), and paged in as-needed at runtime by the PagingCallback.
If the DB is NULL, the 3D texture instanced rendering data is kept
directly in the scene graph, and no paging occurs at tuntime. */
class LATTICEFX_EXPORT StreamlineRenderer : public Renderer
{
public:
    StreamlineRenderer( const std::string& logName = std::string( "" ) );
    StreamlineRenderer( const StreamlineRenderer& rhs );
    virtual ~StreamlineRenderer();

	virtual std::string getClassName() const { return "StreamlineRenderer"; }


    /** \brief Input aliases; use with OperationBase::set/getInputNameAlias to allow
    attaching input ChannelData with arbitrary names. */
    typedef enum
    {
        POSITION,    /**< "positions" */
        DIRECTION,   /**< "directions" */
        RADIUS       /**< "radii" */
    } InputType;


    /** \brief Override base class Renderer.
    \details Invoked by DataSet to obtain a scene graph for rendering points at a
    partivular time step. */
    virtual osg::Node* getSceneGraph( const ChannelDataPtr maskIn );

    /** \brief Override base class Renderer.
    \details Invoked by DataSet to obtain a root node StateSet globally applicable to
    all scene graphs created by getSceneGraph. Helps avoid redundant state setting. */
    virtual osg::StateSet* getRootState();


    /** \brief Rendering style
    \details Enum for supported rendering styles.
    NOTE: POINT_SPRITES is currently unsupported. */
    typedef enum
    {
        SIMPLE_POINTS,
        POINT_SPRITES,
        SPHERES,
        DIRECTION_VECTORS
    } PointStyle;

	std::string getEnumName( PointStyle e ) const;
	PointStyle getEnumFromName( const std::string &name ) const;

    /** \brief Set the rendering style.
    \details The default is SIMPLE_POINTS. */
    void setPointStyle( const PointStyle& pointStyle );
    /** \brief Get the rendering style. */
    PointStyle getPointStyle() const;

	virtual void dumpState( std::ostream &os );

protected:
    osg::Texture3D* createTexture( ChannelDataPtr data );

	virtual void serializeData( JsonSerializer *json ) const;
	virtual bool loadData( JsonSerializer *json, IObjFactory *pfactory, std::string *perr=NULL );

    PointStyle _pointStyle;

private:
    friend class boost::serialization::access;

    template< class Archive >
    void serialize( Archive& ar, const unsigned int version )
    {
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP( Renderer );
        ar& BOOST_SERIALIZATION_NVP( _pointStyle );
    }
};


typedef boost::shared_ptr< StreamlineRenderer > StreamlineRendererPtr;


// core
}
// lfx
}


BOOST_CLASS_VERSION( lfx::core::StreamlineRenderer, 0 );


// __LFX_CORE_STREAMLINE_RENDERER_H__
#endif
