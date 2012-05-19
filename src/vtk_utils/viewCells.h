/*************** <auto-copyright.rb BEGIN do not edit this line> **************
 *
 * VE-Suite is (C) Copyright 1998-2012 by Iowa State University
 *
 * Original Development Team:
 *   - ISU's Thermal Systems Virtual Engineering Group,
 *     Headed by Kenneth Mark Bryden, Ph.D., www.vrac.iastate.edu/~kmbryden
 *   - Reaction Engineering International, www.reaction-eng.com
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
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
 *************** <auto-copyright.rb END do not edit this line> ***************/
#ifndef VIEWCELLS_H
#define VIEWCELLS_H
/*!\file viewCells.h
viewCells API
*/
#include <ves/VEConfig.h>

class vtkUnstructuredGrid;
class vtkDataSet;
class vtkRectilinearGrid;
class vtkActor;
class vtkFollower;
class vtkRenderer;
namespace ves
{
namespace xplorer
{
namespace util
{
///Identifies and returns all of the exterior cells of the data set that is passed in.
///\param *output pointer to the data set of cells for the extraction of exterior cells.
VE_UTIL_EXPORTS vtkUnstructuredGrid*
extractExteriorCellsOnly( vtkUnstructuredGrid* output );
///Renders the data set that is passed in.
///\param *output pointer to the data set to be rendered.
///\param shrinkFactor scaling factor for the rendering.
VE_UTIL_EXPORTS void viewCells( vtkDataSet* output,
                                const float shrinkFactor = 0.95 );
///Renders a cross section of the model halfway along the x-axis.
///\param *output pointer to the data set to be rendered.
VE_UTIL_EXPORTS void viewXSectionOfRectilinearGrid(
    vtkRectilinearGrid* output );
///Create the axes and the associated mapper and actor.
///\param *axesActor (????)Actor for the axes?
VE_UTIL_EXPORTS void GetAxesSymbol( vtkActor* axesActor );
///Create the 3D text and the associated mapper and
///follower for the axis labels.
///\param *xActor The x-axis label.
///\param *yActor The y-axis label.
///\param *zActor The z-axis label.
VE_UTIL_EXPORTS void GetAxesLabels( vtkFollower* xActor,
                                    vtkFollower* yActor, vtkFollower* zActor );
///Generates a new actor and adds it to the renderer.
///\param dataset pointer to the dataset from which the new actor will be extracted.
///\param ren1 (????)The renderer.
///\param shrinkFactor Scaling factor for the new actor
VE_UTIL_EXPORTS void AddToRenderer( vtkDataSet* dataset,
                                    vtkRenderer* ren1, const float shrinkFactor = 1.0 );
}// end of util namesapce
}// end of xplorer namesapce
}// end of ves namesapce
#endif    // VIEWCELLS_H
