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

#include <latticefx/core/PagingCallback.h>
#include <latticefx/core/PagingThread.h>
#include <latticefx/core/LoadRequest.h>
#include <latticefx/core/PageData.h>
#include <latticefx/core/LogMacros.h>

#include <osg/NodeVisitor>
#include <osg/Texture2D>

#include <boost/foreach.hpp>
#include <iostream>



// Forward declaration.
/** \brief Return the pixel size of \c bSphere.
\details Computes the pixel radius of the bounding sphere, then returns
the area of a circle with that radius. */
double computePixelSize( const osg::BoundingSphere& bSphere, const osg::Matrix& model,
    const osg::Vec3& wcEyePosition, const osg::Matrix& proj, const osg::Viewport* vp );


/** \brief Return true if \c validRange and \c childRange overlap.
\details If the paging RangeMode is PIXEL_SIZE_RANGE, both min and max values of
\c validRange are set to the return value of computePixelSize() and \c childRange
comes from the child-specific PageData::RangeData.

If the paging RangeMode is TIME_RANGE, \c validRange is a range of time values specified
by the application, and both min and max values of \c childRange are set to the time
value of the child node. */
inline bool inRange( const lfx::RangeValues& validRange, const lfx::RangeValues& childRange )
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



namespace lfx {


PagingCallback::PagingCallback()
  : osg::NodeCallback(),
    LogBase( "lfx.core.page.cb" ),
    _animationTime( 0. ),
    _timeRange( RangeValues( -0.5, 0.5 ) )
{
}
PagingCallback::PagingCallback( const PagingCallback& rhs )
  : osg::NodeCallback( rhs ),
    LogBase( rhs ),
    _animationTime( rhs._animationTime ),
    _timeRange( rhs._timeRange )
{
}
PagingCallback::~PagingCallback()
{
}

void PagingCallback::setAnimationTime( const double time )
{
    _animationTime = time;
}
double PagingCallback::getAnimationTime() const
{
    return( _animationTime );
}
void PagingCallback::setTimeRange( const RangeValues& timeRange )
{
    _timeRange = timeRange;
}
RangeValues PagingCallback::getTimeRange() const
{
    return( _timeRange );
}


void PagingCallback::operator()( osg::Node* node, osg::NodeVisitor* nv )
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
        // _timeRange is set by the application. The assumption is that this will be large enough
        // to accomodate several children, only one of which will be displayed (by use of NodeMask).
        // This is different from pixel size paging because the time step (and therefore valid child)
        // is expected to change quite rapidly, so we need to page in a buffer around the current
        // play time to help ensure a smooth animation free of paging bottlenecks.
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

        const osg::Matrix modelMat( osg::computeLocalToWorld( nv->getNodePath() ) );
        const osg::BoundingSphere& bSphere( node->getBound() );
        const double pixelSize( computePixelSize( bSphere, modelMat, wcEyePosition, proj, vp.get() ) );

        validRange = RangeValues( pixelSize, pixelSize );
    }

    osg::Group* grp( node->asGroup() );
    if( grp == NULL )
    {
        LFX_WARNING( "operator(): Should not have NULL Group." );
    }
    osg::NodePath childPath( nv->getNodePath() );

    bool removeExpired( false );
    bool continueUpdateTraversal( true );
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
                if( request->_keys.empty() )
                {
                    // No images to load. Turn on this child/branch.
                    rangeData._status = lfx::PageData::RangeData::LOADED;
                    removeExpired = true;
                }
                else
                {
                    LFX_TRACE( "Adding LoadRequest." );
                    pageThread->addLoadRequest( request );
                    rangeData._status = lfx::PageData::RangeData::LOAD_REQUESTED;
                    // We must fulfill this load request before we dive deeper into
                    // the scene graph.
                    continueUpdateTraversal = false;
                }
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
                    LFX_TRACE( "Retrieved LoadRequest." );
                }
                else
                {
                    // We must fulfill this load request before we dive deeper into
                    // the scene graph.
                    continueUpdateTraversal = false;
                    LFX_TRACE( "Waiting for LoadRequest." );
                }
            }
            else
            {
                pageThread->cancelLoadRequest( childPath );
                reclaimImages( child );
                rangeData._status = lfx::PageData::RangeData::UNLOADED;
                LFX_TRACE( "Canceling LoadRequest." );
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
                    child->setNodeMask( 0u );
                }
                break;
            default:
            case lfx::PageData::RangeData::UNLOADED:
            case lfx::PageData::RangeData::LOAD_REQUESTED:
                // Nothing to do.
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


    // Continue traversing if there are no pending load requests.
    if( continueUpdateTraversal )
    {
        // Only ACTIVE children have non-zero node masks, so all other
        // children will be ckipped by this traversal.
        traverse( node, nv );
    }
}


