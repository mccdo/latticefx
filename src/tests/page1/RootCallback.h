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

#ifndef __LATTICEFX_DEV_ROOT_CALLBACK_H__
#define __LATTICEFX_DEV_ROOT_CALLBACK_H__ 1


//#include <latticefx/Export.h>
#include <latticefx/RootCallback.h>
#include <latticefx/PageData.h>
#include <osg/NodeCallback>
#include <osg/Group>
#include <osg/Camera>
#include <latticefx/PageData.h>

#include <vector>


namespace lfxdev {


typedef std::vector< osg::Node* > NodeList;


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
class /*LATTICEFX_EXPORT*/ RootCallback : public lfx::RootCallback
{
public:
    RootCallback();
    RootCallback( const RootCallback& rhs );

    void processPageableGroup( osg::Group& group, lfx::PageData* pageData, const osg::Matrix& xform );

    virtual void operator()( osg::Node* node, osg::NodeVisitor* nv );

protected:
    virtual ~RootCallback();

    /** \brief Select the appropriate child for the current animation time.
    \details TBD
    */
    void updateTime( osg::Group* grp );

    /** \brief Return true if \c validRange and \c childRange overlap.
    \details If the paging RangeMode is PIXEL_SIZE_RANGE, both min and max values of
    \c validRange are set to the return value of computePixelSize() and \c childRange
    comes from the child-specific PageData::RangeData.

    If the paging RangeMode is TIME_RANGE, \c validRange is a range of time values specified
    by the application, and both min and max values of \c childRange are set to the time
    value of the child node. */
    static inline bool inRange( const lfx::RangeValues& validRange, const lfx::RangeValues& childRange );

    void findValidChildrenForTime( NodeList& results, osg::Group* parent );

    osg::Matrix _modelView;
};


/**@}*/


bool RootCallback::inRange( const lfx::RangeValues& validRange, const lfx::RangeValues& childRange )
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


// __LATTICEFX_DEV_DATA_SET_H__
#endif
