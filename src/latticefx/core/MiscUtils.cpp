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

#include <latticefx/core/MiscUtils.h>
#include <math.h>

namespace lfx
{
namespace core
{

bool MiscUtils::is_close( double d1, double d2, double eps )
{
	return ( fabs( d1 - d2 ) < eps );
}

bool MiscUtils::is_close( float f1, float f2, float eps )
{
	return is_close( (double)f1, (double)f2, (double) eps );
}

bool MiscUtils::isnot_close( double d1, double d2, double eps )
{
	return !is_close( d1, d2, eps );
}

bool MiscUtils::isnot_close( float f1, float f2, float eps )
{
	return isnot_close( (double)f1, (double)f2, (double) eps );
}

// core
}
// lfx
}
