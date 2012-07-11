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


#include <latticefx/core/Export.h>
#include <latticefx/core/LoadRequest.h>
#include <latticefx/core/LogBase.h>

#include <osg/NodeCallback>
#include <osg/Group>
#include <osg/Camera>
#include <latticefx/core/PageData.h>

#include <vector>


namespace lfx {


/** \addtogroup PagingSupport */
/**@{*/


/** \class PagingCallback PagingCallback.h <latticefx/core/PagingCallback.h>
\brief Update callback for LatticeFX subgraphs
\details PagingCallback updates a scene graph for correct display in the
presence of paging activity, time series animation, and configuration of
uniforms used by subordinate program objects.

PagingCallback is an OSG update callback to avoid the expense of an
update traversal on the LatticeFX subgraph. Any subgraph elements that
need updating register themselves with PagingCallback. In this way, only
the node owning PagingCallback need be visited by the osgUtil::UpdateVisitor.

PagingCallback exposes its functionality as generic public member methods,
so apps may call those public methods directly, or use PagingCallback as an
OSG update callback. Currently (May 2012), DataSet attaches an instance of
PagingCallback as an update callback to the root of the DataSet's scene graph.

PagingCallback calls updatePaging(), the public method that
interacts with PagingThread to dynamically load and unload subgraph
elements. See PagingCallback.cpp for the definition of PagingCallback::updatePaging(),
which describes paging in detail.

PagingCallback also calls updateTimeSeries to select the best child node
for the current animation time, and set child node masks accordingly.

In the future, PagingCallback might have other functionality,
such as updating a uniform that contains the screen space projection of
volumetric data.
*/
class LATTICEFX_EXPORT PagingCallback : public osg::NodeCallback, protected LogBase
{
public:
    PagingCallback();
    PagingCallback( const PagingCallback& rhs );

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
    virtual ~PagingCallback();

    /** \brief TBD
    \details TBD */
    lfx::LoadRequestPtr createLoadRequest( osg::Node* child, const osg::NodePath& childPath );
    /** \brief TBD
    \details TBD */
    void enableImages( osg::Node* child, lfx::LoadRequestPtr request );
    /** \brief TBD
    \details TBD */
    void reclaimImages( osg::Node* child );

    /** \brief Return \c time, biased into the given time range.
    \details Returns the modulo of \c time and ( \c maxTime / \c minTime ). */
    static double getWrappedTime( const double& time, const double& minTime, const double& maxTime );

    double _animationTime;
    RangeValues _timeRange;
};


/**@}*/


// lfx
}


// __LATTICEFX_DATA_SET_H__
#endif
