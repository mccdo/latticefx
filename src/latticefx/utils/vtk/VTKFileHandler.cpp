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
#include <latticefx/utils/vtk/VTKFileHandler.h>
#include <vtkDataSet.h>
#include <vtkDataObject.h>
#include <vtkXMLFileReadTester.h>
#include <vtkXMLUnstructuredGridReader.h>
#include <vtkXMLStructuredGridReader.h>
#include <vtkXMLRectilinearGridReader.h>
#include <vtkXMLImageDataReader.h>
#include <vtkDataArraySelection.h>
#include <vtkImageData.h>
#include <vtkPolyDataReader.h>
#include <vtkDataSetReader.h>
#include <vtkDataReader.h>
#include <vtkStructuredGrid.h>
#include <vtkStructuredPoints.h>
#include <vtkRectilinearGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkPolyData.h>
#include <vtkXMLDataSetWriter.h>
#include <vtkUnstructuredGridWriter.h>
#include <vtkStructuredGridWriter.h>
#include <vtkRectilinearGridWriter.h>
#include <vtkPolyDataWriter.h>
#include <vtkUnstructuredGridReader.h>
#include <vtkGenericEnSightReader.h>

#include <vtkXMLHierarchicalBoxDataReader.h>
#include <vtkXMLCompositeDataReader.h>
#include <vtkXMLCompositeDataWriter.h>
#include <vtkXMLMultiBlockDataWriter.h>
#include <vtkHierarchicalBoxDataSet.h>
#include <vtkXMLReader.h>
#include <vtkXMLWriter.h>
#include <vtkAlgorithm.h>

#include <vtkXMLHierarchicalDataReader.h>
#include <vtkXMLMultiGroupDataReader.h>
#include <vtkXMLMultiBlockDataReader.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkGenericDataObjectReader.h>
#include <vtkGenericDataObjectWriter.h>
#include <fstream>
#include <iostream>
#include <cstring>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/version.hpp>

namespace fs = boost::filesystem;

using namespace lfx::vtk_utils;

