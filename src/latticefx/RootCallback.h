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
#include <latticefx/LoadRequest.h>

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

    virtual lfx::RootCallback* create() { return( new lfx::RootCallback() ); }

    /** \brief Set the current animation time.
    \details Called by PlayContaol as the time series animation advances.
    This function does not need to be called if time series data is not being
    used. The calling code is responsible for updating the animation time as
    the animation plays forwards or backwards, and handling effects like ffwd,
    rew, or stop/pause. */
    void setAnimationTime( const double time );
    /** \brief Get the current animation time. */
    double getAnimationTime() const;

    /** \brief Set the paging time range.
    \details Children are paged in if their time value falls within the
    specified \c timeRange around the current animation time, though they are
    displayed only when their time value matches the current animation time.
    Times in \c timeRange are relative to the current animation time. For
    example: RangeValues( -.5, 1. ) pages in children whose time values are
    0.5 seconds before and 1.0 seconds after the current animation time.

    Default: RangeValues( -0.5, 0.5 ). */
    void setTimeRange( const RangeValues& timeRange );
    /** \brief Get the paging time range. */
    RangeValues getTimeRange() const;

    virtual void operator()( osg::Node* node, osg::NodeVisitor* nv );

protected:
    virtual ~RootCallback();

    /** \brief TBD
    \details TBD */
    void pageByTime( osg::Group* grp );
    /** \brief TBD
    \details TBD */
    void pageByDistance( osg::Group* grp, const osg::Matrix& modelMat, const osg::NodePath& nodePath );

    /** \brief TBD
    \details TBD */
    lfx::LoadRequestPtr createLoadRequest( osg::Node* child, const osg::NodePath& childPath );
    /** \brief TBD
    \details TBD */
    void enableImages( osg::Node* child, lfx::LoadRequestPtr request );
    /** \brief TBD
    \details TBD */
    void reclaimImages( osg::Node* child );

    /** \brief Return the pixel size of \c bSphere.
    \details Computes the pixel radius of the bounding sphere, then returns
    the area of a circle with that radius. */
    double computePixelSize( const osg::BoundingSphere& bSphere, const osg::Matrix& model,
        const osg::Vec3& wcEyePosition, const osg::Matrix& proj, const osg::Viewport* vp );

    /** \brief Return \c time, biased into the given time range.
    \details Returns the modulo of \c time and ( \c maxTime / \c minTime ). */
    static double getWrappedTime( const double& time, const double& minTime, const double& maxTime );

    /** \brief Return true if \c validRange and \c childRange overlap.
    \details If the paging RangeMode is PIXEL_SIZE_RANGE, both min and max values of
    \c validRange are set to the return value of computePixelSize() and \c childRange
    comes from the child-specific PageData::RangeData.

    If the paging RangeMode is TIME_RANGE, \c validRange is a range of time values specified
    by the application, and both min and max values of \c childRange are set to the time
    value of the child node. */
    static inline bool inRange( const RangeValues& validRange, const RangeValues& childRange );


    double _animationTime;
    RangeValues _timeRange;
};


/**@}*/


bool RootCallback::inRange( const RangeValues& validRange, const RangeValues& childRange )
{
    const bool childFirstGood( childRange.first < validRange.second );
    const bool childSecondGood( childRange.second >= validRange.first );
    if( validRange.first <= validRange.second )
        // Typical case: first (min) < second (max).
        return( childSecondGood && childFirstGood );
    else
        // First (min) might be greater than second (max) due to
        // wrapping of animation time.
        return( childSecondGood || childFirstGood );
}


// lfx
}


// __LATTICEFX_DATA_SET_H__
#endif
