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
#include <wx/file.h>


#include <vtkCellLocator.h>
#include <vtkOBBTree.h>
#include <vtkGenericCell.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDoubleArray.h>
#include <vtkStructuredGridReader.h>
#include <vtkDataReader.h>
#include <vtkUnstructuredGridReader.h>
#include <vtkRectilinearGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkCellDataToPointData.h>
#include <vtkProbeFilter.h>
#include <vtkPoints.h>
#include <iostream>
#include <cmath>

#ifdef WIN32
#include <direct.h>
#endif
#include "textureCreator.h"
#include "tcFrame.h"
#include <ves/xplorer/util/readWriteVtkThings.h>

#include <boost/filesystem/operations.hpp> // includes boost/filesystem/path.hpp
#include <boost/filesystem/path.hpp>

#include <boost/concept_check.hpp>


#include <sys/types.h>
#include <sys/timeb.h>
#ifdef OMPWIN32
#include <omp.h>
#endif

////////////////////////////////////
//Constructor                     //
////////////////////////////////////
VTKDataToTexture::VTKDataToTexture()
{
    _recreateValidityBetweenTimeSteps = false;
    _madeValidityStructure = false;
    _isBatch = false;
    _curPt = 0;
    _nPtDataArrays = 0;
    _resolution[0] = 2;
    _resolution[0] = 2;
    _resolution[0] = 2;

    //_maxDistFromCenter = 0;
    _nScalars = 0;
    _nVectors = 0;
    //_scalarNames = 0;
    //_vectorNames = 0;
    _dataSet = 0;
    _vFileName = 0;
    _outputDir = 0;

    _initTranslation = true;
    _dataConvertCellToPoint = 0;
    _usgrid = 0;
    _sgrid = 0;
    _rgrid = 0;
    _parent = 0;

    _isRGrid = true;
    _isSGrid = false;
    _isUGrid = false;

#ifdef OMPWIN32
    //get number of processors and set number of threads
    numThreads = omp_get_num_procs( );
    numThreads = numThreads - 1;
    if( numThreads == 0 )
    {
        numThreads = 1;
    }

    omp_set_num_threads( numThreads );
#else
    numThreads = 1;
#endif
}
/////////////////////////////////////////////////////////////
VTKDataToTexture::VTKDataToTexture( const VTKDataToTexture& v )
{
    _isBatch = v._isBatch;
    _curPt = v._curPt;
    _isRGrid = v._isRGrid;
    _isSGrid = v._isSGrid;
    _isUGrid = v._isUGrid;
    _rgrid = v._rgrid;
    _sgrid = v._sgrid;
    _usgrid = v._usgrid;
    _resolution[0] = v._resolution[0];
    _resolution[1] = v._resolution[1];
    _resolution[2] = v._resolution[2];
    _dataConvertCellToPoint = v._dataConvertCellToPoint;
    _nPtDataArrays = v._nPtDataArrays;
    _nScalars = v._nScalars;
    _nVectors = v._nVectors;
    _dataSet = v._dataSet;
    _velocity = v._velocity;
    _curScalar = v._curScalar;
    _scalarRanges = v._scalarRanges;
    _vectorRanges = v._vectorRanges;
    /*for( int i=0; i<_nScalars; i++ ){
       delete [] _scalarNames[i];
    }
    delete [] _scalarNames;
    _scalarNames = 0;*/
    _scalarNames.clear();

    _scalarNames = getParameterNames( 1, _nScalars );
    /*for( int i=0; i<_nVectors; i++ ){
       delete [] _vectorNames[i];
    }
    delete [] _vectorNames;
    _vectorNames = 0;*/
    _vectorNames.clear();
    _vectorNames = getParameterNames( 3, _nVectors );

    //_maxDistFromCenter = v._maxDistFromCenter;
    setVelocityFileName( v._vFileName );
    setOutputDirectory( v._outputDir );
    for( size_t i = 0; i < _validPt.size(); ++i )
    {
        if( _validPt.at( i ).second.second )
        {
            delete [] _validPt.at( i ).second.second;
        }
    }
    _validPt.clear();

    unsigned int nPts = v._validPt.size();
    for( unsigned int i = 0; i < nPts; i++ )
    {
        _validPt.push_back( v._validPt.at( i ) );
        if( v._validPt.at( i ).second.second )
        {
            _validPt.back().second.second = new double[ 3 ];
            _validPt.back().second.second[ 0 ] = v._validPt.at( i ).second.second[ 0 ];
            _validPt.back().second.second[ 1 ] = v._validPt.at( i ).second.second[ 1 ];
            _validPt.back().second.second[ 2 ] = v._validPt.at( i ).second.second[ 2 ];
        }
        else
        {
            _validPt.back().second.second = 0;
        }
    }
    /* nPts = v._distance.size();
    for(unsigned int i = 0; i < nPts; i++){
       _distance.push_back(v._distance.at(i));
    }*/
    _initTranslation = v._initTranslation;

}
/////////////////////////////////////
//Destructor                       //
/////////////////////////////////////
VTKDataToTexture::~VTKDataToTexture()
{
    _scalarNames.clear();
    _vectorNames.clear();
    /*for( int i=0; i<_nScalars; i++ ){
       delete [] _scalarNames[i];
    }
    delete [] _scalarNames;
    _scalarNames = 0;
    for( int i=0; i<_nVectors; i++ ){
       delete [] _vectorNames[i];
    }
    delete [] _vectorNames;
    _vectorNames = 0;
    */

    if( _vFileName )
    {
        delete [] _vFileName;
        _vFileName = 0;
    }
    if( _outputDir )
    {
        delete [] _outputDir;
        _outputDir = 0;
    }

    _velocity.clear();
    _curScalar.clear();
    _scalarRanges.clear();
    _vectorRanges.clear();

    if( _usgrid )
    {
        _usgrid->Delete();
    }

    if( _rgrid )
    {
        _rgrid->Delete();
    }

    if( _sgrid )
    {
        _sgrid->Delete();
    }

    for( size_t i = 0; i < _validPt.size(); ++i )
    {
        if( _validPt.at( i ).second.second )
        {
            delete [] _validPt.at( i ).second.second;
        }
    }
    _validPt.clear();

    if( _dataConvertCellToPoint )
    {
        _dataConvertCellToPoint->Delete();
        _dataConvertCellToPoint = 0;
    }
    else if( _dataSet )
    {
        _dataSet->Delete();
        _dataSet = 0;
    }
}
///////////////////////////////
void VTKDataToTexture::reInit()
{
    _initTranslation = true;
}
//////////////////////////////
void VTKDataToTexture::reset()
{
    _scalarNames.clear();
    _vectorNames.clear();

    if( _vFileName )
    {
        delete [] _vFileName;
        _vFileName = 0;
    }
    if( _outputDir )
    {
        delete [] _outputDir;
        _outputDir = 0;
    }

    _velocity.clear();
    _curScalar.clear();
    _scalarRanges.clear();
    _vectorRanges.clear();

    _nPtDataArrays = 0;
    if( _validPt.size() && _recreateValidityBetweenTimeSteps )
    {
        for( size_t i = 0; i < _validPt.size(); ++i )
        {
            if( _validPt.at( i ).second.second )
            {
                delete [] _validPt.at( i ).second.second;
            }
        }
        _validPt.clear();
    }

    if( _dataConvertCellToPoint )
    {
        _dataConvertCellToPoint->Delete();
        _dataConvertCellToPoint = 0;
    }
    else if( _dataSet )
    {
        _dataSet->Delete();
        _dataSet = 0;
    }

}
///////////////////////////////////////////////////
void VTKDataToTexture::setDataset( vtkDataSet* dSet )
{
    _dataSet = dSet;
    return;

    //this is really slow....
    /*if(!_dataSet)
    {
       _dataSet = vtkStructuredGrid::New();
    }

    vtkProbeFilter* probeFilter = vtkProbeFilter::New();
    //probeFilter->DebugOn();

    vtkStructuredGrid* tempGrid = vtkStructuredGrid::New();
    vtkPoints* tempPoints = vtkPoints::New();
    tempPoints->Allocate(_resolution[0]*_resolution[1]*_resolution[2]);

    double bbox[6] = {0,0,0,0,0,0};
    dSet->GetBounds(bbox);

    float delta[3] = {0,0,0};
    //delta x
    delta[0] = fabs((bbox[1] - bbox[0])/(_resolution[0]-1));
    //delta y
    delta[1] = fabs((bbox[3] - bbox[2])/(_resolution[1]-1));
    //delta z
    delta[2] = fabs((bbox[5] - bbox[4])/(_resolution[2]-1));

    //the bottom corner of the bbox/texture
    double origin[3] ={0,0,0};
    origin[0] = bbox[0];
    origin[1] = bbox[2];
    origin[2] = bbox[4];

    unsigned int kOffset = 0;
    unsigned int jOffset = 0;
    double pt[3] = {0,0,0};
    for ( int k=0; k < _resolution[2]; k++)
    {
       pt[2] = k*delta[2] + origin[2];
       kOffset = k * _resolution[0] * _resolution[1];
       for (int j=0; j<_resolution[1]; j++)
       {
          jOffset = j * _resolution[0];
          pt[1] = j*delta[1] + origin[1];

          for ( int i=0; i<_resolution[0]; i++)
          {
             pt[0] = i*delta[0] + origin[0];
             tempPoints->InsertPoint((i + jOffset + kOffset),pt);
          }
       }
    }
    tempGrid->SetDimensions(_resolution);
    tempGrid->SetPoints(tempPoints);
    tempGrid->Update();

    probeFilter->SetInput(tempGrid);
    probeFilter->SetSource(dSet);
    probeFilter->Update();

    _dataSet->DeepCopy(probeFilter->GetStructuredGridOutput());
    _dataSet->Update();

    tempPoints->Delete();
    tempGrid->Delete();
    probeFilter->Delete();*/
}
///////////////////////////////////////////
void VTKDataToTexture::setRectilinearGrid()
{
    _isRGrid = true;
    _isSGrid = false;
    _isUGrid = false;
}
//////////////////////////////////////////
void VTKDataToTexture::setStructuredGrid()
{
    _isSGrid = true;
    _isRGrid = false;
    _isUGrid = false;
}
////////////////////////////////////////////
void VTKDataToTexture::setUnstructuredGrid()
{
    _isUGrid = true;
    _isSGrid = false;
    _isRGrid = false;
}
/////////////////////////////////////////////////////
void VTKDataToTexture::TurnOnDynamicGridResampling()
{
    _recreateValidityBetweenTimeSteps = true;
}
/////////////////////////////////////////////////////
void VTKDataToTexture::TurnOffDynamicGridResampling()
{
    _recreateValidityBetweenTimeSteps = false;
}
///////////////////////////////////////////////////////////
//set the file name                                      //
//example:                                               //
//if vFileName == "vectors"                              //
//output will be "./textures/vectors_p.rgba"             //
//               "./textures/vectors_n.rgba"             //
//representing the positive and negative textures        //
///////////////////////////////////////////////////////////
void VTKDataToTexture::setVelocityFileName( const std::string vFileName )
{
    if( _vFileName )
    {
        delete [] _vFileName;
        _vFileName = 0;
    }
    _vFileName = new char[ strlen( vFileName.c_str() ) + 1];
    strcpy( _vFileName, vFileName.c_str() );
}
/////////////////////////////////////////////////////////////////
void VTKDataToTexture::_updateTranslationStatus( const std::string msg )
{
    if( _parent && !_isBatch )
    {
        _parent->UpdateProgressDialog( msg );
    }
}
/////////////////////////////////////////////////////////////
//set the output directory                                 //
/////////////////////////////////////////////////////////////
void VTKDataToTexture::setOutputDirectory( const std::string outDir )
{
    if( _outputDir )
    {
        delete [] _outputDir;
        _outputDir = 0;
    }
    _outputDir = new char[strlen( outDir.c_str() ) + 1];
    strcpy( _outputDir, outDir.c_str() );

}
//////////////////////////////////////////////////////////////////
//create a dataset from a file                                  //
//////////////////////////////////////////////////////////////////
bool VTKDataToTexture::createDataSetFromFile( const std::string filename )
{
    wxString msg = wxString( "Reading Dataset: ", wxConvUTF8 ) + wxString( filename.c_str(), wxConvUTF8 );
    _updateTranslationStatus( ConvertUnicode( msg.c_str() ) );
    //_confirmFileType(filename);
    ///This will need to be changed to handle both vtkDataset and vtkMultigroupDataSet
    vtkDataSet* tmpDSet = dynamic_cast<vtkDataSet*>( ves::xplorer::util::readVtkThing( filename, 0 ) );
    int dataObjectType = tmpDSet->GetDataObjectType();
    if( dataObjectType == VTK_POLY_DATA )
    {
        std::cout << "Invalid dataset: Polydata!!" << std::endl;
        tmpDSet->Delete();
        return false;
    }
    //get the info about the data in the data set
    _nPtDataArrays = tmpDSet->GetPointData()->GetNumberOfArrays();
    if( !_nPtDataArrays )
    {
        /*std::cout<<"Warning!!!"<<std::endl;
        std::cout<<"No point data found!"<<std::endl;
        std::cout<<"Attempting to convert cell data to point data."<<std::endl;*/

        if( !_dataConvertCellToPoint )
        {
            _dataConvertCellToPoint = vtkCellDataToPointData::New();
        }
        _dataConvertCellToPoint->SetInput( tmpDSet );
        _dataConvertCellToPoint->PassCellDataOff();
        _dataConvertCellToPoint->Update();
        setDataset( _dataConvertCellToPoint->GetOutput() );
        tmpDSet->Delete();
        return true;
    }
    else
    {
        setDataset( tmpDSet );
        return true;
    }
}
/////////////////////////////////////////////////////////////
void VTKDataToTexture::_confirmFileType( const std::string fileName )
{
    if( !fileName.size() )
    {
        vtkDataReader* genericReader = vtkDataReader::New();
        genericReader->SetFileName( fileName.c_str() );
        if( genericReader->IsFileStructuredGrid() && !_isSGrid )
        {
            setStructuredGrid();
        }
        else if( genericReader->IsFileUnstructuredGrid() && !_isUGrid )
        {
            setUnstructuredGrid();
        }
        else if( genericReader->IsFileRectilinearGrid() && !_isRGrid )
        {
            setRectilinearGrid();
        }
        genericReader->Delete();
    }
}
/////////////////////////////////////////
//create the textures for this data set//
/////////////////////////////////////////
void VTKDataToTexture::createTextures()
{
    wxString msg = wxString( "Creating textures.", wxConvUTF8 );
    _updateTranslationStatus( ConvertUnicode( msg.c_str() ) );
    //check for a dataset
    if( !_dataSet )
    {
        if( _vFileName )
        {
            createDataSetFromFile( std::string( _vFileName ) );
        }
        else
        {
            std::cout << "No dataset available to";
            std::cout << " create a texture!!" << std::endl;
            std::cout << "ERROR: VTKDataToTexture::createTextures()" << std::endl;
            return;
        }
    }

    //was the resolution initialized?
    if( _resolution[0] == 2 &&
            _resolution[1] == 2 &&
            _resolution[2] == 2 )
    {
        std::cout << "WARNING: Resolution set to the min!:" << std::endl;
        std::cout << " : VTKDataToTexture::createTextures()" << std::endl;
    }
    msg = wxString( "Building octree.", wxConvUTF8 );
    _updateTranslationStatus( ConvertUnicode( msg.c_str() ) );

    //get the info about the data in the data set
    _nPtDataArrays = _dataSet->GetPointData()->GetNumberOfArrays();
    _nScalars = countNumberOfParameters( 1 );
    _nVectors = countNumberOfParameters( 3 );
    _scalarNames = getParameterNames( 1, _nScalars );
    _vectorNames = getParameterNames( 3, _nVectors );

    _cleanUpFileNames();
    _applyCorrectedNamesToDataArrays();
    //by default, _recreateValidityBetweenTimeSteps is false
    if( !_madeValidityStructure || _recreateValidityBetweenTimeSteps )
    {
        msg = wxString( "Sampling valid domain. . .", wxConvUTF8 );
        _updateTranslationStatus( ConvertUnicode( msg.c_str() ) );
        //build the octree
        long timeID = ( long )time( NULL );
        std::cout << timeID << std::endl;
        /*bbLocator = vtkOBBTree::New();
        bbLocator->CacheCellBoundsOn();
        bbLocator->AutomaticOn();
        //bbLocator->SetNumberOfCellsPerBucket( 50 );
        //bbLocator->SetMaxLevel( 10 )
        vtkDataSet* polyData = ves::xplorer::util::readVtkThing( "./step1_0.vtp" );
        bbLocator->SetDataSet( polyData );
        //build the octree
        bbLocator->BuildLocator();*/

        /*
         vtkNew<vtkCellTreeLocator> locator;
         locator->SetDataSet(sphere2->GetOutput());
         locator->SetCacheCellBounds(cachedCellBounds);
         locator->AutomaticOn();
         locator->BuildLocator();
         */

        vectorCellLocators.resize( numThreads );
        for( int i = 0; i < numThreads; i++ )
        {
            vectorCellLocators.at( i ) = vtkCellLocator::New();
            vectorCellLocators.at( i )->CacheCellBoundsOn();
            vectorCellLocators.at( i )->AutomaticOn();
            vectorCellLocators.at( i )->SetNumberOfCellsPerBucket( 50 );
            //vtkDataSet* polyData = ves::xplorer::util::readVtkThing("./tempDataDir/surface.vtp");
            vectorCellLocators.at( i )->SetDataSet( _dataSet );
            //build the octree
            vectorCellLocators.at( i )->BuildLocator();
        }
        //long endtimeID = (long)time( NULL );
        //std::cout << endtimeID - timeID << std::endl;
        //Now use it...
        _createValidityTexture();
        //bbLocator->Delete();
        for( int i = 0; i < numThreads; i++ )
        {
            vectorCellLocators.at( i )->Delete();
        }

    }

    msg = wxString( "Processing scalars. . .", wxConvUTF8 );
    _updateTranslationStatus( ConvertUnicode( msg.c_str() ) );

    for( int i = 0; i < _nScalars; ++i )
    {
        double bbox[6] = {0, 0, 0, 0, 0, 0};
        //a bounding box
        _dataSet->GetBounds( bbox );

        FlowTexture texture;
        msg = wxString( "Scalar: ", wxConvUTF8 ) + wxString( _scalarNames[i].c_str(), wxConvUTF8 );
        _updateTranslationStatus( ConvertUnicode( msg.c_str() ) );

        texture.setTextureDimension( _resolution[0], _resolution[1], _resolution[2] );
        texture.setBoundingBox( bbox );
        _curScalar.push_back( texture );

        msg = wxString( "Resampling scalar data.", wxConvUTF8 );
        _updateTranslationStatus( ConvertUnicode( msg.c_str() ) );
        _resampleData( i, 1 );
        //std::cout<<"Writing data to texture."<<std::endl;
        writeScalarTexture( i );
        //std::cout<<"Cleaning up."<<std::endl;
        _curScalar.clear();
    }
    //std::cout<<"Processing vectors:"<<std::endl;
    msg = wxString( "Processing vectors. . .", wxConvUTF8 );
    _updateTranslationStatus( ConvertUnicode( msg.c_str() ) );

    for( int i = 0; i < _nVectors; i++ )
    {
        double bbox[6] = {0, 0, 0, 0, 0, 0};
        //a bounding box
        _dataSet->GetBounds( bbox );
        FlowTexture texture;
        wxString msg = wxString( "Vector: ", wxConvUTF8 ) + wxString( _vectorNames[i].c_str(), wxConvUTF8 );
        _updateTranslationStatus( ConvertUnicode( msg.c_str() ) );

        texture.setTextureDimension( _resolution[0], _resolution[1], _resolution[2] );
        texture.setBoundingBox( bbox );
        _velocity.push_back( texture );

        msg = wxString( "Resampling vector data", wxConvUTF8 );
        _updateTranslationStatus( ConvertUnicode( msg.c_str() ) );
        _resampleData( i, 0 );

        //std::cout<<"         Writing data to texture."<<std::endl;
        writeVelocityTexture( i );
        //std::cout<<"      Cleaning up."<<std::endl;
        _velocity.clear();
    }
}
///////////////////////////////////////////////
void VTKDataToTexture::_createValidityTexture()
{

    long timeID = ( long )time( NULL );

    double bbox[6] = {0, 0, 0, 0, 0, 0};
    //a bounding box
    _dataSet->GetBounds( bbox );

    //now create the black/white validity data
    float delta[3] = {0, 0, 0};
    //delta x
    delta[0] = fabs( ( bbox[1] - bbox[0] ) / ( _resolution[0] - 1 ) );
    //delta y
    delta[1] = fabs( ( bbox[3] - bbox[2] ) / ( _resolution[1] - 1 ) );
    //delta z
    delta[2] = fabs( ( bbox[5] - bbox[4] ) / ( _resolution[2] - 1 ) );
    //std::cout<<"Delta: "<<delta[0]<<" "<<delta[1]<<" "<<delta[2]<<std::endl;

    //double deltaDist = sqrt(delta[0]*delta[0] + delta[1]*delta[1]+delta[2]*delta[2]);
    //the bottom corner of the bbox/texture
    double pt[3] = {0, 0, 0};
    pt[0] = bbox[0];
    pt[1] = bbox[2];
    pt[2] = bbox[4];

    //we're going to create a cartiesian mesh that fills the
    //bounding box of the data. This is so that we can represent
    //the data as a texture
    //our original cell that contains our cartesian point
    //vtkGenericCell* cell = vtkGenericCell::New();
    std::vector < vtkGenericCell* > vectorcells;


    for( int thrd = 0; thrd < numThreads; thrd++ )
    {
        vectorcells.push_back( vtkGenericCell::New() );
    }

    vtkIdType cellId;
    int subId;
    double dist = 0;
    double closestPt[3];
    double pcoords[3];
    double* weights = 0;
    weights = new double[24];
    int i = 0;
    int j = 0;
    int k = 0;

    int nX = _resolution[0];
    int nY = _resolution[1];

    //an attempt to correct issues with 512 textures on windows
    int nPixels = _resolution[0] * _resolution[1] * _resolution[2];
    size_t maxS = _validPt.max_size();
    std::pair< bool, std::pair< int, double* > > tempPair =
        std::make_pair< bool, std::pair< int, double* > >
        ( false, std::make_pair< int, double* >( 0, 0 ) );
    try
    {
        // _validPt.resize( nPixels, tempPair );
        _validPt.assign( nPixels, tempPair );
    }
    catch( std::exception& ex )
    {
        int test = 1;
    }
    long lasttime = ( long )time( NULL );

#ifdef OMPWIN32
    #pragma omp parallel for private( i, j, k, pt, closestPt, cellId, subId, dist )
#endif
    for( int l = 0; l < nPixels; l++ )
    {

#ifdef OMPWIN32
        int currentThread = omp_get_thread_num( );
#else
        int currentThread = 0;
#endif
        vtkGenericCell* cell = vectorcells.at( currentThread );
        // we can parallelize this for loop by using a map to store the data and then
        // merge the map after the texture has been created.
        // this would allow this function to be much faster. This function
        // is where the majority of the time is spent currently.

        //set i, j, and k based on the l values
        i = l % nX;
        j = ( l / nX ) % nY; //(i==0)?((jInit==nY-1)?jInit=0:++jInit):jInit;
        k = ( l/*-i-j*nX*/ ) / ( nX * nY );

        pt[2] = bbox[4] + k * delta[2];
        pt[1] = bbox[2] + j * delta[1];
        pt[0] = bbox[0] + i * delta[0];

        /*timeb timeOneB;
        timeb timeTwoB;
        timeb timeThreeB;*/

        //ftime( &timeOneB );

        //long timeOne = (long)time( NULL );
        //new stuff
        {
            vectorCellLocators.at( currentThread )->FindClosestPoint( pt, closestPt, cell, cellId, subId, dist );
        }


        //ftime( &timeTwoB );
        //long timeTwo = (long)time( NULL );
        //std::cout << bbLocator->InsideOrOutside( pt ) << " : " << dist << std::endl;
        //bbLocator->InsideOrOutside( pt );
        //ftime( &timeThreeB );
        //long timeThree = (long)time( NULL );
        //std::cout << timeTwoB.millitm << " : " << timeOneB.millitm << " : " << timeThreeB.millitm << std::endl;
        //std::cout << timeTwoB.millitm - timeOneB.millitm << " : " << timeThreeB.millitm - timeTwoB.millitm << std::endl;

        //_cLocator->InsideOrOutside( pt );
        //weights = new double[cell->GetNumberOfPoints()];
        //check to see if this point is in
        //the returned cell
        //int good = cell->EvaluatePosition(pt,0,subId,pcoords,dist,weights);
        //if(weights){
        //delete [] weights;
        //weights = 0;
        //}

        if( dist == 0.0f )
        {
            //std::cout << "good " << l << " : " <<  dist << " : " << cellId << " : " << subId << " : " <<  closestPt[ 0 ] << " : " <<  closestPt[ 1 ] << " : " <<  closestPt[ 2 ] << std::endl;
            //_dataSet->GetCell( cellId, cell );
            cell->EvaluatePosition( pt, 0, subId, pcoords, dist, weights );
            _validPt.at( l ).first = true;
            _validPt.at( l ).second.first = cellId;
            _validPt.at( l ).second.second = new double[ 3 ];
            _validPt.at( l ).second.second[ 0 ] = pcoords[ 0 ];
            _validPt.at( l ).second.second[ 1 ] = pcoords[ 1 ];
            _validPt.at( l ).second.second[ 2 ] = pcoords[ 2 ];
        }
        else
        {
            //std::cout << dist << " : " << cellId << " : " << subId << " : " <<  closestPt[ 0 ] << " : " <<  closestPt[ 1 ] << " : " <<  closestPt[ 2 ] << std::endl;
            _validPt.at( l ).first = false;
            _validPt.at( l ).second.second = 0;
        }

        /*if ( (unsigned int)i > (unsigned int)nX )
        {
           i = 0;
           j++;
           if ( (unsigned int)j > (unsigned int)nY )
           {
              j = 0;
              k++;
              if ( (unsigned int)k > (unsigned int)nZ )
              {
                 k = 0;
              }
           }
        }*/

        if( l % 100000 == 0 )
        {
            long secondTime  = ( long )time( NULL );;
            //std::cout.seekp(ios::beg);
            std::cout << "Number of pixels processed = " << l
                      << " in " << secondTime - lasttime
                      << " seconds." << std::endl;
            //std::cout.seekp(ios::beg);
            lasttime = secondTime;
        }
    }
    //cell->Delete();
    _madeValidityStructure = true;
    delete [] weights;
    long endTime = ( long )time( NULL );
    std::cout << "Total Time = " << endTime - timeID << std::endl;
}
/////////////////////////////////////////////////////////////////////
void VTKDataToTexture::_resampleData( int dataValueIndex, int isScalar )
{
    double bbox[6] = {0, 0, 0, 0, 0, 0};
    //a bounding box
    _dataSet->GetBounds( bbox );

    //depending on the resolution we need to resample
    //the data
    float delta[3] = {0, 0, 0};
    //delta x
    delta[0] = fabs( ( bbox[1] - bbox[0] ) / ( _resolution[0] - 1 ) );
    //delta y
    delta[1] = fabs( ( bbox[3] - bbox[2] ) / ( _resolution[1] - 1 ) );
    //delta z
    delta[2] = fabs( ( bbox[5] - bbox[4] ) / ( _resolution[2] - 1 ) );

    //the bottom corner of the bbox/texture
    double pt[3] = {0, 0, 0};
    pt[0] = bbox[0];
    pt[1] = bbox[2];
    pt[2] = bbox[4];

    //we're going to create a cartiesian mesh that fills the
    //bounding box of the data. This is so that we can represent
    //the data as a texture
    //our original cell that contains our cartesian point
    vtkGenericCell* cell = vtkGenericCell::New();

    vtkIdType cellId;
    int subId;
    //double dist = 0;
    //double closestPt[3];
    //double pcoords[3];
    double* weights = 0;
    //int index = 0;
    //_curPt = 0;

    unsigned int i = 0;
    unsigned int j = 0;
    unsigned int k = 0;

    unsigned int nX = _resolution[0] - 1;
    unsigned int nY = _resolution[1] - 1;
    unsigned int nZ = _resolution[2] - 1;

    unsigned int nPixels = _resolution[0] * _resolution[1] * _resolution[2];

    //resample the data into a cartesian grid
    for( unsigned int l = 0; l < nPixels; ++l )
    {
        // we can parallelize this for loop by using a map to store the data and then
        // merge the map after the texture has been created.
        // this would allow this function to be much faster. This function
        // is where the majority of the time is spent currently.
        /*pt[2] = bbox[4] + k*delta[2];
        pt[1] = bbox[2] + j*delta[1];
        pt[0] = bbox[0] + (i++)*delta[0];*/
        if( _validPt.at( l ).first )
        {
            //_cLocator->FindClosestPoint(pt,closestPt,
            //                     cell,cellId,subId, dist);
            cellId = _validPt.at( l ).second.first;
            //subId = _validPt.at(l).second.second;
            _dataSet->GetCell( cellId, cell );
            weights = new double[cell->GetNumberOfPoints()];
            //check to see if this point is in
            //the returned cell
            //cell->EvaluatePosition(pt,0,subId,pcoords,dist,weights);
            cell->EvaluateLocation( subId, _validPt.at( l ).second.second , pt, weights );
            _interpolateDataInCell( cell, weights, dataValueIndex, isScalar );
            if( weights )
            {
                delete [] weights;
                weights = 0;
            }
        }
        else
        {
            //point isn't in a cell
            //so set the texture data to 0
            _addOutSideCellDomainDataToFlowTexture( dataValueIndex, isScalar );
        }

        if( ( unsigned int )i > ( unsigned int )nX )
        {
            i = 0;
            j ++;
            if( ( unsigned int )j > ( unsigned int )nY )
            {
                j = 0;
                k ++;
                if( ( unsigned int )k > ( unsigned int )nZ )
                {
                    k = 0;
                }
            }
        }
    }
    //cleanup?
    cell->Delete();
}
///////////////////////////////////////////
void VTKDataToTexture::_cleanUpFileNames()
{
    size_t len = 0;
    std::string tempName;// = 0;
    int index = 0;
    for( int i = 0; i < _nScalars; i++ )
    {
        len = _scalarNames.at( i ).size();
        tempName.clear();
        index = 0;
        for( size_t j = 0; j < len; j++ )
        {
            //skip spaces
            if( _scalarNames.at( i )[j] == ' ' )
            {
                continue;
            }
            //replace :'s, ['s, ]'s, ''s, -> w/ _'s
            if( ( _scalarNames.at( i )[j] == ':' ) ||
                    ( _scalarNames.at( i )[j] == '[' ) ||
                    ( _scalarNames.at( i )[j] == ']' ) ||
                    ( _scalarNames.at( i )[j] == '/' ) ||
                    ( _scalarNames.at( i )[j] == '\'' ) ||
                    ( _scalarNames.at( i )[j] == '^' )
              )
            {
                _scalarNames.at( i )[j] = '_';
            }
            tempName.push_back( _scalarNames.at( i )[j] );

        }
        _scalarNames.at( i ) = tempName;
    }
    for( int i = 0; i < _nVectors; i++ )
    {
        len = _vectorNames.at( i ).size();
        tempName.clear();// = new char[len +1];
        index = 0;
        for( size_t j = 0; j < len; j++ )
        {
            //skip spaces
            if( _vectorNames.at( i )[j] == ' ' )
            {
                continue;
            }
            //replace :'s, ['s, ]'s, ''s, -> w/ _'s
            if( ( _vectorNames.at( i )[j] == ':' ) ||
                    ( _vectorNames.at( i )[j] == '[' ) ||
                    ( _vectorNames.at( i )[j] == ']' ) ||
                    ( _vectorNames.at( i )[j] == '/' ) ||
                    ( _vectorNames.at( i )[j] == '\'' ) ||
                    ( _vectorNames.at( i )[j] == '^' ) )
            {
                _vectorNames.at( i )[j] = '_';
            }
            tempName.push_back( _vectorNames.at( i )[j] );

        }
        _vectorNames.at( i ) = tempName;
    }
    return;
}
///////////////////////////////////////////////////////////////////////
void VTKDataToTexture::_addOutSideCellDomainDataToFlowTexture( int index,
        int isScalar )
{
    if( isScalar )
    {
        FlowPointData data;

        data.setData( _scalarRanges.at( index )[0], 0, 0, 0 );
        data.setDataType( FlowPointData::SCALAR );
        _curScalar.at( 0 ).addPixelData( data );
    }
    else
    {
        FlowPointData data;
        //the vectors
        //this is quantized in the range(-1,1)
        data.setData( 127, 127, 127, 0 );
        data.setDataType( FlowPointData::VECTOR );
        _velocity.at( 0 ).addPixelData( data );
    }
}
///////////////////////////////////////////////////////////////////
void VTKDataToTexture::_interpolateDataInCell( vtkGenericCell* cell,
        double* weights,
        int whichValue,
        int isScalar )
{
    //list of point ids in cell
    vtkIdList* pointIds = 0;

    //list of point ids in cell
    pointIds = cell->GetPointIds();

    //number of verts in the cell
    int nCellPts = cell->GetNumberOfPoints();

    if( isScalar )
    {
        FlowPointData pixelData;// = new FlowPointData();;
        vtkDoubleArray* scalar = vtkDoubleArray::New();
        scalar->SetNumberOfComponents( 1 );
        scalar->SetNumberOfTuples( nCellPts );

        _extractTuplesForScalar( pointIds, scalar, whichValue );
        pixelData.setDataType( FlowPointData::SCALAR );
        _interpolatePixelData( pixelData, scalar, weights, nCellPts, whichValue );
        _curScalar.at( 0 ).addPixelData( pixelData );

        scalar->Delete();
    }
    else
    {
        FlowPointData pixelData;// = new FlowPointData();
        vtkDoubleArray* vector = vtkDoubleArray::New();
        vector->SetNumberOfComponents( 3 );
        vector->SetNumberOfTuples( nCellPts );

        _extractTuplesForVector( pointIds, vector, whichValue );
        pixelData.setDataType( FlowPointData::VECTOR );
        _interpolatePixelData( pixelData, vector, weights, nCellPts, whichValue );
        _velocity.at( 0 ).addPixelData( pixelData );
        vector->Delete();
    }
}
/////////////////////////////////////////////////////////////////
void VTKDataToTexture::_interpolatePixelData( FlowPointData& data,
        vtkDataArray* array,
        double* weights,
        int npts, int whichValue )
{
    if( data.type() == FlowPointData::VECTOR )
    {
        double vector[4];
        vector[0] = 0;
        vector[1] = 0;
        vector[2] = 0;
        double vectorData[3];
        for( int j = 0; j < npts; j++ )
        {
            array->GetTuple( j, vectorData );
            //the weighted sum of the velocities
            vector[0] += vectorData[0] * weights[j];
            vector[1] += vectorData[1] * weights[j];
            vector[2] += vectorData[2] * weights[j];
        }
        //calulate the magnitude
        double vMag = 0;
        double iMag = 0;
        vMag = sqrt( vector[0] * vector[0]
                     + vector[1] * vector[1]
                     + vector[2] * vector[2] );

        //inverse magnitude
        if( vMag >= 1e-6 )
        {
            iMag = 1.0 / vMag;
        }
        else
        {
            iMag = 0;
        }

        //normalize data
        vector[0] *= iMag;
        vector[1] *= iMag;
        vector[2] *= iMag;
        vector[3] = vMag;

        //quantize data
        vector[0] = 255 * ( ( vector[0] + 1 ) * .5 );
        vector[1] = 255 * ( ( vector[1] + 1 ) * .5 );
        vector[2] = 255 * ( ( vector[2] + 1 ) * .5 );

        data.setData( vector[0], vector[1], vector[2], vector[3] );
    }
    else if( data.type() == FlowPointData::SCALAR )
    {
        double scalarData;
        double scalar = 0;
        for( int j = 0; j < npts; j++ )
        {
            array->GetTuple( j, &scalarData );
            scalar += scalarData * weights[j];
        }
        if( scalar == 0 )
        {
            scalar = _scalarRanges.at( whichValue )[0];
        }
        data.setData( scalar, 0, 0, 0 );
    }
}
///////////////////////////////////////////////////////////////////
void VTKDataToTexture::_extractTuplesForVector( vtkIdList* ptIds,
        vtkDataArray* vector,
        int whichVector )
{
    int vecNumber = 0;
    for( int i = 0; i < _nPtDataArrays; i++ )
    {
        vtkDataArray* array = _dataSet->GetPointData()->GetArray( i );

        if( array->GetNumberOfComponents() != 3 )
        {
            continue;
        }
        // also, ignore arrays of normals...
        if( array->GetNumberOfComponents() == 3
                && ( !strcmp( array->GetName(), "normals" ) ) )
        {
            continue;
        }

        if( vecNumber != whichVector )
        {
            vecNumber++;
            continue;
        }
        else
        {
            array->GetTuples( ptIds, vector );
            vecNumber++;
        }
    }

}
///////////////////////////////////////////////////////////////////
void VTKDataToTexture::_extractTuplesForScalar( vtkIdList* ptIds,
        vtkDataArray* scalar,
        int whichScalar )
{
    int scaleNum = 0;
    for( int i = 0; i < _nPtDataArrays; i++ )
    {
        vtkDataArray* array = _dataSet->GetPointData()->GetArray( i );

        if( array->GetNumberOfComponents() != 1 )
        {
            continue;
        }

        if( scaleNum != whichScalar )
        {
            scaleNum++;
            continue;
        }
        else
        {
            array->GetTuples( ptIds, scalar );
            scaleNum++;
        }
    }
}
/////////////////////////////////////////////////////////////////////////
int VTKDataToTexture::countNumberOfParameters( const int numComponents )
{
    int numParameters = 0;

    // count the number of paraneters containing numComponents components...
    for( int i = 0; i < _nPtDataArrays; i++ )
    {
        vtkDataArray* array = _dataSet->GetPointData()->GetArray( i );

        if( array->GetNumberOfComponents() != numComponents )
        {
            continue;
        }

        // also, ignore arrays of normals...
        if( numComponents == 3 && ( ! strcmp( array->GetName(), "normals" ) ) )
        {
            continue;
        }

        numParameters++;
    }
    return numParameters;
}
/////////////////////////////////////////////////////////////////////
std::vector<std::string> VTKDataToTexture::getParameterNames( const int numComponents,
        const int numParameters )
{
    boost::ignore_unused_variable_warning( numParameters );

    std::vector<std::string> names;
    int ii = 0;

    for( int i = 0; i < _nPtDataArrays; i++ )
    {
        vtkDataArray* array = _dataSet->GetPointData()->GetArray( i );

        if( array->GetNumberOfComponents() != numComponents )
        {
            continue;
        }

        // also, ignore arrays of normals...
        if( numComponents == 3 && ( ! strcmp( array->GetName(), "normals" ) ) )
        {
            continue;
        }
        if( numComponents == 1 )
        {
            double* range = new double[2];
            array->GetRange( range, -1 );
            _scalarRanges.push_back( range );

        }
        else if( numComponents == 3 )
        {
            double* range = new double[2];
            array->GetRange( range, -1 );
            _vectorRanges.push_back( range );
        }
        /*name[ii] = new char [ strlen( array->GetName() ) + 1 ];
        strcpy( name[ii], array->GetName() );*/
        names.push_back( array->GetName() );
        ii++;
    }
    return names;
}
/////////////////////////////////////////////////////////
void VTKDataToTexture::_applyCorrectedNamesToDataArrays()
{
    unsigned int scalarIndex = 0;
    unsigned int vectorIndex = 0;
    unsigned int numComponents = 1;
    for( int i = 0; i < _nPtDataArrays; i++ )
    {
        vtkDataArray* array = _dataSet->GetPointData()->GetArray( i );
        numComponents = array->GetNumberOfComponents();
        if( numComponents == 1 )
        {
            array->SetName( _scalarNames[scalarIndex++].c_str() );
            continue;
        }

        // also, ignore arrays of normals...
        if( numComponents == 3 && ( strcmp( array->GetName(), "normals" ) ) )
        {
            array->SetName( _vectorNames[vectorIndex++].c_str() );
            continue;
        }
    }
}
////////////////////////////////////////////////////////////
void VTKDataToTexture::writeVelocityTexture( int whichVector )
{
    std::string nameString;

    if( _outputDir )
    {
        nameString = _outputDir;
    }
    else
    {
        nameString = "./";
    }

    boost::filesystem::path vectorPath( nameString, boost::filesystem::no_check );
    vectorPath /= std::string( "vectors" );
    try
    {
        if( !boost::filesystem::is_directory( vectorPath ) )
        {
            boost::filesystem::create_directory( vectorPath );
        }
    }
    catch( const std::exception& ex )
    {
        std::cout << ex.what() << std::endl;
        boost::filesystem::create_directory( vectorPath );
        std::cout << "...so we made it for you..." << std::endl;
    }

    vectorPath /= ( _vectorNames[whichVector] );
    try
    {
        if( !boost::filesystem::is_directory( vectorPath ) )
        {
            boost::filesystem::create_directory( vectorPath );
        }
    }
    catch( const std::exception& ex )
    {
        std::cout << ex.what() << std::endl;
        boost::filesystem::create_directory( vectorPath );
        std::cout << "...so we made it for you..." << std::endl;
    }
    nameString.clear();
    //nameString.append( "/" );
    nameString.append( vectorPath.string() );
    nameString.append( "/" );
    nameString.append( _vectorNames[whichVector] );

    if( _vFileName )
    {
        nameString.append( _vFileName );
    }
    else
    {
        nameString.append( "flow" );
    }
    nameString.append( ".vti" );

    double velRange[2] = {0, 0};
    velRange[0] = _vectorRanges.at( whichVector )[0];
    velRange[1] = _vectorRanges.at( whichVector )[1];
    wxString msg = wxString( "Writing file: ", wxConvUTF8 ) + wxString( "\n", wxConvUTF8 ) + wxString( nameString.c_str(), wxConvUTF8 );
    _updateTranslationStatus( ConvertUnicode( msg.c_str() ) );
    _velocity.at( 0 ).writeFlowTexture( nameString, std::string( _vectorNames[whichVector] ) );
}
///////////////////////////////////////////
void VTKDataToTexture::writeScalarTexture( int whichScalar )
{
    std::string nameString;
    if( _outputDir )
    {
        nameString = _outputDir;
    }
    else
    {
        nameString = ".";
    }

    boost::filesystem::path scalarPath( nameString, boost::filesystem::no_check );
    scalarPath /= std::string( "scalars" );
    try
    {
        if( !boost::filesystem::is_directory( scalarPath ) )
        {
            boost::filesystem::create_directory( scalarPath );
        }
    }
    catch( const std::exception& ex )
    {
        std::cout << ex.what() << std::endl;
        boost::filesystem::create_directory( scalarPath );
        std::cout << "...so we made it for you..." << std::endl;
    }

    scalarPath /= ( _scalarNames[whichScalar] );
    try
    {
        if( !boost::filesystem::is_directory( scalarPath ) )
        {
            boost::filesystem::create_directory( scalarPath );
        }
    }
    catch( const std::exception& ex )
    {
        std::cout << ex.what() << std::endl;
        boost::filesystem::create_directory( scalarPath );
        std::cout << "...so we made it for you..." << std::endl;
    }

    nameString.clear();
    //nameString.append( "/" );
    nameString.append( scalarPath.string() );
    nameString.append( "/" );
    nameString.append( _scalarNames[whichScalar] );
    if( _vFileName )
    {
        nameString.append( _vFileName );
    }
    else
    {
        nameString.append( "scalar" );
    }
    nameString.append( ".vti" );

    double scalarRange[2] = {0, 0};
    scalarRange[0] = _scalarRanges.at( whichScalar )[0];
    scalarRange[1] = _scalarRanges.at( whichScalar )[1];
    wxString msg = wxString( "Writing file: ", wxConvUTF8 ) + wxString( "\n", wxConvUTF8 ) + wxString( nameString.c_str(), wxConvUTF8 );
    _updateTranslationStatus( ConvertUnicode( msg.c_str() ) );
    _curScalar.at( 0 ).writeFlowTexture( nameString, std::string( _scalarNames[whichScalar] ) );
}

