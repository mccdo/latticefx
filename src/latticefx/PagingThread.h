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

#ifndef __LATTICEFX_PAGING_THREAD_H__
#define __LATTICEFX_PAGING_THREAD_H__ 1


#include <latticefx/Export.h>
#include <latticefx/DBUtils.h>
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

Note that the Group parent of pageable children initially has no bounding volume in the
typical case where all   children are pageable and therefore initially are all stub Group
placeholders. In order for RootCallback to know the spatial location of the parent Group,
the application should call setInitialBound() on the parent Group.

Note that the empty Group placeholders must be unique and not shared. Their addresses are used
by the RootCallback to uniquely identify the page requests sent to the PagingThread, and
check for their load status.

Work to be done (TBD):

Need to implement a mechanism for intelligently deciding how many OpenGL objects to
create per frame. OSG does this by adding a GraphicsOperation to each draw thread and
only creating a limited number of objects per frame. We should be able to do something
similar in a Camera post-draw callback, estimating the size of each OpenGL object and
limiting the number created per frame based on amount of data sent over the bus.
*/
/**@{*/


/** \class PagingThread PagingThread.h <latticefx/PagingThread.h>
\brief Class to manage paging thread
\details
PagingThread manages a boost thread that loads OSG subgraphs from disk file or
from a database. PagingThread is a singleton so that it can oversee paging on
an application-wide basis and therefore avoid the possible bus contention that
might occur if multiple paging threads were active.
*/
class LATTICEFX_EXPORT PagingThread
{
protected:
    PagingThread();

public:
    static PagingThread* instance();
    virtual ~PagingThread();

    /** \struct LoadRequest PagingThread.h <latticefx/PagingThread.h>
    \brief Contains data related to loading a subgraph.
    \details Loads a subgraph using the \c dbKey and references the root
    of the subgraph in \c _loadedModel. LoadRequest is stored in multiple
    PagingThread member lists of type LoadRequestList. */
    struct LoadRequest {
        LoadRequest();
        LoadRequest( osg::Node* location, const DBKey& dbKey );

        LoadRequest& operator=( const LoadRequest& rhs );

        osg::ref_ptr< osg::Node > _location;
        DBKey _dbKey;

        osg::ref_ptr< osg::Node > _loadedModel;
    };
    typedef std::list< LoadRequest > LoadRequestList;

    /** \brief Request that the paging thread stop execution.
    \details Sets \c _haltRequest to true. The paging thread queries
    the status of \c _haltRequest with getHaltRequest() and stops execution
    when true. The thread might not stop immediately if it is actively
    loading data. The paging thread ignores any pending load requests.
    Loaded data not retrieved by client code remains referenced by the
    PagingThread class. (TBD need a singleton destructor.)
    
    Thread safe. */
    void halt();

    /** \brief Return true if client code has called halt().

    Thread safe. */
    bool getHaltRequest() const;


    /** \brief Add a request to load data from disk.
    \details \c location is a unique address associated with the load request.
    Client code should use it in subsequent retrieveRequest() calls.
    
    Thread safe. In typical usage, client code calls this during the update
    traversal. */
    void addLoadRequest( osg::Node* location, const DBKey& dbKey );
    /** \overload
    \details Adds multiple requests with a single call, holding the mutex only once
    to reduce blocking. */
    void addLoadRequest( const LoadRequestList& requests );


    /* \overload Add a request to load data from disk. */
    //void addRequest( const osg::Node* location, const int dbKey );

    /** \brief Attempt to retrieve the result of a load request.
    \details If the load request associated with \c location has not yet
    completed, retrieveRequest() returns NULL. Otherwise, retrieveRequest()
    returns the address of the loaded subgraph.
    
    Thread safe. In typical usage, client code calls this during the update
    traversal. */
    osg::Node* retrieveRequest( const osg::Node* location );

    /** TBD remove when no longer needed. */
    bool debugChechReturnsEmpty();

    /** \brief Cancel a load request.
    \details If \c location is found in any of the PagingThread's queues,
    it is removed and cancelLoadRequest() returns true. Otherwise, cancelLoadRequest()
    returns false.

    TBD. Possible bug: Seems plausible that the paging thread could take a request
    off the \c _loadRequestList and start loading it. Before completion, the client code
    could cancel the load. Then the paging thread would complete the load and add the
    request to \c _completedList. This should be fixed.

    Thread safe. In typical usage, client code calls this during the update
    traversal. */
    bool cancelLoadRequest( const osg::Node* location );

    /** \brief Main loop executed by the paging thread.
    \details Obtains requests from the \c _loadRequestList, loads the data, then adds the
    loaded data to the \c _completedList. In the current implementation, this function also
    moves requests from \c _completedList to \c _returnList once per loop iteration. Once on the
    \c _returnList, client code can retrieve the request. If there are no requests in the
    \c _loadRequestList, the paging thread sleep for 16 milliseconds. */
    void operator()();

protected:
    boost::thread* _thread;
    mutable boost::mutex _requestMutex, _completedMutex, _retrieveMutex;

    bool _haltRequest;

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
