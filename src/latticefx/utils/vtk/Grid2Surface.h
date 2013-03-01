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
#ifndef LFX_CFD_GRID_2_SURFACE_H
#define LFX_CFD_GRID_2_SURFACE_H
/*!\file Grid2Surface.h
*Grid to surface converting API
*/

class vtkDataObject;
class vtkPolyData;

#include <latticefx/utils/vtk/Export.h>

namespace lfx
{
namespace vtk_utils
{
// function declarations
///Reads in a grid (vtkDataSet) and returns it as a surface (vtkPolyData).
LATTICEFX_VTK_UTILS_EXPORT vtkPolyData * Grid2Surface( vtkDataObject *dataSet, float deciVal );
}// end of util namesapce
}// end of xplorer namesapce
#endif
