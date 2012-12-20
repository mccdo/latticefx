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

#include <latticefx/core/OperationBase.h>

#include <boost/foreach.hpp>


namespace lfx {
namespace core {


OperationBase::OperationBase( const OperationType opType )
  : _opType( opType ),
    _enable( true )
{
}
OperationBase::OperationBase( const OperationBase& rhs )
  : _inputs( rhs._inputs ),
    _inputNames( rhs._inputNames ),
    _inputTypeMap( rhs._inputTypeMap ),
    _opType( rhs._opType ),
    _enable( rhs._enable ),
    _nameValueMap( rhs._nameValueMap ),
    _db( rhs._db )
{
}
OperationBase::~OperationBase()
{
}


void OperationBase::addInput( ChannelDataPtr input )
{
    _inputs.push_back( input );
}
void OperationBase::setInputs( ChannelDataList inputList )
{
    _inputs = inputList;
}
ChannelDataList OperationBase::getInputs()
{
    return( _inputs );
}

void OperationBase::addInput( const std::string& name )
{
    _inputNames.push_back( name );
}
ChannelDataPtr OperationBase::getInput( const std::string& name )
{
    // Find the ChannelData in _inputs with the same name as the parameter,
    // and return it.
    BOOST_FOREACH( ChannelDataPtr cdp, _inputs )
    {
        if( ( cdp != NULL ) && ( cdp->getName() == name ) )
            return( cdp );
    }
    // Didn't find a match. Return a pointer to NULL.
    return( ChannelDataPtr( (ChannelData*)NULL ) );
}
const ChannelDataPtr OperationBase::getInput( const std::string& name ) const
{
    OperationBase* nonConstThis( const_cast< OperationBase* >( this ) );
    return( nonConstThis->getInput( name ) );
}
void OperationBase::setInputs( StringList& inputList )
{
    _inputNames = inputList;
}
OperationBase::StringList OperationBase::getInputNames()
{
    return( _inputNames );
}
const OperationBase::StringList& OperationBase::getInputNames() const
{
    return( _inputNames );
}

void OperationBase::setInputNameAlias( const int inputType, const std::string& alias )
{
    _inputTypeMap[ inputType ] = alias;
}
std::string OperationBase::getInputNameAlias( const int inputType ) const
{
    InputTypeMap::const_iterator it( _inputTypeMap.find( inputType ) );
    if( it != _inputTypeMap.end() )
        // Found it.
        return( it->second );
    else
        // Should never happen, as the derived-classes'constructors assigns defaults.
        return( "" );
}


void OperationBase::setEnable( const bool enable )
{
    _enable = enable;
}
bool OperationBase::getEnable() const
{
    return( _enable );
}

void OperationBase::setValue( const std::string& name, const OperationValue& value )
{
    _nameValueMap[ name ] = value;
}
bool OperationBase::hasValue( const std::string& name ) const
{
    return( _nameValueMap.find( name ) != _nameValueMap.end() );
}
const OperationValue* OperationBase::getValue( const std::string& name ) const
{
    // Return the value associated with the name.
    if( hasValue( name ) )
        return( &( _nameValueMap.find( name )->second ) );
    else
        // Didn't find it.
        return( NULL );
}


// core
}
// lfx
}
