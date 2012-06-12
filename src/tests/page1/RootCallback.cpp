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

#include "RootCallback.h"
#include "PagingThread.h"
#include "LoadRequest.h"
#include <latticefx/PageData.h>
#include <osg/NodeVisitor>
#include <boost/foreach.hpp>

#include <iostream>


namespace lfxdev {


class UpdatePagingVisitor : public osg::NodeVisitor
{
public:
    UpdatePagingVisitor( NodeList nodes, lfxdev::RootCallback* rootcb )
        : osg::NodeVisitor( osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN ),
          _rootcb( rootcb ),
          _pageThread( lfxdev::PagingThread::instance() )
    {
        BOOST_FOREACH( osg::Node* node, nodes )
        {
            node->accept( *this );
        }
    }
    ~UpdatePagingVisitor()
    {
    }
    
    void apply( osg::Group& group )
    {
        lfx::PageData* pageData = static_cast< lfx::PageData* >( group.getUserData() );
        if( ( pageData == NULL ) ||
            ( pageData->getRangeMode() != lfx::PageData::PIXEL_SIZE_RANGE ) )
        {
            traverse( group );
            return;
        }

        const osg::Matrix matrix( osg::computeLocalToWorld( getNodePath(), false ) );
        _rootcb->processPageableGroup( group, pageData, matrix );
        // TBD Need to know which subgraphs to traverse,
        // and how to set the node masks.
        traverse( group );
    }

protected:
    osg::ref_ptr< lfxdev::RootCallback > _rootcb;
    lfxdev::PagingThread* _pageThread;
};




RootCallback::RootCallback()
  : lfx::RootCallback()
{
}
RootCallback::RootCallback( const RootCallback& rhs )
  : lfx::RootCallback( rhs )
{
}
RootCallback::~RootCallback()
{
}


void RootCallback::findValidChildrenForTime( NodeList& results, osg::Group* parent )
{
    if( parent->getNumChildren() == 1 )
        results.push_back( parent->getChild( 0 ) );
}

void RootCallback::updateTime( osg::Group* grp )
{
    osg::Node* bestChild( NULL );
    double minTimeDifference( FLT_MAX );

    {
        if( grp->getUserData() == NULL )
        {
            OSG_WARN << "RootCallback::updateTimeSeries: time series parent has NULL UserData. Should be lfx::PageData." << std::endl;
            return;
        }
        lfx::PageData* pageData( static_cast< lfx::PageData* >( grp->getUserData() ) );
        //std::cout << "  lfx::PageData." << std::endl;
        if( pageData->getRangeMode() != lfx::PageData::TIME_RANGE )
        {
            OSG_WARN << "RootCallback::updateTimeSeries: RangeMode is not TIME_RANGE." << std::endl;
            return;
        }

        BOOST_FOREACH( lfx::PageData::RangeDataMap::value_type& rangeDataPair, pageData->getRangeDataMap() )
        {
            const unsigned int childIndex( rangeDataPair.first );
            lfx::PageData::RangeData& rangeData( rangeDataPair.second );
            if( rangeData._status != lfx::PageData::RangeData::ACTIVE )
                // Currently trying to find the best child for the current time, so if
                // the status isn't ACTIVE, we don't care about it.
                continue;

            double timeDifference( osg::absolute( rangeData._rangeValues.first - _animationTime ) );
            if( timeDifference < minTimeDifference )
            {
                minTimeDifference = timeDifference;
                bestChild = pageData->getParent()->getChild( childIndex );
            }
        }
    }
    if( bestChild == NULL )
    {
        // On first frame, might not have anything paged in yet. This is *not* an error.
        //OSG_WARN << "RootCallback::updateTimeSeries(): No best child available." << std::endl;
        //OSG_WARN << "\tCheck to make sure there is at least one ACTIVE child." << std::endl;
        //return;
    }
    BOOST_FOREACH( GroupList::value_type grp, _timeSeriesParentList )
    {
        unsigned int childIndex;
        for( childIndex=0; childIndex < grp->getNumChildren(); ++childIndex )
        {
            osg::Node* child( grp->getChild( childIndex ) );
            child->setNodeMask( ( child == bestChild ) ? 0xffffffff : 0x0 );
        }
    }
}

void RootCallback::processPageableGroup( osg::Group& group, lfx::PageData* pageData, const osg::Matrix& xform )
{
    lfxdev::PagingThread* pageThread( lfxdev::PagingThread::instance() );

    const osg::Matrix modelView( xform * _modelView );

    // If the owning parent Group has nothing but paged children, it must use Node::setInitialBound()
    // to give it some spatial location and size. Retrieve that bound.
    const osg::BoundingSphere& bSphere( group.getBound() );
    double pixelSize( computePixelSize( bSphere, modelView ) );

    // Valid range is only the pixelSize. We'll see if it's inside the childRange,
    // which is a min and max pixelSize to display the child.
    lfx::RangeValues validRange = lfx::RangeValues( pixelSize, pixelSize );

    std::cout << "Custom RootCallback: processPageableGroup." << std::endl;
}

void RootCallback::operator()( osg::Node* node, osg::NodeVisitor* nv )
{
    NodeList rootGroupsToTraverse;
    if( node->getUserData() != NULL )
    {
        lfx::PageData* pageData( static_cast< lfx::PageData* >( node->getUserData() ) );
        if( pageData->getRangeMode() == lfx::PageData::TIME_RANGE )
            findValidChildrenForTime( rootGroupsToTraverse, node->asGroup() );
        else
            rootGroupsToTraverse.push_back( node );
    }
    else
        rootGroupsToTraverse.push_back( node );

    if( getCamera() == NULL )
    {
        // Paging based on LOD calls computePixelSize(), which
        // requires non-NULL _camera.
        OSG_WARN << "RootCallback::updatePaging(): NULL _camera." << std::endl;
        return;
    }
    else
    {
        // Traverse scene graph(s) to update pageable data.
        _modelView = osg::computeLocalToWorld( nv->getNodePath(), false );
        UpdatePagingVisitor updatePaging( rootGroupsToTraverse, this );
    }

    // Find the appropriate child for the current time.
    updateTime( node->asGroup() );


    // TBD Possible future update uniforms containing projection of volume vis into screen space.

    traverse( node, nv );
}


// lfx
}
