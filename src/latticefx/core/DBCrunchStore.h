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

#ifndef __LFX_CORE_DB_CRUNCH_STORE_H__
#define __LFX_CORE_DB_CRUNCH_STORE_H__ 1

#include <latticefx/core/Export.h>
#include <latticefx/core/DBBase.h>

#include <crunchstore/DataManagerPtr.h>

#include <boost/shared_ptr.hpp>


namespace lfx
{
namespace core
{


/** \class DBCrunchStore DBCrunchStore.h <latticefx/core/DBCrunchStore.h>
\brief A DBBase-derived class that supports OSG Image and Array storage in CrunchStore.
\details Application code is responsible for specifying a crunchstore::DataManager
and configuring that DataManager with a CrunchStore Buffer, Cache, and Store.

CrunchStore doesn't support use of arbitrary keys. DBCrunchStore supports this
by keeping a map of DBKey objects (std::strings) to the boost::uuids::uuid keys
that CrunchStore uses. This map of DBKeys to UUIDs is loaded from the DataManager
when the DataManager is set, and is written to the DataManager in the DBCrunchStore
destructor. */
class LATTICEFX_EXPORT DBCrunchStore : public DBBase
{
public:
    DBCrunchStore();
    DBCrunchStore( const DBCrunchStore& rhs );
    ~DBCrunchStore();


    void setDataManager( crunchstore::DataManagerPtr dm );
    crunchstore::DataManagerPtr getDataManager();


    virtual bool storeImage( const osg::Image* image, const DBKey& dbKey );
    virtual osg::Image* loadImage( const DBKey& dbKey );

    virtual bool storeArray( const osg::Array* array, const DBKey& dbKey );
    virtual osg::Array* loadArray( const DBKey& dbKey );

    virtual DBBase::StringSet getAllKeys() const;

protected:
    void storeUUIDMap();
    void loadUUIDMap();

    crunchstore::DataManagerPtr _dm;

    typedef std::map< std::string, std::string > UUIDMap;
    UUIDMap _uuidMap;
};

typedef boost::shared_ptr< DBCrunchStore > DBCrunchStorePtr;


// core
}
// lfx
}


// __LFX_CORE_DB_CRUNCH_STORE_H__
#endif
