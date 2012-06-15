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
#include "LoadRequest.h"
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

    virtual lfx::RootCallback* create() { return( new lfxdev::RootCallback() ); }

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
    lfxdev::LoadRequestPtr createLoadRequest( osg::Node* child, const osg::NodePath& childPath );
    /** \brief TBD
    \details TBD */
    void enableImages( osg::Node* child, lfxdev::LoadRequestPtr request );
    /** \brief TBD
    \details TBD */
    void reclaimImages( osg::Node* child );

    /** \brief TBD
    \details TBD */
    double computePixelSize( const osg::BoundingSphere& bSphere, const osg::Matrix& mv,
        const osg::Matrix& proj, const osg::Viewport* vp );
};


/**@}*/


// lfx
}


// __LATTICEFX_DEV_DATA_SET_H__
#endif
