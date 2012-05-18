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

#include <latticefx/RootCallback.h>
#include <latticefx/PagingThread.h>
#include <latticefx/PageData.h>
#include <osg/Transform>
#include <boost/foreach.hpp>

#include <iostream>


namespace lfx {


RootCallback::RootCallback()
  : _animationTime( 0. )
{
}
RootCallback::RootCallback( const RootCallback& rhs )
  : osg::NodeCallback( rhs ),
    _camera( rhs._camera ),
    _animationTime( rhs._animationTime ),
    _pageParentList( rhs._pageParentList ),
    _timeSeriesParentList( rhs._timeSeriesParentList )
{
}
RootCallback::~RootCallback()
{
}

void RootCallback::addPageParent( osg::Group* parent )
{
    _pageParentList.push_back( parent );
}
void RootCallback::addTimeSeriesParent( osg::Group* parent )
{
    _timeSeriesParentList.push_back( parent );
}

void RootCallback::setCamera( osg::Camera* camera )
{
    _camera = camera;
}
void RootCallback::setAnimationTime( const double time )
{
    _animationTime = time;
}
double RootCallback::getAnimationTime() const
{
    return( _animationTime );
}

void RootCallback::updatePaging( const osg::Matrix& modelView )
{
    lfx::PagingThread* pageThread( lfx::PagingThread::instance() );

    // Add any new page requests.
    BOOST_FOREACH( GroupList::value_type grp, _pageParentList )
    {
        if( grp->getUserData() == NULL )
        {
            OSG_WARN << "RootCallback::updatePaging(): page parent has NULL UserData. Should be ;fx::PageData." << std::endl;
            continue;
        }
        lfx::PageData* pageData( static_cast< lfx::PageData* >( grp->getUserData() ) );
        //std::cout << "  PageData." << std::endl;

        double testValue( 0. );
        PageData::RangeValues range;
        if( pageData->getRangeMode() == PageData::PIXEL_SIZE_RANGE )
        {
            if( _camera == NULL )
            {
                // computePixelSize() requires non-NULL _camera.
                OSG_WARN << "RootCallback::updatePaging(): NULL _camera." << std::endl;
                return;
            }

            // If the owning parent Group has nothing but paged children, it must use Node::setInitialBound()
            // to give it some spatial location and size. Retrieve that bound.
            const osg::BoundingSphere& bSphere( grp->getBound() );
            testValue = computePixelSize( bSphere, modelView );
            //std::cout << "Pixel size: " << testValue << std::endl;
        }
        else if( pageData->getRangeMode() == PageData::TIME_RANGE )
        {
            // TBD Range should be a buffer around current time, but
            // needs to be configurable.
            range = PageData::RangeValues( -1., 10. );
        }

        // removeExpired is initially false and only set to true if we add a child.
        // This prevents us from removing expired children before the required children
        // are available, which would render nothing for a couple frames.
        bool removeExpired( false );
        BOOST_FOREACH( PageData::RangeDataMap::value_type& rangeDataPair, pageData->getRangeDataMap() )
        {
            const unsigned int childIndex( rangeDataPair.first );
            PageData::RangeData& rangeData( rangeDataPair.second );

            if( pageData->getRangeMode() == PageData::PIXEL_SIZE_RANGE )
            {
                // Child-specific PageData::RangeData contains min and max values as a std::pair.
                range = rangeData._rangeValues;
            }
            else
            {
                // If paging based on time, only the min value of the range -- the time for
                // the child -- is relevant. We'll compare it against a range around the
                // current play time.
                testValue = rangeData._rangeValues.first;
            }

            switch( rangeData._status )
            {
            case PageData::RangeData::UNLOADED:
            {
                //std::cout << "    RangeData UNLOADED" << std::endl;
                if( inRange( testValue, range ) )
                {
                    // Child state is UNLOADED, but it's in range. Add a request to the PagingThread.
                    pageThread->addLoadRequest( pageData->getParent()->getChild( childIndex ),
                        rangeData._fileName );
                    rangeData._status = PageData::RangeData::LOAD_REQUESTED;
                }
                break;
            }
            case PageData::RangeData::LOAD_REQUESTED:
            {
                //std::cout << "    RangeData LOAD_REQUESTED" << std::endl;
                // Check to see if our request has completed. If so, add it into the scene graph
                // in place of the Group stub.
                osg::Node* loadedModel( pageThread->retrieveRequest( pageData->getParent()->getChild( childIndex ) ) );
                if( loadedModel != NULL )
                {
                    //std::cout << "      Retrieved: " << std::hex << loadedModel << std::endl;
                    // Now that we know we have something in range to render, set removeExpired to true
                    // so we can remove any expired children (avoids rendering no children).
                    removeExpired = true;
                    pageData->getParent()->setChild( childIndex, loadedModel ); // Replaces Group stub at childIndex.
                    rangeData._status = PageData::RangeData::LOADED;
                }
                break;
            }
            case PageData::RangeData::LOADED:
            {
                //std::cout << "    RangeData LOADED" << std::endl;
                // Nothing to do.
                break;
            }
            case PageData::RangeData::ACTIVE:
            {
                //std::cout << "    RangeData ACTIVE" << std::endl;
                // Nothing to do.
                break;
            }
            }
        }

        // Remove any expired children. Do *not* remove any children with status LOADED;
        // these are the children we just added! Instead, set their status to ACTIVE.
        if( removeExpired )
        {
            BOOST_FOREACH( PageData::RangeDataMap::value_type& rangeDataPair, pageData->getRangeDataMap() )
            {
                const unsigned int childIndex( rangeDataPair.first );
                PageData::RangeData& rangeData( rangeDataPair.second );

                // Get range and testValue info just as we did previously.
                if( pageData->getRangeMode() == PageData::PIXEL_SIZE_RANGE )
                {
                    range = rangeData._rangeValues;
                }
                else
                {
                    testValue = rangeData._rangeValues.first;
                }

                switch( rangeData._status )
                {
                case PageData::RangeData::LOAD_REQUESTED:
                {
                    if( !inRange( testValue, range ) )
                    {
                        // Cancel request. Note there's a possible thread safety issue
                        // with this (see PagingThread::cancelLoadRequest() for more info.
                        pageThread->cancelLoadRequest( pageData->getParent()->getChild( childIndex ) );
                        rangeData._status = PageData::RangeData::UNLOADED;
                    }
                    break;
                }
                case PageData::RangeData::ACTIVE:
                {
                    if( !inRange( testValue, range ) )
                    {
                        // Remove this expired child by placing a Group stub in its place.
                        pageData->getParent()->setChild( childIndex, new osg::Group );
                        rangeData._status = PageData::RangeData::UNLOADED;
                    }
                    break;
                }
                case PageData::RangeData::LOADED:
                {
                    // We just loaded this one. Change status to ACTIVE.
                    // If it's no longer valid, we'll remove it the *next* time we
                    // load something (unloading it now might leave a gap where
                    // nothing is rendered).
                    rangeData._status = PageData::RangeData::ACTIVE;
                }
                }
            }
        }
    }
}

void RootCallback::updateTimeSeries()
{
    osg::Node* bestChild( NULL );
    double minTimeDifference( FLT_MAX );

    // Iterate over all parents registered as time series parents,
    // and find best child for current _animationTime.
    BOOST_FOREACH( GroupList::value_type grp, _timeSeriesParentList )
    {
        if( grp->getUserData() == NULL )
        {
            OSG_WARN << "RootCallback::updateTimeSeries: time series parent has NULL UserData. Should be ;fx::PageData." << std::endl;
            return;
        }
        lfx::PageData* pageData( static_cast< lfx::PageData* >( grp->getUserData() ) );
        //std::cout << "  PageData." << std::endl;
        if( pageData->getRangeMode() != PageData::TIME_RANGE )
        {
            OSG_WARN << "RootCallback::updateTimeSeries: RangeMode is not TIME_RANGE." << std::endl;
            return;
        }

        BOOST_FOREACH( PageData::RangeDataMap::value_type& rangeDataPair, pageData->getRangeDataMap() )
        {
            const unsigned int childIndex( rangeDataPair.first );
            PageData::RangeData& rangeData( rangeDataPair.second );
            if( rangeData._status != PageData::RangeData::ACTIVE )
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
        OSG_WARN << "RootCallback::updateTimeSeries(): No best child available." << std::endl;
        OSG_WARN << "\tCheck to make sure there is at least one ACTIVE child." << std::endl;
        return;
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

void RootCallback::operator()( osg::Node* node, osg::NodeVisitor* nv )
{
    //std::cout << "Update." << std::endl;

    if( !( _pageParentList.empty() ) )
    {
        // modelView matrix required for bounding sphere pixel size computation (LOD).
        osg::Matrix modelMatrix( osg::computeLocalToWorld( nv->getNodePath() ) );
        osg::Matrix modelView( ( _camera == NULL ) ? modelMatrix :
            modelMatrix * _camera->getViewMatrix() );
        updatePaging( modelView );
    }

    if( !( _timeSeriesParentList.empty() ) )
        updateTimeSeries();

    // TBD Possible future update uniforms containing projection of volume vis into screen space.

    traverse( node, nv );
}


double RootCallback::computePixelSize( const osg::BoundingSphere& bSphere, const osg::Matrix& modelView )
{
    const osg::Vec3 ecCenter = bSphere.center() * modelView ;
    if( -( ecCenter.z() ) < bSphere.radius() )
    {
        // Inside the bounding sphere.
        return( FLT_MAX );
    }

    // Compute pixelRadius, the sphere radius in pixels.

    const osg::Viewport* vp( _camera->getViewport() );

    // Get clip coord center, then get NDX x value (div by w), then get window x value.
    osg::Vec4 ccCenter( osg::Vec4( ecCenter, 1. ) * _camera->getProjectionMatrix() );
    ccCenter.x() /= ccCenter.w();
    double cx( ( ( ccCenter.x() + 1. ) * .5 ) * vp->width() + vp->x() );

    // Repeast, but start with an eye coord point that is 'radius' units to the right of center.
    // Result is the pixel location of the rightmost edge of the bounding sphere.
    const osg::Vec4 ecRight( ecCenter.x() + bSphere.radius(), ecCenter.y(), ecCenter.z(), 1. );
    osg::Vec4 ccRight( ecRight * _camera->getProjectionMatrix() );
    ccRight.x() /= ccRight.w();
    double rx( ( ( ccRight.x() + 1. ) * .5 ) * vp->width() + vp->x() );

    // Pixel radius is the rightmost edge of the sphere minus the center.
    const double pixelRadius( rx - cx );

    // Circle area A = pi * r^2
    return( osg::PI * pixelRadius * pixelRadius );
}


// lfx
}
