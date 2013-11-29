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

#ifndef __LFX_CORE_RENDERER_H__
#define __LFX_CORE_RENDERER_H__ 1


#include <latticefx/core/Export.h>
#include <latticefx/core/OperationBase.h>
#include <latticefx/core/LogBase.h>

#include <osg/Shader>
#include <osg/Uniform>
#include <osg/Image>
#include <osg/ref_ptr>

#include <boost/smart_ptr/shared_ptr.hpp>

#include <list>
#include <map>
#include <string>
#include <ostream>


namespace osg
{
class Node;
class StateSet;
}

namespace lfx
{
namespace core
{


/** \class Renderer Renderer.h <latticefx/core/Renderer.h>
\brief Base class for Rendering Framework operations.
\details Applications should create an instance of a class that derives from Renderer,
such as VectorRenderer or VolumeRenderer, and attach it to a DataSet using
DataSet::setRenderer(). Exactly one Renderer may be attached to a DataSet.

Applications should attach any necessary input ChannelData to the Renderer instance.
No ChannelData are required by the Renderer base class, but derived classes might require
specific ChannelData inputs. For example, when rendering simple points, VectorRenderer
requires position data:

\code
    VectorRendererPtr renderOp( new VectorRenderer() );
    renderOp->setPointStyle( VectorRenderer::SIMPLE_POINTS );
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

However, the implementation might be more complex, depending on the rendering algorithm.
*/
class LATTICEFX_EXPORT Renderer : public OperationBase, protected LogBase
{
public:
    Renderer( const std::string logNameSuffix = std::string( "unknown" ),
            const std::string& logName = std::string( "" ) );
    Renderer( const Renderer& rhs );
    virtual ~Renderer();

	virtual std::string getClassName() const { return std::string( "Renderer" ); }

    /** \brief Create a scene graph from the base class ChannelData.
    \details Override this function in derived classes and add your own code to
    create a scene graph. Input ChannelData is stored in OperationBase::_inputs.

    DataSet calls this function once for each time step. The \c maskIn will likely
    be different for every time step. */
    virtual osg::Node* getSceneGraph( const ChannelDataPtr maskIn ) = 0;

    /** \brief Create a single state set for all scene graphs.
    \details Create a single state set that applies to all scene graphs created by
    the getSceneGraph() method. One example of state that probably applies to all
    scene graphs would be the osg::Program object.

    Any state that varies over time steps should be in the time step scene graph's
    StateSet. One example is a uniform for the number of data elements in the
    ChannelData, which could vary depending on how many elements passed the host
    mask at a given time step. */
    virtual osg::StateSet* getRootState()
    {
        return( NULL );
    }



    /** \name Uniform Information
    \details These functions allow access to Renderer-specific uniform variable information.

    Renderer, and Renderer-derived classes, typically use shader-based rendering controlled by
    host-side uniforms. Some of these uniforms are internal to the Renderer, but others are public
    and the application can override their setting at an ancestor Node's StateSet in the scene
    graph. In order to override such a uniform, the application needs to know the uniform name and
    data type. The application might also decide to display available uniforms to the user, along
    with a description of each uniform, and obtain new values from user input.

    To facilitate this usage, each Renderer-derived class should register all available uniforms
    by creating a UniformInfo struct and passing it to the registerUniform() method. The UniformInfo
    constructor takes a name string, osg::Uniform::Type,
    description string, and UniformInfo::AccessType, then creates an instance of a Uniform (using
    the name string and osg::Uniform::Type) and stores it in Renderer::UniformInfo::_prototype. The
    Renderer should then set an initial value for the prototype Uniform using osg::Uniform::set().

    The application can obtain a list of all registered UniformInfo structs (using getUniforms())
    or a single named UniformInfo struct (using getUniform(const std::string&)). Both the
    application and the Renderer-derived class can create an actual instance of a registered
    uniform using createUniform(const UniformInfo& ).

    In general, the application should ignore any registered uniforms with UniformInfo::AccessType
    set to PRIVATE. These uniforms are internal to the Renderer; some might be controllable by
    public Renderer class methods, but others might be purely internal.

    When a Renderer registers a UniformInfo with UniformInfo::AccessType PRIVATE, and eventually
    creates and adds a corresponding osg::Uniform to a StateSet in the scene graph, the Renderer
    should set the StateAttribute::PROTECTED bit to ensure the application doesn't override the
    uniform. */
    /**@{*/

    /** \struct UniformInfo Renderer.h <latticefx/core/Renderer.h>
    \brief Record of information regarding a registered osg::Uniform.
    \details Contains a description string and AccessType, along with a prototype instance of an
    osg::Uniform with a name string, osg::Uniform::Type data type, and corresponding value.
    */
    struct UniformInfo
    {
        /** \brief Access information
        \details Denotes whether the described Uniform is internal to the Renderer, or available
        for the application to override.
        \li PRIVATE Internal to the Renderer, and (probably) added to the scene graph with the
        PROTECTED osg::StateAttribute mode bit to prevent application overriding.
        \li PUBLIC Intended for applications to override via setting the Uniform in an ancestor
        Node's StateSet with the osg::StateAttribute OVERRIDE mode bit.
        */
        typedef enum
        {
            PRIVATE,
            PUBLIC
        } AccessType;

