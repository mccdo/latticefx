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

#ifndef __LFX_CORE_PAGING_THREAD_H__
#define __LFX_CORE_PAGING_THREAD_H__ 1


#include <latticefx/core/Export.h>
#include <latticefx/core/LoadRequest.h>
#include <latticefx/core/DBUtils.h>
#include <latticefx/core/LogBase.h>

#include <osg/Camera>
#include <osg/Viewport>

#include <boost/thread.hpp>

#include <list>
#include <vector>


namespace lfx {
namespace core {


/** \addtogroup PagingSupport Support for threaded data loading
\brief Support for run-time loading/unloading of scene graph elements.
\details
The LatticeFX paging system does not use the osgDB::DatabasePager or the osg::PagedLOD
node. Instead, LatticeFX uses a boost thread for paging, generic Group nodes in place
of the PagedLOD, and the OSG update callback and user data mechanisms for paging mechanics
and metadata. LatticeFX supports paging scene graph elements based on either estimated
pixel size or time (in support of time series data animation).

LatticeFX assumes the geometry in the scene graph will be negligible in size (a single
instanced quad, point, vector, etc), but the textures will consume significant RAM and
must be paded. For this reason, LatticeFX builds a complete scene graph containing all
geometry, with empty stub textures. The entire paging system is designed to support
paging in textures and attaching them to the scene graph when needed, and reclaiming those
textures when they have expired. The geometry itself, and Nodes and Groups in general,
are never paged by LatticeFX.

Each Group that contains pageable children has both a PagingCallback update traversal
callback object and a PageData user data object. DataSet is responsible for creating
and attaching these objects during scene graph creation. DataSet attaches them to the
root node to support time series data, and to subordinate nodes to support
ChannelDataLOD.

PageData contains PageData::RangeData for each pageable child. RangeData contains a
RangeValues std::pair of ranges for the child. When paging based on pixel size,
a given child is paged in (and eventually displayed) when the computed pixel size
of its projected bounding volume falls between the RangeValue's min and max values.

When paging based on time, the RangeValues pair contains a single time value. A given
child is paged in when this time value falls within range of the owning PageData's
min and max sime (see PageData::setMinMaxTime()), and displayed if it has the "best"
time (closest to actual time) of all pageable children for the owning Group.

During update, the PagingCallback iterates over all PageData objects and checks child
PageData::RangeData to see if an unloaded child needs to be loaded. If so, it recursively
descends the child to build a LoadRequest containing all required scene graph elements,
and passes that LoadRequest to PagingThread::addLoadRequest(). This causes the
PagingThread to process and return the LoadRequest. When the PagingCallback retrieves the
filled reqauest, it again recursively descends the child to distribute the loaded elements.
At that time, any children that are no longer valid are masked off and their expired scene
graph elements are reclaimed.

Work to be done (TBD):

Need to implement a mechanism for intelligently deciding how many OpenGL objects to
create per frame. OSG does this by adding a GraphicsOperation to each draw thread and
only creating a limited number of objects per frame. We should be able to do something
similar in a Camera post-draw callback, estimating the size of each OpenGL object and
limiting the number created per frame based on amount of data sent over the bus.

We need a pool of osg::Texture objects to limit and reuse the OpenGL texture object IDs.
As the PagingThread loads data, it would acquire an available Texture from the pool,
and when a texture expires, the reclamation process would return the Texture to the pool.
*/
/**@{*/


/** \class PagingThread PagingThread.h <latticefx/core/PagingThread.h>
\brief Class to manage paging thread
\details
PagingThread manages a boost thread that processes LoadRequest objects (loads OSG
scene graph elements) from a database. PagingThread is a singleton so that it can
oversee paging on an application-wide basis and therefore avoid the possible bus
contention that might occur if multiple paging threads were active.
*/
class LATTICEFX_EXPORT PagingThread : protected LogBase
{
protected:
    PagingThread();

public:
    static PagingThread* instance();
    virtual ~PagingThread();


    static LoadRequestList::iterator find( LoadRequestList& requestList, const osg::NodePath& path );

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


    /** \brief Adds a request to load data in the PagingThread.
    \details Pushes \c request onto the end of the \c _requestList.
    
    Thread safe. In typical usage, client code calls this during the update
    traversal. */
    void addLoadRequest( LoadRequestPtr request );


    /** \brief Attempt to retrieve the result of a previous load request.
    \details Returns a pointer to the LoadRequest identified by \c path, or
    a pointer to NULL of the LoadRequest can't be found or is not yet complete.
    
    Thread safe. In typical usage, client code calls this during the update
    traversal. */
    LoadRequestPtr retrieveLoadRequest( const osg::NodePath& path );

    /** \brief Cancel the specified load request.
    \details Adds \c path to the internal \c _cancelList for later processing
    by PagingThread::processCancellations(), which is called by the page thread.

    Thread safe. In typical usage, client code calls this during the update
    traversal. */
    void cancelLoadRequest( const osg::NodePath& path );

    /** \brief Main loop executed by the paging thread.
    \details Obtains requests from the \c _requestList, loads the data, then adds the
    loaded data to the \c _completedList. In the current implementation, this function also
    moves requests from \c _completedList to \c _availableList once per loop iteration. Once on the
    \c _availableList, client code can retrieve the request. If there are no requests in the
    \c _requestList, the paging thread sleep for 16 milliseconds. */
    void operator()();

    void setTransforms( const osg::Vec3& wcEyePosition );
    void setTransforms( const osg::Matrix& proj, const osg::Viewport* vp );
    void setTransforms( const osg::Vec3& wcEyePosition, const osg::Matrix& proj,
        const osg::Viewport* vp );
    void getTransforms( osg::Vec3& wcEyePosition, osg::Matrix& proj,
        osg::ref_ptr< const osg::Viewport >& vp ) const;

protected:
    /** \brief Process pending canceled LoadRequests.
    \details Iterates over all database keys stored in the _cancelList. For each each, all
    internal lists (_requestList, _completedList, and _availableList) are searched. If a
    LoadRequest with the matching key is found on any list, the LoadRequest is removed.

    This function should be called by the page thread. _requestMutex must be locked before
    calling this function. */
    void processCancellations();

    void dump( const std::string& header, const LoadRequestList& requests );

    boost::thread* _thread;
    mutable boost::mutex _requestMutex, _availableMutex, _cancelMutex;

    bool _haltRequest;

    LoadRequestList _requestList;
    LoadRequestList _completedList;
    LoadRequestList _availableList;
    osg::NodePathList _cancelList;

    mutable boost::mutex _transformMutex;
    osg::Vec3 _wcEyePosition;
    osg::Matrix _proj;
    osg::ref_ptr< const osg::Viewport > _vp;
};


/**@}*/


// core
}
// lfx
}


// __LFX_CORE_PAGING_THREAD_H__
#endif
