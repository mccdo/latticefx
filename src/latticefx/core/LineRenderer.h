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

#ifndef __LFX_CORE_LINE_RENDERER_H__
#define __LFX_CORE_LINE_RENDERER_H__ 1


#include <latticefx/core/Export.h>
#include <latticefx/core/Renderer.h>

#include <boost/serialization/access.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/base_object.hpp>
#include <osgwTools/SerializerSupport.h>
#include <boost/serialization/nvp.hpp>

#include <boost/smart_ptr/shared_ptr.hpp>


// Forward
namespace osg
{
class Geometry;
}

namespace lfx
{
namespace core
{


// Forward
class SurfaceRenderer;


/** \class LineRenderer LineRenderer.h <latticefx/core/LineRenderer.h>
\brief TBD
*/
class LATTICEFX_EXPORT LineRenderer : public Renderer
{
public:
    LineRenderer( const std::string& logName = std::string( "" ) );
    LineRenderer( const LineRenderer& rhs );
    virtual ~LineRenderer();

	virtual std::string getClassName() const { return "SurfaceRenderer"; }

    /** \brief Input aliases; use with OperationBase::set/getInputNameAlias to allow
    attaching input ChannelData with arbitrary names. */
    typedef enum
    {
        VERTEX,        /**< "positions" */
        NORMAL,        /**< "normals" */
        WARP_VERTEX,   /**< "warp_vertex" */
        WARP_NORMAL,   /**< "warp_normal" */
    } InputType;

    virtual osg::Node* getSceneGraph( const ChannelDataPtr maskIn );
    virtual osg::StateSet* getRootState();

	virtual void dumpState( std::ostream &os );

protected:

	virtual void serializeData( JsonSerializer *json ) const;
	virtual bool loadData( JsonSerializer *json, IObjFactory *pfactory, std::string *perr=NULL );

private:
    friend class boost::serialization::access;

    template< class Archive >
    void serialize( Archive& ar, const unsigned int version )
    {
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP( Renderer );
        //ar& BOOST_SERIALIZATION_NVP( _primitiveSetGenerator );
    }
};


typedef boost::shared_ptr< SurfaceRenderer > SurfaceRendererPtr;



// core
}
// lfx
}


BOOST_CLASS_VERSION( lfx::core::LineRenderer, 0 );


// __LFX_CORE_LINE_RENDERER_H__
#endif
