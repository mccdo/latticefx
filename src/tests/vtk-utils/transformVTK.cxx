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
#include <iostream>

#include <ves/xplorer/util/fileIO.h>
#include <ves/xplorer/util/readWriteVtkThings.h>

#include <vtkUnstructuredGrid.h>
#include <vtkTransform.h>
#include <vtkTransformFilter.h>
#include <vtkDataSet.h>
#include <vtkPointSet.h>

using namespace ves::xplorer::util;

int main( int argc, char *argv[] )
{    
   // Possibly read in an input vtk file name and an output file...
    std::string inFileName;// = NULL;
    std::string outFileName;// = NULL;
   fileIO::processCommandLineArgs( argc, argv, "transform file", inFileName, outFileName );
   if ( ! inFileName.c_str() ) return 1;

   ///This will need to be changed to handle both vtkDataset and vtkMultigroupDataSet
   vtkDataSet * dataset = dynamic_cast<vtkDataSet*>(readVtkThing( inFileName, 1 )); // "1" means print info to screen
   if ( ! dataset->IsA("vtkPointSet") )
   { 
      inFileName.erase();//delete [] inFileName;   inFileName = NULL;
      outFileName.erase();//delete [] outFileName;  outFileName = NULL;
      return 1;
   }

   float rotX, rotY, rotZ;
   float transX, transY, transZ;

   int arg = 3;
   if ( argc == 9 )
   {   
      rotX   = (float)atof( argv[ arg++ ] );
      rotY   = (float)atof( argv[ arg++ ] );
      rotZ   = (float)atof( argv[ arg++ ] );
      transX = (float)atof( argv[ arg++ ] );
      transY = (float)atof( argv[ arg++ ] );
      transZ = (float)atof( argv[ arg++ ] );
      std::cout << "Using commandline-set extents..." << std::endl;
      std::cout << "\trotX: " << rotX << std::endl;
      std::cout << "\trotY: " << rotY << std::endl;
      std::cout << "\trotZ: " << rotZ << std::endl;
      std::cout << "\ttransX: " << transX << std::endl;
      std::cout << "\ttransY: " << transY << std::endl;
      std::cout << "\ttransZ: " << transZ << std::endl;
   }
   else
   {
      std::cout << "\nTo rotate X (degrees) : ";
      std::cin >> rotX;

      std::cout << "To rotate Y (degrees) : ";
      std::cin >> rotY;

      std::cout << "To rotate Z (degrees) : ";
      std::cin >> rotZ;

      std::cout << "\nTo translate X value : ";
      std::cin >> transX;
      std::cout << "To translate Y value : ";
      std::cin >> transY;
      std::cout << "To translate Z value : ";
      std::cin >> transZ;
   }

   // Transform the geometry 
   vtkTransform * aTransform = vtkTransform::New();
   aTransform->RotateX( rotX );
   aTransform->RotateY( rotY );
   aTransform->RotateZ( rotZ );

   // Create a translation matrix and concatenate it with the current
   // transformation according to PreMultiply or PostMultiply semantics
   aTransform->Translate( transX, transY, transZ );

   // Transformation 
   vtkTransformFilter *transFilter = vtkTransformFilter::New();
   //        vtkPointSet* ptSet = vtkPointSet::SafeDownCast( dataset );
   //        if ( ptSet == NULL ) std:cout << "SafeDownCast to a vtkPointSet failed";
   transFilter->SetInput( (vtkPointSet *)dataset );
   transFilter->SetTransform( aTransform );

   writeVtkThing( transFilter->GetOutput(), outFileName, 1 ); // one is for binary

   transFilter->Delete();
   aTransform->Delete();
   inFileName.erase();//delete [] inFileName;   inFileName = NULL;
   outFileName.erase();//delete [] outFileName;  outFileName = NULL;
   dataset->Delete();
   return 0;
}

