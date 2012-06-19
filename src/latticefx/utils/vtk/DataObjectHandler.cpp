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
#include <latticefx/utils/vtk/DataObjectHandler.h>

#include <vtkDataSet.h>
#include <vtkDataObject.h>
#include <vtkUnstructuredGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkStructuredPoints.h>
#include <vtkRectilinearGrid.h>
#include <vtkCellDataToPointData.h>
#include <vtkCellData.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkCompositeDataSet.h>
#include <vtkCompositeDataIterator.h>

#include <vtkXMLUnstructuredGridWriter.h>

#include <iostream>

using namespace lfx::vtk_utils;

DataObjectHandler::DataObjectHandler()
        : m_numberOfPointDataArrays( 0 ),
        m_numberOfCellDataArrays( 0 )
{
    m_datasetOperator = 0;
}

DataObjectHandler::~DataObjectHandler()
{}

void DataObjectHandler::OperateOnAllDatasetsInObject( vtkDataObject* dataObject )
{
    vtkDataSet* currentDataset = 0;
    if( dataObject->IsA( "vtkCompositeDataSet" ) )
    {
        try
        {
            vtkCompositeDataSet* mgd = dynamic_cast<vtkCompositeDataSet*>( dataObject );
            vtkCompositeDataIterator* mgdIterator = vtkCompositeDataIterator::New();
            mgdIterator->SetDataSet( mgd );
            ///For traversal of nested multigroupdatasets
            mgdIterator->VisitOnlyLeavesOn();
            mgdIterator->GoToFirstItem();
          
            while( !mgdIterator->IsDoneWithTraversal() )
            {
                currentDataset = dynamic_cast<vtkDataSet*>( mgdIterator->GetCurrentDataObject() );
                _convertCellDataToPointData( currentDataset );
                if( m_datasetOperator )
                {
                    m_datasetOperator->SetIsMultiBlockDataset( true );
                    m_datasetOperator->OperateOnDataset( currentDataset );
                }
                UpdateNumberOfPDArrays( currentDataset );
                
                mgdIterator->GoToNextItem();
            }

            if( mgdIterator )
            {
                mgdIterator->Delete();
                mgdIterator = 0;
            }
        }
        catch ( ... )
        {
            std::cout << "*********** Invalid Dataset: " 
                << dataObject->GetClassName() << " ***********" << std::endl;
        }
    }
    else //Assume this is a regular vtkdataset
    {
        currentDataset = dynamic_cast<vtkDataSet*>( dataObject );
        _convertCellDataToPointData( currentDataset );
        if( m_datasetOperator )
        {
            m_datasetOperator->SetIsMultiBlockDataset( false );
            m_datasetOperator->OperateOnDataset( currentDataset );
        }
        UpdateNumberOfPDArrays( currentDataset );
    }

}
////////////////////////////////////////////////////////////////////////////////
void DataObjectHandler::_convertCellDataToPointData( vtkDataSet* dataSet )
{
    //UpdateNumberOfPDArrays( dataSet );
    if( dataSet->GetCellData()->GetNumberOfArrays() > 0 ) //&& m_numberOfPointDataArrays  == 0 )
    {
        std::cout << "|\tThe dataset has no point data -- "
            << "will try to convert cell data to point data" << std::endl;

        vtkCellDataToPointData* converter = vtkCellDataToPointData::New();
        converter->SetInput( 0, dataSet );
        converter->PassCellDataOff();
        converter->Update();

        ///Why do we need to do this only for unstructured grids?
        if( dataSet->GetDataObjectType() == VTK_UNSTRUCTURED_GRID )
        {
            ///We need a shallow copy here or else polyhedral grids crash things
            ///I think this is a bug with deep copies in the unstructuredgrid
            dataSet->ShallowCopy( converter->GetUnstructuredGridOutput() );
            converter->Delete();
        }
        else if( dataSet->GetDataObjectType() == VTK_POLY_DATA )
        {
            dataSet->DeepCopy( converter->GetPolyDataOutput() );
            converter->Delete();
        }
        else if( dataSet->GetDataObjectType() == VTK_RECTILINEAR_GRID )
        {
            dataSet->DeepCopy( converter->GetRectilinearGridOutput() );
            converter->Delete();
        }
        else if( dataSet->GetDataObjectType() == VTK_STRUCTURED_POINTS )
        {
            dataSet->DeepCopy( converter->GetStructuredPointsOutput() );
            converter->Delete();
        }
        else if( dataSet->GetDataObjectType() == VTK_STRUCTURED_GRID  )
        {
            dataSet->DeepCopy( converter->GetStructuredGridOutput() );
            converter->Delete();
        }
        else
        {
            converter->Delete();
            std::cout << "\nAttempt failed: can not currently handle "
                << "this type of data - " 
                << "DataObjectHandler::_convertCellDataToPointData" << std::endl;
            exit( 1 );
        }
    }
    return;
}
////////////////////////////////////////////////////////////////////////////////
unsigned int DataObjectHandler::GetNumberOfDataArrays( bool isPointData )
{
    return ( isPointData ) ? m_numberOfPointDataArrays : m_numberOfCellDataArrays;
}
////////////////////////////////////////////////////////////////////////////////
void DataObjectHandler::SetDatasetOperatorCallback( DatasetOperatorCallback* dsoCbk )
{
    m_datasetOperator = dsoCbk;
}
////////////////////////////////////////////////////////////////////////////////
void DataObjectHandler::UpdateNumberOfPDArrays( vtkDataSet* dataSet )
{
    unsigned int numberOfPointDataArrays = dataSet->GetPointData()->GetNumberOfArrays();
    if( numberOfPointDataArrays > m_numberOfPointDataArrays )
    {
        m_numberOfPointDataArrays = numberOfPointDataArrays;
    }
    unsigned int numberOfCellDataArrays = dataSet->GetCellData()->GetNumberOfArrays();
    if( numberOfCellDataArrays > m_numberOfCellDataArrays )
    {
        m_numberOfCellDataArrays = numberOfCellDataArrays;
    }
}
////////////////////////////////////////////////////////////////////////////////
