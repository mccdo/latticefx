
#ifndef __LATTICEFX_RENDERER_H__
#define __LATTICEFX_RENDERER_H__ 1


#include <latticefx/Export.h>
#include <latticefx/OperationBase.h>

#include <osg/Image>
#include <osg/ref_ptr>

#include <boost/smart_ptr/shared_ptr.hpp>

#include <list>
#include <map>


namespace osg {
    class Node;
    class StateSet;
}

namespace lfx {


/** \class Renderer Renderer.h <latticefx/Renderer.h>
\brief Base class for Rendering Framework operations.
\details Applications should create an instance of a class that derives from Renderer,
such as lfx::VectorRenderer or lfx::VolumeRenderer, and attach it to a DataSet using
DataSet::setRenderer(). Exactly one Renderer may be attached to a DataSet.

Applications should attach any necessary input ChannelData to the Renderer instance.
No ChannelData are required by the Renderer base class, but derived classes might require
specific ChannelData inputs. For example, when rendering simple points, VectorRenderer
requires position data:

\code
    lfx::VectorRendererPtr renderOp( new lfx::VectorRenderer() );
    renderOp->setPointStyle( lfx::VectorRenderer::SIMPLE_POINTS );
    renderOp->addInput( "positions" );
\endcode

\section DeriveRenderer Creating a New Renderer

Derived classes must override the getSceneGraph() and should almost certainly
override the getRootState() methods.

getSceneGraph() will be invoked by DataSet once per time step to create a
scene graph. DataSet will configure OperationBase::_inputs with the required
inputs for the time step. getSceneGraph() constructs a scene graph using those
inputs. In this way, getSceneGraph() typically doesn't need to know the time
for the time step.

getRootState() will be called once by DataSet to obtain a StateSet that
applies to all time step scene graphs created by getSceneGraph(). This allows
a Renderer to avoid setting duplicate state in each time step scene graph.

Derived classes might want to add additional functionality, such as the
point rendering styles supported by VectorRenderer (see
VectorRenderer::setPointStyle()). Alternatively, OperationBase supports
a name-value pair interface for passing control parameters. This is
particularly useful if the derived Renderer is to be loaded from a plugin,
in which case compile-time access to public methods is not available.

LatticeFX and the Renderer base class provide several tools and utilities to
aid in Renderer creation.

\li \ref BoundUtils
\li \ref TextureUtils
\li \ref TransferFunctionUtils
\li getOrAssignTextureUnit() for assigning and organizing texture unit usage.
\li addHardwareFeatureUniforms() for setting commonly used uniform variables.
*/
class LATTICEFX_EXPORT Renderer : public lfx::OperationBase
{
public:
    Renderer();
    Renderer( const Renderer& rhs );
    virtual ~Renderer();

    /** \brief Create a scene graph from the base class ChannelData.
    \details Override this function in derived classes and add your own code to
    create a scene graph. Input ChannelData is stored in OperationBase::_inputs.

    DataSet calls this function once for each time step. The \c maskIn will likely
    be different for every time step. */
    virtual osg::Node* getSceneGraph( const lfx::ChannelDataPtr maskIn ) = 0;

    /** \brief Create a single state set for all scene graphs.
    \details Create a single state set that applies to all scene graphs created by
    the getSceneGraph() method. One example of state that probably applies to all
    scene graphs would be the osg::Program object.

    Any state that varies over time steps should be in the time step scene graph's
    StateSet. One example is a uniform for the number of data elements in the
    ChannelData, which could vary depending on how many elements passed the host
    mask at a given time step. */
    virtual osg::StateSet* getRootState() { return( NULL ); }


    /** \name Texture Unit Usage
    \details TBD */
    /**@{*/

    /** \brief Set the base unit for Renderer texture usage.
    \details If the Renderer uses textures, they will be stored in \c baseUnit
    or higher. The default for \c baseUnit is 8. This function implicitly calls
    resetTextureUnitAssignments(). */
    void setTextureBaseUnit( const unsigned int baseUnit );
    /** \brief Get the base texture unit used by the Renderer.
    \details Returns the base texture unit. */
    unsigned int getTextureBaseUnit() const;

