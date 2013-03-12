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

#ifndef __LFX_CORE_DB_BASE_H__
#define __LFX_CORE_DB_BASE_H__ 1

#include <latticefx/core/Export.h>
#include <latticefx/core/types.h>
#include <latticefx/core/LogBase.h>

#include <osg/Image>
#include <osg/Array>
#include <osg/ref_ptr>

#include <boost/shared_ptr.hpp>
#include <map>
#include <set>


namespace osg
{
class Image;
class Array;
}


namespace lfx
{
namespace core
{


/** \class DBBase DBBase.h <latticefx/core/DBBase.h>
\brief Base class for LatticeFX interface to database support.
\details Application code typically configures a derived class, such as
DBCrunchStore (for use with CrunchStore), or DBDisk (to use the filesystem
to store data files). The application must pass this object (as a DBBasePtr)
to DataSet, and also to any OperationBase-derived classes that require it.
Internal LatticeFX code attaches the instantiated class to the PagingCallback
for use by LoadRequest / PagingThread, and OperationBase-derived classes
load and store data from it. */
class LATTICEFX_EXPORT DBBase : protected LogBase
{
public:
    typedef enum
    {
        MEMORY,
        DISK,
        CRUNCHSTORE
    } ImplementationType;

    DBBase( const ImplementationType implType );
    DBBase( const DBBase& rhs );
    virtual ~DBBase();

    ImplementationType getImplementationType() const
    {
        return( _implType );
    }


    virtual DBKey generateDBKey();
    virtual DBKey generateDBKey( const std::string& baseName, const TimeValue time = ( TimeValue )0. );


    virtual bool storeImage( const osg::Image* image, const DBKey& dbKey );
    virtual osg::Image* loadImage( const DBKey& dbKey );

    virtual bool storeArray( const osg::Array* array, const DBKey& dbKey );
    virtual osg::Array* loadArray( const DBKey& dbKey );


    typedef std::set< std::string > StringSet;

    virtual StringSet getAllKeys() const;

protected:
    ImplementationType _implType;

private:
    // TBD DB temp hack.
    // Should DBBase substitute as RAM storage? Or would we require
    // use of CrunchStore for this? Can CrunchStore do this?
    // DBBase as RAM storage is currently untested.

    typedef std::map< DBKey, osg::ref_ptr< osg::Image > > ImageVec;
    ImageVec _images;

    typedef std::map< DBKey, osg::ref_ptr< osg::Array > > ArrayVec;
    ArrayVec _arrays;
};

typedef boost::shared_ptr< DBBase > DBBasePtr;


// core
}
// lfx
}


// __LFX_CORE_DB_BASE_H__
#endif
