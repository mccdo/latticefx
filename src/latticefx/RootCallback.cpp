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
#include <latticefx/LoadRequest.h>
#include <latticefx/PageData.h>

#include <osg/NodeVisitor>
#include <osg/Texture2D>

#include <boost/foreach.hpp>
#include <iostream>


namespace lfx {


RootCallback::RootCallback()
  : osg::NodeCallback(),
    _animationTime( 0. ),
    _timeRange( RangeValues( -0.5, 0.5 ) )
{
}
RootCallback::RootCallback( const RootCallback& rhs )
  : osg::NodeCallback( rhs ),
    _animationTime( rhs._animationTime ),
    _timeRange( rhs._timeRange )
{
}
RootCallback::~RootCallback()
{
}

void RootCallback::setAnimationTime( const double time )
{
    _animationTime = time;
}
double RootCallback::getAnimationTime() const
{
    return( _animationTime );
}
void RootCallback::setTimeRange( const RangeValues& timeRange )
{
    _timeRange = timeRange;
}
RangeValues RootCallback::getTimeRange() const
{
    return( _timeRange );
}


void RootCallback::operator()( osg::Node* node, osg::NodeVisitor* nv )
{
    if( node->getUserData() == NULL )
    {
        traverse( node, nv );
        return;
    }

    lfx::PagingThread* pageThread( lfx::PagingThread::instance() );

    RangeValues validRange;
    lfx::PageData* pageData( static_cast< lfx::PageData* >( node->getUserData() ) );
    if( pageData->getRangeMode() == lfx::PageData::TIME_RANGE )
    {
        // validRange is set by the application. The assumption is that this will be large enough
        // to accomodate several children, only one of which will be displayed (by use of NodeMask).
        // This is different from pixel size paging because the time step (and therefore valid child)
        // is expected to change quite rapidly, so wee need to page in a buffer around the current
        // play time to help ensure a smooth animation free ofpaging bottlenecks.
        //
        // The current animation time could be anything, but we want a time range around it with
        // min and max values between the PageData's min and max time values. Note that when the
        // animation reaches the end, it's possible that the min validRange value could be greater than
        // the max validRange value to support smooth playback as time wraps around.
        double minTime, maxTime;
        pageData->getMinMaxTime( minTime, maxTime );

        const double time( getAnimationTime() );
        validRange.first = getWrappedTime( _timeRange.first + time, minTime, maxTime );
        validRange.second = getWrappedTime( _timeRange.second + time, minTime, maxTime );
    }
    else
    {
        osg::Vec3 wcEyePosition;
        osg::Matrix proj;
        osg::ref_ptr< const osg::Viewport> vp;
        pageThread->getTransforms( wcEyePosition, proj, vp );

        const osg::Matrix modelMat( osg::computeLocalToWorld( nv->getNodePath(), false ) );
        const osg::BoundingSphere& bSphere( node->getBound() );
        const double pixelSize( computePixelSize( bSphere, modelMat, wcEyePosition, proj, vp.get() ) );

        validRange = RangeValues( pixelSize, pixelSize );
    }

    osg::Group* grp( node->asGroup() );
    osg::NodePath childPath( nv->getNodePath() );

    bool removeExpired( false );
    BOOST_FOREACH( lfx::PageData::RangeDataMap::value_type& rangeDataPair, pageData->getRangeDataMap() )
    {
        const unsigned int childIndex( rangeDataPair.first );
        lfx::PageData::RangeData& rangeData( rangeDataPair.second );
        osg::Node* child( grp->getChild( childIndex ) );
        childPath.push_back( child );

        const bool isInRange( inRange( validRange, rangeData._rangeValues ) );

        switch( rangeData._status )
        {
        case lfx::PageData::RangeData::UNLOADED:
            if( isInRange )
            {
                lfx::LoadRequestPtr request( createLoadRequest( child, childPath ) );
                pageThread->addLoadRequest( request );
                rangeData._status = lfx::PageData::RangeData::LOAD_REQUESTED;
            }
            break;

        case lfx::PageData::RangeData::LOAD_REQUESTED:
            if( isInRange )
            {
                lfx::LoadRequestPtr request( pageThread->retrieveLoadRequest( childPath ) );
                if( request != NULL )
                {
                    enableImages( child, request );
                    rangeData._status = lfx::PageData::RangeData::LOADED;
                    removeExpired = true;
                }
            }
            else
            {
                pageThread->cancelLoadRequest( childPath );
                rangeData._status = lfx::PageData::RangeData::UNLOADED;
            }

        default:
        case lfx::PageData::RangeData::LOADED:
        case lfx::PageData::RangeData::ACTIVE:
            // Nothing to do.
            break;
        }

        childPath.pop_back();
    }

    if( removeExpired )
    {
        BOOST_FOREACH( lfx::PageData::RangeDataMap::value_type& rangeDataPair, pageData->getRangeDataMap() )
        {
            const unsigned int childIndex( rangeDataPair.first );
            lfx::PageData::RangeData& rangeData( rangeDataPair.second );
            osg::Node* child( grp->getChild( childIndex ) );

            const bool isInRange( inRange( validRange, rangeData._rangeValues ) );
            switch( rangeData._status )
            {
            case lfx::PageData::RangeData::LOADED:
                rangeData._status = lfx::PageData::RangeData::ACTIVE;
                child->setNodeMask( ~0u );
                break;
            case lfx::PageData::RangeData::ACTIVE:
                if( !isInRange )
                {
                    reclaimImages( child );
                    rangeData._status = lfx::PageData::RangeData::UNLOADED;
                }
                // Intentional fallthrough.
            default:
                child->setNodeMask( 0u );
                break;
            }
        }
    }

    if( pageData->getRangeMode() == lfx::PageData::TIME_RANGE )
    {
        osg::Node* bestChild( grp->getChild( 0 ) );
        double minTimeDifference( FLT_MAX );

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

        unsigned int childIndex;
        for( childIndex=0; childIndex < grp->getNumChildren(); ++childIndex )
        {
            osg::Node* child( grp->getChild( childIndex ) );
            child->setNodeMask( ( child == bestChild ) ? 0xffffffff : 0x0 );
        }
    }

#if 1
    traverse( node, nv );
    // TBD this is temporary. Traverse everyone. Will probably
    // nor render correctly. OK for dev.
    // 
    // Probably want to traverse specific children based on
    // the range data status, then afterwards, set the
    // nodemask to all 1s for the active child and 0 for others.
    // That way we only cull/draw the active scene graph branch.
#else
    BOOST_FOREACH( lfx::PageData::RangeDataMap::value_type& rangeDataPair, pageData->getRangeDataMap() )
    {
        const unsigned int childIndex( rangeDataPair.first );
        lfx::PageData::RangeData& rangeData( rangeDataPair.second );

        switch( rangeData._status )
        {
        case lfx::PageData::RangeData::LOAD_REQUESTED:
            traverse( grp->getChild( childIndex ), nv );
            break;

        default:
        case lfx::PageData::RangeData::UNLOADED:
        case lfx::PageData::RangeData::LOADED:
        case lfx::PageData::RangeData::ACTIVE:
            break;
        }
    }
#endif
}


class CollectImagesVisitor : public osg::NodeVisitor
{
public:
    CollectImagesVisitor()
      : osg::NodeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN ),
        _request( lfx::LoadRequestImagePtr( new lfx::LoadRequestImage ) )
    {
        // Always traverse every node.
        setNodeMaskOverride( ~0u );
    }

