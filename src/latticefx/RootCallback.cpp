
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
    std::cout << "Update." << std::endl;

    lfx::PagingThread* pageThread( lfx::PagingThread::instance() );

    // Add any new page requests.
    BOOST_FOREACH( GroupList::value_type grp, _pageParentList )
    {
        lfx::PageData* pageData( static_cast< lfx::PageData* >( grp->getUserData() ) );
        std::cout << "  PageData." << std::endl;

        double rangeValue( 0. );
        if( pageData->getRangeMode() == PageData::PIXEL_SIZE_RANGE )
        {
            const osg::BoundingSphere& bSphere( pageData->getBound() );
            rangeValue = computePixelSize( bSphere, nv );
            std::cout << "Pixel size: " << rangeValue << std::endl;
        }
        else if( pageData->getRangeMode() == PageData::TIME_RANGE )
        {
            // rangeValue = (current timestamp)
        }

        BOOST_FOREACH( PageData::RangeDataMap::value_type& rangeDataPair, pageData->getRangeDataMap() )
        {
            const unsigned int childIndex( rangeDataPair.first );
            PageData::RangeData& rangeData( rangeDataPair.second );
            switch( rangeData._status )
            {
            case PageData::RangeData::UNLOADED:
            {
                std::cout << "    RangeData UNLOADED" << std::endl;
                if( ( rangeValue >= rangeData._rangeValues.first ) &&
                    ( rangeValue <= rangeData._rangeValues.second ) )
                {
                    pageThread->addLoadRequest( pageData->getParent()->getChild( childIndex ),
                        rangeData._fileName );
                    rangeData._status = PageData::RangeData::LOAD_REQUESTED;
                }
                break;
            }
            case PageData::RangeData::LOAD_REQUESTED:
            {
                std::cout << "    RangeData LOAD_REQUESTED" << std::endl;
                osg::Node* loadedModel( pageThread->retrieveRequest( pageData->getParent()->getChild( childIndex ) ) );
                if( loadedModel != NULL )
                {
                    std::cout << "      Retrieved: " << std::hex << loadedModel << std::endl;
                    //    remove expired children
                    pageData->getParent()->setChild( childIndex, loadedModel );
                    rangeData._status = PageData::RangeData::LOADED;
                }
                break;
            }
            case PageData::RangeData::LOADED:
            {
                std::cout << "    RangeData LOADED" << std::endl;
                break;
            }
            case PageData::RangeData::UNLOAD_REQUESTED:
            {
                std::cout << "    RangeData UNLOAD_REQUESTED" << std::endl;
                break;
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
