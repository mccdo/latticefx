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
#include <ves/xplorer/util/cfdAccessoryFunctions.h>

#include <vtkDataSet.h>
#include <vtkDataArray.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkDoubleArray.h>
using namespace ves::xplorer::util;

int main( int argc, char* argv[] )
{
    // If the command line contains an input vtk file name, then use it.
    // Otherwise, get it from the user...
    std::string inFileName;
    std::string outFilename;
    if( argc > 2 )
    {
        inFileName.assign( argv[ 1 ] );
        outFilename.assign( argv[ 2 ] );
    }
    else  // then get filename from user...
    {
        std::string tempText( "the file to remove scalars/vectors from" );
        inFileName = fileIO::getReadableFileFromDefault( tempText, "inFile.vtk" );
        tempText.assign( "the file to write to" );
        outFilename = fileIO::getFilenameFromDefault( tempText, "outFile.vtk" );
    }

    // read the data set ("1" means print info to screen)
    vtkDataSet* dataset = readVtkThing( inFileName, 1 );

    std::string input;
    do
    {
        std::cout << "Enter scalar or vector to remove (enter exit to quit):" << std::endl;
        std::cin >> input;
        if( input != "exit" )
        {
            if( dataset->GetPointData() )
            {
                dataset->GetPointData()->RemoveArray( input.c_str() );
            }
            else if( dataset->GetCellData() )
            {
                dataset->GetCellData()->RemoveArray( input.c_str() );
            }
        }
    }
    while( input != "exit" );

    // write vtk file
    ves::xplorer::util::writeVtkThing( dataset, outFilename, 1 );

    dataset->Delete();

    return 0;
}