    virtual void apply( osg::Node& node )
    {
        if( node.getUserData() == NULL )
        {
            osg::StateSet* stateSet( node.getStateSet() );
            if( stateSet != NULL )
            {
                for( unsigned int unit=0; unit<16; unit++ )
                {
                    osg::Texture* tex( static_cast< osg::Texture* >(
                        stateSet->getTextureAttribute( unit, osg::StateAttribute::TEXTURE ) ) );
                    if( ( tex != NULL ) && ( tex->getName() != "donotpage" ) &&
                        ( tex->getImage( 0 ) != NULL ) &&
                        ( tex->getImage( 0 )->data() == NULL ))
                    {
                        lfx::DBKey key( tex->getImage( 0 )->getFileName() );
                        _request->_keys.push_back( key );
                    }
                }
            }
        }
        traverse( node );
    }

    lfx::LoadRequestImagePtr _request;
};

lfx::LoadRequestPtr RootCallback::createLoadRequest( osg::Node* child, const osg::NodePath& childPath )
{
    CollectImagesVisitor collect;
    child->accept( collect );

    LoadRequestImagePtr request( collect._request );
    request->_path = childPath;
    return( request );
}

class DistributeImagesVisitor : public osg::NodeVisitor
{
public:
    DistributeImagesVisitor( lfx::LoadRequestImagePtr request )
        : osg::NodeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN ),
        _request( request )
    {
        // Always traverse every node.
        setNodeMaskOverride( ~0u );
    }

    virtual void apply( osg::Node& node )
    {
        if( node.getUserData() == NULL )
        {
            osg::StateSet* stateSet( node.getStateSet() );
            if( stateSet != NULL )
            {
                for( unsigned int unit=0; unit<16; unit++ )
                {
                    osg::Texture* tex( static_cast< osg::Texture* >(
                        stateSet->getTextureAttribute( unit, osg::StateAttribute::TEXTURE ) ) );
                    if( ( tex != NULL ) && ( tex->getName() != "donotpage" ) &&
                        ( tex->getImage( 0 ) != NULL ) )
                    {
                        lfx::DBKey key( tex->getImage( 0 )->getFileName() );
                        if( key.empty() )
                            OSG_WARN << "Got empty key." << std::endl;
                        osg::Image* image( _request->findAsImage( key ) );
                        if( image != NULL )
                            tex->setImage( 0, image );
                    }
                }
            }
        }
        traverse( node );
    }

