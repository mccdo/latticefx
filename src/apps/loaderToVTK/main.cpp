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
#include <iostream>

#include <latticefx/translators/vtk/DataLoader.h>

#include <vtkDataObject.h>

using namespace lfx::vtk_translator;
///////////////////////////////////////////////////////////////////
//Example of how to read dicom files and create vtk files for the//
//dicom data                                                     //
///////////////////////////////////////////////////////////////////
int main( int argc, char** argv )
{
    DataLoader loader;
    loader.SetInputData( "something", "somedir" );
    vtkDataObject* tempData = loader.GetVTKDataSet( argc, argv );
    if( tempData )
    {
        tempData->Delete();
    }
    return 0;
}


/** \page AppLoaderToVTK Application loaderToVTK

<h2>Example Usage</h2>

In the data/vtk_particles directory, run this command:
\code
loaderToVTK -singleFile binary.case -o . -loader ens -w file
\endcode

View the output with \ref TestVtkTimeseriesPoints "vtk-timeseries-points".
*/
