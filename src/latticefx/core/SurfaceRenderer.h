/*************** <auto-copyright.rb BEGIN do not edit this line> *************
*
* latticeFX is (C) Copyright 2011-2012 by Ames Laboratory
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Library General Public
* License as published by the Free Software Foundation; 
* version 2.1 of the License.
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
* -----------------------------------------------------------------
* Date modified: $Date$
* Version:       $Rev$
* Author:        $Author$
* Id:            $Id$
* -----------------------------------------------------------------
*
*************** <auto-copyright.rb END do not edit this line> **************/

#ifndef __LATTICEFX_SURFACE_RENDERER_H__
#define __LATTICEFX_SURFACE_RENDERER_H__ 1


#include <latticefx/core/Export.h>
#include <latticefx/core/Renderer.h>

#include <boost/smart_ptr/shared_ptr.hpp>


// Forward
namespace osg {
    class Geometry;
}

namespace lfx {


/** \class PrimitiveSetGenerator SurfaceRenderer.h <latticefx/core/SurfaceRenderer.h>
\brief Creates OSG PrimitiveSets for the SurfaceRenderer.
\details TBD.
*/
class LATTICEFX_EXPORT PrimitiveSetGenerator
{
public:
    PrimitiveSetGenerator();
    PrimitiveSetGenerator( const PrimitiveSetGenerator& rhs );
    virtual ~PrimitiveSetGenerator();

    virtual void operator()( osg::Geometry* geom, unsigned int numElements ) = 0;
};

typedef boost::shared_ptr< PrimitiveSetGenerator > PrimitiveSetGeneratorPtr;



/** \class SurfaceRenderer SurfaceRenderer.h <latticefx/core/SurfaceRenderer.h>
\brief Renders arbitrary surface geometry with optional vertex warping.
\details TBD.
*/
class LATTICEFX_EXPORT SurfaceRenderer : public Renderer
{
public:
    SurfaceRenderer();
    SurfaceRenderer( const SurfaceRenderer& rhs );
    virtual ~SurfaceRenderer();

    virtual osg::Node* getSceneGraph( const lfx::ChannelDataPtr maskIn );
    virtual osg::StateSet* getRootState();

    /** \brief TBD
    \details TBD */
    void setPrimitiveSetGenerator( PrimitiveSetGeneratorPtr primitiveSetGenerator );
    /** \brief TBD
    \details TBD */
    PrimitiveSetGeneratorPtr getPrimitiveSetGenerator();

    /** \brief Enum for input data type. */
    typedef enum {
        VERTEX,
        NORMAL,
        WARP_VERTEX,
        WARP_NORMAL,
    } InputType;
    typedef std::map< InputType, std::string > InputTypeMap;

    /** \brief Associate a ChannelData name with an InputType.
    \details This method allows the application to use arbitrarily named ChannelData
    with the VectorRenderer. */
    void setInputNameAlias( const InputType& inputType, const std::string& alias );
    /** \brief Get the ChannelData name alias for the specified \c inputType. */
    std::string getInputTypeAlias( const InputType& inputType ) const;

protected:
    PrimitiveSetGeneratorPtr _primitiveSetGenerator;

    InputTypeMap _inputTypeMap;
};

typedef boost::shared_ptr< SurfaceRenderer > SurfaceRendererPtr;



class LATTICEFX_EXPORT SimpleTrianglePrimitiveSetGenerator : public PrimitiveSetGenerator
{
public:
    SimpleTrianglePrimitiveSetGenerator();
    SimpleTrianglePrimitiveSetGenerator( const SimpleTrianglePrimitiveSetGenerator& rhs );
    virtual ~SimpleTrianglePrimitiveSetGenerator();

    virtual void operator()( osg::Geometry* geom, unsigned int numElements );
};


// lfx
}


// __LATTICEFX_SURFACE_RENDERER_H__
#endif
