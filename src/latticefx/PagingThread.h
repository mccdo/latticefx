
#ifndef __LATTICEFX_PAGING_THREAD_H__
#define __LATTICEFX_PAGING_THREAD_H__ 1


#include <latticefx/Export.h>

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

    /** \brief
    \details Called by the paging thread. */
    void operator()();

protected:
    boost::thread* _thread;
};


// lfx
}


// __LATTICEFX_PAGING_THREAD_H__
#endif
