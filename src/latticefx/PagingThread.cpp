
#include <latticefx/PagingThread.h>

#include <iostream>


namespace lfx {


PagingThread::PagingThread()
{
    _thread = new boost::thread( boost::ref( *this ) );
}
PagingThread* PagingThread::instance()
{
    static PagingThread* s_pagingThreadInstance( new PagingThread );
    return( s_pagingThreadInstance );
}
PagingThread::~PagingThread()
{
}

void PagingThread::operator()()
{
    std::cout << "thread" << std::endl;
}


// lfx
}
