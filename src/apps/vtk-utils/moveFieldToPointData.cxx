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

#include "vtkFieldData.h"
#include "vtkFloatArray.h"
#include "vtkDataSet.h"
#include "vtkPointData.h"
using namespace ves::xplorer::util;


int main( int argc, char* argv[] )
{
    // Possibly read in an input vtk file name and an output file...
    std::string inFileName;// = NULL;
    std::string outFileName;// = NULL;
    fileIO::processCommandLineArgs( argc, argv, "move field to point data arrays in", inFileName, outFileName );
    if( ! inFileName.c_str() )
    {
        return 1;
    }

    ///This will need to be changed to handle both vtkDataset and vtkMultigroupDataSet
    vtkDataSet* dataset = dynamic_cast<vtkDataSet*>( readVtkThing( inFileName, 1 ) );
    /*
       std::cout << "\nback in main..." << std::endl;
       std::cout << "dataset->IsA(\"vtkDataSet\") =          " << dataset->IsA("vtkDataSet") << std::endl;
       std::cout << "dataset->IsA(\"vtkPointSet\") =         " << dataset->IsA("vtkPointSet") << std::endl;
       std::cout << "dataset->IsA(\"vtkUnstructuredGrid\") = " << dataset->IsA("vtkUnstructuredGrid") << std::endl;
       std::cout << "dataset->IsA(\"vtkStructuredGrid\") =   " << dataset->IsA("vtkStructuredGrid") << std::endl;
       std::cout << "dataset->IsA(\"vtkPolyData\") =         " << dataset->IsA("vtkPolyData") << std::endl;
       std::cout << "dataset->GetDataObjectType() =          " << dataset->GetDataObjectType() << std::endl;
       std::cout << std::endl;
    */

    // if the data set has FIELD data, then move over appropriate data to point data arrays...
    char scalarName [100], vectorName [100];

    int numFieldArrays = dataset->GetFieldData()->GetNumberOfArrays();
    //std::cout << "numFieldArrays = " << numFieldArrays << std::endl;
    std::cout << std::endl;

    // If there are field data, move it (and scalar and vector) to the point data section...
    if( numFieldArrays )
    {
        int i = 0;
        for( i = 0; i < numFieldArrays; i++ )
        {
            std::cout << "moving field \"" << dataset->GetFieldData()->GetArray( i )->GetName() << "\" to the point data field" << std::endl;
            dataset->GetPointData()->AddArray( dataset->GetFieldData()->GetArray( i ) );
        }

        //std::cout << "dataset->GetPointData()->GetScalars() = " << dataset->GetPointData()->GetScalars() << std::endl;
        if( dataset->GetPointData()->GetScalars() )
        {
            vtkFloatArray* scalars = vtkFloatArray::New();
            scalars->DeepCopy( dataset->GetPointData()->GetScalars() );
            strcpy( scalarName, dataset->GetPointData()->GetScalars()->GetName() );
            std::cout << "moving scalar \"" << scalarName << "\" to the point data field" << std::endl;
            dataset->GetPointData()->RemoveArray( scalarName );
            dataset->Update();
            scalars->SetName( scalarName ); // have to do this or the name will be NULL
            dataset->GetPointData()->AddArray( scalars );
            scalars->Delete();
        }

        //std::cout << "dataset->GetPointData()->GetVectors() = " << dataset->GetPointData()->GetVectors() << std::endl;
        if( dataset->GetPointData()->GetVectors() )
        {
            vtkFloatArray*   vectors = vtkFloatArray::New();
            vectors->DeepCopy( dataset->GetPointData()->GetVectors() );
            strcpy( vectorName, dataset->GetPointData()->GetVectors()->GetName() );
            std::cout << "moving vector \"" << vectorName << "\" to the point data field" << std::endl;
            dataset->GetPointData()->RemoveArray( vectorName );
            vectors->SetName( vectorName ); // have to do this or the name will be NULL
            dataset->GetPointData()->AddArray( vectors );
            vectors->Delete();
        }

        for( i = 0; i < numFieldArrays; i++ )
        {
            dataset->GetFieldData()->RemoveArray( dataset->GetFieldData()->GetArrayName( 0 ) );
        }

        writeVtkThing( dataset, outFileName, 1 );    // 1 is binary
    }
    else
    {
        std::cout << "\nNOTE: This file did not require reformatting. No changes were made.\n" << std::endl;
    }

    inFileName.erase();//delete [] inFileName;   inFileName = NULL;
    outFileName.erase();//delete [] outFileName;  outFileName = NULL;
    dataset->Delete();
    return 0;
}


