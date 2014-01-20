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
\brief Renders vector data as an animated streamline.

\code
    DataSetPtr dsp( new DataSet() );
    StreamlineRendererPtr renderOp( new StreamlineRenderer() );
    dsp->setRenderer( renderOp );
\endcode

Required ChannelData inputs:
<table border="0">
  <tr>
    <td><b>InputType enum</b></td>
    <td><b>ChannelData data type</b></td>
    <td><b>Notes</b></td>
  </tr>
  <tr>
    <td>POSITION</td>
    <td>ChannelDataOSGArray (osg::Vec3Array)</td>
    <td>xyz position data.</td>
  </tr>
</table>

Required name-value pair inputs (see OperationBase): None.

StreamlineRenderer stores point and related rendering data in a specially
formatted 3D texture data block, and extracts that data at runtime using
the OpenGL instanced rendering feature.

If the DB is non-NULL (see OperationBase::setDB() ), the 3D texture
instanced rendering data is stored in a DB during execution of
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


    /** \brief Specify the number of traces rendered along the streamline vector input data.
    \details Default is 1. */
    void setNumTraces( const int numTraces );
    /** \brief Get the number of streamline traces. */
    int getNumTraces() const;
    /** \brief Set trace length as a percent of total data.
    \details This is a normalized percent and will be clamped to the range (0.0 - 1.0).
    Default is 0.25, or 1/4 the data length. */
    void setTraceLengthPercent( float traceLengthPercent );
    /** \brief Get the trace length percent. */
    float getTraceLengthPercent() const;
    /** \brief Set animation speed.
    \details Specifies the animation speed as a normalized percentage of
    iterating over the entire vector input data per second. For example,
    a speed of 1.0 means the trace iteraties over the entire data set once
    per second. A speed of 0.2 (the default) means 20% of the data is
    traversed in a second (5 second for the entire data). Values are not
    clamped and can be > 1.0. Negative values reverse the animation. */
    void setTraceSpeed( const float traceSpeed );
    /** \brief Get the trace animation speed. */
    float getTraceSpeed() const;
    /** \brief Enable or disable animation.
    \details Default: true. When disabled (false), all sample points
    are visible. */
    void setAnimationEnable( bool enable=true );
    /** \brief Get enaimation enable state. */
    bool getAnimationEnable() const;
    /** \brief Scale the apparent streamline diameter.
    \details By default, the streamline billboard images are rendered such that the
    apparent diameter of the streamline is 1.0 world coordinate. Set this scalar to
    modify the diameter. The default is 1.0. */
    bool setImageScale( float scale );
    /** \brief Get the streamline diameter scale factor. */
    float getImageScale() const;


    /** \brief Input aliases; use with OperationBase::set/getInputNameAlias to allow
    attaching input ChannelData with arbitrary names. */
    typedef enum
    {
        POSITION,    /**< "positions" */
    } InputType;


    /** \brief Override base class Renderer.
    \details Invoked by DataSet to obtain a scene graph for rendering points at a
    partivular time step. */
    virtual osg::Node* getSceneGraph( const ChannelDataPtr maskIn );

    /** \brief Override base class Renderer.
    \details Invoked by DataSet to obtain a root node StateSet globally applicable to
    all scene graphs created by getSceneGraph. Helps avoid redundant state setting. */
    virtual osg::StateSet* getRootState();


	virtual void dumpState( std::ostream &os );

protected:
    osg::Texture3D* createTexture( ChannelDataPtr data );

    int _numTraces;
    float _traceLengthPercent;
    float _traceSpeed;
    bool _enableAnimation;
    float _imageScale;

	virtual void serializeData( JsonSerializer *json ) const;
	virtual bool loadData( JsonSerializer *json, IObjFactory *pfactory, std::string *perr=NULL );

private:
    friend class boost::serialization::access;

    template< class Archive >
    void serialize( Archive& ar, const unsigned int version )
    {
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP( Renderer );
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
