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

#ifndef __LFX_CORE_DB_UTILS_H__
#define __LFX_CORE_DB_UTILS_H__ 1

#include <latticefx/core/Export.h>
#include <latticefx/core/types.h>
#ifdef LFX_USE_CRUNCHSTORE
#  include <crunchstore/Persistable.h>
#endif

#include <string>
#include <list>


#ifdef LFX_USE_CRUNCHSTORE
#  define PersistPtr crunchstore::PersistablePtr
#else
#  define PersistPtr void*
#endif


namespace osg {
    class Image;
    class Array;
}

namespace lfx {
namespace core {



LATTICEFX_EXPORT bool DBUsesCrunchStore();

LATTICEFX_EXPORT void s_setPersistable( PersistPtr persist );
LATTICEFX_EXPORT PersistPtr s_getPersistable();

LATTICEFX_EXPORT DBKey generateDBKey();
LATTICEFX_EXPORT DBKey generateDBKey( const std::string& baseName, const TimeValue time=(TimeValue)0. );

LATTICEFX_EXPORT bool storeImage( const osg::Image* image, const DBKey& dbKey );
LATTICEFX_EXPORT osg::Image* loadImage( const DBKey& dbKey );

LATTICEFX_EXPORT bool storeArray( const osg::Array* array, const DBKey& dbKey );
LATTICEFX_EXPORT osg::Array* loadArray( const DBKey& dbKey );


// core
}
// lfx
}


// __LFX_CORE_DB_UTILS_H__
#endif
