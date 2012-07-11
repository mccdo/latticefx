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

#ifndef __LATTICEFX_RENDERER_H__
#define __LATTICEFX_RENDERER_H__ 1


#include <latticefx/core/Export.h>
#include <latticefx/core/OperationBase.h>
#include <latticefx/core/LogBase.h>

#include <osg/Shader>
#include <osg/Image>
#include <osg/ref_ptr>

#include <boost/smart_ptr/shared_ptr.hpp>

#include <list>
#include <map>
#include <string>


namespace osg {
    class Node;
    class StateSet;
}

namespace lfx {


/** \class Renderer Renderer.h <latticefx/core/Renderer.h>
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
\li OperationBase::getInput(const std::string&) Returns the ChannelData for the specified name string.
\li getOrAssignTextureUnit() for assigning and organizing texture unit usage.
\li addHardwareFeatureUniforms() for setting commonly used uniform variables.

Derived classes should support the transfer function and hardware mask early in the rendering pipeline.
In particular, these operations should be performed before lighting, aa the transfer function determines
the ambient and diffuse material colors. Early execution of the hardware mask can improve performance
by discarding vertices or fragments prior to expensive downstream operations.

Derived classes should support OpenGL FFP clip planes by computing gl_ClipVertex in the vertex shader.
This is usually just the eye coordinate vertex:

\code
    gl_ClipVertex = gl_ModelViewMatrix * gl_Vertex;
\endcode
*/
class LATTICEFX_EXPORT Renderer : public OperationBase, protected LogBase
{
public:
    Renderer( const std::string logNameSuffix=std::string("unknown") );
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
    \details The Renderer may use 0 or more textures in the scene graph it creates. These methods
    control the first texture unit a Renderer will use -- the texture base unit. The Renderer may
    use that, or any higher unit. The default base unit is 8. Typically, modern GPUs are limited
    to 16 total texture units.

    This API also allows names to be associated with texture units. This is useful in
    Renderer-derived classes that need to specify the texture in getSceneGraph() and the texture
    unit uniform in getRootState(). The derived class queries the unit by name and is guaranteed
    to get the same unit in both functions. */
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
    \details This API allows a ChannelData input to be used as an index to a transfer
    function texture lookup. The output of the transfer function can be directed into
    the color RGB, RGBA, or alpha only.

    The transfer function determines the color of rendered data. It's executed
    before lighting, and the results affect the ambient and diffuse material colors.
    It's also executed before the hardware mask so that rendering may be halted based
    on the transfer function result.

    The intent is that this will be implemented in a GPU shader program, but it could
    execute in either a vertex or fragment shader depending on the implementation.
    /**@{*/

    /** \brief Set the transfer function image.
    \details  The transfer function is implemented as a texture lookup. This method
    specifies the osg::Image used to create the transfer function texture.

    The image may be 1D, 2D, or 3D, and the derived Renderer must support all
    three variants. The dimension of the image determines the required dimension of the
    input ChannelData (setTransferFunctionInput()) and behavior is undefined in the event
    of a mismatch.

    Set the transfer function to NULL to disable the transfer function. In this case, lighting
    ambient and diffuse material colors are taken from the OpenGL primary color. */
    void setTransferFunction( osg::Image* image );
    /** \brief Get the transfer function image. */
    osg::Image* getTransferFunction();
    /** \overload */
    const osg::Image* getTransferFunction() const;

    /** \brief Specify the ChannelData used as an index into the transfer function.
    \details Specifies the ChannelData indirectly by its \c name. The ChannelData dimensions
    must match the dimensions of the image (setTransferFunction()) or behavior is undefined.

    Because the transfer function is a texture lookup, LatticeFX supports mapping a range
    of transfer function input values into the (texture coordinate) range 0.0 to 1.0. See
    setTransferFunctionInputRange(). Indices outside the specified range are clamped. */
    void setTransferFunctionInput( const std::string& name );
    /** \brief Get the name of the transfer function input ChannelData. */
    const std::string& getTransferFunctionInput() const;

    /** \brief Specify the valid range of transfer function input values.
    \details Transfer function is a texture lookup. Values outside the
    specified range will clamp. The default range is (0., 1.). */
    void setTransferFunctionInputRange( const osg::Vec2& range );
    /** \brief TBD
    \details TBD */
    const osg::Vec2& getTransferFunctionInputRange() const;

    typedef enum {
        TF_RGB = 0,
        TF_RGBA = 1,
        TF_ALPHA = 2
    } TransferFunctionDestination;
    /** \brief Specify the transfer function destination.
    \details The transfer function image (setTransferFunction()) is indexed by the input
    (setTransferFunctionInput()) producing a GLSL vec4 result. This function determines how that
    result is directed.
    \li TF_RGB The result determines the RGB values of the ambient and diffuse material colors.
    The alpha values are taken from the OpenGL primary color.
    \li TF_RGBA The result determines the RGBA values of the ambient and diffuse material colors.
    \li TF_ALPHA The result alpha value determines the alpha values of the ambient and diffuse
    material colors. The RGB values are taken from the OpenGL primary color.

    The default is TF_ALPHA. */
    void setTransferFunctionDestination( const TransferFunctionDestination dest );
    /** \brief Get the transfer function destination. */
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
    /** \brief Specify the input source for the hardware mask.
    \details If the input is HM_SOURCE_ALPHA or HM_SOURCE_RED, those input values
    come from the output of the transfer function. If the input is HM_SOURCE_SCALAR,
    the input value comes from the ChannelData specified with setHardwareMaskInput().

    The default is HM_SOURCE_ALPHA */
    void setHardwareMaskInputSource( const HardwareMaskInputSource source );
    /** \brief Get the input source setting. */
    const HardwareMaskInputSource getHardwareMaskInputSource() const;

