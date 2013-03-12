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

#include <latticefx/utils/vtk/fileIO.h>
#include <latticefx/utils/vtk/readWriteVtkThings.h>

#include <vtkDataSet.h>
#include <vtkDataObject.h>
using namespace lfx::vtk_utils;

int main( int argc, char* argv[] )
{
    int printInfoToScreen = 0; // "1" means print info to screen

    // If the command line contains an input vtk file name and an output file,
    // set them up.  Otherwise, get them from the user...
    std::string inFileName;// = NULL;
    std::string outFileName;// = NULL;
    fileIO::processCommandLineArgs( argc, argv, "convert binary",
                                    inFileName, outFileName );
    if( ! inFileName.c_str() )
    {
        return 1;
    }
    ///This will need to be changed to handle both vtkDataset and vtkMultigroupDataSet
    vtkDataObject* dataset = ( readVtkThing( inFileName, 1 ) );
    if( printInfoToScreen )
    {
        std::cout << "\nback in main..." << std::endl;
        printWhatItIs( dataset );
    }

    writeVtkThing( dataset, outFileName );

    dataset->Delete();
    inFileName.erase();//delete [] inFileName;   inFileName = NULL;
    outFileName.erase();//delete [] outFileName;  outFileName = NULL;

    return 0;
}

