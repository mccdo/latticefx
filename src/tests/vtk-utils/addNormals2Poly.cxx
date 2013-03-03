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
#include <vtkDataObject.h>
#include <vtkPolyData.h>
#include <vtkPolyDataNormals.h>
#include <vtkGeometryFilter.h>

#include <ves/xplorer/util/fileIO.h>
#include <ves/xplorer/util/readWriteVtkThings.h>

using namespace ves::xplorer::util;

int main( int argc, char *argv[] )
{    
   // If the command line contains an input vtk file name and an output file set them up.
   // Otherwise, get them from the user...
   std::string inFileName;// = NULL;
   std::string outFileName;// = NULL;
   fileIO::processCommandLineArgs( argc, argv, "add normals to", inFileName, outFileName );
   if ( ! inFileName.c_str() ) return 1;

   vtkDataObject* dataset= (readVtkThing( inFileName, 1 ));
   
   // convert vtkUnstructuredGrid to vtkPolyData    
   vtkGeometryFilter *gFilter = vtkGeometryFilter::New();
      gFilter->SetInput( dataset );

   vtkPolyDataNormals * pdWithNormals = vtkPolyDataNormals::New();
      pdWithNormals->SetInput( gFilter->GetOutput() );
      //Specify the angle that defines a sharp edge. If the difference in angle across neighboring
      //polygons is greater than this value, the shared edge is considered "sharp".    
      pdWithNormals->SetFeatureAngle( 60 );

   writeVtkThing( pdWithNormals->GetOutput(), outFileName );

   dataset->Delete();
   gFilter->Delete();
   pdWithNormals->Delete();
   inFileName.erase();//delete [] inFileName;   inFileName = NULL;
   outFileName.erase();//delete [] outFileName;  outFileName = NULL;

   return 0;
}

