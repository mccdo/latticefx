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

#include <latticefx/core/OperationValue.h>

#include <boost/concept_check.hpp>


namespace lfx {
namespace core {


////////////////////////////////////////////////////////////////////////////////
OperationValue::OperationValue( boost::any value )
    :
    m_value( value )
{
    ;
}
////////////////////////////////////////////////////////////////////////////////
OperationValue::OperationValue( const OperationValue& orig ):
    m_value( orig.m_value )
{
    ;
}
////////////////////////////////////////////////////////////////////////////////
OperationValue::~OperationValue()
{
    ;
}
////////////////////////////////////////////////////////////////////////////////
boost::any OperationValue::GetValue() const
{
    return m_value;
}
////////////////////////////////////////////////////////////////////////////////
bool OperationValue::SetValue( boost::any value )
{
    m_value = value;
    return true;
}
////////////////////////////////////////////////////////////////////////////////
bool OperationValue::IsBool() const
{
    return IsBool( m_value );
}
////////////////////////////////////////////////////////////////////////////////
bool OperationValue::IsInt() const
{
    return IsInt( m_value );
}
////////////////////////////////////////////////////////////////////////////////
bool OperationValue::IsFloat() const
{
    return IsFloat( m_value );
}
////////////////////////////////////////////////////////////////////////////////
bool OperationValue::IsDouble() const
{
    return IsDouble( m_value );
}
////////////////////////////////////////////////////////////////////////////////
bool OperationValue::IsString() const
{
    return IsString( m_value );
}
////////////////////////////////////////////////////////////////////////////////
bool OperationValue::IsIntVector() const
{
    return IsIntVector( m_value );
}
////////////////////////////////////////////////////////////////////////////////
bool OperationValue::IsFloatVector() const
{
    return IsFloatVector( m_value );
}
////////////////////////////////////////////////////////////////////////////////
bool OperationValue::IsDoubleVector() const
{
    return IsDoubleVector( m_value );
}
////////////////////////////////////////////////////////////////////////////////
bool OperationValue::IsBLOB() const
{
    return IsBLOB( m_value );
}
////////////////////////////////////////////////////////////////////////////////
bool OperationValue::IsStringVector() const
{
    return IsStringVector( m_value );
}
////////////////////////////////////////////////////////////////////////////////
bool OperationValue::IsVectorized() const
{
    return IsVectorized( m_value );
}
////////////////////////////////////////////////////////////////////////////////
bool OperationValue::IsBool( const boost::any& value ) const
{
    return value.type() == typeid ( bool );
}
////////////////////////////////////////////////////////////////////////////////
bool OperationValue::IsInt( const boost::any& value ) const
{
    return value.type() == typeid ( int );
}
////////////////////////////////////////////////////////////////////////////////
bool OperationValue::IsFloat( const boost::any& value ) const
{
    return value.type() == typeid ( float );
}
////////////////////////////////////////////////////////////////////////////////
bool OperationValue::IsDouble( const boost::any& value ) const
{
    return value.type() == typeid ( double );
}
////////////////////////////////////////////////////////////////////////////////
bool OperationValue::IsString( const boost::any& value ) const
{
    return boost::any_cast<std::string > ( &value );
}
////////////////////////////////////////////////////////////////////////////////
bool OperationValue::IsIntVector( const boost::any& value ) const
{
    return boost::any_cast< std::vector< int > >( &value );
}
////////////////////////////////////////////////////////////////////////////////
bool OperationValue::IsFloatVector( const boost::any& value ) const
{
    return boost::any_cast< std::vector< float > >( &value );
}
////////////////////////////////////////////////////////////////////////////////
bool OperationValue::IsDoubleVector( const boost::any& value ) const
{
    return boost::any_cast< std::vector< double > >( &value );
}
////////////////////////////////////////////////////////////////////////////////
bool OperationValue::IsStringVector( const boost::any& value ) const
{
    return boost::any_cast< std::vector< std::string > >( &value );
}
////////////////////////////////////////////////////////////////////////////////
bool OperationValue::IsBLOB( const boost::any& value ) const
{
    // If it's not any of our known types, it's a BLOB.
    if( !( (IsBool( value)) || (IsInt( value )) || (IsFloat( value )) ||
           (IsDouble( value )) || (IsString( value )) || (IsVectorized( value ))
        ) )
    {
        return true;
    }
    else
    {
        return false;
    }
}
////////////////////////////////////////////////////////////////////////////////
bool OperationValue::IsVectorized( const boost::any& value ) const
{
    if( ( IsIntVector( value ) ) ||
            ( IsFloatVector( value ) ) ||
            ( IsDoubleVector( value ) ) ||
            ( IsStringVector( value ) )
            )
    {
        return true;
    }
    else
    {
        return false;
    }
}
////////////////////////////////////////////////////////////////////////////////


// core
}
// lfx
}
