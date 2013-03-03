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

#include <vtkAppendFilter.h>
#include <vtkTransformFilter.h>
#include <vtkUnstructuredGrid.h>
#include <vtkTransform.h>
#include <vtkTransformFilter.h>
using namespace ves::xplorer::util;

int main( int argc, char *argv[] )
{    
    // Read in a file containing an unstructured grid...
    std::string inFileName1;// = 0;
    std::string inFileName2;// = 0;
    std::string outFileName;// = 0;
    if (argc > 1)
    {
        //inFileName1 = new char [100];
        inFileName1.assign( argv[1] );//strcpy( inFileName1, argv[1]);

        if (argc > 2)
        {
            //inFileName2 = new char [100];
            inFileName2.assign( argv[2] );//strcpy( inFileName2, argv[2]);
        }
        else inFileName2 = fileIO::getReadableFileFromDefault( "the file to be appended", "inFile.vtk" );

        if (argc > 3)
        {
            //outFileName = new char [100];
            outFileName.assign( argv[3] );//strcpy( outFileName, argv[3]);
        }
        else outFileName = fileIO::getWritableFile( "outFile.vtk" );

        char response;
        do 
        {
            std::cout << "\nSo you want to add " << inFileName1 << " to " << inFileName2  << " to get " << outFileName << "? (y/n): ";
            std::cin >> response;
        } while (response != 'y' && response != 'Y' && response != 'n' && response != 'N');

        //if anything other than y/Y was input then get filenames from user...
        if (response != 'y' && response != 'Y') argc = 1;
    }

    if (argc == 1)  // then get filenames from user...
    {
        inFileName1 = fileIO::getReadableFileFromDefault( "the first file", "inFile.vtk" );
        inFileName2 = fileIO::getReadableFileFromDefault( "the file to be appended", "inFile.vtk" );
        outFileName = fileIO::getWritableFile( "outFile.vtk" );
    }

   ///This will need to be changed to handle both vtkDataset and vtkMultigroupDataSet
   vtkDataSet* dataset1 = dynamic_cast<vtkDataSet*>(readVtkThing( inFileName1, 1 ));
   
   ///This will need to be changed to handle both vtkDataset and vtkMultigroupDataSet
   vtkDataSet* dataset2 = dynamic_cast<vtkDataSet*>(readVtkThing( inFileName2, 1 ));
   
    // Transform the geometry 
    int B_trans;
    float rotX = 0.0, rotY = 0.0, rotZ = 0.0;
    float transX = 0.0, transY = 0.0, transZ = 0.0;

    std::cout << "\nTransform (translate/rotate) the geometry of " << inFileName2 << "? (0) No (1) Yes " << std::endl;
    std::cin >> B_trans;

    if ( B_trans )
    {
        std::cout << "\nTo translate X value : ";
        std::cin >> transX;
        
        std::cout << "To translate Y value : ";
        std::cin >> transY;

        std::cout << "To translate Z value : ";
        std::cin >> transZ;

        std::cout << "\nTo rotate X (degrees) : ";
        std::cin >> rotX;

        std::cout << "To rotate Y (degrees) : ";
        std::cin >> rotY;

        std::cout << "To rotate Z (degrees) : ";
        std::cin >> rotZ;
    }

    vtkTransform * t = vtkTransform::New();
       t->Translate( transX, transY, transZ );
       t->RotateX( rotX );
       t->RotateY( rotY );
       t->RotateZ( rotZ );

    vtkTransformFilter * tFilter = vtkTransformFilter::New();
       tFilter->SetInput( (vtkPointSet *)dataset2 );
       tFilter->SetTransform( t );
       tFilter->Update();

   vtkAppendFilter * aFilter = vtkAppendFilter::New();
       aFilter->AddInput( dataset1 );
       aFilter->AddInput( tFilter->GetOutput() );
       aFilter->Update();

  writeVtkThing( aFilter->GetOutput(), outFileName, 1 ); // one is for binary

   tFilter->Delete();
   aFilter->Delete();
   t->Delete();
   //delete [] inFileName1;  inFileName1 = NULL;
   //delete [] inFileName2;  inFileName2 = NULL;
   //delete [] outFileName;  outFileName = NULL;
   dataset1->Delete();
   dataset2->Delete();

   std::cout << "... done" << std::endl;
   std::cout << std::endl;

   return 0;
}

