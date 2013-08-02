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
#include <latticefx/core/JsonSerializer.h>

#include <boost/foreach.hpp>

namespace lfx
{
namespace core
{

OperationBase::OperationBase( const OperationType opType )
    : ObjBase(),
      _opType( opType ),
      _enable( true )
{



	//_opTypes[(int)UnspecifiedType] = std::string("UnspecifiedType");

	//((int)PreprocessCacheType, "PreprocessCacheType")(RunTimeProcessingType, "RunTimeProcessingType")(RendererType, "RendererType");
}

OperationBase::OperationBase( const OperationBase& rhs )
    : ObjBase( rhs ),
      _inputs( rhs._inputs ),
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

std::string OperationBase::getEnumName( OperationType e ) const
{
	switch (e)
	{
	case PreprocessCacheType:
		return "PreprocessCacheType";
	case RunTimeProcessingType:
		return "RunTimeProcessingType";
	case RendererType:
		return "RendererType";
	}
	
	return "UnspecifiedType";
}

 OperationBase::OperationType OperationBase::getEnumFromName( const std::string &name ) const
{
	if( !name.compare( "PreprocessCacheType" ) ) return PreprocessCacheType;
	else if( !name.compare( "RunTimeProcessingType" ) ) return RunTimeProcessingType;
	else if( !name.compare( "RendererType" ) ) return RendererType;
	
	return UnspecifiedType;
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
        {
            return( cdp );
        }
    }
    // Didn't find a match. Return a pointer to NULL.
    return( ChannelDataPtr( ( ChannelData* )NULL ) );
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
    {
        return( it->second );
    }
    else
        // Should never happen, as the derived-classes'constructors assigns defaults.
    {
        return( "" );
    }
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
    {
        return( &( _nameValueMap.find( name )->second ) );
    }
    else
        // Didn't find it.
    {
        return( NULL );
    }
}

void OperationBase::setPluginData( const std::string &pluginName, const std::string &pluginClassName )
{
	_dataMapObj["pluginName"] = pluginName;
	_dataMapObj["pluginClassName"] = pluginClassName;
}

void OperationBase::serializeData( JsonSerializer *json ) const
{
	// let the parent write its data
	ObjBase::serializeData( json );

	json->insertObj( OperationBase::getClassName(), true);
	json->insertObjValue( "opType",  getEnumName( _opType ) );
	json->insertObjValue( "enable", _enable );

	json->insertArray( "inputNames" );
	BOOST_FOREACH( std::string name, _inputNames )
	{
		json->insertArrValue( name );
	}
	json->popParent();

	json->popParent();
}

bool OperationBase::loadData( JsonSerializer *json, IObjFactory *pfactory, std::string *perr )
{
	// let the parent load its data
	if ( !ObjBase::loadData( json, pfactory, perr )) return false;

	// get to this classes data
	if ( !json->getObj( OperationBase::getClassName() ) )
	{
		if (perr) *perr = "Json: Failed to get OperationBase data";
		return false;
	}

	std::string name;
	json->getValue( "opType", &name, getEnumName( _opType ) );
	_opType = getEnumFromName( name );

	json->getValue( "enable", &_enable, _enable );

	// start get input names
	_inputNames.clear();
	if( !json->getArray( "inputNames", true ) )
	{
		if (perr) *perr = "Json: Failed to get OperationBase inputNames";
		return false;
	}

	for( unsigned int i=0; i<json->size(); i++ )
	{
		std::string s;
		if( !json->getValue( i, &s ) )
		{
			if (perr) *perr = "Json: Failed to get OperationBase inputNames string";
			return false;
		}

		_inputNames.push_back( s );
	}

	json->popParent();
	// end get input names

	json->popParent();
	return true;
}

// core
}
// lfx
}
