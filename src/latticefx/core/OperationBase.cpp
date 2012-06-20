/*************** <auto-copyright.rb BEGIN do not edit this line> *************
*
* latticeFX is (C) Copyright 2011-2012 by Ames Laboratory
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Library General Public
* License as published by the Free Software Foundation; 
* version 2.1 of the License.
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
* -----------------------------------------------------------------
* Date modified: $Date$
* Version:       $Rev$
* Author:        $Author$
* Id:            $Id$
* -----------------------------------------------------------------
*
*************** <auto-copyright.rb END do not edit this line> **************/

#include <latticefx/core/OperationBase.h>

#include <boost/foreach.hpp>


namespace lfx {


OperationBase::OperationBase( const OperationType opType )
  : _opType( opType ),
    _enable( true )
{
}
OperationBase::OperationBase( const OperationBase& rhs )
  : _inputs( rhs._inputs ),
    _inputNames( rhs._inputNames ),
    _opType( rhs._opType ),
    _enable( rhs._enable ),
    _nameValueMap( rhs._nameValueMap )
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
        if( cdp->getName() == name )
            return( cdp );
    }
    // Didn't find a match. Return a pointer to NULL.
    return( ChannelDataPtr( (ChannelData*)NULL ) );
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
        // Didn't find it. Return NULL.
        return( NULL );
}




OperationValue::OperationValue()
  : _valueType( NO_VALUE )
{
}
OperationValue::OperationValue( const OperationValue& rhs )
  : _valueType( rhs._valueType ),
    _value( rhs._value ),
    _valueString( rhs._valueString )
{
}
OperationValue::~OperationValue()
{
}

OperationValue::OperationValue( const int value )
{
    set( value );
}
OperationValue::OperationValue( const float value )
{
    set( value );
}
OperationValue::OperationValue( const osg::Vec3& value )
{
    set( value );
}
OperationValue::OperationValue( const std::string& value )
{
    set( value );
}
OperationValue::OperationValue( const bool value )
{
    set( value );
}

void OperationValue::set( const int value )
{
    _valueType = INT_VALUE;
    _value._int = value;
}
void OperationValue::set( const float value )
{
    _valueType = FLOAT_VALUE;
    _value._float = value;
}
void OperationValue::set( const osg::Vec3& value )
{
    _valueType = VEC3_VALUE;
    _value._vec3[ 0 ] = value[ 0 ];
    _value._vec3[ 1 ] = value[ 1 ];
    _value._vec3[ 2 ] = value[ 2 ];
}
void OperationValue::set( const std::string& value )
{
    _valueType = STRING_VALUE;
    _valueString = value;
}
void OperationValue::set( const bool value )
{
    _valueType = BOOL_VALUE;
    _value._bool = value;
}

OperationValue::ValueType OperationValue::getType() const
{
    return( _valueType );
}

int OperationValue::getInt() const
{
    return( _value._int );
}
float OperationValue::getFloat() const
{
    return( _value._float );
}
osg::Vec3 OperationValue::getVec3() const
{
    return( osg::Vec3( _value._vec3[ 0 ], _value._vec3[ 1 ], _value._vec3[ 2 ] ) );
}
const std::string& OperationValue::getString() const
{
    return( _valueString );
}
bool OperationValue::getBool() const
{
    return( _value._bool );
}


// lfx
}
