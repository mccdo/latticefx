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
#include <cstdlib>

#include <vtk_utils/readWriteVtkThings.h>

#include <vtkDataSet.h>
#include <vtkDataObject.h>
#include <vtkDataSetReader.h>
#include <vtkInformationStringKey.h>
#include <vtkPolyData.h>
#include <vtkPolyDataWriter.h>
#include <vtkRectilinearGrid.h>
#include <vtkRectilinearGridWriter.h>
#include <vtkStructuredGrid.h>
#include <vtkStructuredGridWriter.h>
#include <vtkUnstructuredGrid.h>
#include <vtkUnstructuredGridWriter.h>
#include <vtkXMLUnstructuredGridReader.h>

#include <vtk_utils/VTKFileHandler.h>
#include <vtk_utils/DataObjectHandler.h>
#include <vtk_utils/ComputeDataObjectBoundsCallback.h>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

using namespace lfx::vtk_utils;

///////////////////////////////////////////////////////////////////////////////
void lfx::vtk_utils::printWhatItIs( vtkDataObject * dataSet )
{
    if( dataSet == NULL )
    {
        std::cout << "\tdataSet == NULL" << std::endl;
        return;
    }
    std::cout << dataSet->GetClassName() << std::endl;
}
///////////////////////////////////////////////////////////////////////////////
void lfx::vtk_utils::printBounds( vtkDataObject* dataObject )
{
    double bounds[6];
    DataObjectHandler dataObjectHandler;
    ComputeDataObjectBoundsCallback* boundsCallback =
        new ComputeDataObjectBoundsCallback();
    dataObjectHandler.SetDatasetOperatorCallback( boundsCallback );
    dataObjectHandler.OperateOnAllDatasetsInObject( dataObject );
    std::cout << "Geometry bounding box information..." << std::endl;
    boundsCallback->GetDataObjectBounds( bounds );
    std::cout << "\tx-min = \t" << bounds[0]
    << "\tx-max = \t" << bounds[1] << std::endl;
    std::cout << "\ty-min = \t" << bounds[2]
    << "\ty-max = \t" << bounds[3] << std::endl;
    std::cout << "\tz-min = \t" << bounds[4]
    << "\tz-max = \t" << bounds[5] << std::endl;

    if( boundsCallback )
    {
        delete boundsCallback;
        boundsCallback = 0;
    }
}
///////////////////////////////////////////////////////////////////////////////
vtkDataObject* lfx::vtk_utils::readVtkThing( 
    std::string vtkFilename, int printFlag )
{
    try
    {
        if( !boost::filesystem::exists( vtkFilename ) )
        {
            std::cout << "|\tFile " << vtkFilename 
            << " does not exist." << std::endl;
            return 0;
        }
    }
    catch( ... )
    {
        std::cout << "|\tFile " << vtkFilename 
            << " does not exist." << std::endl;
        return 0;
    }

    VTKFileHandler fileReader;
    vtkDataObject* temp = fileReader.GetDataSetFromFile( vtkFilename );
    if( printFlag )
    {
        lfx::vtk_utils::printBounds( temp );
        lfx::vtk_utils::printWhatItIs( temp );
    }
    return temp;
}
///////////////////////////////////////////////////////////////////////////////
bool lfx::vtk_utils::writeVtkThing( 
    vtkDataObject* vtkThing, std::string vtkFilename, int binaryFlag )
{
    VTKFileHandler fileWriter;
    if( !binaryFlag )
        fileWriter.SetOutFileWriteMode( VTKFileHandler::CFD_ASCII );
    return fileWriter.WriteDataSet( vtkThing, vtkFilename );
}

