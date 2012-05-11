
#ifndef __LATTICEFX_RENDERER_H__
#define __LATTICEFX_RENDERER_H__ 1


#include <latticefx/Export.h>
#include <latticefx/OperationBase.h>

#include <boost/smart_ptr/shared_ptr.hpp>

#include <list>



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
    void setTransferFunctionDestination( TransferFunctionDestination dest );
    TransferFunctionDestination getTransferFunctionDestination() const;

    /**@}*/


    /** \name Hardware Mask Parameters
    \details */
    /**@{*/
    /**@}*/

protected:
    void addTransferFunctionUniforms( osg::StateSet* stateSet, int& baseUnit );

    unsigned int _baseUnit;

    osg::ref_ptr< osg::Image > _tfImage;
    std::string _tfInputName;
    TransferFunctionDestination _tfDest;
};

typedef boost::shared_ptr< Renderer > RendererPtr;
typedef std::list< RendererPtr > RendererList;


// lfx
}


// __LATTICEFX_RENDERER_H__
#endif
