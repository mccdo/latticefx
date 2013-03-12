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
#include <vector>

#include <ves/xplorer/util/fileIO.h>
#include <ves/xplorer/util/readWriteVtkThings.h>

#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkDataObject.h>
#include <vtkCompositeDataSet.h>

using namespace ves::xplorer::util;

void removeVtkPointData( vtkDataObject* dataObject )
{

    // if there are data arrays, count the number of arrays
    int numPDArrays;
    if( dataObject->IsA( "vtkCompositeDataSet" ) )
    {
        vtkCompositeDataSet* mgd = dynamic_cast<vtkCompositeDataSet*>( dataObject );

        //count number of arrays from the first data set in the mgd, use this as the superset of
        //data arrays of point data
        numPDArrays = mgd->GetFieldData()->GetNumberOfArrays();
    }
    else
    {
        numPDArrays = dynamic_cast<vtkDataSet*>( dataObject )
                      ->GetPointData()->GetNumberOfArrays();
    }

    std::cout << "numPDArrays = " << numPDArrays << std::endl;
    if( numPDArrays > 0 )
    {
        std::vector< std::string > names;//char **names = new char * [numPDArrays];
        for( int i = 0; i < numPDArrays; i++ )
        {
            //get the names of the arrays
            if( dataObject->IsA( "vtkCompositeDataSet" ) )
            {
                vtkCompositeDataSet* mgd = dynamic_cast<vtkCompositeDataSet*>( dataObject );
                names.push_back( mgd->GetFieldData()->GetArray( i )->GetName() );
            }
            else
            {
                vtkDataSet* dataset = dynamic_cast<vtkDataSet*>( dataObject );
                names.push_back( dataset->GetPointData()->GetArray( i )->GetName() );
            }
        }

        for( int i = 0; i < numPDArrays; i++ )
        {
            char response;
            do
            {
                std::cout << "Do you want parameter \"" << names[i]
                          << "\" retained in the flowdata file? [y/n]: ";
                std::cin >> response;
            }
            while( response != 'y' && response != 'Y' && response != 'n' && response != 'N' );

            // go to next scalar if anything other than n/N was input...
            if( response != 'n' && response != 'N' )
            {
                continue;
            }

            std::cout << "Removing array :" << names[i].c_str() << std::endl;
            vtkDataSet* dataset = dynamic_cast<vtkDataSet*>( dataObject );
            dataset->GetPointData()->RemoveArray( names[i].c_str() );
        }

        for( int i = 0; i < numPDArrays; i++ )
        {
            //delete [] names[i];
            //names[i] = NULL;
        }
        names.clear();//delete [] names;
        //names = NULL;
    }
    return;
}

int main( int argc, char* argv[] )
{
    // If the command line contains an input vtk file name and an output file set them up.
    // Otherwise, get them from the user...
    std::string inFileName;// = NULL;
    std::string outFileName;// = NULL;
    fileIO::processCommandLineArgs( argc, argv, "remove point data parameters from", inFileName, outFileName );
    if( ! inFileName.c_str() )
    {
        return 1;
    }

    vtkDataObject* dataset = ( readVtkThing( inFileName, 1 ) );
    removeVtkPointData( dataset );

    writeVtkThing( dataset, outFileName );

    dataset->Delete();

    return 0;
}

