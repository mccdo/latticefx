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

#include <latticefx/core/DBBase.h>
#include <latticefx/core/LogBase.h>
#include <latticefx/core/LogMacros.h>

#include <boost/foreach.hpp>

#include <sstream>
#include <iomanip>


namespace lfx {
namespace core {


DBBase::DBBase( const ImplementationType implType )
  : LogBase( (implType == CRUNCHSTORE)?"lfx.db.cs":
             ( (implType == DISK)?"lfx.db.disk":
               "lfx.db.base" ) ),
    _implType( implType )
{
}
DBBase::DBBase( const DBBase& rhs )
  : LogBase( rhs ),
    _implType( rhs._implType )
{
}
DBBase::~DBBase()
{
}


DBKey DBBase::generateDBKey()
{
    static unsigned int keyCounter( 0 );

    std::ostringstream ostr;
    ostr << "dbKey" << std::setfill( '0' ) <<
        std::setw( 10 ) << keyCounter++;
    return( DBKey( ostr.str() ) );
}
DBKey DBBase::generateDBKey( const std::string& baseName, const TimeValue time )
{
    std::ostringstream ostr;
    ostr.precision( 20 );
    ostr << baseName << "-" << time;
    return( (DBKey) ostr.str() );
}


bool DBBase::storeImage( const osg::Image* image, const DBKey& dbKey )
{
    _images[ dbKey ] = osg::ref_ptr< osg::Image >(
        const_cast< osg::Image* >( image ) );
    return( true );
}
osg::Image* DBBase::loadImage( const DBKey& dbKey )
{
    ImageVec::iterator it( _images.find( dbKey ) );
    if( it != _images.end() )
        return( it->second.get() );
    else
        return( NULL );
}

bool DBBase::storeArray( const osg::Array* array, const DBKey& dbKey )
{
    _arrays[ dbKey ] = osg::ref_ptr< osg::Array >(
        const_cast< osg::Array* >( array ) );
    return( true );
}
osg::Array* DBBase::loadArray( const DBKey& dbKey )
{
    ArrayVec::iterator it( _arrays.find( dbKey ) );
    if( it != _arrays.end() )
        return( it->second.get() );
    else
        return( NULL );
}


DBBase::StringSet DBBase::getAllKeys() const
{
    StringSet keys;

    BOOST_FOREACH( const ImageVec::value_type& info, _images )
    {
        keys.insert( info.first );
    }
    BOOST_FOREACH( const ArrayVec::value_type& info, _arrays )
    {
        keys.insert( info.first );
    }

    return( keys );
}


// core
}
// lfx
}
