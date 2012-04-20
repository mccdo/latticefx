
#include <latticefx/RootCallback.h>
#include <latticefx/PagingThread.h>
#include <latticefx/PageData.h>
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

void RootCallback::operator()( osg::Node* node, osg::NodeVisitor* nv )
{
    std::cout << "Update." << std::endl;

    lfx::PagingThread* pageThread( lfx::PagingThread::instance() );

    // Check for subgraphs ready to be attached.

    // Add any new page requests.
    BOOST_FOREACH( GroupList::value_type grp, _pageParentList )
    {
        lfx::PageData* pageData( static_cast< lfx::PageData* >( grp->getUserData() ) );
        std::cout << "  PageData." << std::endl;

        double rangeValue( 0. );
        if( pageData->getRangeMode() == PageData::PIXEL_SIZE_RANGE )
        {
            const osg::BoundingSphere& bSphere( pageData->getBound() );
            // rangeValue = (compute pixelSize here)
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
                // if rangeValue is in range
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

// lfx
}