    /** \brief Specify the hardware mask input ChannelData.
    \details Specifies the ChannelData to use as the hardware mask source input when
    the source is set to HM_SOURCE_SCALAR (setHardwareMaskInputSource()). If the
    input source is not HM_SOURCE_SCALAR, this method is ignored. */
    void setHardwareMaskInput( const std::string& name );
    /** \brief Get the input ChannelData used when the source is HM_SOURCE_SCALAR. */
    const std::string& getHardwareMaskInput() const;

    /** \brief Set the hardware mask comparison reference value.
    \details The input source value is compared against this reference value using
    the comparison operator (setHardwareMaskOperator()). If the expression evaluates
    to true, rendering proceeds, and is halted otherwise.

    The default is 0.0f. */
    void setHardwareMaskReference( const float reference );
    /** \brief Get the hardware mask comparison reference value. */
    float getHardwareMaskReference() const;

    typedef enum {
        HM_OP_OFF = 0,
        HM_OP_EQ = ( 0x1 << 0 ),
        HM_OP_LT = ( 0x1 << 1 ),
        HM_OP_GT = ( 0x1 << 2 ),
        HM_OP_NOT = ( 0x1 << 3 )
    } HardwareMaskOperator;
    /** \brief Specifies the hardware mask comparison operator.
    \details Specifies how to compare the source input values against the reference value.
    If the comparison evaluates to true, rendering proceeds. If the comparison evaluates
    to false, rendering is halted.

    Note that HM_OP_NOT may be bitwise-ORed with HM_OP_EQ, HM_OP_LT, and HM_OP_GT to negate
    the comparison result.

    The default is HM_OFF, which disables the hardware mask. */
    void setHardwareMaskOperator( const unsigned int& maskOp );
    /** \brief Get the hardware mask comparison operatos. */
    unsigned int getHardwareMaskOperator() const;

    /**@}*/

protected:
    /** \brief Set general uniforms and textures for base Renderer functionality.
    \details This is a convenience function for use by derived classes. It sets many uniforms
    and textures used by the transfer function and hardware mask features. Derived classes call
    this function to add the uniforms and textures to the \c stateSet.

    The uniforms and textures set in this function include:
    <ul>
    <li> sampler1D tf1d (and associated osg::Texture1D) contains the 1D transfer function.
    <li> sampler2D tf2d (and associated osg::Texture2D) contains the 2D transfer function.
    <li> sampler3D tf3d (and associated osg::Texture3D) contains the 3D transfer function.
    <li> int tfDimension, set to 0 if the transfer function is disabled. Otherwise, values of 1, 2, or
    3 indicate the transfer function sampler uniform: tf1d, tf2d, or tf3d, respectively.
    <li> int tfDest, the transfer function destination, set to 0, 1, or 2 for TF_RGB,
    TF_RGBA, or TF_ALPHA destinations.
    <li> vec4 hmParams, containing all parameters for the hardware mask packed into a single
    uniform. The elements are set as follows:
      <ul>
        <li> 0: Input source (0=alpha, 1=red, 2=scalar
        <li> 1: Mask operator (0=OFF, 1=EQ, 2=LT, 3=GT).
        <li> 2: Operator negate flag (0=no negate, 1=negate).
        <li> 3: Reference value.
      </ul>
    </ul>

    Note that addHardwareFeatureUniforms() doesn't handle the transfer function input. This
    must be specified in the Renderer-derived class in order to support Renderers that
    require the input data in a specific format (such as a 3D texture or as a generic vertex
    attrib). This also applies to the hardware mask input. */
    void addHardwareFeatureUniforms( osg::StateSet* stateSet );

    /** \brief Convenience routine for loading a shader from source.
    \details Takes care of finding the source file, setting an appropriate name for the
    shader, loading the source, and displaying error messages as needed. Calling code
    merely needs to add the return value to a Program.
    \returns A valid Shader object on success. Returns NULL on failure. Note that
    osg::Program::addShader() is a no-op if the shader parameter is NULL. */
    osg::Shader* loadShader( const osg::Shader::Type type, const std::string& fileName );

    unsigned int _baseUnit;
    unsigned int _unitAssignmentCounter;
    typedef std::map< std::string, unsigned int > UnitAssignmentMap;
    UnitAssignmentMap _unitAssignmentMap;

    osg::ref_ptr< osg::Image > _tfImage;
    std::string _tfInputName;
    osg::Vec2 _tfRange;
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