    /** \brief Assign, or return the previously assigned texture unit.
    \details Primarily for use by derived classes that need to ensure the same
    texture unit is used for both the setTextureAttribute() call and the shader
    uniform value. */
    unsigned int getOrAssignTextureUnit( const std::string& key );
    /** \brief Clears all previous texture unit assignments.
    \details Restores texture unit assignments to its initial state. A subsequent
    call to getOrAssignTextureUnit() will return the base unit. */
    void resetTextureUnitAssignments();

    /**@}*/


    /** \name Transfer Function Parameters
    \details */
    /**@{*/

    void setTransferFunction( osg::Image* image );
    osg::Image* getTransferFunction();
    const osg::Image* getTransferFunction() const;

    void setTransferFunctionInput( const std::string& name );
    const std::string& getTransferFunctionInput() const;

    typedef enum {
        TF_RGB = 0,
        TF_RGBA = 1,
        TF_ALPHA = 2
    } TransferFunctionDestination;
    /** \brief TBD
    \details Default is TF_ALPHA. */
    void setTransferFunctionDestination( const TransferFunctionDestination dest );
    TransferFunctionDestination getTransferFunctionDestination() const;

    /**@}*/


    /** \name Hardware Mask Parameters
    \details
    The hardware mask discards values based on the result of a comparison to a reference
    value. This comparison and discard may be performed in the vertex or fragment shader,
    depending on the implementation of the Renderer derived class.

    The hardware mask must be executed after the transfer function, but before any
    lighting computations.

    The hardware mask input must be a single float. It can be the color's alpha value,
    the color's red value, or a named scalar. To use the output of the transfer function
    as the hardware mask input, apps should call setTransferFunctionDestination() and
    setHardwareMaskInputSource() with the appropriate parameters.

    If the hardware mask input is set to HM_SOURCE_SCALAR, the input is a ChannelData
    specified with a call to setHardwareMaskInput().

    The input is compared against a floating point reference value specified with a call
    to setHardwareMaskReference().

    The comparison operation is set with setHardwareMaskOperator(). To disable the hardware
    mask, specify HM_OP_OFF. Rendering proceeds if the comparison operation evaluates to
    true. For example, if the operator is set to HM_OP_LT and the reference value is 1.0,
    then rendering proceeds if the input is less than 1.0, and vertices/fragments are
    discarded otherwise. Bitwise-OR the value HM_OP_NOT to degate the operation. For example,
    "HM_OP_EQ | HM_OP_NOT" is analougous to the "!=" operator.
    */
    /**@{*/

    typedef enum {
        HM_SOURCE_ALPHA,
        HM_SOURCE_RED,
        HM_SOURCE_SCALAR
    } HardwareMaskInputSource;
    /** \brief
    \details Default is HM_SOURCE_ALPHA */
    void setHardwareMaskInputSource( const HardwareMaskInputSource source );
    const HardwareMaskInputSource getHardwareMaskInputSource() const;

    /** \brief
    \details */
    void setHardwareMaskInput( const std::string& name );
    const std::string& getHardwareMaskInput() const;

    /** \brief
    \details Default is 0.0f. */
    void setHardwareMaskReference( const float reference );
    float getHardwareMaskReference() const;

    typedef enum {
        HM_OP_OFF = 0,
        HM_OP_EQ = ( 0x1 << 0 ),
        HM_OP_LT = ( 0x1 << 1 ),
        HM_OP_GT = ( 0x1 << 2 ),
        HM_OP_NOT = ( 0x1 << 3 )
    } HardwareMaskOperator;
    /** \brief TBD
    \details Default is HM_OFF. */
    void setHardwareMaskOperator( const unsigned int& maskOp );
    unsigned int getHardwareMaskOperator() const;
    /**@}*/

protected:
    /** \brief TBD
    \details TBD */
    void addHardwareFeatureUniforms( osg::StateSet* stateSet );

    unsigned int _baseUnit;
    unsigned int _unitAssignmentCounter;
    typedef std::map< std::string, unsigned int > UnitAssignmentMap;
    UnitAssignmentMap _unitAssignmentMap;

    osg::ref_ptr< osg::Image > _tfImage;
    std::string _tfInputName;
    TransferFunctionDestination _tfDest;

    HardwareMaskInputSource _hmSource;
    std::string _hmInputName;
    float _hmReference;
    unsigned int _hmOperator;
};

typedef boost::shared_ptr< Renderer > RendererPtr;
typedef std::list< RendererPtr > RendererList;


// lfx
}


// __LATTICEFX_RENDERER_H__
#endif
