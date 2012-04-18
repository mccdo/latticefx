
#include <latticefx/RootCallback.h>
#include <latticefx/PagingThread.h>

#include <iostream>


namespace lfx {


RootCallback::RootCallback()
  : _pagingActive( false )
{
}
RootCallback::~RootCallback()
{
}

void RootCallback::operator()( osg::Node* node, osg::NodeVisitor* nv )
{
    std::cout << "Update." << std::endl;

    if( !_pagingActive )
    {
        std::cout << "Start paging." << std::endl;
        lfx::PagingThread::instance();
        _pagingActive = true;
    }

    traverse( node, nv );
}

// lfx
}
