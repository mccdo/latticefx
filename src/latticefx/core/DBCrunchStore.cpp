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

#include <latticefx/core/DBCrunchStore.h>
#include <latticefx/core/LogMacros.h>

#include <crunchstore/DataManager.h>
#include <crunchstore/Persistable.h>

#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/foreach.hpp>

#include <sstream>


namespace csdb = crunchstore;


namespace lfx {
namespace core {


DBCrunchStore::DBCrunchStore()
  : DBBase( CRUNCHSTORE )
{
}
DBCrunchStore::DBCrunchStore( const DBCrunchStore& rhs )
  : DBBase( rhs ),
    _dm( rhs._dm ),
    _uuidMap( rhs._uuidMap )
{
}
DBCrunchStore::~DBCrunchStore()
{
    storeUUIDMap();
}


void DBCrunchStore::setDataManager( crunchstore::DataManagerPtr dm )
{
    _dm = dm;
    loadUUIDMap();
}
crunchstore::DataManagerPtr DBCrunchStore::getDataManager()
{
    return( _dm );
}


typedef std::vector< char > CharVec;

bool DBCrunchStore::storeImage( const osg::Image* image, const DBKey& dbKey )
{
    if( _dm == NULL )
    {
        LFX_WARNING( "storeImage(): No DataManager." );
        return( false );
    }

    const std::string keyString( dbKey );
    csdb::Persistable persist( "OSGImage" );

    {
        const size_t sz( sizeof( *image ) );
        const char* imageStart( (char*)image );
        const char* imageEnd( imageStart + sz );
        const CharVec imageMetadata( imageStart, imageEnd );

        const std::string metadataKey( "Metadata" );
        persist.AddDatum( metadataKey, imageMetadata );
    }
    {
        const size_t sz( image->getTotalSizeInBytes() );
        const char* dataStart( (char*)image->data() );
        const char* dataEnd( dataStart + sz );
        const CharVec imageData( dataStart, dataEnd );

        const std::string dataKey( "Data" );
        persist.AddDatum( dataKey, imageData );
    }

    // GetUUIDAsString() automatically generates a random boost uuis.
    // Must do this before saving to the DataManager.
    const std::string& uuidString( persist.GetUUIDAsString() );
    _uuidMap[ keyString ] = uuidString;

    LFX_TRACE( "storeImage(): Saving key \"" + keyString + "\", uuid: " + uuidString );
    _dm->Save( persist );

    return( true );
}

struct CharVecUserData : public CharVec, public osg::Object
{
    CharVecUserData()
      : CharVec(),
        osg::Object()
    {}
    CharVecUserData( CharVec& charVec )
      : CharVec( charVec ),
        osg::Object()
    {}
    CharVecUserData( const CharVecUserData& rhs, const osg::CopyOp copyOp=osg::CopyOp::SHALLOW_COPY )
      : CharVec( rhs ),
        osg::Object( rhs )
    {}

