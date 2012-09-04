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

#ifndef __LFX_CORE_VECTOR_RENDERER_H__
#define __LFX_CORE_VECTOR_RENDERER_H__ 1


#include <latticefx/core/Export.h>
#include <latticefx/core/Renderer.h>

#include <boost/smart_ptr/shared_ptr.hpp>
#include <string>
#include <map>


// Forward
namespace osg {
    class Texture3D;
}


namespace lfx {
namespace core {


/** \class VectorRenderer VectorRenderer.h <latticefx/core/VectorRenderer.h>
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

VectorRenderer supports arbitrarily named ChannelData inputs using an alias system.
For example, if your position ChannelData is names "MyXYZPositionData", specify an
aloas as follows:

\code
    // 'renderop' is a VectorRendererPtr
    renderop->setInputNameAlias( VectorRenderer::POSITION, "MyXYZPositionData" );
\endcode

If no alias is specified, VectorRenderer looks for ChannelData with the following default names:

<table border="0">
  <tr>
    <td><b>InputType enum</b></td>
    <td><b>Default name</b></td>
  </tr>
  <tr>
    <td>POSITION</td>
    <td>"positions"</td>
  </tr>
  <tr>
    <td>DIRECTION</td>
    <td>"directions"</td>
  </tr>
  <tr>
    <td>RADIUS</td>
    <td>"radii"</td>
  </tr>
</table>

*/
class LATTICEFX_EXPORT VectorRenderer : public Renderer
{
public:
    VectorRenderer();
    VectorRenderer( const VectorRenderer& rhs );
    virtual ~VectorRenderer();

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
    typedef enum {
        SIMPLE_POINTS,
        POINT_SPRITES,
        SPHERES,
        DIRECTION_VECTORS
    } PointStyle;

    /** \brief Set the rendering style.
    \details The default is SIMPLE_POINTS. */
    void setPointStyle( const PointStyle& pointStyle );
    /** \brief Get the rendering style. */
    PointStyle getPointStyle() const;

    /** \brief Enum for input data type. */
    typedef enum {
        POSITION,
        DIRECTION,
        RADIUS
    } InputType;
    typedef std::map< InputType, std::string > InputTypeMap;

    /** \brief Associate a ChannelData name with an InputType.
    \details This method allows the application to use arbitrarily named ChannelData
    with the VectorRenderer. */
    void setInputNameAlias( const InputType& inputType, const std::string& alias );
    /** \brief Get the ChannelData name alias for the specified \c inputType. */
    std::string getInputTypeAlias( const InputType& inputType ) const;

protected:
    static osg::Texture3D* createDummyDBTexture( ChannelDataPtr data );

    PointStyle _pointStyle;
    InputTypeMap _inputTypeMap;
};

typedef boost::shared_ptr< VectorRenderer > VectorRendererPtr;


// core
}
// lfx
}


// __LFX_CORE_VECTOR_RENDERER_H__
#endif
