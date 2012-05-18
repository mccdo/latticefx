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
\details RootCallback updates a scene graph for correct display in the
presence of paging activity, time series animation, and configuration of
uniforms used by subordinate program objects.

RootCallback is an OSG update callback to avoid the expense of an
update traversal on the LatticeFX subgraph. Any subgraph elements that
need updating register themselves with RootCallback. In this way, only
the node owning RootCallback need be visited by the osgUtil::UpdateVisitor.

RootCallback exposes its functionality as generic public member methods,
so apps may call those public methods directly, or use RootCallback as an
OSG update callback. Currently (May 2012), DataSet attaches an instance of
RootCallback as an update callback to the root of the DataSet's scene graph.

RootCallback calls updatePaging(), the public method that
interacts with PagingThread to dynamically load and unload subgraph
elements. See RootCallback.cpp for the definition of RootCallback::updatePaging(),
which describes paging in detail.

RootCallback also calls updateTimeSeries to select the best child node
for the current animation time, and set child node masks accordingly.

In the future, RootCallback might have other functionality,
such as updating a uniform that contains the screen space projection of
volumetric data.
*/
class LATTICEFX_EXPORT RootCallback : public osg::NodeCallback
{
public:
    RootCallback();
    RootCallback( const RootCallback& rhs );

    /** \brief Register a scene graph node as the parent of pageable children.
    \details This function adds \c parent to \c _pageParentList. During update,
    RootCallback::operator()(osg::Node*,osg::NodeVisitor*) iterates over
    \c _pageParentList, modifying the children of each added osg::Group if
    necessary based on the PageData stored in \c parent's UserData.

    TBD If the parent is time series, implicitly call addTimeSeriesParent.
    For now, app must manually call addTimeSeriesParent regardless of
    whether the parent is paged or not.
    
    Requirement: \c parent must have a PageData stored in its UserData. */
    void addPageParent( osg::Group* parent );

    /** \brief Register a scene graph node as the parent of time series data.
    \details TBD.
    */
    void addTimeSeriesParent( osg::Group* parent );

    /** \brief Specify a Camera for use in LOD computations.
    \details The Camera is used to transform pageable child bounding volumes
    into screen space to determine their pixel size. */
    void setCamera( osg::Camera* camera );

    /** \brief
    \details
    */
    void setAnimationTime( const double time );
    double getAnimationTime() const;

    /** \brief Dynamically load and unload data using the paging thread.
    \details See RootCallback.cpp for the definition of RootCallback::updatePaging(),
    which describes paging in detail. */
    void updatePaging( const osg::Matrix& modelView );

    /** \brief Select the appropriate child for the current animation time.
    \details TBD
    */
    void updateTimeSeries();

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

    double _animationTime;

    typedef std::vector< osg::ref_ptr< osg::Group > > GroupList;
    GroupList _pageParentList;
    GroupList _timeSeriesParentList;
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
