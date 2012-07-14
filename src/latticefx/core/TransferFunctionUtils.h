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

#ifndef __LFX_CORE_TRANSFER_FUNCTION_UTILS_H__
#define __LFX_CORE_TRANSFER_FUNCTION_UTILS_H__ 1


#include <latticefx/core/Export.h>

#include <string>


namespace osg {
    class Image;
}


namespace lfx {
namespace core {


/** \defgroup TransferFunctionUtils Utilities for transfer functions
*/
/**@{*/


/** \def LFX_ALPHA_CONSTANT
\brief Set all alpha values to the \c alpha parameter. */
#define LFX_ALPHA_CONSTANT 0
/** \def LFX_ALPHA_RAMP_0_TO_1
\brief Set alpha values to a linear ramp from 0.0 to 1.0. */
#define LFX_ALPHA_RAMP_0_TO_1 1
/** \def LFX_ALPHA_RAMP_1_TO_0
\brief Set alpha values to a linear ramp from 1.0 to 0.0. */
#define LFX_ALPHA_RAMP_1_TO_0 2

/** \brief Load a transfer function from a .dat file.
\details The .dat file format is described here:
http://local.wasp.uwa.edu.au/~pbourke/texture_colour/colourramp/
It is essentially a text file consisting of an unsigned char index
followed by an unsigned char rgb triple. Multiple unsigned char quadruples
may appear on a line.

Future work: Lines that begin with '#' are skipped. They may be used as
comments. THIS IS NOT YET IMPLEMENTED.

This function reads the .dat file into an osg::Image with one dimension. */
LATTICEFX_EXPORT osg::Image* loadImageFromDat( const std::string& fileName,
    const unsigned int alphaPolicy=LFX_ALPHA_CONSTANT, float alpha=1.f );


/**@}*/


// core
}
// lfx
}


// __LFX_CORE_TRANSFER_FUNCTION_UTILS_H__
#endif
