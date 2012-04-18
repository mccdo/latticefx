
#include <latticefx/OperationBase.h>


namespace lfx {


OperationBase::OperationBase( const OperationType opType )
  : _opType( opType ),
    _enable( true )
{
}

OperationBase::~OperationBase()
{
}


void OperationBase::addInput( ChannelDataPtr input )
{
    _inputs.push_back( input );
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
    if( hasValue( name ) )
        return( &( _nameValueMap.find( name )->second ) );
    else
        return( NULL );
}




OperationValue::OperationValue()
  : _valueType( NO_VALUE )
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
