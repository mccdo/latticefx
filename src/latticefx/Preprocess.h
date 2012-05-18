/*************** <auto-copyright.rb BEGIN do not edit this line> *************
*
* latticeFX is (C) Copyright 2011-2012 by Ames Laboratory
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Library General Public
* License as published by the Free Software Foundation; 
* version 2.1 of the License.
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
* -----------------------------------------------------------------
* Date modified: $Date$
* Version:       $Rev$
* Author:        $Author$
* Id:            $Id$
* -----------------------------------------------------------------
*
*************** <auto-copyright.rb END do not edit this line> **************/

#ifndef __LATTICEFX_PREPROCESS_H__
#define __LATTICEFX_PREPROCESS_H__ 1


#include <latticefx/Export.h>
#include <latticefx/OperationBase.h>

#include <boost/smart_ptr/shared_ptr.hpp>

#include <list>



namespace lfx {


/** \class Preprocess Preprocess.h <latticefx/Preprocess.h>
\brief Base class for Preprocessing & Caching operations.
\details TBD
*/
class LATTICEFX_EXPORT Preprocess : public lfx::OperationBase
{
public:
    Preprocess();
    Preprocess( const Preprocess& rhs );
    virtual ~Preprocess();

protected:
};

typedef boost::shared_ptr< Preprocess > PreprocessPtr;
typedef std::list< PreprocessPtr > PreprocessList;


// lfx
}


// __LATTICEFX_PREPROCESS_H__
#endif