//////////////////////////////////////
//Constructors                      //
//////////////////////////////////////
VTKFileHandler::VTKFileHandler():
    _xmlTester( 0 ),
    _dataSet( 0 ),
    mDataReader( 0 )
{
    _outFileType = CFD_XML;
    _outFileMode = CFD_BINARY;
}
/////////////////////////////////////////////////////////////////
VTKFileHandler::VTKFileHandler( const VTKFileHandler& fh )
{
    _outFileType = fh._outFileType;
    _outFileMode = fh._outFileMode;

    _inFileName.assign( fh._inFileName );
    _outFileName.assign( fh._outFileName );

    _xmlTester = vtkXMLFileReadTester::New();
    _xmlTester = fh._xmlTester;
    _dataSet = fh._dataSet;
    std::cout << "VTKFileHandler::VTKFileHandler Bad News" << std::endl;
}
///////////////////////////////////////
//Destructor                         //
///////////////////////////////////////
VTKFileHandler::~VTKFileHandler()
{
    if( _xmlTester )
    {
        _xmlTester->Delete();
    }
}
////////////////////////////////////////////////////////////
void VTKFileHandler::SetVTKOutFileType( OutFileType type )
{
    _outFileType = type;
}
/////////////////////////////////////////////////////////////
void VTKFileHandler::SetOutFileWriteMode( OutFileMode mode )
{
    _outFileMode = mode;
}
//////////////////////////////////////////////////////
void VTKFileHandler::SetInputFileName( std::string inFile )
{
    _inFileName = inFile;
}
//////////////////////////////////////////////////////
void VTKFileHandler::SetOutputFileName( std::string oFile )
{
    _outFileName = oFile;
}
//////////////////////////////////////////////////////
/*vtkAlgorithm* VTKFileHandler::GetAlgorithm()
{
    return mDataReader;
}*/
////////////////////////////////////////////////////////////////////
vtkDataObject* VTKFileHandler::GetDataSetFromFile( const std::string& vtkFileName )
{
    if( vtkFileName.empty() )
    {
        return 0;
    }
    std::cout << "|\tLoading: " << vtkFileName << std::endl;
    _inFileName = vtkFileName;

    if( !_xmlTester )
    {
        _xmlTester = vtkXMLFileReadTester::New();
    }
    _xmlTester->SetFileName( _inFileName.c_str() );

    try
    {
        if( _xmlTester->TestReadFile() )
        {
            std::cout << "|\t\tXML ";
            std::cout << _xmlTester->GetFileDataType() << std::endl;
            //process xml file
            if( !std::strcmp( _xmlTester->GetFileDataType(), "UnstructuredGrid" ) )
            {
                _xmlTester->Delete();
                _xmlTester = 0;
                _getXMLUGrid();
            }
            else if( !std::strcmp( _xmlTester->GetFileDataType(), "StructuredGrid" ) )
            {
                _getXMLSGrid();
            }
            else if( !std::strcmp( _xmlTester->GetFileDataType(), "RectilinearGrid" ) )
            {
                _getXMLRGrid();
            }
            else if( !std::strcmp( _xmlTester->GetFileDataType(), "PolyData" ) )
            {
                _getXMLPolyData();
            }
            else if( !std::strcmp( _xmlTester->GetFileDataType(), "ImageData" ) )
            {
                GetXMLImageData();
            }
            else if( !std::strcmp( _xmlTester->GetFileDataType(), "vtkMultiBlockDataSet" ) )
            {
                _getXMLMultiGroupDataSet();
            }
            else if( !std::strcmp( _xmlTester->GetFileDataType(), "vtkMultiGroupDataSet" ) )
            {
                _getXMLMultiGroupDataSet( false );
            }
            else if( !std::strcmp( _xmlTester->GetFileDataType(), "vtkHierarchicalDataSet" ) )
            {
                GetXMLHierarchicalDataSet();
            }
        }
        else
        {
            //this is a "classic" style vtk file
            _readClassicVTKFile();
        }
    }
    catch( ... )
    {
        std::cerr << "VTKFileHandler::GetDataSetFromFile "
                  << "Memory allocation error." << std::endl;
        _dataSet = 0;
    }
    return _dataSet;
}
/////////////////////////////////////////////
void VTKFileHandler::_readClassicVTKFile()
{
    if( !_inFileName.c_str() )
    {
        return;
    }

    vtkGenericDataObjectReader* genericReader = vtkGenericDataObjectReader::New();
    genericReader->SetFileName( _inFileName.c_str() );
    genericReader->Update();

    {
        if( genericReader->IsFileUnstructuredGrid() )
        {
            std::cout << "|\t\tUnstructured Grid..." << std::endl;
            _dataSet = vtkUnstructuredGrid::New();
        }
        else if( genericReader->IsFileStructuredGrid() )
        {
            std::cout << "|\t\tStructured Grid..." << std::endl;
            _dataSet = vtkStructuredGrid::New();
        }
        else if( genericReader->IsFileRectilinearGrid() )
        {
            std::cout << "|\t\tRectilinear Grid..." << std::endl;
            _dataSet = vtkRectilinearGrid::New();
        }
        else if( genericReader->IsFilePolyData() )
        {
            std::cout << "|\t\tPolyData..." << std::endl;
            _dataSet = vtkPolyData::New();
        }
        else if( genericReader->IsFileStructuredPoints() )
        {
            std::cout << "|\t\tStructured Points..." << std::endl;
            _dataSet = vtkStructuredPoints::New();
        }
        else
        {
            std::cerr << "\nERROR - Unable to read this vtk file format"
                      << std::endl;
            return ;
        }
        _dataSet->ShallowCopy( genericReader->GetOutput() );
        genericReader->Delete();
    }
}
/////////////////////////////////////////////////
void VTKFileHandler::_getXMLMultiGroupDataSet( bool isMultiBlock )
{
    vtkXMLCompositeDataReader* mgdReader = 0;
    if( isMultiBlock )
    {
        mgdReader = vtkXMLMultiBlockDataReader::New();
        _dataSet = vtkMultiBlockDataSet::New();
    }
    else
    {
        mgdReader = vtkXMLMultiGroupDataReader::New();
        _dataSet = vtkMultiBlockDataSet::New();
    }
    mgdReader->SetFileName( _inFileName.c_str() );
    mgdReader->Update();
    UpdateReaderActiveArrays( mgdReader );
    _dataSet->ShallowCopy( mgdReader->GetOutput() );
    mgdReader->Delete();
}
/////////////////////////////////////////////////
void VTKFileHandler::GetXMLHierarchicalDataSet()
{
    vtkXMLHierarchicalDataReader* mgdReader = 0;
    mgdReader = vtkXMLHierarchicalDataReader::New();
    _dataSet = vtkHierarchicalBoxDataSet::New();
    mgdReader->SetFileName( _inFileName.c_str() );
    mgdReader->Update();
    UpdateReaderActiveArrays( mgdReader );
    _dataSet->ShallowCopy( mgdReader->GetOutput() );
    mgdReader->Delete();
}
//////////////////////////////////////
void VTKFileHandler::_getXMLUGrid()
{
    vtkXMLUnstructuredGridReader* ugReader
        = vtkXMLUnstructuredGridReader::New();
    ugReader->SetFileName( _inFileName.c_str() );
    ugReader->Update();
    UpdateReaderActiveArrays( ugReader );
    _dataSet = ugReader->GetOutput();
    _dataSet->Register( ugReader->GetOutput() );
    //_dataSet = vtkUnstructuredGrid::New();
    //_dataSet->ShallowCopy( ugReader->GetOutput() );
    ugReader->Delete();
}
//////////////////////////////////////
void VTKFileHandler::_getXMLSGrid()
{
    vtkXMLStructuredGridReader* sgReader
        = vtkXMLStructuredGridReader::New();
    sgReader->SetFileName( _inFileName.c_str() );
    sgReader->Update();
    UpdateReaderActiveArrays( sgReader );
    _dataSet = vtkStructuredGrid::New();
    _dataSet->ShallowCopy( sgReader->GetOutput() );
    sgReader->Delete();
}
/////////////////////////////////////////////
void VTKFileHandler::_getXMLRGrid()
{
    vtkXMLRectilinearGridReader* rgReader
        = vtkXMLRectilinearGridReader::New();
    rgReader->SetFileName( _inFileName.c_str() );
    rgReader->Update();
    UpdateReaderActiveArrays( rgReader );
    _dataSet = vtkRectilinearGrid::New();
    _dataSet->ShallowCopy( rgReader->GetOutput() );
    rgReader->Delete();
}
////////////////////////////////////////////////
void VTKFileHandler::_getXMLPolyData()
{
    vtkXMLPolyDataReader* pdReader
        = vtkXMLPolyDataReader::New();
    pdReader->SetFileName( _inFileName.c_str() );
    pdReader->Update();
    UpdateReaderActiveArrays( pdReader );
    _dataSet = vtkPolyData::New();
    _dataSet->ShallowCopy( pdReader->GetOutput() );
    pdReader->Delete();
}
////////////////////////////////////////////////
void VTKFileHandler::GetXMLImageData( void )
{
    vtkXMLImageDataReader* reader = vtkXMLImageDataReader::New();
    reader->SetFileName( _inFileName.c_str() );
    reader->Update();
    _dataSet = vtkImageData::New();
    _dataSet->ShallowCopy( reader->GetOutput() );
    reader->Delete();
}
/////////////////////////////////////////////////////////////////////////////////
bool VTKFileHandler::WriteDataSet( vtkDataObject* dataSet, std::string outFileName )
{
    if( outFileName.empty() )
    {
        return false;
    }

    //ImageData (.vti) — Serial vtkImageData (structured).
    //PolyData (.vtp) — Serial vtkPolyData (unstructured).
    //RectilinearGrid (.vtr) — Serial vtkRectilinearGrid (structured).
    //StructuredGrid (.vts) — Serial vtkStructuredGrid (structured).
    //UnstructuredGrid (.vtu) — Serial vtkUnstructuredGrid (unstructured).
    //Multiblock (.vtm) - Serial vtkMultiBlockDataSet
    //PImageData (.pvti) — Parallel vtkImageData (structured).
    //PPolyData (.pvtp) — Parallel vtkPolyData (unstructured).
    //PRectilinearGrid (.pvtr) — Parallel vtkRectilinearGrid (structured).
    //PStructuredGrid (.pvts) — Parallel vtkStructuredGrid (structured).
    //PUnstructuredGrid (.pvtu) — Parallel vtkUnstructuredGrid (unstructured).
    fs::path file_name( outFileName );

    if( dataSet->IsA( "vtkMultiBlockDataSet" ) )
    {
#if BOOST_VERSION > 103301
        file_name.replace_extension( "vtm" );
#endif
        vtkXMLMultiBlockDataWriter* writer = vtkXMLMultiBlockDataWriter::New();
        writer->SetFileName( file_name.string().c_str() );
        writer->SetInput( dataSet );
        if( _outFileMode == CFD_ASCII )
        {
            writer->SetDataModeToAscii();
        }
        if( writer->Write() )
        {
            writer->Delete();
            return true;
        }
        writer->Delete();
        return false;
    }
    else
    {
#if BOOST_VERSION > 103301
        if( dataSet->IsA( "vtkPolyData" ) )
        {
            file_name.replace_extension( "vtp" );
        }
        else if( dataSet->IsA( "vtkImageData" ) )
        {
            file_name.replace_extension( "vti" );
        }
        else if( dataSet->IsA( "vtkStructuredGrid" ) )
        {
            file_name.replace_extension( "vts" );
        }
        else if( dataSet->IsA( "vtkUnstructuredGrid" ) )
        {
            file_name.replace_extension( "vtu" );
        }
        else
        {
            file_name.replace_extension( "vtk" );
        }
#endif
        vtkXMLDataSetWriter* writer = vtkXMLDataSetWriter::New();
        writer->SetFileName( file_name.string().c_str() );
        writer->SetInput( dynamic_cast<vtkDataSet*>( dataSet ) );

        if( _outFileMode == CFD_ASCII )
        {
            writer->SetDataModeToAscii();
        }

        if( writer->Write() )
        {
            writer->Delete();
            return true;
        }
        writer->Delete();
        return false;
    }
    return false;
}
////////////////////////////////////////////////////////////////////////
void VTKFileHandler::_writeClassicVTKFile( vtkDataObject* vtkThing,
        std::string vtkFilename, int binaryFlag )
{
    if( vtkThing == NULL )
    {
        std::cout << "Error: Can't write because vtkThing == NULL" << std::endl;
        return;
    }

    vtkThing->Update();

    std::cout << "Writing \"" << vtkFilename << "\", " << std::flush;

    if( vtkThing->IsA( "vtkUnstructuredGrid" ) )
    {
        std::cout << "a vtkUnstructuredGrid... " << std::flush;

        vtkUnstructuredGrid* uGrid = vtkUnstructuredGrid::SafeDownCast( vtkThing );
        if( uGrid == NULL )
        {
            std::cout << "SafeDownCast to a vtkUnstructuredGrid failed";
        }

        vtkUnstructuredGridWriter* writer = vtkUnstructuredGridWriter::New();
        //writer->SetInput( (vtkUnstructuredGrid *)vtkThing );
        writer->SetInput( uGrid );
        writer->SetFileName( vtkFilename.c_str() );
        if( binaryFlag )
        {
            writer->SetFileTypeToBinary();
        }
        writer->Write();
        writer->Delete();
    }
    else if( vtkThing->IsA( "vtkStructuredGrid" ) )
    {
        std::cout << "a vtkStructuredGrid... " << std::flush;

        vtkStructuredGrid* sGrid = vtkStructuredGrid::SafeDownCast( vtkThing );
        if( sGrid == NULL )
        {
            std::cout << "SafeDownCast to a vtkStructuredGrid failed";
        }
        // Part of old vtk build may need to fix later
        //sGrid->BlankingOff();

        vtkStructuredGridWriter* writer = vtkStructuredGridWriter::New();
        writer->SetInput( sGrid );
        //writer->SetInput( (vtkStructuredGrid *)vtkThing );    //core dumped
        //writer->SetInput( vtkThing );                         //won't compile
        writer->SetFileName( vtkFilename.c_str() );
        if( binaryFlag )
        {
            writer->SetFileTypeToBinary();
        }
        writer->Write();
        writer->Delete();
        //sGrid->Delete();
    }
    else if( vtkThing->IsA( "vtkRectilinearGrid" ) )
    {
        std::cout << "a vtkRectilinearGrid... " << std::flush;
        vtkRectilinearGridWriter* writer = vtkRectilinearGridWriter::New();
        writer->SetInput( ( vtkRectilinearGrid* )vtkThing );
        writer->SetFileName( vtkFilename.c_str() );
        if( binaryFlag )
        {
            writer->SetFileTypeToBinary();
        }
        writer->Write();
        writer->Delete();
    }
    else if( vtkThing->IsA( "vtkPolyData" ) )
    {
        std::cout << "a vtkPolyData... " << std::flush;
        vtkPolyDataWriter* writer = vtkPolyDataWriter::New();
        writer->SetInput( ( vtkPolyData* )vtkThing );
        writer->SetFileName( vtkFilename.c_str() );
        if( binaryFlag )
        {
            writer->SetFileTypeToBinary();
        }
        writer->Write();
        writer->Delete();
    }
    else
    {
        std::cerr << "\nERROR - Unsupported vtk file format: file not written"
                  << std::endl;
        return;
    }

    std::cout << "... done\n" << std::endl;
}
////////////////////////////////////////////////////////////////////////////////
VTKFileHandler& VTKFileHandler::operator=( const VTKFileHandler& fh )
{
    if( this != &fh )
    {
        _outFileType = fh._outFileType;
        _outFileMode = fh._outFileMode;

        _inFileName.clear();
        _outFileName.clear();

        _inFileName.assign( fh._inFileName );
        _outFileName.assign( fh._outFileName );

        _xmlTester = fh._xmlTester;
        _dataSet = fh._dataSet;
    }
    return *this;
}
////////////////////////////////////////////////////////////////////////////////
void VTKFileHandler::SetScalarsAndVectorsToRead( std::vector< std::string > activeArrays )
{
    m_activeArrays = activeArrays;
}
////////////////////////////////////////////////////////////////////////////////
void VTKFileHandler::UpdateReaderActiveArrays( vtkXMLReader* reader )
{
    if( !m_activeArrays.empty() )
    {
        vtkDataArraySelection* arraySelector =
            reader->GetPointDataArraySelection();
        arraySelector->DisableAllArrays();
        for( size_t i = 0; i < m_activeArrays.size(); ++i )
        {
            std::cout << "Passed arrays are: "
                      << m_activeArrays[ i ] << std::endl;
            arraySelector->EnableArray( m_activeArrays[ i ].c_str() );
        }
        //Need to update again before the output of the reader is read
        reader->Update();
    }
}
////////////////////////////////////////////////////////////////////////////////
std::vector< std::string > VTKFileHandler::GetDataSetArraysFromFile( const std::string& vtkFileName )
{
    if( vtkFileName.empty() )
    {
        return m_activeArrays;
    }
    _inFileName = vtkFileName;
    fs::path file_name( _inFileName );

    std::cout << "|\tLoading " << _inFileName
              << " and extension " << file_name.extension() << std::endl;;

    if( !_xmlTester )
    {
        _xmlTester = vtkXMLFileReadTester::New();
    }
    _xmlTester->SetFileName( _inFileName.c_str() );

    //Test for ensight file to activate scalars
    if( file_name.extension() == ".case" )
    {
        vtkGenericEnSightReader* reader = vtkGenericEnSightReader::New();
        reader->SetCaseFileName( _inFileName.c_str() );
        reader->Update();

        vtkDataArraySelection* arraySelector =
            reader->GetCellDataArraySelection();
        int numArrays = arraySelector->GetNumberOfArrays();
        for( int i = 0; i < numArrays; ++i )
        {
            std::string arrayName = arraySelector->GetArrayName( i );
            //std::cout << arrayName << std::endl;
            m_activeArrays.push_back( arrayName );
        }
        arraySelector = reader->GetPointDataArraySelection();
        numArrays = arraySelector->GetNumberOfArrays();
        for( int i = 0; i < numArrays; ++i )
        {
            std::string arrayName = arraySelector->GetArrayName( i );
            //std::cout << arrayName << std::endl;
            m_activeArrays.push_back( arrayName );
        }
        reader->Delete();
        _xmlTester->Delete();
        _xmlTester = 0;
        return m_activeArrays;
    }
    else if( !_xmlTester->TestReadFile() )
    {
        _xmlTester->Delete();
        _xmlTester = 0;
        return m_activeArrays;
    }

    vtkXMLReader* reader = 0;
    std::cout << "|\t\tXML " << _xmlTester->GetFileDataType() << std::endl;
    //process xml file
    if( !std::strcmp( _xmlTester->GetFileDataType(), "UnstructuredGrid" ) )
    {
        reader = vtkXMLUnstructuredGridReader::New();
    }
    else if( !std::strcmp( _xmlTester->GetFileDataType(), "StructuredGrid" ) )
    {
        reader = vtkXMLStructuredGridReader::New();
    }
    else if( !std::strcmp( _xmlTester->GetFileDataType(), "RectilinearGrid" ) )
    {
        reader = vtkXMLRectilinearGridReader::New();
    }
    else if( !std::strcmp( _xmlTester->GetFileDataType(), "PolyData" ) )
    {
        reader = vtkXMLPolyDataReader::New();
    }
    else if( !std::strcmp( _xmlTester->GetFileDataType(), "ImageData" ) )
    {
        reader = vtkXMLImageDataReader::New();
    }
    else if( !std::strcmp( _xmlTester->GetFileDataType(), "vtkMultiBlockDataSet" ) )
    {
        reader = vtkXMLMultiBlockDataReader::New();
    }
    else if( !std::strcmp( _xmlTester->GetFileDataType(), "vtkMultiGroupDataSet" ) )
    {
        reader = vtkXMLMultiGroupDataReader::New();
    }
    else if( !std::strcmp( _xmlTester->GetFileDataType(), "vtkHierarchicalDataSet" ) )
    {
        reader = vtkXMLHierarchicalDataReader::New();
    }

    _xmlTester->Delete();
    _xmlTester = 0;
    reader->SetFileName( _inFileName.c_str() );
    reader->Update();

    vtkDataArraySelection* arraySelector =
        reader->GetCellDataArraySelection();
    int numArrays = arraySelector->GetNumberOfArrays();
    for( int i = 0; i < numArrays; ++i )
    {
        std::string arrayName = arraySelector->GetArrayName( i );
        //std::cout << arrayName << std::endl;
        m_activeArrays.push_back( arrayName );
    }
    arraySelector = reader->GetPointDataArraySelection();
    numArrays = arraySelector->GetNumberOfArrays();
    for( int i = 0; i < numArrays; ++i )
    {
        std::string arrayName = arraySelector->GetArrayName( i );
        //std::cout << arrayName << std::endl;
        m_activeArrays.push_back( arrayName );
    }
    reader->Delete();

    return m_activeArrays;
}
////////////////////////////////////////////////////////////////////////////////
/*
 void VTKFileHandler::OperateOnAllDatasetsInObject( vtkDataObject* dataObject )
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
 dataSet->DeepCopy( converter->GetUnstructuredGridOutput() );
 converter->Delete();
 }
 else if( dataSet->GetDataObjectType() == VTK_POLY_DATA )
 {
 dataSet->DeepCopy( converter->GetPolyDataOutput() );
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
*/
