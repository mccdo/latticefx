
#ifndef __LATTICEFX_RENDERER_H__
#define __LATTICEFX_RENDERER_H__ 1


#include <latticefx/Export.h>
#include <latticefx/OperationBase.h>

#include <osg/Image>
#include <osg/ref_ptr>

#include <boost/smart_ptr/shared_ptr.hpp>

#include <list>


namespace osg {
    class Node;
    class StateSet;
}

namespace lfx {


/** \class Renderer Renderer.h <latticefx/Renderer.h>
\brief Base class for Rendering Framework operations.
\details TBD
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
    or higher. The default for \c baseUnit is 8. */
    void setTextureBaseUnit( const unsigned int baseUnit );

    /** \brief Get the base texture unit used by the Renderer.
    \details Returns the base texture unit. */
    unsigned int getTextureBaseUnit() const;

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
    void addHardwareFeatureUniforms( osg::StateSet* stateSet, int& baseUnit );

    unsigned int _baseUnit;

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
