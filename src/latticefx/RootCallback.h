
#ifndef __LATTICEFX_ROOT_CALLBACK_H__
#define __LATTICEFX_ROOT_CALLBACK_H__ 1


#include <latticefx/Export.h>
#include <osg/NodeCallback>
#include <osg/Group>
#include <osg/Camera>
#include <latticefx/PageData.h>

#include <vector>


namespace lfx {


/** \addtogroup PagingSupport */
/**@{*/


/** \class RootCallback RootCallback.h <latticefx/RootCallback.h>
\brief Update callback for LatticeFX subgraphs
\details The main purpose of RootCallback is to avoid the expense of an
update traversal on the LatticeFX subgraph. Any subgraph elements that
need updating register themselves with RootCallback, and RootCallback is
attached to the root node of the LatticeFX scene graph. This means the
osgUtil::UpdateVisitor only traverses as far as the root node, where all
update work for the root's subgraph is performed.

RootCallback exposes its functionality as generic public member methods,
so apps may call those public methods directly, or use RootCallback as an
OSG update callback. Currently (April 2012), classes derived from Renderer
must attach RootCallback to the root node of the subgraph they create.

Currently, RootCallback calls updatePaging(), the public method that
interacts with PagingThread to dynamically load and unload subgraph
elements. In the future, RootCallback might have other functionality,
such as updating a uniform that contains the screen space projection of
volumetric data.

See RootCallback.cpp for the definition of RootCallback::updatePaging(),
which describes paging in detail.
*/
class LATTICEFX_EXPORT RootCallback : public osg::NodeCallback
{
public:
    RootCallback();

    /** \brief Register a scene graph node as the parent of pageable children.
    \details This function adds \c parent to \c _pageParentList. During update,
    RootCallback::operator()(osg::Node*,osg::NodeVisitor*) iterates over
    \c _pageParentList, modifying the children of each added osg::Group if
    necessary based on the PageData stored in \c parent's UserData.
    
    Requirement: \c parent must have a PageData stored in its UserData. */
    void addPageParent( osg::Group* parent );

    /** \brief Specify a Camera for use in LOD computations.
    \details The Camera is used to transform pageable child bounding volumes
    into screen space to determine their pixel size. */
    void setCamera( osg::Camera* camera );

    /** \brief Dynamically load and unload data using the paging thread.
    \details See RootCallback.cpp for the definition of RootCallback::updatePaging(),
    which describes paging in detail. */
    void updatePaging( const osg::Matrix& modelView );

    virtual void operator()( osg::Node* node, osg::NodeVisitor* nv );

protected:
    virtual ~RootCallback();

    /** \brief Return the pixel size of \c bSphere.
    \details Computes the pixel radius of the bounding sphere, then returns
    the area of a circle with that radius. */
    double computePixelSize( const osg::BoundingSphere& bSphere, const osg::Matrix& modelView );

    /** \brief Return true if \c testValue falls within the given \c range.
    \details If the paging RangeMode is PIXEL_SIZE_RANGE, \c testValue is obtained from
    computePixelSize() and \c range comes from the child-specific PageData::RangeData.
    If the paging RangeMode is TIME_RANGE, \c testValue comes from the minimum value
    in the child-specific PageData::RangeData, and \c range comes from (TBD, based on
    current play time and buffer around that time). */
    static inline bool inRange( const double testValue, const PageData::RangeValues& range );

    osg::ref_ptr< osg::Camera > _camera;

    typedef std::vector< osg::ref_ptr< osg::Group > > GroupList;
    GroupList _pageParentList;
};


/**@}*/


bool RootCallback::inRange( const double testValue, const PageData::RangeValues& range )
{
    return( ( testValue >= range.first ) &&
            ( testValue < range.second ) );
}


// lfx
}


// __LATTICEFX_DATA_SET_H__
#endif
