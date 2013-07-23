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

#ifndef __LFX_CORE_OBJBASE_H__
#define __LFX_CORE_OBJBASE_H__ 1

#include <latticefx/core/Export.h>
#include <boost/shared_ptr.hpp>
#include <string>
#include <map>
#include <vector>
#include <list>

namespace lfx
{
namespace core
{

class JsonSerializer;
class IObjFactory;


class ObjBase;
typedef boost::shared_ptr<ObjBase> ObjBasePtr;

/** \class ObjBase ObjBase.h <latticefx/core/ObjBase.h>
\brief Base class for objects.
*/
class LATTICEFX_EXPORT ObjBase
{
public:
	typedef std::map< std::string, std::string > KeyDataMap;

public:
	ObjBase();

	virtual std::string getClassName() const { return "ObjBase"; }

	static ObjBasePtr loadObj( JsonSerializer *json, IObjFactory *pfactory, std::string *perr=NULL );
	static ObjBasePtr loadObj( JsonSerializer *json, IObjFactory *pfactory, unsigned int arrIdx, std::string *perr=NULL );
	static bool loadArrObjList( JsonSerializer *json, IObjFactory *pfactory, std::vector<ObjBasePtr> *pList, std::string *perr=NULL );
	template <typename TPtr, typename TObj>
	static bool loadArrListType( JsonSerializer *json, IObjFactory *pfactory, const std::string &listname, std::list<TPtr> *plist, std::string *perr=NULL);

	virtual void serialize( JsonSerializer *json ) const;

protected:
	virtual void serializeStart( JsonSerializer *json ) const;
	virtual void serializeData( JsonSerializer *json ) const;
	virtual void serializeEnd( JsonSerializer *json ) const;
	virtual bool loadData( JsonSerializer *json, IObjFactory *pfactory, std::string *perr=NULL );

	static ObjBasePtr loadObj( JsonSerializer *json, IObjFactory *pfactory, const std::string &typeName, const KeyDataMap &map, std::string *perr=NULL );
	static bool loadObjInfoFromObj( JsonSerializer *json, std::string *ptypeName, KeyDataMap *pmap, std::string *perr=NULL, bool leaveObjOnStack=true );
	static bool loadObjInfoFromArr( JsonSerializer *json, unsigned int loc, std::string *ptypeName, KeyDataMap *pmap, std::string *perr=NULL, bool leaveObjOnStack=true );
	static bool loadObjInfo( JsonSerializer *json, std::string *ptypeName, KeyDataMap *pmap, std::string *perr=NULL );

	KeyDataMap _dataMapObj; // written into the ObjBase group, here you can provide object specific info, such as plugin path to load the object from
	
};

template <typename TPtr, typename TObj>
bool ObjBase::loadArrListType( JsonSerializer *json, IObjFactory *pfactory, const std::string &listname, std::list<TPtr> *plist, std::string *perr)
{
	if( !json->getArray( listname ) )
	{
		if (perr) *perr = std::string("Json: Failed to get the array list type: ") + listname;
		return false;
	} 

	std::vector<ObjBasePtr> v;
	if( !loadArrObjList( json, pfactory, &v, perr ) )
	{
		return false;
	}
	
	for( unsigned int i=0; i<v.size(); i++ )
	{
		TPtr p = boost::dynamic_pointer_cast<TObj>( v[i] );
		if( p ) plist->push_back( p );
	}

	json->popParent();
	return true;
}


class LATTICEFX_EXPORT IObjFactory
{
public:
	IObjFactory() {};

	virtual ObjBasePtr createObj(const std::string &typeName, const ObjBase::KeyDataMap &map, std::string *perr=NULL) = 0;
};
// core
}
// lfx
}


// __LFX_CORE_OBJBASE_H__
#endif