protected:
    lfx::LoadRequestImagePtr _request;
};

void RootCallback::enableImages( osg::Node* child, lfx::LoadRequestPtr request )
{
    DistributeImagesVisitor distribute(
        boost::static_pointer_cast< lfx::LoadRequestImage >( request ) );
    child->accept( distribute );
}

class ReclaimImagesVisitor : public osg::NodeVisitor
{
public:
    ReclaimImagesVisitor()
        : osg::NodeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN )
    {
        // Always traverse every node.
        setNodeMaskOverride( ~0u );
    }

    virtual void apply( osg::Node& node )
    {
        if( node.getUserData() == NULL )
        {
            osg::StateSet* stateSet( node.getStateSet() );
            if( stateSet != NULL )
            {
                for( unsigned int unit=0; unit<16; unit++ )
                {
                    osg::Texture* tex( static_cast< osg::Texture* >(
                        stateSet->getTextureAttribute( unit, osg::StateAttribute::TEXTURE ) ) );
                    if( ( tex != NULL ) && ( tex->getName() != "donotpage" ) &&
                        ( tex->getImage( 0 ) != NULL ) )
                    {
                        lfx::DBKey key( tex->getImage( 0 )->getFileName() );
                        osg::Image* image( new osg::Image() );
                        image->setFileName( key );
                        tex->setImage( 0, image );
                    }
                }
            }
        }
        traverse( node );
    }
};

void RootCallback::reclaimImages( osg::Node* child )
{
    ReclaimImagesVisitor reclaim;
    child->accept( reclaim );
}

double RootCallback::computePixelSize( const osg::BoundingSphere& bSphere, const osg::Matrix& model,
        const osg::Vec3& wcEyePosition, const osg::Matrix& proj, const osg::Viewport* vp )
{
    const osg::Vec3 wcCenter = bSphere.center() * model;
    const float ecDistance( ( wcCenter - wcEyePosition ).length() );
    if( ecDistance < bSphere.radius() )
    {
        // Inside the bounding sphere.
        return( FLT_MAX );
    }

    // Compute pixelRadius, the sphere radius in pixels.

    // Get clip coord center, then get NDX x value (div by w), then get window x value.
#if 1
    // Optimized computation for bounding sphere at view center.
    const double winCenter( .5 * vp->width() + vp->x() );
#else
    // This is the unoptimized general case for the above "if 1" code path.
    const osg::Vec4 ecCenter( 0., 0., ecDistance, 1. );
    osg::Vec4 ccCenter( ecCenter * proj );
    ccCenter.x() /= ccCenter.w();
    const double cx( ( ( ccCenter.x() + 1. ) * .5 ) * vp->width() + vp->x() );
#endif

    // Repeast, but start with an eye coord point that is 'radius' units to the right of center.
    // Result is the pixel location of the rightmost edge of the bounding sphere.
    const osg::Vec4 ecRight( bSphere.radius(), 0., ecDistance, 1. );
    osg::Vec4 ccRight( ecRight * proj );
    ccRight.x() /= ccRight.w();
    const double winRight( ( ( ccRight.x() + 1. ) * .5 ) * vp->width() + vp->x() );

    // Pixel radius is the rightmost edge of the sphere minus the center.
    const double pixelRadius( winCenter - winRight );

    // Circle area A = pi * r^2
    return( osg::PI * pixelRadius * pixelRadius );
}


double RootCallback::getWrappedTime( const double& time, const double& minTime, const double& maxTime )
{
    const double span( maxTime - minTime );
    double intPart;
    const double fractPart( modf( time / span, &intPart ) );
    return( fractPart * span + minTime );
}


// lfx
}
