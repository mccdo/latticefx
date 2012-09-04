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
#include <latticefx/translators/vtk/EnSightTranslator.h>

#include <latticefx/translators/vtk/converter.h>
#include <latticefx/utils/vtk/VTKFileHandler.h>

#include <vtkDataSet.h>
#include <vtkGenericEnSightReader.h>          // will open any ensight file
#include <vtkUnstructuredGrid.h>
#include <vtkCellDataToPointData.h>
#include <vtkPointData.h>
#include <vtkFloatArray.h>
#include <vtkAppendFilter.h>
#include <vtkDataArrayCollection.h>
#include <vtkDataArray.h>
#include <vtkCompositeDataIterator.h>
#include <vtkCharArray.h>
#include <vtkDataArraySelection.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkDataObject.h>
#include <vtkIVWriter.h>
#include <vtkTriangleFilter.h>
#include <vtkCompositeDataGeometryFilter.h>
#include <vtkPolyDataNormals.h>
#include <vtkTemporalDataSet.h>

#include <iostream>
#include <iomanip>
#include <sstream>

using namespace lfx::vtk_translator;
////////////////////////////////////////
//Constructors                        //
////////////////////////////////////////
EnSightTranslator::EnSightTranslator()
{

    SetTranslateCallback( &ensightToVTK );
    SetPreTranslateCallback( &cmdParser );
}
/////////////////////////////////////////
EnSightTranslator::~EnSightTranslator()
{}
//////////////////////////////////////////////////////////////////////////
void EnSightTranslator::EnSightPreTranslateCbk::Preprocess( int argc, char** argv,
        cfdTranslatorToVTK* toVTK )
{
    PreTranslateCallback::Preprocess( argc, argv, toVTK );
}
////////////////////////////////////////////////////////////////////////////////
void EnSightTranslator::EnSightTranslateCbk::Translate( vtkDataObject*& outputDataset,
        cfdTranslatorToVTK* toVTK,
        vtkAlgorithm*& )
{
    EnSightTranslator* EnSightToVTK =
        dynamic_cast< EnSightTranslator* >( toVTK );
    if( !EnSightToVTK )
    {
        return;
    }

    std::vector< std::string > activeArrays = toVTK->GetActiveArrays();

    vtkGenericEnSightReader* reader = vtkGenericEnSightReader::New();
    //reader->DebugOn();
    reader->SetCaseFileName( EnSightToVTK->GetFile( 0 ).c_str() );
    reader->Update();

    if( !activeArrays.empty() )
    {
        //Disable user choosen arrays
        vtkDataArraySelection* arraySelector =
            reader->GetPointDataArraySelection();
        arraySelector->DisableAllArrays();
        for( size_t i = 0; i < activeArrays.size(); ++i )
        {
            std::cout << "Passed arrays are: "
                      << activeArrays[ i ] << std::endl;
            arraySelector->EnableArray( activeArrays[ i ].c_str() );
        }

        arraySelector =
            reader->GetCellDataArraySelection();
        arraySelector->DisableAllArrays();
        for( size_t i = 0; i < activeArrays.size(); ++i )
        {
            std::cout << "Passed arrays are: "
                      << activeArrays[ i ] << std::endl;
            arraySelector->EnableArray( activeArrays[ i ].c_str() );
        }
        //Need to update again before the output of the reader is read
        reader->Update();
    }

    vtkDataArrayCollection* tempArray = reader->GetTimeSets();
    //this must be an int because i goes negative
    if( tempArray->GetNumberOfItems() == 0 )
    {
        if( !outputDataset )
        {
            outputDataset = vtkMultiBlockDataSet::New();
        }
        outputDataset->ShallowCopy( reader->GetOutput() );
        outputDataset->Update();
    }
    else
    {
        //can still work with new datasets where there is only one timestep
        for( int i = tempArray->GetNumberOfItems() - 1; i >= 0; --i )
        {
            int numTimeSteps =
                tempArray->GetItem( i )->GetNumberOfTuples();
            std::cout << "Number of Timesteps = " << numTimeSteps << std::endl;

            //This should only ever be executed once.
            if( toVTK->GetWriteOption() != "file" )
            {
                outputDataset = vtkTemporalDataSet::New();
                vtkTemporalDataSet::SafeDownCast( outputDataset )->
                SetNumberOfTimeSteps( numTimeSteps );
            }

            // This allows the timesteps to go through the loop with positive values.
            for( int j = numTimeSteps - 1; j >= 0; --j )
            {
                float currentTimeStep = tempArray->GetItem( i )->GetTuple1( j );
                std::cout << "Translating Timestep = " << currentTimeStep << std::endl;
                reader->SetTimeValue( currentTimeStep );
                reader->Update();

                //Now dump geometry if it is available
                if( toVTK->GetExtractGeometry() )
                {
                    vtkCompositeDataGeometryFilter* geomFilter = vtkCompositeDataGeometryFilter::New();
                    geomFilter->SetInputConnection( reader->GetOutputPort() );
                    //geomFilter->ExtentClippingOn();
                    //geomFilter->PointClippingOn();
                    //geomFilter->CellClippingOn();
                    //geomFilter->Update();


                    vtkTriangleFilter* triFilter = vtkTriangleFilter::New();
                    triFilter->SetInputConnection( geomFilter->GetOutputPort() );
                    triFilter->PassVertsOn();
                    triFilter->PassLinesOff();
                    //triFilter->Update();

                    /*
                    vtkPolyDataNormals* pdNormals = vtkPolyDataNormals::New();
                    pdNormals->SplittingOff();
                    pdNormals->ConsistencyOn();
                    pdNormals->ComputeCellNormalsOn();
                    pdNormals->SetInputConnection( triFilter->GetOutputPort() );
                    pdNormals->Update();
                    */

                    std::ostringstream strm;
                    strm << EnSightToVTK->GetOutputFileName()
                         << "_"
                         << std::setfill( '0' )
                         << std::setw( 6 )
                         << j << ".iv";

                    vtkIVWriter* ivWriter = vtkIVWriter::New();
                    ivWriter->SetInputConnection( triFilter->GetOutputPort() );
                    ivWriter->SetFileName( strm.str().c_str() );
                    ivWriter->Write();
                    triFilter->Delete();
                    //pdNormals->Delete();
                    ivWriter->Delete();
                    geomFilter->Delete();
                }

                if( toVTK->GetWriteOption() == "file" )
                {
                    if( !outputDataset )
                    {
                        outputDataset = vtkMultiBlockDataSet::New();
                    }
                    outputDataset->ShallowCopy( reader->GetOutput() );
                    outputDataset->Update();
                    //Remember that this is a reverse iterator loop so time step
                    //0 is the last time step to be addressed.
                    if( j > 0 )
                    {
                        std::ostringstream strm;
                        strm << EnSightToVTK->GetOutputFileName()
                             << "_"
                             << std::setfill( '0' )
                             << std::setw( 6 )
                             << j << ".vtu";

                        lfx::vtk_utils::VTKFileHandler* trans = new lfx::vtk_utils::VTKFileHandler();
                        trans->WriteDataSet( outputDataset, strm.str() );
                        delete trans;
                        outputDataset->Delete();
                        outputDataset = 0;
                        EnSightToVTK->SetIsTransient();
                    }
                }
                else
                {
                    vtkMultiBlockDataSet* tempDataSet = vtkMultiBlockDataSet::New();
                    tempDataSet->ShallowCopy( reader->GetOutput() );
                    tempDataSet->Update();
                    vtkTemporalDataSet::SafeDownCast( outputDataset )->
                    SetTimeStep( j, tempDataSet );
                    /*std::ostringstream strm;
                    strm << EnSightToVTK->GetOutputFileName()
                            << "_"
                            << std::setfill( '0' )
                            << std::setw( 6 )
                            << j << ".vtm";

                    lfx::vtk_utils::VTKFileHandler* trans = new ves::xplorer::util::cfdVTKFileHandler();
                    trans->WriteDataSet( tempDataSet, strm.str() );
                    delete trans;*/
                    tempDataSet->Delete();
                }
            }
        }
    }
    /*
    vtkCompositeDataSet* mgd = dynamic_cast<vtkCompositeDataSet*>( outputDataset );
    //unsigned int nGroups = mgd->GetNumberOfGroups();
    unsigned int nDatasetsInGroup = 0;
    vtkCompositeDataIterator* mgdIterator = vtkCompositeDataIterator::New();
    mgdIterator->SetDataSet( mgd );
    ///For traversal of nested multigroupdatasets
    mgdIterator->VisitOnlyLeavesOn();
    mgdIterator->GoToFirstItem();

    while( !mgdIterator->IsDoneWithTraversal() )
    {
        vtkDataSet* currentDataset = dynamic_cast<vtkDataSet*>( mgdIterator->GetCurrentDataObject() );

        vtkCharArray* tempChar = dynamic_cast< vtkCharArray* >( currentDataset->GetFieldData()->GetArray( "Name" ) );
        std::cout << "test out " << tempChar->WritePointer( 0, 0 ) << std::endl;

        mgdIterator->GoToNextItem();
    }
    //if( mgdIterator )
    {
        mgdIterator->Delete();
        mgdIterator = 0;
    }
    */
    reader->Delete();
}
////////////////////////////////////////////////////////////////////////////////
void EnSightTranslator::DisplayHelp( void )
{
    std::cout << "|\tEnSight Translator Usage:" << std::endl
              << "\t -singleFile <filename_to_load> -o <output_dir> "
              << "-outFileName <output_filename> -loader ens -w file" << std::endl;
}
