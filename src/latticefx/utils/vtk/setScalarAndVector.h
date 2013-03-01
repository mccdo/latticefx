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
#ifndef LFX_setScalarAndVector_H
#define LFX_setScalarAndVector_H
/*!\file setScalarAndVector.h
setScalarAndVector API
*/
#include <latticefx/utils/vtk/Export.h>

// class declarations
class vtkDataSet;

namespace lfx
{
namespace vtk_utils
{
// function declarations
///Select a scalar and make it active
///\param dataSet the dataset containing the scalar of interest.
LATTICEFX_VTK_UTILS_EXPORT void activateScalar( vtkDataSet* dataSet );
///Select a vector and make it active
///\param dataSet the dataset containing the vector of interest.
LATTICEFX_VTK_UTILS_EXPORT void activateVector( vtkDataSet* dataSet );
}// end of util namesapce
}// end of xplorer namesapce
#endif //setScalarAndVector_H

