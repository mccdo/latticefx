/*************** <auto-copyright.rb BEGIN do not edit this line> **************
 *
 * Copyright 2012-2012 by Ames Laboratory
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2.1 as published by the Free Software Foundation.
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
 *************** <auto-copyright.rb END do not edit this line> ***************/

#ifndef __LFX_CORE_LOAD_REQUEST_H__
#define __LFX_CORE_LOAD_REQUEST_H__ 1


//#include <latticefx/core/Export.h>
#include <latticefx/core/DBUtils.h>
#include <osg/Image>
#include <osg/Node>

#include <boost/shared_ptr.hpp>

#include <list>
#include <vector>


namespace lfx {
namespace core {


/** \addtogroup PagingSupport */
/**@{*/


/** \struct LoadRequest LoadRequest.h <latticefx/core/LoadRequest.h>
\brief Base class for loading OSG data.
\details Derived classes override the load() method to load OSG Image
files, Node files, or Object files as needed. */
struct LoadRequest {
    LoadRequest();
    LoadRequest( const osg::NodePath& path, const DBKeyList& keys );
    LoadRequest( const LoadRequest& rhs );
    ~LoadRequest();

    virtual bool load() = 0;

    osg::Object* find( const DBKey& dbKey );


    osg::NodePath _path;
    DBKeyList _keys;

    typedef std::map< DBKey, osg::ref_ptr< osg::Object > > ResultsMap;
    ResultsMap _results;
};

typedef boost::shared_ptr< LoadRequest > LoadRequestPtr;
typedef std::list< LoadRequestPtr > LoadRequestList;



/** \struct LoadRequestImage LoadRequest.h <latticefx/core/LoadRequest.h>
\brief LoadRequest for loading OSG Image data.
\details Load OSG Image data in the PagingThread. */
struct LoadRequestImage : public LoadRequest
{
    virtual bool load();

    osg::Image* findAsImage( const DBKey& dbKey );
};

typedef boost::shared_ptr< LoadRequestImage > LoadRequestImagePtr;


/**@}*/


// core
}
// lfx
}


// __LFX_CORE_LOAD_REQUEST_H__
#endif