class CollectImagesRecursive
{
public:
    CollectImagesRecursive( const osg::Matrix& rootModelMat )
      : _request( lfx::LoadRequestImagePtr( new lfx::LoadRequestImage ) ),
        _rootModelMat( rootModelMat )
    {
        // Get transform info for LOD calculations.
        PagingThread::instance()->getTransforms( _wcEyePosition, _proj, _vp );
    }

    void recurse( osg::Node& node )
    {
        _nodePath.push_back( &node );

        // Collect the images for this node.
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

        osg::Group* grp( node.asGroup() );
        lfx::PageData* pageData( static_cast< lfx::PageData* >( node.getUserData() ) );
        if( ( pageData == NULL ) ||
            ( pageData->getRangeMode() == lfx::PageData::TIME_RANGE ) )
        {
            // It's a normal Node or Group. Just traverse all children.
            if( grp != NULL )
            {
                for( unsigned int idx=0; idx<grp->getNumChildren(); idx++ )
                    recurse( *( grp->getChild( idx ) ) );
            }
            _nodePath.pop_back();
            return;
        }
        if( grp == NULL )
        {
            LFX_WARNING_STATIC( "lfx.core.page", "CollectImagesVisitor: Should not have NULL Group." );
        }

        const osg::Matrix modelMat( osg::computeLocalToWorld( _nodePath ) * _rootModelMat );
        const osg::BoundingSphere& bSphere( node.getBound() );
        const double pixelSize( computePixelSize( bSphere, modelMat, _wcEyePosition, _proj, _vp.get() ) );
        const RangeValues validRange( pixelSize, pixelSize );

        BOOST_FOREACH( lfx::PageData::RangeDataMap::value_type& rangeDataPair, pageData->getRangeDataMap() )
        {
            const unsigned int childIndex( rangeDataPair.first );
            lfx::PageData::RangeData& rangeData( rangeDataPair.second );
            osg::Node* child( grp->getChild( childIndex ) );

            const bool isInRange( inRange( validRange, rangeData._rangeValues ) );

            if( isInRange )
            {
                switch( rangeData._status )
                {
                case PageData::RangeData::UNLOADED:
                {
                    rangeData._status = PageData::RangeData::LOAD_REQUESTED;
                    break;
                }
                case PageData::RangeData::LOAD_REQUESTED:
                {
                    LFX_WARNING_STATIC( "lfx.core.page", "CollectImagesVisitor: Unextected LOAD_REQUESTED status." );
                    break;
                }
                case PageData::RangeData::LOADED:
                {
                    LFX_WARNING_STATIC( "lfx.core.page", "CollectImagesVisitor: Unextected LOADED status." );
                    break;
                }
                case PageData::RangeData::ACTIVE:
                {
                    LFX_WARNING_STATIC( "lfx.core.page", "CollectImagesVisitor: Unextected ACTIVE status." );
                    break;
                }
                }
                recurse( *child );
            }
        }
        _nodePath.pop_back();
    }

    lfx::LoadRequestImagePtr _request;

private:
    osg::Matrix _rootModelMat;

    osg::Vec3 _wcEyePosition;
    osg::Matrix _proj;
    osg::ref_ptr< const osg::Viewport> _vp;

    osg::NodePath _nodePath;
};

