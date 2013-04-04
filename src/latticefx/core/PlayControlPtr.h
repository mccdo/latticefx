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
#ifndef LFX_PLAY_CONTROL_PTR_H
#define LFX_PLAY_CONTROL_PTR_H

#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
//#include <boost/scoped_ptr.hpp>

/**
 * \file
 *
 * Include this file to get a forward declaration of the type
 * lfx::core::PlayControl and its pointer types.
 * For the full declaration of lfx::core::PlayControl
 * latticefx/core/PlayControl.h must be included, too.
 */

namespace lfx
{
namespace core
{
class PlayControl;
/// Typedef for the SmartPtr types.
//typedef ves::util::ClassPtrDef<DataSet>::type  DataSetPtr;
//typedef ves::util::SharedPtrDef<DataSet>::type DataSetSharedPtr;
// WeakPtrDef used for getting around circular references only.
typedef boost::weak_ptr< PlayControl > PlayControlWeakPtr;
//typedef ves::util::ScopedPtrDef<DataSet>::type DataSetScopedPtr;
typedef boost::shared_ptr< PlayControl > PlayControlPtr;

}
}
#endif
