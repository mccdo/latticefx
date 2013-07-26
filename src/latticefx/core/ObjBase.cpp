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

#include <latticefx/core/ObjBase.h>


namespace lfx
{
namespace core
{

////////////////////////////////////////////////////////////////////////////////
ObjBase::ObjBase()
{
}
////////////////////////////////////////////////////////////////////////////////
ObjBase::ObjBase( const ObjBase& rhs )
    : _dataMapObj( rhs._dataMapObj )
{
}
////////////////////////////////////////////////////////////////////////////////
ObjBase::~ObjBase()
{
}

////////////////////////////////////////////////////////////////////////////////
ObjBasePtr ObjBase::loadObj( JsonSerializer *json, IObjFactory *pfactory, std::string *perr )
{
	std::string typeName;
	KeyDataMap map;
	if( !loadObjInfoFromObj( json, &typeName, &map, perr, true ) ) return ObjBasePtr();

	ObjBasePtr p = loadObj( json, pfactory, typeName, map, perr );

	json->popParent(); // remove object from the stack
	return p; // success
}

////////////////////////////////////////////////////////////////////////////////
ObjBasePtr ObjBase::loadObj( JsonSerializer *json, IObjFactory *pfactory, unsigned int arrIdx, std::string *perr )
{
	std::string typeName;
	KeyDataMap map;
	if( !loadObjInfoFromArr( json, arrIdx, &typeName, &map, perr, true ) ) return ObjBasePtr();

	ObjBasePtr p = loadObj( json, pfactory, typeName, map, perr );

	json->popParent(); // remove object from the stack
	return p; // success
}

////////////////////////////////////////////////////////////////////////////////
ObjBasePtr ObjBase::loadObj( JsonSerializer *json, IObjFactory *pfactory, const std::string &typeName, const KeyDataMap &map, std::string *perr )
{
	ObjBasePtr p = pfactory->createObj( typeName, map, perr );

	if( !p )
	{
		return p; // failed to create the object
	}

	p->_dataMapObj = map;

	// get to the data
	if( !json->getObj( "data" ) )
	{
		if (perr) *perr = "Json: Failed to get data object";
		return ObjBasePtr();
	}

	if( !p->loadData( json, pfactory, perr ) )
	{
	
		p = ObjBasePtr(); // failed to load the data
	}

	json->popParent(); // pop data
	return p; // success
}

////////////////////////////////////////////////////////////////////////////////
bool ObjBase::loadData( JsonSerializer *json, IObjFactory *pfactory, std::string *perr )
{
	if ( !json->getObj( "ObjBase" ) )
	{
		if (perr) *perr = "Json: Failed to get ObjBase data";
		return false;
	}

	// currently no data, but in the future load data elements now
	// child classes should follow this model, call parents loadData first then load its object data
	// see serializeData

	json->popParent();
	return true;
}

////////////////////////////////////////////////////////////////////////////////
bool ObjBase::loadObjInfoFromObj( JsonSerializer *json, std::string *ptypeName, KeyDataMap *pmap, std::string *perr, bool leaveObjOnStack )
{
	if ( !json->getObj( "object" ) )
	{
		if (perr) *perr = "Json: Failed to get object object";
		return false;
	}

	if( !loadObjInfo( json, ptypeName, pmap, perr ) )
	{
		json->popParent();
		return false;
	}

	if ( !leaveObjOnStack )  json->popParent(); // object
	return true;
}

////////////////////////////////////////////////////////////////////////////////
bool ObjBase::loadObjInfoFromArr( JsonSerializer *json, unsigned int loc, std::string *ptypeName, KeyDataMap *pmap, std::string *perr, bool leaveObjOnStack )
{
	if ( !json->getObj( loc ) )
	{
		if (perr) *perr = "Json: Failed to get object object";
		return false;
	}

	if( !loadObjInfo( json, ptypeName, pmap, perr ) )
	{
		json->popParent();
		return false;
	}

	if ( !leaveObjOnStack )  json->popParent(); // object
	return true;
}

////////////////////////////////////////////////////////////////////////////////
bool ObjBase::loadObjInfo( JsonSerializer *json, std::string *ptypeName, KeyDataMap *pmap, std::string *perr )
{
	if ( !json->getObj( "objectInfo" ) )	
	{
		if (perr) *perr = "Json: Failed to get objectInfo object";
		return false;
	}

	if ( !json->getValue( "typeName", ptypeName ) )
	{
		if (perr) *perr = "Json: Failed to get typeName value";

		json->popParent();
		return false;
	}

	if ( !json->getObj( "keyDataMap" ) )
	{
		if (perr) *perr = "Json: Failed to get keyDataMap object";

		json->popParent();
		return false;
	}

	for (unsigned int i=0; i<(unsigned int)json->size(); i++)
	{
		std::string key, value;
		if( !json->getObjKey( i, &key ) ) continue;
		if ( !json->getValue( key, &value ) ) continue;

		(*pmap)[key] = value;
	}

	json->popParent(); // keyDataMap
	json->popParent(); // objectInfo

	return true;

	/*
	if ( !json->getObj( "object" ) )
	{
		if (perr) *perr = "Json: Failed to get object object";
		return false;
	}

	if ( !json->getObj( "objectInfo" ) )	
	{
		if (perr) *perr = "Json: Failed to get objectInfo object";

		json->popParent();
		return false;
	}

	if ( !json->getValue( "typeName", ptypeName ) )
	{
		if (perr) *perr = "Json: Failed to get typeName value";

		json->popParent();
		json->popParent();
		return false;
	}

	if ( !json->getObj( "keyDataMap" ) )
	{
		if (perr) *perr = "Json: Failed to get keyDataMap object";

		json->popParent();
		json->popParent();
		return false;
	}

	for (unsigned int i=0; i<(unsigned int)json->size(); i++)
	{
		std::string key, value;
		if( !json->getObjKey( i, &key ) ) continue;
		if ( !json->getValue( key, &value ) ) continue;

		(*pmap)[key] = value;
	}

	json->popParent(); // keyDataMap
	json->popParent(); // objectInfo

	if ( !leaveObjOnStack )  json->popParent(); // object
	return true;
	*/
}

////////////////////////////////////////////////////////////////////////////////
bool ObjBase::loadArrObjList( JsonSerializer *json, IObjFactory *pfactory, std::vector<ObjBasePtr> *pList, std::string *perr )
{
	size_t sz = json->size();
	if( sz == 0 ) return true; // empty list but no error

	for( unsigned int i=0; i<(unsigned int)sz; i++ )
	{
		ObjBasePtr p = loadObj( json, pfactory, i, perr );
		if( !p ) return false;

		pList->push_back( p );
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////
void ObjBase::serialize( JsonSerializer *json ) const
{
	serializeStart( json );
	serializeData( json );
	serializeEnd( json );
}

////////////////////////////////////////////////////////////////////////////////
void ObjBase::serializeStart( JsonSerializer *json ) const
{
	json->insertObj( "object", true );

	// write out the object info, this information is helpful for knowing what type of object to load or create
	//
	json->insertObj( "objectInfo", true );
	json->insertObjValue( "typeName", getClassName() );
	json->insertObj( "keyDataMap", true );
	KeyDataMap::const_iterator it =  _dataMapObj.begin();
	while( it != _dataMapObj.end() )
	{
		json->insertObjValue( it->first, it->second );
		it++;
	}
	json->popParent();
	json->popParent();

	// insert a data object, all data will be written under this node,
	// each derived class should wrap its data in its own named object
	//
	json->insertObj( "data", true );
}

////////////////////////////////////////////////////////////////////////////////
void ObjBase::serializeData( JsonSerializer *json ) const
{
	// currently no data, but put the object in for future use
	json->insertObj( "ObjBase", true );
	json->popParent();
}

////////////////////////////////////////////////////////////////////////////////
void ObjBase::serializeEnd( JsonSerializer *json ) const
{
	json->popParent(); // data
	json->popParent(); // object
} 

// core
}
// lfx
}
