
#ifndef __LATTICEFX_PAGING_THREAD_H__
#define __LATTICEFX_PAGING_THREAD_H__ 1


#include <latticefx/Export.h>
#include <osg/Node>

#include <boost/thread.hpp>


namespace lfx {


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

    /** \brief
    \details Called by the paging thread. */
    void operator()();

protected:
    boost::thread* _thread;
    mutable boost::mutex _mutex;

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
};


// lfx
}


// __LATTICEFX_PAGING_THREAD_H__
#endif