        /** \brief Constructor
        \details Creates an instance of an osg::Uniform with \c name and \c type and stored it in _prototype. */
		UniformInfo();
        UniformInfo( const std::string& name, const osg::Uniform::Type& type, const std::string& description = std::string( "" ), const AccessType access = PUBLIC, const int numElements = 1 );
        UniformInfo( const UniformInfo& rhs );
        ~UniformInfo();

        std::string _description;
        AccessType _access;

        /** \brief Pointer to an prototype instance of a Uniform.
        \details The uniform's  name and type are taken from
        Renderer::UniformInfo(const std::string&, const osg::Uniform::Type&, const std::string&, const AccessType).
        When a Renderer creates a UniformInfo and registers it, the Renderer should set the prototype
        Uniform's value using osg::Uniform::set().

        This prototype is used as a parameter to the osg::Uniform() copy constructor when createUniform()
        is called. */
        osg::ref_ptr< osg::Uniform > _prototype;
    };
    typedef std::vector< UniformInfo > UniformInfoVector;

    /** \brief Get all registered UniformInfo structs.
    \details Returns a const vector of all registered UniformInfo structs. */
    const UniformInfoVector& getUniforms() const;
    /** \brief Get a registered UniformInfo by name.
    \details Returns the registered UniformInfo struct with name \c name. If there are
    no registered UniformInfo structs named \c name, a warning is displayed and the
    first UniformInfo struct is returned. */
    const UniformInfo& getUniform( const std::string& name ) const;

    /** \brief Create a new instance of a Uniform based on the specified \c info.
    \details This static method simply invokes the Uniform copy constructor on the \c info's
    UniformInfo::_prototype. */
    static osg::Uniform* createUniform( const UniformInfo& info );

    /** \brief Dump uniform information to a std::ostream.
    \details Dumps all uniforms in the Renderer's \c  _uniformInfo vector.
    By default, dumps PUBLIC uniforms only. To dump both PUBLIC and
    PROTECTED uniforms, set the \c publicOnly parameter to false. */
    void dumpUniformInfo( std::ostream& ostr, const bool publicOnly = true );

    /** \brief Convert the Uniform data type to a string.
    \details This static method could be useful when displaying uniform data, such as in
    a GUI or to the console. */
    static std::string uniformTypeAsString( const osg::Uniform::Type type );

    /** \brief Dynamically modify the displayed range of the hardware mask.
    \details This static function adds hardware mask Uniforms to the given
    \c ss StateSet. In a typical use case, \c ss is the StateSet of the
    parent Node of the LatticeFX subgraph.

    The Uniforms configure the hardware mask such that only data whose
    hardware mask input scalar falls into the specified \c minVal and \c maxVal
    range will be displayed. This function reuses existing OSG Uniform
    objects in \c cc if they already exist, otherwise it creates and adds
    new Uniform objects.

    Preconditions: Prior to executing the LatticeFX pipeline
    (DataSet::getSceneData() or DataSet::updateAll() ), the application
    should configure the Renderer's hardware mask with an appropriate
    input scalar:
    \code
Renderer::setHardwareMaskInputSource( Renderer::HM_SOURCE_SCALAR );
Renderer::setHardwareMaskInput( "scalar-string-name" );
Renderer::addInput( "scalar-string-name" );
    \endcode

    This function can be called during the OSG update traversal to
    dynamically modify the hardware mask. */
	static void setHardwareMaskRange( osg::StateSet* ss, float minVal, float maxVal );

    /**@}*/

	void updateUniforms( osg::StateSet *ss );


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
    or higher. The default for \c baseUnit is 4. This function implicitly calls
    resetTextureUnitAssignments(). */
    void setTextureBaseUnit( const int baseUnit );
    /** \brief Get the base texture unit used by the Renderer.
    \details Returns the base texture unit. */
    int getTextureBaseUnit() const;

    /** \brief Assign, or return the previously assigned texture unit.
    \details Primarily for use by derived classes that need to ensure the same
    texture unit is used for both the setTextureAttribute() call and the shader
    uniform value. */
    int getOrAssignTextureUnit( const std::string& key );
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
    execute in either a vertex or fragment shader depending on the implementation. */
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
    setTransferFunctionInputRange(). Indices outside the specified range are clamped.

    This function is ignored for the VolumeRenderer. The VolumeRenderer transfer function
    input is always taken from the volume data itself. */
    void setTransferFunctionInput( const std::string& name );
    /** \brief Get the name of the transfer function input ChannelData. */
    const std::string& getTransferFunctionInput() const;

    /** \brief Specify the valid range of transfer function input values.
    \details Transfer function is a texture lookup. Values outside the
    specified range will clamp. The default range is (0., 1.). */
    void setTransferFunctionInputRange( const osg::Vec2f& range );
    /** \brief TBD
    \details TBD */
    const osg::Vec2f& getTransferFunctionInputRange() const;

