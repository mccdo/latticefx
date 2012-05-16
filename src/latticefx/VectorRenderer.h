
#ifndef __LATTICEFX_VECTOR_RENDERER_H__
#define __LATTICEFX_VECTOR_RENDERER_H__ 1


#include <latticefx/Export.h>
#include <latticefx/Renderer.h>

#include <boost/smart_ptr/shared_ptr.hpp>
#include <string>
#include <map>


namespace lfx {


/** \class VectorRenderer VectorRenderer.h <latticefx/VectorRenderer.h>
\brief Renders vector data in a variety of styles from ChannelDataOSGArray inputs.
\details Attach the VectorRenderer to a DataSet as follows:

\code
    lfx::DataSetPtr dsp( new lfx::DataSet() );
    lfx::VectorRendererPtr renderOp( new lfx::VectorRenderer() );
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
    renderop->setInputNameAlias( lfx::VectorRenderer::POSITION, "MyXYZPositionData" );
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
class LATTICEFX_EXPORT VectorRenderer : public lfx::Renderer
{
public:
    VectorRenderer();
    VectorRenderer( const VectorRenderer& rhs );
    virtual ~VectorRenderer();

    /** \brief Override base class lfx::Renderer.
    \details Invoked by DataSet to obtain a scene graph for rendering points at a
    partivular time step. */
    virtual osg::Node* getSceneGraph( const lfx::ChannelDataPtr maskIn );

    /** \brief Override base class lfx::Renderer.
    \details Invoked by DataSet to obtain a root node StateSet globally applicable to
    all scene graphs creaTED BY getSceneGraph. Helps avoid redundant state setting. */
    virtual osg::StateSet* getRootState();


    /** \brief
    \details */
    typedef enum {
        SIMPLE_POINTS,
        POINT_SPRITES,
        SPHERES,
        DIRECTION_VECTORS
    } PointStyle;

    /** \brief
    \details */
    void setPointStyle( const PointStyle& pointStyle );
    /** \brief
    \details */
    PointStyle getPointStyle() const;

    /** \brief
    \details */
    typedef enum {
        POSITION,
        DIRECTION,
        RADIUS
    } InputType;
    typedef std::map< InputType, std::string > InputTypeMap;

    /** \brief
    \details */
    void setInputNameAlias( const InputType& inputType, const std::string& alias );
    /** \brief
    \details */
    std::string getInputTypeAlias( const InputType& inputType ) const;

protected:
    PointStyle _pointStyle;
    InputTypeMap _inputTypeMap;
};

typedef boost::shared_ptr< VectorRenderer > VectorRendererPtr;


// lfx
}


// __LATTICEFX_VECTOR_RENDERER_H__
#endif
