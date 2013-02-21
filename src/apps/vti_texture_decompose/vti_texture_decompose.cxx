#include <utility>
#include <iostream>

#include <boost/filesystem/operations.hpp> // includes boost/filesystem/path.hpp
#include <boost/filesystem/path.hpp>

#include <latticefx/utils/vtk/readWriteVtkThings.h>

#include <vtkXMLDataSetWriter.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkPolygon.h>
#include <vtkImageData.h>
#include <vtkPointData.h>
#include <vtkDataArray.h>
#include <vtkFloatArray.h>

////////////////////////////////////////////////////////////////////////////////
void writeFlowTexture( vtkImageData* currentFile, std::string const& fileName, std::string const& scalarName)
{
    vtkImageData* flowImage = vtkImageData::New();
    // used to determine texture size
    flowImage->SetOrigin( currentFile->GetOrigin() );
    //texture dimensions
    flowImage->SetDimensions( currentFile->GetDimensions() );
    // used to determine texture size
    flowImage->SetSpacing( currentFile->GetSpacing() );


    // setup container for scalar data
    vtkFloatArray* flowData = vtkFloatArray::New();
    flowData->DeepCopy( currentFile->GetPointData()->GetArray( scalarName.c_str() ) );
    flowData->SetName( scalarName.c_str() );
    //flowData->SetNumberOfComponents( 1 );
    //flowData->SetNumberOfTuples( _dims[0]*_dims[1]*_dims[2] );

    //loop through and quantize the data
    /*for(int k = 0; k < _dims[2]; k++)
    {
        for(int j = 0; j < _dims[1]; j++)
        {
            for(int i = 0; i < _dims[0]; i++)
            {
                pixelNum = k*(_dims[0]*_dims[1]) + _dims[0]*j + i;
                if ( _pointData.at(0).type() == FlowPointData::SCALAR )
                {
                    flowData->SetTuple1( pixelNum, _pointData[ pixelNum ].data( 0 ) );
                }
                else
                {
                    double data[ 4 ];
                    for ( unsigned int i = 0; i < 4; ++i )
                        data[ i ] = _pointData[ pixelNum ].data( i );
                    
                    flowData->SetTuple( pixelNum, data );
                }
            }
        }
    }*/
    flowImage->GetPointData()->AddArray( flowData );
    lfx::vtk_utils::writeVtkThing( flowImage, fileName, 1 );

    flowImage->Delete();
    flowData->Delete();
}
////////////////////////////////////////////////////////////////////////////////
void writeScalarTexture( vtkImageData* flowImage, std::string const& directory, std::string const& newFilename, std::string const& dataName )
{
    boost::filesystem::path scalarPath( directory );
    scalarPath/=std::string("tbet_data");
    try
    {
        if( !boost::filesystem::is_directory( scalarPath ) )
        {
            boost::filesystem::create_directory( scalarPath );
        }
    }
    catch ( const std::exception& ex )
    {
        std::cout << ex.what() << std::endl;
        boost::filesystem::create_directory( scalarPath );
        std::cout << "...so we made it for you..." << std::endl;
    }

    scalarPath/=std::string("scalars");
    try
    {
        if( !boost::filesystem::is_directory( scalarPath ) )
        {
            boost::filesystem::create_directory( scalarPath );
        }
    }
    catch ( const std::exception& ex )
    {
        std::cout << ex.what() << std::endl;
        boost::filesystem::create_directory( scalarPath );
        std::cout << "...so we made it for you..." << std::endl;
    }
    
    scalarPath/=(dataName);
    try
    {
        if( !boost::filesystem::is_directory( scalarPath ) )
        {
            boost::filesystem::create_directory( scalarPath );
        }
    }
    catch ( const std::exception& ex )
    {
        std::cout << ex.what() << std::endl;
        boost::filesystem::create_directory( scalarPath );
        std::cout << "...so we made it for you..." << std::endl;
    }   
    
    std::string nameString;
    nameString.append(scalarPath.string());
    nameString.append( "/" );
    nameString.append( newFilename );
    nameString.append( ".vti" );
    
    writeFlowTexture( flowImage, nameString, dataName );  
}
////////////////////////////////////////////////////////////////////////////////
int main( int argc, char* argv[] )
{
    if( argc == 1 )
    {
        std::cout << "Please specify a vti file to decompose." << std::endl;
    }
    std::string vtiFilename( argv[ 1 ] );
    
    std::cout << "Decomposing vti file " << vtiFilename << std::endl;
 
    vtkImageData* flowImage = 
        dynamic_cast< vtkImageData* >( lfx::vtk_utils::readVtkThing( vtiFilename, 0 ) );
    if( !flowImage )
    {
        std::cout << "File must be a vtk image file." << std::endl;
    }

    if( flowImage->GetPointData()->GetNumberOfArrays() == 1 )
    {
        std::cout 
            << " Warning : There need to be multiple scalars in this vti file " 
            << std::endl;
    }
    
    vtkIdType numArrays = flowImage->GetPointData()->GetNumberOfArrays();
    for( vtkIdType i = 0; i < numArrays; ++i )
    {
        vtkDataArray* flowData = flowImage->GetPointData()->GetArray( i );
        std::string dataName = flowData->GetName();
        std::cout << "This vti file has scalar " << dataName << std::endl;
    }
    
    //Get base file name from vtiFilename
    boost::filesystem::path scalarPath( vtiFilename );
    std::string newFilename = scalarPath.filename().string();
    //Get base directory name from vtiFilename
    std::string const directory = scalarPath.remove_filename().string();
    
    for( vtkIdType i = 0; i < numArrays; ++i )
    {
        vtkDataArray* flowData = flowImage->GetPointData()->GetArray( i );
        std::string dataName = flowData->GetName();
        //Create the appropriate directory for the new image
        //<base dir>/scalars/<scalar name>/filename
        //Create the new image
        //Add in the appropriate scalar
        //Write out the image file
        writeScalarTexture( flowImage, directory, newFilename, dataName );
        //writeFlowTexture( flowImage, newFilename, dataName );
        //ves::xplorer::util::writeVtkThing( vtkDataObject* vtkThing, std::string vtkFilename, 1 )
    }

    return 1;
}
////////////////////////////////////////////////////////////////////////////////