    META_Object(lfx,CharVecUserData);
};

osg::Image* DBCrunchStore::loadImage( const DBKey& dbKey )
{
    if( _dm == NULL )
    {
        LFX_WARNING( "loadImage(): No DataManager." );
        return( NULL );
    }

    const std::string keyString( dbKey );
    UUIDMap::const_iterator it( _uuidMap.find( keyString ) );
    if( it == _uuidMap.end() )
    {
        LFX_WARNING( "loadImage(): Can't find key \"" + keyString + "\"." );
        return( NULL );
    }
    LFX_TRACE( "loadImage(): Loading key \"" + dbKey + "\", uuid: " + it->second );

    csdb::Persistable persist( "OSGImage" );
    boost::uuids::string_generator strGen;
    const boost::uuids::uuid keyID( strGen( it->second.c_str() ) );
    persist.SetUUID( keyID );

    const std::string metadataKey( "Metadata" );
    persist.AddDatum( metadataKey, (char*)NULL );
    const std::string dataKey( "Data");
    persist.AddDatum( dataKey, (char*)NULL );

    _dm->Load( persist );

    CharVec metadata( persist.GetDatumValue<CharVec>( "Metadata" ) );
    osg::Image* image( (osg::Image*)&metadata[0] );

    // Copy image data from DB into a ref-counted struct, which we can
    // attach as UserData. This avoids a second data copy.
    CharVec charVec = persist.GetDatumValue<CharVec>( "Data" );
    osg::ref_ptr< CharVecUserData > imageData( new CharVecUserData( charVec ) );
    unsigned char* dataPtr( (unsigned char*)&(*imageData)[0] );

    osg::ref_ptr< osg::Image > tempImage( new osg::Image() );
    tempImage->setImage( image->s(), image->t(), image->r(),
        image->getInternalTextureFormat(), image->getPixelFormat(),
        image->getDataType(), dataPtr,
        osg::Image::NO_DELETE, image->getPacking() );
    tempImage->setUserData( imageData.get() );

    if( tempImage->getFileName().empty() )
        // Required for paging, in case the image load doesn't set it.
        tempImage->setFileName( dbKey );

    return( tempImage.release() );
}

bool DBCrunchStore::storeArray( const osg::Array* array, const DBKey& dbKey )
{
    if( _dm == NULL )
    {
        LFX_WARNING( "storeArray(): No DataManager." );
        return( false );
    }

    return( false );
}
osg::Array* DBCrunchStore::loadArray( const DBKey& dbKey )
{
    if( _dm == NULL )
    {
        LFX_WARNING( "loadArray(): No DataManager." );
        return( NULL );
    }

    return( NULL );
}


DBBase::StringSet DBCrunchStore::getAllKeys() const
{
    DBBase::StringSet keys;
    BOOST_FOREACH( const UUIDMap::value_type& info, _uuidMap )
    {
        keys.insert( info.first );
    }
    return( keys );
}


void DBCrunchStore::storeUUIDMap()
{
    if( _dm == NULL )
        return;

    LFX_TRACE( "storeUUIDMap()." );

    std::ostringstream ostr;
    ostr << _uuidMap.size() << ",";
    BOOST_FOREACH( const UUIDMap::value_type& info, _uuidMap )
    {
        ostr << info.first << "," << info.second << ",";
    }

    const std::string mapString( ostr.str() );
    const char* dataStart( (char*)&mapString[0] );
    const char* dataEnd( dataStart + mapString.size() );
    CharVec mapData( dataStart, dataEnd );
    // Add NULL terminator
    mapData.push_back( '\0' );

    boost::uuids::string_generator strGen;
    const boost::uuids::uuid knownID( strGen( "00000000-0000-0000-0000-000000000000" ) );
    const std::string typeName( "UUIDMap" );

    csdb::Persistable persist( typeName );
    persist.SetUUID( knownID );
    persist.AddDatum( "UUIDMap", mapData );
    _dm->Save( persist );
}
void DBCrunchStore::loadUUIDMap()
{
    if( _dm == NULL )
        return;

    LFX_TRACE( "loadUUIDMap()" );

    boost::uuids::string_generator strGen;
    const boost::uuids::uuid knownID( strGen( "00000000-0000-0000-0000-000000000000" ) );
    const std::string typeName( "UUIDMap" );
    if( !_dm->HasIDForTypename( knownID, typeName ) )
        return;

    csdb::Persistable persist( typeName );
    persist.SetUUID( knownID );
    persist.AddDatum( "UUIDMap", (char*)NULL );
    _dm->Load( persist );

    CharVec mapData( persist.GetDatumValue<CharVec>( "UUIDMap" ) );
    const std::string mapStr( &mapData[0] );

    // Get the number of entries
    const char comma( ',' );
    size_t pos( mapStr.find( comma ) );
    const std::string entriesStr( mapStr.substr( 0, pos ) );
    std::istringstream istr( entriesStr );
    unsigned int entries;
    istr >> entries;

    for( unsigned int idx=0; idx<entries; ++idx )
    {
        size_t nextPos( mapStr.find( comma, pos+1 ) );
        size_t finalPos( mapStr.find( comma, nextPos+1 ) );
        const std::string fileName( mapStr.substr( pos+1, nextPos-pos-1 ) );
        const std::string uuidStr( mapStr.substr( nextPos+1, finalPos-nextPos-1 ) );
        pos = finalPos;

        _uuidMap[ fileName ] = uuidStr;
    }
}


// core
}
// lfx
}
