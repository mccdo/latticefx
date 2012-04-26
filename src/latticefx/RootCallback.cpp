
#include <latticefx/RootCallback.h>
#include <latticefx/PagingThread.h>
#include <latticefx/PageData.h>
#include <osg/Transform>
#include <boost/foreach.hpp>

#include <iostream>


namespace lfx {


RootCallback::RootCallback()
{
}
RootCallback::~RootCallback()
{
}

void RootCallback::addPageParent( osg::Group* parent )
{
    _pageParentList.push_back( parent );
}
void RootCallback::setCamera( osg::Camera* camera )
{
    _camera = camera;
}

void RootCallback::operator()( osg::Node* node, osg::NodeVisitor* nv )
{
    //std::cout << "Update." << std::endl;

    lfx::PagingThread* pageThread( lfx::PagingThread::instance() );

    // Add any new page requests.
    BOOST_FOREACH( GroupList::value_type grp, _pageParentList )
    {
        if( grp->getUserData() == NULL )
        {
            OSG_WARN << "RootCallback::operator(): page parent has NULL UserData. Should be ;fx::PageData." << std::endl;
            continue;
        }
        lfx::PageData* pageData( static_cast< lfx::PageData* >( grp->getUserData() ) );
        //std::cout << "  PageData." << std::endl;

        double testValue( 0. );
        PageData::RangeValues range;
        if( pageData->getRangeMode() == PageData::PIXEL_SIZE_RANGE )
        {
            // If the owning parent Group has nothing but paged children, it must use Node::setInitialBound()
            // to give it some spatial location and size. Retrieve that bound.
            const osg::BoundingSphere& bSphere( grp->getBound() );
            testValue = computePixelSize( bSphere, nv );
            //std::cout << "Pixel size: " << testValue << std::endl;
        }
        else if( pageData->getRangeMode() == PageData::TIME_RANGE )
        {
            // range = (current time range)
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
                    /* Huh? This is a child we just added. Don't remove it!
                    if( !inRange( testValue, range ) )
                    {
                        pageData->getParent()->setChild( childIndex, new osg::Group );
                        rangeData._status = PageData::RangeData::UNLOADED;
                    }
                    else
                    */
                    {
                        rangeData._status = PageData::RangeData::ACTIVE;
                    }
                }
                }
            }
        }
    }

    traverse( node, nv );
}


double RootCallback::computePixelSize( const osg::BoundingSphere& bSphere, const osg::NodeVisitor* nv )
{
    if( _camera == NULL )
    {
        OSG_WARN << "RootCallback: NULL _camera." << std::endl;
        return( 0. );
    }

    // TBD optimization: This only needs to be done once per update.
    osg::Matrix modelView( osg::computeLocalToWorld( nv->getNodePath() ) * _camera->getViewMatrix() );
    const osg::Vec3 ecCenter = bSphere.center() * modelView ;
    if( -( ecCenter.z() ) < bSphere.radius() )
    {
        // Inside the bounding sphere.
        return( FLT_MAX );
    }

    // Compute pixelRadius, the sphere radius in pixels.

    const osg::Viewport* vp( _camera->getViewport() );

    osg::Vec4 ccCenter( osg::Vec4( ecCenter, 1. ) * _camera->getProjectionMatrix() );
    ccCenter.x() /= ccCenter.w();
    double cx( ( ccCenter.x() * 2. + 1. ) * vp->width() + vp->x() );

    const osg::Vec4 ecRight( ecCenter.x() + bSphere.radius(), ecCenter.y(), ecCenter.z(), 1. );
    osg::Vec4 ccRight( ecRight * _camera->getProjectionMatrix() );
    ccRight.x() /= ccRight.w();
    double rx( ( ccRight.x() * 2. + 1. ) * vp->width() + vp->x() );

    const double pixelRadius( rx - cx );

    // Circle area A = pi * r^2
    return( osg::PI * pixelRadius * pixelRadius );
}


// lfx
}