lfx::LoadRequestPtr PagingCallback::createLoadRequest( osg::Node* child, const osg::NodePath& childPath )
{
    osg::Matrix modelMat( osg::computeLocalToWorld( childPath ) );
    CollectImagesRecursive collect( modelMat );
    collect.recurse( *child );

    LoadRequestImagePtr request( collect._request );
    request->_path = childPath;
    return( request );
}


class DistributeImagesRecursive
{
public:
    DistributeImagesRecursive( const lfx::LoadRequestImagePtr request )
      : _request( request )
    {
    }

    void recurse( osg::Node& node )
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
                    {
                        LFX_WARNING_STATIC( "lfx.core.page", "Got empty key." );
                    }
                    osg::Image* image( _request->findAsImage( key ) );
                    if( image != NULL )
                        tex->setImage( 0, image );
                }
            }
        }

        osg::Group* grp( node.asGroup() );
        lfx::PageData* pageData( static_cast< lfx::PageData* >( node.getUserData() ) );
        if( ( pageData == NULL ) ||
            ( pageData->getRangeMode() == lfx::PageData::TIME_RANGE ) )
        {
            // It's a normal Node or Group. Just traverse all children.
            if( grp != NULL )
            {
                for( unsigned int idx=0; idx<grp->getNumChildren(); idx++ )
                    recurse( *( grp->getChild( idx ) ) );
            }
            return;
        }
        if( grp == NULL )
        {
            LFX_WARNING_STATIC( "lfx.core.page", "DistributeImagesRecursive: Should not have NULL Group." );
        }

        BOOST_FOREACH( lfx::PageData::RangeDataMap::value_type& rangeDataPair, pageData->getRangeDataMap() )
        {
            const unsigned int childIndex( rangeDataPair.first );
            lfx::PageData::RangeData& rangeData( rangeDataPair.second );
            osg::Node* child( grp->getChild( childIndex ) );

            switch( rangeData._status )
            {
            case PageData::RangeData::LOAD_REQUESTED:
                rangeData._status = PageData::RangeData::ACTIVE;
                child->setNodeMask( ~0u );
                break;
            default:
                break;
            }
            recurse( *child );
        }
    }

protected:
    const lfx::LoadRequestImagePtr _request;
};

void PagingCallback::enableImages( osg::Node* child, lfx::LoadRequestPtr request )
{
    LoadRequestImagePtr imageRequest( boost::static_pointer_cast< LoadRequestImage >( request ) );
    DistributeImagesRecursive distribute( imageRequest );
    distribute.recurse( *child );
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

        lfx::PageData* pageData( static_cast< lfx::PageData* >( node.getUserData() ) );
        if( ( pageData == NULL ) ||
            ( pageData->getRangeMode() == lfx::PageData::TIME_RANGE ) )
        {
            // It's a normal Node or Group. Just traverse all children.
            traverse( node );
            return;
        }
        osg::Group* grp( node.asGroup() );
        if( grp == NULL )
        {
            LFX_WARNING_STATIC( "lfx.core.page", "DistributeImagesRecursive: Should not have NULL Group." );
        }

        BOOST_FOREACH( lfx::PageData::RangeDataMap::value_type& rangeDataPair, pageData->getRangeDataMap() )
        {
            const unsigned int childIndex( rangeDataPair.first );
            lfx::PageData::RangeData& rangeData( rangeDataPair.second );
            osg::Node* child( grp->getChild( childIndex ) );

            rangeData._status = PageData::RangeData::UNLOADED;
            child->setNodeMask( 0u );
        }

        traverse( node );
    }
};

void PagingCallback::reclaimImages( osg::Node* child )
{
    ReclaimImagesVisitor reclaim;
    child->accept( reclaim );
}

double PagingCallback::getWrappedTime( const double& time, const double& minTime, const double& maxTime )
{
    const double span( maxTime - minTime );
    if( span == 0 )
        return( time );
    double intPart;
    const double fractPart( modf( time / span, &intPart ) );
    return( fractPart * span + minTime );
}


// lfx
}


double computePixelSize( const osg::BoundingSphere& bSphere, const osg::Matrix& model,
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
