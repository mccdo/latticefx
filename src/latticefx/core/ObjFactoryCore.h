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

#ifndef __LFX_CORE_OBJ_FACTORY_CORE_H__
#define __LFX_CORE_OBJ_FACTORY_CORE_H__ 1

#include <latticefx/core/Export.h>
#include <latticefx/core/ObjBase.h>
#include <latticefx/core/PluginManager.h>

namespace lfx
{
namespace core
{
class LATTICEFX_EXPORT ObjFactoryCore : public IObjFactory
{
public:
	ObjFactoryCore( PluginManager *pm );

	virtual ObjBasePtr createObj( const std::string &typeName, const ObjBase::KeyDataMap &map, std::string *perr=NULL );

protected:
	virtual ObjBasePtr createPlugin( const ObjBase::KeyDataMap &map, bool *pbErr=NULL, std::string *perr=NULL );
	virtual ObjBasePtr createStandardObj( const std::string &typeName, std::string *perr=NULL );

protected:
	PluginManager *_pm;
};

// core
}
// lfx
}


// __LFX_CORE_OBJ_FACTORY_CORE_H__
#endif