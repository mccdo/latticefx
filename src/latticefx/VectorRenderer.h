
#ifndef __LATTICEFX_VECTOR_RENDERER_H__
#define __LATTICEFX_VECTOR_RENDERER_H__ 1


#include <latticefx/Export.h>
#include <latticefx/Renderer.h>

#include <boost/smart_ptr/shared_ptr.hpp>
#include <string>
#include <map>


namespace lfx {


/** \class VectorRenderer VectorRenderer.h <latticefx/VectorRenderer.h>
\brief TBD
\details TBD
*/
class LATTICEFX_EXPORT VectorRenderer : public lfx::Renderer
{
public:
    VectorRenderer();
    VectorRenderer( const VectorRenderer& rhs );
    virtual ~VectorRenderer();

    /** \brief Override base class lfx::Renderer
    \details Invoked by DataSet to obtain a scene graph for rendering points. */
    virtual osg::Node* getSceneGraph( const lfx::ChannelDataPtr maskIn );

    /** \brief
    \details */
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
