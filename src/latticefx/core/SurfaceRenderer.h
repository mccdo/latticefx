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

#ifndef __LFX_CORE_SURFACE_RENDERER_H__
#define __LFX_CORE_SURFACE_RENDERER_H__ 1


#include <latticefx/core/Export.h>
#include <latticefx/core/Renderer.h>

#include <boost/smart_ptr/shared_ptr.hpp>


// Forward
namespace osg {
    class Geometry;
}

namespace lfx {
namespace core {


/** \class PrimitiveSetGenerator SurfaceRenderer.h <latticefx/core/SurfaceRenderer.h>
\brief Creates OSG PrimitiveSets for the SurfaceRenderer.
\details The SurfaceRenderer requires an instance of this class to create OSG
Geometry PrimitiveSets.

Derive from this class and override the function call operator to implement
your own PrimitiveSet generation code. The function call operator is
responsible for adding the PrimitiveSets to the \c geom Geometry object.
*/
class LATTICEFX_EXPORT PrimitiveSetGenerator
{
public:
    /** Constructor */
    PrimitiveSetGenerator();
    /** Copy constructor */
    PrimitiveSetGenerator( const PrimitiveSetGenerator& rhs );
    /** Destructor */
    virtual ~PrimitiveSetGenerator();

    /** \brief Creates and attaches PrimitiveSets to the specified \c geom.
    \details Derived classed must override this function, generate PrimitiveSets
    to render the \c geom Geometry object, and attach those PrimitiveSets to
    \c geom. The specified Geometry is configured with all array data (vertices,
    normals, tex coords, vertex attribs, etc) prior to this function call
    operator invocation.
    \param numElements is the number of vertecies in the \c geom vertex array.
    */
    virtual void operator()( osg::Geometry* geom, unsigned int numElements ) = 0;
};

typedef boost::shared_ptr< PrimitiveSetGenerator > PrimitiveSetGeneratorPtr;



/** \class SurfaceRenderer SurfaceRenderer.h <latticefx/core/SurfaceRenderer.h>
\brief Renders arbitrary surface geometry with optional vertex warping.
\details This class supports rendering arbitrary surface geometry. By attaching
optional warp vector arrays, the application can render the geometry with
hardware vertex warping.

There are many ways to assemble raw vertices into an OSG Geometry for rendering.
To support any such method, SurfaceRenderer requires an instance of
PrimitiveSetGenerator to create and attach PrimitiveSets to the Geometry. If
no PrimitiveSetGenerator is attached, SurfaceRenderer creates and uses an
instance of SimpleTrianglePrimitiveSetGenerator.

The name "SurfaceRenderer" is really a misnomer, as the customiability of the
PrimitiveSetGenerator allows for vertices to be rendered as lines and points,
not just polygons.

Warp vector arrays should be added as ChannelData. Warp data for both vertices
and normals is required; if either are missing, vertex warping is disabled.
Warp vectors point from the source value to the "fully warped" value, and could
be created by subtracting the fully warped values from the source values.

During rendering the amount of warping is controlled with the warpScale uniform
variable, ranging from 0.0 (no warping) to 1.0 (fully warped). Values aren't
clamped, which supported exaggerated warping.

Warping can be toggled on/off with the warpEnabled uniform variable.
*/
class LATTICEFX_EXPORT SurfaceRenderer : public Renderer
{
public:
    SurfaceRenderer();
    SurfaceRenderer( const SurfaceRenderer& rhs );
    virtual ~SurfaceRenderer();

    virtual osg::Node* getSceneGraph( const ChannelDataPtr maskIn );
    virtual osg::StateSet* getRootState();

    /** \brief Specify the PrimitiveSetGenerator.
    \details The default is NULL, in which case SurfaceRenderer creates and
    uses an instance of SimpleTrianglePrimitiveSetGenerator. */
    void setPrimitiveSetGenerator( PrimitiveSetGeneratorPtr primitiveSetGenerator );
    /** \brief Get the attached PrimitiveSetGenerator. */
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



/** \class SimpleTrianglePrimitiveSetGenerator SurfaceRenderer.h <latticefx/core/SurfaceRenderer.h>
\brief Creates TRIANGLES PrimitiveSets for the SurfaceRenderer.
\details Interprets the \c geom vertex array as vertices suitable for rendering
as GL_TRIANGLES. In other words, vertices n, n+1, and n+2 constiture a triangle,
with no vertex sharing between triangles.

The function call operator simply creates an osg::DrawArrays PrimitiveSet to
render all the vertices, and attaches it to the \c geom.

SurfaceRenderer creates and uses an instance of this PrimitiveSetGenerator
by default.
*/
class LATTICEFX_EXPORT SimpleTrianglePrimitiveSetGenerator : public PrimitiveSetGenerator
{
public:
    SimpleTrianglePrimitiveSetGenerator();
    SimpleTrianglePrimitiveSetGenerator( const SimpleTrianglePrimitiveSetGenerator& rhs );
    virtual ~SimpleTrianglePrimitiveSetGenerator();

    virtual void operator()( osg::Geometry* geom, unsigned int numElements );
};


// core
}
// lfx
}


// __LFX_CORE_SURFACE_RENDERER_H__
#endif
