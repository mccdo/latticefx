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

#include <latticefx/core/RTPOperation.h>
#include <latticefx/core/JsonSerializer.h>

namespace lfx
{
namespace core
{

RTPOperation::RTPOperation( const RTPOpType rtpOpType )
    : OperationBase( OperationBase::RunTimeProcessingType ),
      _rtpOpType( rtpOpType )
{
}
RTPOperation::RTPOperation( const RTPOperation& rhs )
    : OperationBase( rhs ),
      _rtpOpType( rhs._rtpOpType )
{
}
RTPOperation::~RTPOperation()
{
}

std::string RTPOperation::getEnumName( RTPOpType e ) const
{
	switch (e)
	{
	case Mask:
		return "Mask";
	case Filter:
		return "Filter";
	case Channel:
		return "Channel";
	}	
	return "Undefined";
}

RTPOperation::RTPOpType RTPOperation::getEnumFromName( const std::string &name ) const
{
	if( !name.compare( "Mask" )) return Mask;
	else if( !name.compare( "Filter" )) return Filter;
	else if( !name.compare( "Channel" )) return Channel;
	return Undefined;
}

void RTPOperation::serializeData( JsonSerializer *json ) const
{
	// let the parent write its data
	OperationBase::serializeData( json );

	json->insertObj( RTPOperation::getClassName(), true);
	json->insertObjValue( "rtpOpType",  getEnumName( _rtpOpType ) );
	json->popParent();
}

bool RTPOperation::loadData( JsonSerializer *json, IObjFactory *pfactory, std::string *perr )
{
	// let the parent load its data
	if ( !OperationBase::loadData( json, pfactory, perr )) return false;

	// get to this classes data
	if ( !json->getObj( RTPOperation::getClassName() ) )
	{
		if (perr) *perr = "Json: Failed to get RTPOperation data";
		return false;
	}

	std::string name;
	json->getValue( "rtpOpType", &name, getEnumName( _rtpOpType ) );
	_rtpOpType = getEnumFromName( name );

	json->popParent();
	return true;
}

void RTPOperation::dumpState( std::ostream &os )
{
	OperationBase::dumpState( os );

	dumpStateStart( RTPOperation::getClassName(), os );
	os << "_rtpOpType: " << getEnumName( _rtpOpType ) << std::endl;
	dumpStateEnd( RTPOperation::getClassName(), os );
}

// core
}
// lfx
}
