
#ifndef __LATTICEFX_PAGING_THREAD_H__
#define __LATTICEFX_PAGING_THREAD_H__ 1


#include <latticefx/Export.h>
#include <osg/Node>

#include <boost/thread.hpp>


namespace lfx {


/** \addtogroup PagingSupport Support for threaded data loading
\brief Support for run-time loading/unloading of scene graph elements.
\details
The LatticeFX paging system does not use the osgDB::DatabasePager or the osg::PagedLOD
node. Instead, LatticeFX uses a boost thread for paging, generic Group nodes in place
of the PagedLOD, and the OSG update callback and UserData mechanisms for paging mechanics
and metadata. LatticeFX supports paging subgraphs based on either estimated pixel size or
time (in support of time series data animation).

To avoid traversing the entire scene graph searching for nodes that might potentially need
paging, the LatticeFX paging system depends on the RootCallback class. Applications create
an instance of this callback and attach it as an update callback to the root of some Node
in their scene (usually the root of a subgraph containing Groups with pageable nodes). If
the RootCallback instance is attached near the top of the scene graph and there are no other
update callbacks in the scene graph, OSG's update visitor will do very little work.

For each Group that contains pageable children, the app creates an instance of PageData and
adds it to the RootCallback instance. The PageData contains PageData::RangeData for each
pageable child. The app must add empty Group nodes as placeholders for each pageable child.

During update, the RootCallback iterates over all PageData objects and checks child
PageData::RangeData to see if an unloaded child needs to be loaded. If so, it calls
PagingThread::addLoadRequest(). This causes the PagingThread to load the child and eventually
return it. When the RootCallback detects that the load is complete, it adds the loaded
child in place of the empty Group node placeholder. At that time, any children that are no
longer valid are removed from the parent Group and empty Group placeholders added back in
their place.

Note that the empty Group placeholders must be unique and not shared. Their address is used
by the RootCallback to uniquely identify the page requests sent to the PagingThread, and
check for their load status.
*/
/**@{*/


/** \class PagingThread PagingThread.h <latticefx/PagingThread.h>
\brief 
\details 
*/
class LATTICEFX_EXPORT PagingThread
{
protected:
    PagingThread();

public:
    static PagingThread* instance();
    virtual ~PagingThread();

    void halt();
    bool getHaltRequest() const;

    void addLoadRequest( osg::Node* location, const std::string& fileName );
    //void addRequest( const osg::Node* location, const int dbKey );

    osg::Node* retrieveRequest( const osg::Node* location );

    bool cancelLoadRequest( const osg::Node* location );

    /** \brief
    \details Called by the paging thread. */
    void operator()();

protected:
    boost::thread* _thread;
    mutable boost::mutex _requestMutex, _completedMutex, _retrieveMutex;

    bool _haltRequest;

    struct LoadRequest {
        LoadRequest();
        LoadRequest( osg::Node* location, const std::string& fileName );
        LoadRequest( osg::Node* location, const int dbKey );

        LoadRequest& operator=( const LoadRequest& rhs );

        osg::ref_ptr< osg::Node > _location;
        std::string _fileName;
        //const int _dbKey;

        osg::ref_ptr< osg::Node > _loadedModel;
    };
    typedef std::list< LoadRequest > LoadRequestList;
    LoadRequestList _loadRequestList;
    LoadRequestList _completedList;
    LoadRequestList _returnList;


    static osg::Node* retrieveAndRemove( const osg::Node* location, LoadRequestList& theList,
            boost::mutex& theMutex );
};


/**@}*/


// lfx
}


// __LATTICEFX_PAGING_THREAD_H__
#endif
