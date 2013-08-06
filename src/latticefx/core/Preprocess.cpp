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

#include <latticefx/core/Preprocess.h>
#include <latticefx/core/JsonSerializer.h>

namespace lfx
{
namespace core
{

Preprocess::Preprocess()
    : OperationBase( OperationBase::PreprocessCacheType ),
      _action( IGNORE_DATA )
{
}
Preprocess::Preprocess( const Preprocess& rhs )
    : OperationBase( rhs ),
      _action( rhs._action )
{
}
Preprocess::~Preprocess()
{
}

std::string Preprocess::getEnumName( ActionType e ) const
{
	switch (e)
	{
	case ADD_DATA:
		return "ADD_DATA";
	case REPLACE_DATA:
		return "REPLACE_DATA";
	}
	return "IGNORE_DATA";
}

Preprocess::ActionType Preprocess::getEnumFromName( const std::string &name ) const
{
	if( !name.compare( "ADD_DATA" )) return ADD_DATA;
	else if( !name.compare( "REPLACE_DATA" )) return REPLACE_DATA;
	return IGNORE_DATA;
}

void Preprocess::setActionType( const ActionType& action )
{
    _action = action;
}
Preprocess::ActionType Preprocess::getActionType() const
{
    return( _action );
}

void Preprocess::serializeData( JsonSerializer *json ) const
{
	// let the parent write its data
	OperationBase::serializeData( json );

	json->insertObj( Preprocess::getClassName(), true);
	json->insertObjValue( "action",  getEnumName( _action ) );
	json->popParent();
}

bool Preprocess::loadData( JsonSerializer *json, IObjFactory *pfactory, std::string *perr )
{
	// let the parent load its data
	if ( !OperationBase::loadData( json, pfactory, perr )) return false;

	// get to this classes data
	if ( !json->getObj( Preprocess::getClassName() ) )
	{
		if (perr) *perr = "Json: Failed to get Preprocess data";
		return false;
	}

	std::string name;
	json->getValue( "action", &name, getEnumName( _action ) );
	_action = getEnumFromName( name );

	json->popParent();
	return true;
}

void Preprocess::dumpState( std::ostream &os )
{
	OperationBase::dumpState( os );

	dumpStateStart( Preprocess::getClassName(), os );
	os << "_action: " << getEnumName( _action ) << std::endl;
	dumpStateEnd( Preprocess::getClassName(), os );
}

// core
}
// lfx
}
