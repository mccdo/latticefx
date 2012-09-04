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
#ifndef CLEAN_VTK_H
#define CLEAN_VTK_H
/*!\file cleanVTK.h
cleanVTK API
*/
#include <latticefx/utils/vtk/Export.h>
#include <string>

class vtkPointSet;

namespace lfx
{
namespace vtk_utils
{
// function declarations
///Takes in a vtkPointSet and removes vertices not used by a cell.
LATTICEFX_VTK_UTILS_EXPORT void dumpVerticesNotUsedByCells( vtkPointSet * );
///Takes in a vtkPointSet and removes vertices not used by a cell and writes it back out.
///param vtkFileName The name of the file to be written out.
LATTICEFX_VTK_UTILS_EXPORT void dumpVerticesNotUsedByCells( vtkPointSet *, std::string vtkFileName );
}// end of util namesapce
}// end of xplorer namesapce
#endif
