/*************** <auto-copyright.rb BEGIN do not edit this line> **************
 *
 * Copyright 2012-2013 by Ames Laboratory
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

#include <latticefx/core/JsonSerializer.h>
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/DefaultHandler.h>

#include <boost/filesystem.hpp>

using namespace Poco::JSON;
using namespace Poco::Dynamic;

namespace lfx
{
namespace core
{

////////////////////////////////////////////////////////////////////////////////
size_t JsonSerializer::JsonParent::size() const
{
	if (_obj) return _obj->size();
	else if (_arr) return _arr->size();

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
bool JsonSerializer::JsonParent::set( const Poco::Dynamic::Var &var )
{
	if (_arr)
	{
		_arr->add( var );
		return true;
	}

	return false; // object values require a name
}

////////////////////////////////////////////////////////////////////////////////
bool JsonSerializer::JsonParent::set( const std::string &name, const Poco::Dynamic::Var &var )
{
	if( _obj )
	{
		_obj->set( name, var );
		return true;
	}
	else if( _arr )
	{
		_arr->add( var ); // array values do not have a name
		return true;
	}

	return false; 
}

////////////////////////////////////////////////////////////////////////////////
bool JsonSerializer::JsonParent::get( const std::string &name, Poco::Dynamic::Var *var ) const
{
	if( _obj )
	{
		if ( !_obj->has( name ) ) return false;

		*var = _obj->get( name );
		return true;
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////
bool JsonSerializer::JsonParent::get( unsigned int idx, Poco::Dynamic::Var *var ) const
{
	if( _obj )
	{
		std::string key;
		if( !getKey( idx, &key ) ) return false;

		return get( key, var );
	}

	if( idx >= size() ) return false;
	if( !_arr ) return false;

	*var = _arr->get( idx );
	return true;
}

////////////////////////////////////////////////////////////////////////////////
bool JsonSerializer::JsonParent::getKey( unsigned int idx, std::string *pkey ) const
{
	if( !_obj ) return false;
	if( idx >= size() ) return false;

	std::vector<std::string> names;
	_obj->getNames( names );

	*pkey = names[idx];
	return true;
}

////////////////////////////////////////////////////////////////////////////////
std::string JsonSerializer::JsonParent::toString( unsigned int indent ) const
{
	std::stringstream ss;

	if( _obj )
	{
		_obj->stringify( ss, indent );	
	}
	else if( _arr )
	{
		_arr->stringify( ss, indent );
	}

	return ss.str();
}

////////////////////////////////////////////////////////////////////////////////
void JsonSerializer::JsonParent::toStream( std::ostream *pos, unsigned int indent ) const
{
	if( _obj )
	{
		_obj->stringify( *pos, indent );	
	}
	else if( _arr )
	{
		_arr->stringify( *pos, indent );
	}
}

////////////////////////////////////////////////////////////////////////////////
JsonSerializer::JsonSerializer( const char *fileFullPath, int version, const char *sourceName )
{
	_fileFullPath = fileFullPath;
	_version = version;
	_sourceName = sourceName;
}

////////////////////////////////////////////////////////////////////////////////
bool JsonSerializer::isObj()
{
	JsonParentPtr parent = getParent();
	if ( !parent ) return false;

	return parent->isObj();
}

////////////////////////////////////////////////////////////////////////////////
bool JsonSerializer::isArray()
{
	JsonParentPtr parent = getParent();
	if ( !parent ) return false;

	return parent->isArray();
}

////////////////////////////////////////////////////////////////////////////////
size_t JsonSerializer::size()
{
	JsonParentPtr parent = getParent();
	if ( !parent ) return 0;

	return parent->size();
}

////////////////////////////////////////////////////////////////////////////////
bool JsonSerializer::load( const std::string &json )
{
	Parser parser;
	DefaultHandler handler;
	parser.setHandler(&handler);
	parser.parse(json);

	Var result = handler.result();
	Object::Ptr object = result.extract<Object::Ptr>();

	JsonParentPtr parent;
	if (object)
	{
		parent.reset( new JsonParent( object ) );
	}
	else
	{
		Array::Ptr ar = result.extract<Array::Ptr>();
		if (ar)
		{
			parent.reset( new JsonParent( ar ) );
		}
	}

	if (!parent) return false;

	_root = parent;
	pushParent( parent );

	getValue( "source", &_sourceName, "" );
	getValue( "version", &_version, -1 );

	return true;
}

////////////////////////////////////////////////////////////////////////////////
bool JsonSerializer::getObj( const std::string &name, bool push )
{
	return getObjArr( name, push, true );
}

////////////////////////////////////////////////////////////////////////////////
bool JsonSerializer::getObj( unsigned int idx, bool push )
{
	JsonParentPtr parent = getParent();
	if ( !parent ) return false;

	Var v;
	if ( !parent->get( idx, &v ) ) return false;

	JsonParentPtr newParent;
	Object::Ptr obj = v.extract<Object::Ptr>();

	if ( !obj ) return false;
	newParent.reset( new JsonParent( obj ) );

	if ( push ) pushParent( newParent );
	
	return true;
}

////////////////////////////////////////////////////////////////////////////////
bool JsonSerializer::getArray( const std::string &name, bool push )
{
	return getObjArr( name, push, false );
}

////////////////////////////////////////////////////////////////////////////////
bool JsonSerializer::getObjArr( const std::string &name, bool push, bool obj )
{
	JsonParentPtr parent = getParent();
	if ( !parent ) return false;

	Var v;
	if ( !parent->get( name, &v ) ) return false;

	JsonParentPtr newParent;
	if (obj)
	{
		Object::Ptr obj = v.extract<Object::Ptr>();
		if ( !obj ) return false;
		newParent.reset( new JsonParent( obj ) );
	}
	else
	{
		Array::Ptr arr = v.extract<Array::Ptr>();
		if ( !arr ) return false;
		newParent.reset( new JsonParent( arr ) );
	}

	if ( push ) pushParent( newParent );
	
	return true;
} 

////////////////////////////////////////////////////////////////////////////////
bool JsonSerializer::getObjKey( unsigned int idx, std::string *pKey ) const
{
	JsonParentPtr parent = getParentConst();
	if ( !parent ) return false;

	return parent->getKey( idx, pKey );
}

////////////////////////////////////////////////////////////////////////////////
bool JsonSerializer::getValue( const std::string &name, std::string *pvalue, std::string def ) const
{
	return getValue<std::string>( name, pvalue, def );
}

////////////////////////////////////////////////////////////////////////////////
bool JsonSerializer::getValue( const std::string &name, int *pvalue, int def ) const
{
	return getValue<int>( name, pvalue, def );
}

////////////////////////////////////////////////////////////////////////////////
bool JsonSerializer::getValue( const std::string &name, unsigned int *pvalue, unsigned int def ) const
{
	return getValue<unsigned int>( name, pvalue, def );
}

////////////////////////////////////////////////////////////////////////////////
bool JsonSerializer::getValue( const std::string &name, double *pvalue, double def ) const
{
	return getValue<double>( name, pvalue, def );
}

////////////////////////////////////////////////////////////////////////////////
bool JsonSerializer::getValue( const std::string &name, float *pvalue, float def ) const
{
	return getValue<float>( name, pvalue, def );
}

////////////////////////////////////////////////////////////////////////////////
bool JsonSerializer::getValue( const std::string &name, bool *pvalue, bool def ) const
{
	return getValue<bool>( name, pvalue, def );
}

////////////////////////////////////////////////////////////////////////////////
bool JsonSerializer::getValue( unsigned int idx, std::string *pvalue, std::string def ) const
{
	return getValue<std::string>( idx, pvalue, def );
}

////////////////////////////////////////////////////////////////////////////////
bool JsonSerializer::getValue( unsigned int idx, int *pvalue, int def ) const
{
	return getValue<int>( idx, pvalue, def );
}

////////////////////////////////////////////////////////////////////////////////
bool JsonSerializer::getValue( unsigned int idx, unsigned int *pvalue, unsigned int def ) const
{
	return getValue<unsigned int>( idx, pvalue, def );
}

////////////////////////////////////////////////////////////////////////////////
bool JsonSerializer::getValue( unsigned int idx, double *pvalue, double def ) const
{
	return getValue<double>( idx, pvalue, def );
}

////////////////////////////////////////////////////////////////////////////////
bool JsonSerializer::getValue( unsigned int idx, float *pvalue, float def ) const
{
	return getValue<float>( idx, pvalue, def );
}

////////////////////////////////////////////////////////////////////////////////
bool JsonSerializer::getValue( unsigned int idx, bool *pvalue, bool def ) const
{
	return getValue<bool>( idx, pvalue, def );
}

////////////////////////////////////////////////////////////////////////////////
void JsonSerializer::insertObj( const std::string &name, bool push )
{
	insertObj( name, createObj(), push );
}

////////////////////////////////////////////////////////////////////////////////
void JsonSerializer::insertArray( const std::string &name, bool push )
{
	insertArray( name, createArray(), push );
}

////////////////////////////////////////////////////////////////////////////////
void JsonSerializer::insertObj( const std::string &name, Object::Ptr obj, bool push )
{
	Var var( obj );
	insertParentData( name, JsonParentPtr( new JsonParent( obj ) ), var, push);
}

////////////////////////////////////////////////////////////////////////////////
void JsonSerializer::insertArray( const std::string &name, Array::Ptr arr, bool push )
{
	Var var( arr );
	insertParentData( name, JsonParentPtr( new JsonParent( arr ) ), var, push);
}

////////////////////////////////////////////////////////////////////////////////
void JsonSerializer::insertParentData( const std::string &name, JsonParentPtr newParent, const Poco::Dynamic::Var &varParent, bool push)
{
	JsonParentPtr parent = getParent();

	parent->set( name, varParent );
	if (push) pushParent( newParent );
}

////////////////////////////////////////////////////////////////////////////////
bool JsonSerializer::insertObjValue( const std::string &name, const std::string &value )
{
	Var v( value );
	return insertObjValue( name, v );
}

////////////////////////////////////////////////////////////////////////////////
bool JsonSerializer::insertObjValue( const std::string &name, const char *value )
{
	return insertObjValue( name, std::string( value ) );
}

////////////////////////////////////////////////////////////////////////////////
bool JsonSerializer::insertObjValue( const std::string &name, int value )
{
	Var v( value );
	return insertObjValue( name, v );
}

////////////////////////////////////////////////////////////////////////////////
bool JsonSerializer::insertObjValue( const std::string &name, unsigned int value )
{
	Var v( value );
	return insertObjValue( name, v );
}

////////////////////////////////////////////////////////////////////////////////
bool JsonSerializer::insertObjValue( const std::string &name, double value )
{
	Var v( value );
	return insertObjValue( name, v );
}

////////////////////////////////////////////////////////////////////////////////
bool JsonSerializer::insertObjValue( const std::string &name, float value )
{
	Var v( value );
	return insertObjValue( name, v );
}

bool JsonSerializer::insertObjValue( const std::string &name, bool value )
{
	Var v( value );
	return insertObjValue( name, v );
}

////////////////////////////////////////////////////////////////////////////////
bool JsonSerializer::insertObjValue( const std::string &name, const Poco::Dynamic::Var &value )
{
	return getParent()->set( name, value );
}

////////////////////////////////////////////////////////////////////////////////
bool JsonSerializer::insertArrValue( const std::string &value )
{
	Var v( value );
	return insertArrValue( v );
}

////////////////////////////////////////////////////////////////////////////////
bool JsonSerializer::insertArrValue( const char *value )
{
	return insertArrValue( std::string( value ) );
}

////////////////////////////////////////////////////////////////////////////////
bool JsonSerializer::insertArrValue( int value )
{
	Var v( value );
	return insertArrValue( v );
}

////////////////////////////////////////////////////////////////////////////////
bool JsonSerializer::insertArrValue( double value )
{
	Var v( value );
	return insertArrValue( v );
}

////////////////////////////////////////////////////////////////////////////////
bool JsonSerializer::insertArrValue( const Poco::Dynamic::Var &value )
{
	return getParent()->set( value );
}

////////////////////////////////////////////////////////////////////////////////
void JsonSerializer::markParentStack()
{
	_markStack.push_back( 0 );
}

////////////////////////////////////////////////////////////////////////////////
void JsonSerializer::popMark()
{
	if( !_markStack.size() ) return;
	unsigned int cnt = _markStack[_markStack.size()-1];

	for( unsigned int i=0; i<cnt; i++ )
	{
		if( _parentStack.size() <= 0 ) break;
		_parentStack.pop();
	}

	_markStack.pop_back();
}

////////////////////////////////////////////////////////////////////////////////
void JsonSerializer::pushParent( JsonParentPtr parent )
{
	_parentStack.push( parent );

	if( _markStack.size() ) 
	{
		_markStack[_markStack.size()-1] = _markStack[_markStack.size()-1]++;
	}
}

////////////////////////////////////////////////////////////////////////////////
void JsonSerializer::popParent()
{
	if( _parentStack.size() <= 0 ) return;
	_parentStack.pop();

	if( _markStack.size() && _markStack[_markStack.size()-1] > 0 ) 
	{
		_markStack[_markStack.size()-1] = _markStack[_markStack.size()-1]--;
	}
}

////////////////////////////////////////////////////////////////////////////////
std::string JsonSerializer::toString( unsigned int indent ) const
{
	if (!_root) return std::string();

	return _root->toString( indent );
}

////////////////////////////////////////////////////////////////////////////////
void JsonSerializer::toStream( std::ostream *pos, unsigned int indent ) const
{
	if (!_root) return;

	_root->toStream( pos, indent );
}

////////////////////////////////////////////////////////////////////////////////
JsonSerializer::JsonParentPtr JsonSerializer::getParent()
{
	if (_parentStack.size() > 0) return _parentStack.top();

	if (!_root) 
	{
		_root.reset( new JsonParent ( new Object() ) );
		pushParent( _root );
		insertObjValue( "source", _sourceName );
		insertObjValue( "version", _version );
	}
	return _root;
}

////////////////////////////////////////////////////////////////////////////////
JsonSerializer::JsonParentPtr JsonSerializer::getParentConst() const
{
	if (_parentStack.size() > 0) return _parentStack.top();

	return JsonParentPtr();
}

////////////////////////////////////////////////////////////////////////////////
Object::Ptr JsonSerializer::createObj() 
{ 
	return Object::Ptr( new Poco::JSON::Object() ); 
}

////////////////////////////////////////////////////////////////////////////////
Array::Ptr JsonSerializer::createArray() 
{ 
	return Array::Ptr( new Poco::JSON::Array() ); 
}

////////////////////////////////////////////////////////////////////////////////
void JsonSerializer::setProperty( Object::Ptr obj, const std::string &name, const Var &value )
{
	if ( !obj ) return;

	obj->set( name, value );
}

////////////////////////////////////////////////////////////////////////////////
std::string JsonSerializer::getFileName()
{
	if( _fileFullPath.size() <= 0 ) return std::string();

	boost::filesystem::path path( _fileFullPath );
	return path.stem().string();
}

////////////////////////////////////////////////////////////////////////////////
std::string JsonSerializer::getFileDir()
{
	if( _fileFullPath.size() <= 0 ) return std::string();

	boost::filesystem::path path( _fileFullPath );
	return path.parent_path().string();
}

////////////////////////////////////////////////////////////////////////////////
// core
}
// lfx
}