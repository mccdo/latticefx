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
#ifndef LFX_VIEWCELLS_H
#define LFX_VIEWCELLS_H
/*!\file viewCells.h
viewCells API
*/
#include <latticefx/utils/vtk/Export.h>

class vtkUnstructuredGrid;
class vtkDataSet;
class vtkRectilinearGrid;
class vtkActor;
class vtkFollower;
class vtkRenderer;
namespace lfx
{
namespace vtk_utils
{
///Identifies and returns all of the exterior cells of the data set that is passed in.
///\param *output pointer to the data set of cells for the extraction of exterior cells.
LATTICEFX_VTK_UTILS_EXPORT vtkUnstructuredGrid*
extractExteriorCellsOnly( vtkUnstructuredGrid* output );
///Renders the data set that is passed in.
///\param *output pointer to the data set to be rendered.
///\param shrinkFactor scaling factor for the rendering.
LATTICEFX_VTK_UTILS_EXPORT void viewCells( vtkDataSet* output,
        const float shrinkFactor = 0.95 );
///Renders a cross section of the model halfway along the x-axis.
///\param *output pointer to the data set to be rendered.
LATTICEFX_VTK_UTILS_EXPORT void viewXSectionOfRectilinearGrid(
    vtkRectilinearGrid* output );
///Create the axes and the associated mapper and actor.
///\param *axesActor (????)Actor for the axes?
LATTICEFX_VTK_UTILS_EXPORT void GetAxesSymbol( vtkActor* axesActor );
///Create the 3D text and the associated mapper and
///follower for the axis labels.
///\param *xActor The x-axis label.
///\param *yActor The y-axis label.
///\param *zActor The z-axis label.
LATTICEFX_VTK_UTILS_EXPORT void GetAxesLabels( vtkFollower* xActor,
        vtkFollower* yActor, vtkFollower* zActor );
///Generates a new actor and adds it to the renderer.
///\param dataset pointer to the dataset from which the new actor will be extracted.
///\param ren1 (????)The renderer.
///\param shrinkFactor Scaling factor for the new actor
LATTICEFX_VTK_UTILS_EXPORT void AddToRenderer( vtkDataSet* dataset,
        vtkRenderer* ren1, const float shrinkFactor = 1.0 );
}// end of util namesapce
}// end of xplorer namesapce
#endif    // VIEWCELLS_H
