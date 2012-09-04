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


namespace lfx {
namespace core {


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

void Preprocess::setActionType( const ActionType& action )
{
    _action = action;
}
Preprocess::ActionType Preprocess::getActionType() const
{
    return( _action );
}


// core
}
// lfx
}