    typedef enum
    {
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

	std::string getEnumName( TransferFunctionDestination e ) const;
	TransferFunctionDestination getEnumFromNameTrans( const std::string &name ) const;
	void getEnumListTrans( std::vector< std::string > *enumList ) const;

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
    to setHardwareMaskReference(). The values are considered equal if they are within the
    epsilon value specified with setHardwareMaskEpsilon().

    The comparison operation is set with setHardwareMaskOperator(). To disable the hardware
    mask, specify HM_OP_OFF. Rendering proceeds if the comparison operation evaluates to
    true. For example, if the operator is set to HM_OP_LT and the reference value is 1.0,
    then rendering proceeds if the input is less than 1.0, and vertices/fragments are
    discarded otherwise. Bitwise-OR the value HM_OP_NOT to degate the operation. For example,
    "HM_OP_EQ | HM_OP_NOT" is analougous to the "!=" operator.
    */
    /**@{*/

    typedef enum
    {
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
    const HardwareMaskInputSource& getHardwareMaskInputSource() const;

	std::string getEnumName( HardwareMaskInputSource e ) const;
	HardwareMaskInputSource getEnumFromNameMaskInput( const std::string &name ) const;
	void getEnumListMaskInput( std::vector< std::string > *enumList ) const;

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

    /** \brief Set the hardware mask comparison epsilon value.
    \details When performing the hardware mask comparison against the reference value,
    the input source value is considered to be equal to the reference value if the
    source value falls within the range +/- this epsilon value.

    The default in 0.0f. */
    void setHardwareMaskEpsilon( const float epsilon );
    /** \brief Get the hardware mask comparison epsilon value. */
    float getHardwareMaskEpsilon() const;

    typedef enum
    {
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

	std::string getEnumName( HardwareMaskOperator e ) const;
	HardwareMaskOperator getEnumFromNameHardwareMaskOperator( const std::string &name ) const;
	void getEnumListHardwareMaskOperator( std::vector< std::string > *enumList ) const;

	virtual void serialize( JsonSerializer *json ) const { OperationBase::serialize( json ); }
	static void serialize( JsonSerializer *json, const std::string &name, const osg::Vec2f &v );
	static void serialize( JsonSerializer *json, const std::string &name, const osg::Vec3f &v );
	static void serialize( JsonSerializer *json, const std::string &name, const osg::Vec4f &v );
	static bool load( JsonSerializer *json, const std::string &name, osg::Vec2f &v );
	static bool load( JsonSerializer *json, const std::string &name, osg::Vec3f &v );
	static bool load( JsonSerializer *json, const std::string &name, osg::Vec4f &v );
	 
	virtual void dumpState( std::ostream &os );

	enum PropType
	{
		PT_ENUM_BEGIN,
			PT_TF_DST,
			PT_HM_SRC,
			PT_HM_OPE,
		PT_ENUM_END,

		PT_FLOAT_BEGIN,
			PT_HM_REF,
			PT_HM_EPS,
			PT_RTP_BEGIN,
				PT_RTP_PTMASK,
				PT_RTP_ROIBOX_X_MIN,
				PT_RTP_ROIBOX_X_MAX,
				PT_RTP_ROIBOX_Y_MIN,
				PT_RTP_ROIBOX_Y_MAX,
				PT_RTP_ROIBOX_Z_MIN,
				PT_RTP_ROIBOX_Z_MAX,
			PT_RTP_END,
		PT_FLOAT_END,

		PT_FLOATRNG_BEGIN,
			PT_TF_INR,
		PT_FLOATRNG_END
	};


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

    /** \brief Register a uniform. */
    void registerUniform( const UniformInfo& info );

	/** \brief Get a non-const UniformInfo with the given \c name.
    \details This method is useful for setting the value of the UniformInfo::_prototype. */
    UniformInfo& getUniform( const std::string& name );

	virtual void serializeData( JsonSerializer *json ) const;
	virtual bool loadData( JsonSerializer *json, IObjFactory *pfactory, std::string *perr=NULL );

    UniformInfoVector _uniformInfo;

    int _baseUnit;
    unsigned int _unitAssignmentCounter;
    typedef std::map< std::string, unsigned int > UnitAssignmentMap;
    UnitAssignmentMap _unitAssignmentMap;

    osg::ref_ptr< osg::Image > _tfImage;
    std::string _tfInputName;
    osg::Vec2f _tfRange;
    TransferFunctionDestination _tfDest;
    osg::Vec4f _tfDestMask;

    HardwareMaskInputSource _hmSource;
    std::string _hmInputName;
    float _hmReference;
    float _hmEpsilon;
    unsigned int _hmOperator;

};


typedef boost::shared_ptr< Renderer > RendererPtr;
typedef std::list< RendererPtr > RendererList;


// core
}
// lfx
}

// __LFX_CORE_RENDERER_H__
#endif
