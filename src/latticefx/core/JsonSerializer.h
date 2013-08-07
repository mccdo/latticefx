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

#ifndef __LFX_CORE_JSON_SERIALIZER_H__
#define __LFX_CORE_JSON_SERIALIZER_H__ 1

#include <latticefx/core/Export.h>

#include <Poco/Foundation.h>
#if( POCO_OS == POCO_OS_MAC_OS_X )
// Include this first to work around OSX gcc 4.2 build issue
// Put isinf(), isnan() in global namespace, which Poco assumes.
#include <cmath>
using std::isinf;
using std::isnan;
#endif

#include <Poco/JSON/Object.h>
#include <Poco/JSON/JSONException.h>
#include <Poco/Dynamic/Var.h>
#include <boost/smart_ptr/shared_ptr.hpp>

#include <stack>

//typedef boost::shared_ptr<Poco::JSON::Object> JsonObjectPtr;

namespace lfx
{
namespace core
{


/** \class JsonSerializer JsonSerializer.h <latticefx/core/JsonSerializer.h>
\brief Base class for Serializing to Json
\details TBD
*/
class LATTICEFX_EXPORT JsonSerializer
{
protected:
	class JsonParent
	{
	public:
		JsonParent() {};
		JsonParent( Poco::JSON::Object::Ptr obj ) { _obj = obj; }
		JsonParent( Poco::JSON::Array::Ptr arr ) { _arr = arr; }

		bool isArray() const { if ( !_arr ) return false; return true; }
		bool isObj() const { if ( !_obj ) return false; return true; }
		size_t size() const;

		bool set ( const Poco::Dynamic::Var &var );
		bool set ( const std::string &name, const Poco::Dynamic::Var &var );
		bool get( const std::string &name, Poco::Dynamic::Var *var ) const;
		bool get( unsigned int idx, Poco::Dynamic::Var *var ) const;
		bool getKey( unsigned int idx, std::string *pkey ) const;

		std::string toString( unsigned int indent=2 ) const;
		void toStream( std::ostream *pos, unsigned int indent=2 ) const;

		Poco::JSON::Object::Ptr _obj;
		Poco::JSON::Array::Ptr _arr;
	};
	typedef boost::shared_ptr< JsonParent > JsonParentPtr;

public:
    JsonSerializer( const char *fileName="", int version=1, const char *srcName="latticefx" );

	// of the current parent
	bool isObj();
	bool isArray();
	size_t size();

	bool load( const std::string &json );
	bool getObj( const std::string &name, bool push=true );
	bool getObj( unsigned int idx, bool push=true );
	bool getArray( const std::string &name, bool push=true );
	bool getObjKey( unsigned int idx, std::string *pKey ) const;

	template <typename T> 
	bool getValue( const std::string &name, T *pvalue, T def ) const;
	bool getValue( const std::string &name, std::string *pvalue, std::string def="" ) const;
	bool getValue( const std::string &name, int *pvalue, int def=0 ) const;
	bool getValue( const std::string &name, unsigned int *pvalue, unsigned int def=0 ) const;
	bool getValue( const std::string &name, double *pvalue, double def=0 ) const;
	bool getValue( const std::string &name, float *pvalue, float def=0 ) const;
	bool getValue( const std::string &name, bool *pvalue, bool def=false ) const;
	template <typename T> 
	bool getValue( unsigned int idx, T *pvalue, T def ) const;
	bool getValue( unsigned int idx, std::string *pvalue, std::string def="" ) const;
	bool getValue( unsigned int idx, int *pvalue, int def=0 ) const;
	bool getValue( unsigned int idx, unsigned int *pvalue, unsigned int def=0 ) const;
	bool getValue( unsigned int idx, double *pvalue, double def=0 ) const;
	bool getValue( unsigned int idx, float *pvalue, float def=0 ) const;
	bool getValue( unsigned int idx, bool *pvalue, bool def=false ) const;

	void insertObj( const std::string &name, bool push=true );
	void insertArray( const std::string &name, bool push=true );
	bool insertObjValue( const std::string &name, const std::string &value );
	bool insertObjValue( const std::string &name, const char *value );
	bool insertObjValue( const std::string &name, int value );
	bool insertObjValue( const std::string &name, unsigned int value );
	bool insertObjValue( const std::string &name, double value );
	bool insertObjValue( const std::string &name, float value );
	bool insertObjValue( const std::string &name, bool value );
	bool insertArrValue( const std::string &value );
	bool insertArrValue( const char *value );
	bool insertArrValue( int value );
	bool insertArrValue( double value );

	void markParentStack();
	void popMark();
	void popParent();

	std::string toString( unsigned int indent=2 ) const;
	void toStream( std::ostream *pos, unsigned int indent=2 ) const;

	void setFileName( const char *file ) { _fileName = file; }
	const char* getFileName() { return _fileName.c_str(); }

protected:
	bool getObjArr( const std::string &name, bool push, bool obj);
	
	void insertObj( const std::string &name, Poco::JSON::Object::Ptr obj, bool push=false );
	void insertArray( const std::string &name, Poco::JSON::Array::Ptr arr, bool push=false );
	void insertParentData( const std::string &name, JsonParentPtr newParent, const Poco::Dynamic::Var &varParent, bool push=true);

	bool insertObjValue( const std::string &name, const Poco::Dynamic::Var &value );
	bool insertArrValue( const Poco::Dynamic::Var &value );

	void pushParent( JsonParentPtr parent );
	JsonParentPtr getParent();
	JsonParentPtr getParentConst() const;

	static Poco::JSON::Object::Ptr createObj();
	static Poco::JSON::Array::Ptr createArray();
	static void setProperty( Poco::JSON::Object::Ptr obj, const std::string &name, const Poco::Dynamic::Var &value );

protected:
	JsonParentPtr _root;
	std::stack< JsonParentPtr > _parentStack;
	std::vector< unsigned int > _markStack;
	int _version;
	std::string _sourceName;
	std::string _fileName;
};

typedef boost::shared_ptr< JsonSerializer > JsonSerializerPtr;

template <typename T>
bool JsonSerializer::getValue( const std::string &name, T *pvalue, T def ) const
{
	JsonParentPtr parent = getParentConst();
	if ( !parent ) return false;

	Poco::Dynamic::Var v;
	bool ret = parent->get( name, &v );

	if (ret) *pvalue = v.convert<T>();
	else *pvalue = def;

	return ret;
}

template <typename T>
bool JsonSerializer::getValue( unsigned int idx, T *pvalue, T def ) const
{
	JsonParentPtr parent = getParentConst();
	if ( !parent ) return false;

	Poco::Dynamic::Var v;
	bool ret = parent->get( idx, &v );

	if (ret) *pvalue = v.convert<T>();
	else *pvalue = def;

	return ret;
}

// core
}
// lfx
}


// __LFX_CORE_JSON_SERIALIZER_H__
#endif
