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

#include <latticefx/core/vtk/DataSet.h>
#include <latticefx/core/vtk/vtkActorToOSG.h>

#include <latticefx/utils/vtk/AccessoryFunctions.h>
#include <latticefx/utils/vtk/fileIO.h>
#include <latticefx/utils/vtk/readWriteVtkThings.h>
#include <latticefx/utils/vtk/Grid2Surface.h>
#include <latticefx/utils/vtk/VTKFileHandler.h>
#include <latticefx/utils/vtk/ComputeVectorMagnitudeRangeCallback.h>
#include <latticefx/utils/vtk/ComputeDataObjectBoundsCallback.h>
#include <latticefx/utils/vtk/CountNumberOfParametersCallback.h>
#include <latticefx/utils/vtk/GetNumberOfPointsCallback.h>
#include <latticefx/utils/vtk/ProcessScalarRangeCallback.h>
#include <latticefx/utils/vtk/ActiveDataInformationCallback.h>
#include <latticefx/utils/vtk/CreateDataObjectBBoxActorsCallback.h>
#include <latticefx/utils/vtk/ComputeVectorMagnitudeAndScalarsCallback.h>

#include <latticefx/translators/vtk/DataLoader.h>

#include <vtkLookupTable.h>
#include <vtkPointData.h>
#include <vtkDataSet.h>
#include <vtkDataObject.h>
#include <vtkPolyData.h>
#include <vtkFloatArray.h>
#include <vtkUnstructuredGrid.h>
#include <vtkUnstructuredGridReader.h>
#include <vtkSystemIncludes.h>  // for VTK_POLY_DATA
#include <vtkCellTypes.h>
#include <vtkCellDataToPointData.h>
#include <vtkCellData.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkGeometryFilter.h>
#include <vtkCompositeDataGeometryFilter.h>
#include <vtkCompositeDataSet.h>
#include <vtkCompositeDataIterator.h>
#include <vtkTemporalDataSet.h>
#include <vtkAlgorithm.h>
#include <vtkCharArray.h>
#include <vtkMultiBlockDataSet.h>

#include <iostream>
#include <sstream>
#include <vector>

#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>

//Used for strcpy to setup the data translator code
#include <cstring>

namespace lfx
{
namespace core
{
namespace vtk
{
////////////////////////////////////////////////////////////////////////////////
DataSet::DataSet()
    :
    //m_tempModel( 0 ),
    actualScalarRange( 0 ),
    displayedScalarRange( 0 ),
    isNewlyActivated( 0 ),
    range( 0 ),
    definedRange( 0 ),
    vectorMagRange( 0 ),
    bbDiagonal( 10 ),
    meanCellBBLength( 0.0 ),
    stepLength( 0.0 ),
    maxTime( 1000 ),
    timeStep( 1 ),
    m_greyscaleFlag( false ),
    lut( vtkLookupTable::New() ),
    m_dataSet( 0 ),
    datasetType( -1 ),
    activeScalar( -1 ),
    activeVector( -1 ),
    //x_planes( 0 ),
    //y_planes( 0 ),
    //z_planes( 0 ),
    arrow( 0 ),
    numPtDataArrays( 0 ),
    numScalars( 0 ),
    numVectors( 0 ),
    //dataSetAxes( 0 ),
    //dataSetScalarBar( 0 ),
    _vtkFHndlr( 0 ),
    m_dataObjectHandler( 0 ),
    partOfTransientSeries( 0 ),
    m_externalFileLoader( 0 ),
    m_isPartOfCompositeDataset( false )//,
    //m_logger( Poco::Logger::get( "xplorer.DataSet" ) ),
    //m_logStream( ves::xplorer::LogStreamPtr( new Poco::LogStream( m_logger ) ) )
{
    this->range = new double [ 2 ];
    this->range[ 0 ] = 0.0f;
    this->range[ 1 ] = 1.0f;
    this->definedRange = new double [ 2 ];
    this->definedRange[ 0 ] = 0.0f;
    this->definedRange[ 0 ] = 1.0f;
    // precomputed data that descends from a flowdata.vtk should
    // automatically have the same color mapping as the "parent"
    // By default, the dataset is assumed to have no parent, that is,
    // use its own range to determine color mapping.
    this->switchNode = new osg::Switch();
    this->switchNode->setName( "switch_for_data_viz" );
    this->classic = new osg::Group();
    this->classic->setName( "classic" );
    this->switchNode->addChild( this->classic.get() );
    this->textureBased = new osg::Group();
    this->textureBased->setName( "textureBased" );
    this->switchNode->addChild( this->textureBased.get() );
    this->switchNode->setSingleChildOn( 0 );

    m_bounds[0] = 100000;
    m_bounds[1] = -100000;
    m_bounds[2] = 100000;
    m_bounds[3] = -100000;
    m_bounds[4] = 100000;
    m_bounds[5] = -100000;

    m_dataObjectOps["Compute Bounds"] = new lfx::vtk_utils::ComputeDataObjectBoundsCallback();
    m_dataObjectOps["Compute Vector Magnitude Range"] = new lfx::vtk_utils::ComputeVectorMagnitudeRangeCallback();
    m_dataObjectOps["Count Number Of Vectors And Scalars"] = new lfx::vtk_utils::CountNumberOfParametersCallback();
    m_dataObjectOps["Number Of Grid Points"] = new lfx::vtk_utils::GetNumberOfPointsCallback();
    m_dataObjectOps["Scalar Range Information"] = new lfx::vtk_utils::ProcessScalarRangeCallback();
    m_dataObjectOps["Active Data Information"] = new lfx::vtk_utils::ActiveDataInformationCallback();
    m_dataObjectOps["Create BBox Actors"] = new lfx::vtk_utils::CreateDataObjectBBoxActorsCallback();
    m_dataObjectOps["Compute Vector Mag and Scalars"] = new lfx::vtk_utils::ComputeVectorMagnitudeAndScalarsCallback();
}
////////////////////////////////////////////////////////////////////////////////
DataSet::~DataSet()
{
    dataSetUUIDMap.clear();
    m_activeDataArrays.clear();
    parent = vtk::DataSetPtr();

    this->lut->Delete();
    this->lut = NULL;

    delete [] this->definedRange;
    this->definedRange = NULL;

    delete [] this->range;
    this->range = NULL;

    if( this->numScalars > 0 )
    {
        scalarName.clear();

        for( int i = 0; i < this->numScalars; i++ )
        {
            delete [] this->actualScalarRange[i];
            delete [] this->displayedScalarRange[i];
        }
        delete [] this->actualScalarRange;
        this->actualScalarRange = NULL;
        delete [] this->displayedScalarRange;
        this->displayedScalarRange = NULL;
    }

    if( this->numVectors > 0 )
    {
        vectorName.clear();

        delete [] this->vectorMagRange;
        this->vectorMagRange = NULL;
    }

    for( std::map<std::string, lfx::vtk_utils::DataObjectHandler::DatasetOperatorCallback* >::const_iterator
            iter = m_dataObjectOps.begin(); iter != m_dataObjectOps.end(); ++iter )
    {
        delete iter->second;
    }
    m_dataObjectOps.clear();

    m_childDataSets.clear();

    if( m_dataSet )
    {
        //This dataset could be part of a composite dataset which would mean
        //its memory is handled by another destructor
        if( !m_isPartOfCompositeDataset )
        {
            m_dataSet->Delete();
        }

        m_dataSet = NULL;
    }

    if( _vtkFHndlr )
    {
        delete _vtkFHndlr;
        _vtkFHndlr = 0;
    }

    if( m_dataObjectHandler )
    {
        delete m_dataObjectHandler;
        m_dataObjectHandler = 0;
    }

    if( dcs.valid() )
    {
        if( dcs->getNumParents() > 0 )
        {
            osg::Group* parent = dcs->getParent( 0 );
            parent->removeChild( dcs.get() );
        }
    }
}
////////////////////////////////////////////////////////////////////////////////
void DataSet::SetRange( double* dataRange )
{
    this->SetRange( dataRange[ 0 ], dataRange[ 1 ] );
}
////////////////////////////////////////////////////////////////////////////////
void DataSet::SetRange( double dataMin, double dataMax )
{
    this->range[ 0 ] = dataMin;
    this->range[ 1 ] = dataMax;
}
////////////////////////////////////////////////////////////////////////////////
void DataSet::GetRange( double* dataRange )
{
    this->GetRange( dataRange[ 0 ], dataRange[ 1 ] );
}
////////////////////////////////////////////////////////////////////////////////
void DataSet::GetRange( double& dataMin, double& dataMax )
{
    dataMin = this->range[ 0 ];
    dataMax = this->range[ 1 ];
}
////////////////////////////////////////////////////////////////////////////////
double* DataSet::GetRange()
{
    return this->range;
}
////////////////////////////////////////////////////////////////////////////////
void DataSet::SetUserRange( double userRange[2] )
{
    this->SetUserRange( userRange[0], userRange[1] );
}
////////////////////////////////////////////////////////////////////////////////
void DataSet::SetUserRange( double userMin, double userMax )
{
    this->definedRange[0] = userMin;
    this->definedRange[1] = userMax;
    this->SetDisplayedScalarRange( this->activeScalar, this->definedRange );
}
////////////////////////////////////////////////////////////////////////////////
void DataSet::GetUserRange( double userRange[2] )
{
    this->GetUserRange( userRange[0], userRange[1] );
}
////////////////////////////////////////////////////////////////////////////////
void DataSet::GetUserRange( double& userMin, double& userMax )
{
    userMin = this->definedRange[0];
    userMax = this->definedRange[1];
}
////////////////////////////////////////////////////////////////////////////////
double* DataSet::GetUserRange()
{
    return this->definedRange;
}
////////////////////////////////////////////////////////////////////////////////
void DataSet::SetLength( float len )
{
    this->bbDiagonal = len;
}
////////////////////////////////////////////////////////////////////////////////
/*void DataSet::GetLength( float &len )
{
   len = this->bbDiagonal;
}

float DataSet::GetLength()
{
   return this->bbDiagonal;
}*/

/*void DataSet::GetMeanCellLength( float &len )
{
   len = this->meanCellBBLength;
}

float DataSet::GetMeanCellLength()
{
   return this->meanCellBBLength;
}*/
////////////////////////////////////////////////////////////////////////////////
void DataSet::SetStepLength( float sLen )
{
    this->stepLength = sLen;
}
////////////////////////////////////////////////////////////////////////////////
void DataSet::GetStepLength( float& sLen )
{
    sLen = this->stepLength;
}
////////////////////////////////////////////////////////////////////////////////
float DataSet::GetStepLength()
{
    return this->stepLength;
}
////////////////////////////////////////////////////////////////////////////////
void DataSet::SetMaxTime( float mT )
{
    this->maxTime = mT;
}
////////////////////////////////////////////////////////////////////////////////
void DataSet::GetMaxTime( float& mT )
{
    mT = this->maxTime;
}
////////////////////////////////////////////////////////////////////////////////
float DataSet::GetMaxTime()
{
    return this->maxTime;
}
////////////////////////////////////////////////////////////////////////////////
void DataSet::SetTimeStep( float tStep )
{
    this->timeStep = tStep;
}
////////////////////////////////////////////////////////////////////////////////
void DataSet::GetTimeStep( float& tStep )
{
    tStep = this->timeStep;
}
////////////////////////////////////////////////////////////////////////////////
float DataSet::GetTimeStep()
{
    return this->timeStep;
}
////////////////////////////////////////////////////////////////////////////////
vtkLookupTable* DataSet::GetLookupTable()
{
    if( m_greyscaleFlag )
    {
        SetGreyscale();
    }
    else
    {
        SetColorscale();
    }
    return this->lut;
}
////////////////////////////////////////////////////////////////////////////////
void DataSet::SetGreyscale()
{
    this->lut->SetNumberOfColors( 402 );            //default is 256
    this->lut->SetHueRange( 0.0f , 0.0f );
    this->lut->SetSaturationRange( 0.0f , 0.0f );
    this->lut->SetValueRange( 0.2f , 1.0f );
    this->lut->SetTableRange( this->definedRange );
    this->lut->ForceBuild();
}
////////////////////////////////////////////////////////////////////////////////
void DataSet::SetColorscale()
{
    //http://www.ncsu.edu/scivis/lessons/colormodels/color_models2.html#hue.
    this->lut->SetNumberOfColors( 256 );            //default is 256
    this->lut->SetHueRange( 2.0f / 3.0f, 0.0f );    //a blue-to-red scale
    this->lut->SetSaturationRange( 1.0f , 1.0f );
    //double* temp = this->lut->GetSaturationRange();
    //std::cout << temp[ 0 ] << " " << temp[ 1 ] << std::endl;
    this->lut->SetValueRange( 1.0f , 1.0f );
    //temp = this->lut->GetValueRange();
    //std::cout << temp[ 0 ] << " " << temp[ 1 ] << std::endl;
    this->lut->SetTableRange( this->definedRange );
    this->lut->ForceBuild();
}
////////////////////////////////////////////////////////////////////////////////
bool DataSet::GetGreyscaleFlag()
{
    return m_greyscaleFlag;
}
////////////////////////////////////////////////////////////////////////////////
void DataSet::SetGreyscaleFlag( bool flag )
{
    m_greyscaleFlag = flag;
}
////////////////////////////////////////////////////////////////////////////////
vtkUnstructuredGrid* DataSet::GetUnsData()
{
    if( ! this->m_dataSet )
    {
        return NULL;
    }

    if( ! this->m_dataSet->IsA( "vtkUnstructuredGrid" ) )
    {
        return NULL;
    }

    return ( vtkUnstructuredGrid* )this->m_dataSet;
}
////////////////////////////////////////////////////////////////////////////////
vtkPolyData* DataSet::GetPolyData()
{
    if( ! this->m_dataSet )
    {
        return NULL;
    }

    if( ! this->m_dataSet->IsA( "vtkPolyData" ) )
    {
        return NULL;
    }

    return ( vtkPolyData* )this->m_dataSet;
}
////////////////////////////////////////////////////////////////////////////////
vtkDataObject* DataSet::GetDataSet()
{
    return this->m_dataSet;
}
////////////////////////////////////////////////////////////////////////////////
int DataSet::GetAllDataSets(std::vector<vtkDataSet*> *pList)
{
	int count = 0;
	if (m_dataSet->IsA( "vtkDataSet" ))
	{
		vtkDataSet* pset = vtkDataSet::SafeDownCast(m_dataSet);
		if (pset)
		{
			pList->push_back(pset);
			count++;
		}
	}

	for (unsigned int i=0; i<m_childDataSets.size(); i++)
	{
		count += m_childDataSets[i]->GetAllDataSets(pList);
	}
	
	return count;
}
////////////////////////////////////////////////////////////////////////////////
void DataSet::SetType()
{
    // this code only needs to be done once for each dataset...
    if( this->datasetType == -1 )
    {
        int dataObjectType = this->m_dataSet->GetDataObjectType();

        this->datasetType = 0;
        // see if file is a polydata containing only vertex cells
        // (droplet or particle)
        if( dataObjectType == VTK_POLY_DATA )
        {
            vtkCellTypes* types = vtkCellTypes::New();
            vtkPolyData* pData = this->GetPolyData();
            pData->GetCellTypes( types );
            if( types->GetNumberOfTypes() == 1 &&
                    pData->GetCellType( 0 ) == VTK_VERTEX )
            {
                this->datasetType = 1;
            }
            else
            {
                this->datasetType = 2;
            }
            types->Delete();
        }
    }
}
////////////////////////////////////////////////////////////////////////////////
void DataSet::SetType( int type )
{
    if( 0 <= type && type < 3 )
    {
        this->datasetType = type;
    }
}
////////////////////////////////////////////////////////////////////////////////
int DataSet::GetType()
{
    return this->datasetType;
}
////////////////////////////////////////////////////////////////////////////////
#ifdef USE_OMP
vtkUnstructuredGrid* DataSet::GetData( int i )
{
    return this->data[i];
}
////////////////////////////////////////////////////////////////////////////////
int DataSet::GetNoOfDataForProcs()
{
    return this->noOfData;
}
#endif
////////////////////////////////////////////////////////////////////////////////
void DataSet::LoadData( vtkUnstructuredGrid* dataset, int datasetindex )
{
    std::cout << "[DBG]...Inside LoadData of DataSet" << std::endl;
    this->SetFileName_OnFly( datasetindex );

    vtkPointSet* pointset = NULL;
    pointset = dataset;
    this->meanCellBBLength = lfx::vtk_utils::AccessoryFunctions::ComputeMeanCellBBLength( pointset );

    vtkFloatArray* array = vtkFloatArray::New();
    array->SetName( "meanCellBBLength" );
    array->SetNumberOfComponents( 1 );
    array->SetNumberOfTuples( 1 );
    array->SetTuple1( 0, meanCellBBLength );
    pointset->GetFieldData()->AddArray( array );

    //array->Delete();
    //dumpVerticesNotUsedByCells(pointset);

    this->LoadData();
}
////////////////////////////////////////////////////////////////////////////////
void DataSet::LoadData()
{
    if( m_dataSet != NULL )
    {
        return;
    }

    std::string extension = lfx::vtk_utils::fileIO::getExtension( fileName );
    //What should the extension of the star.param file be?
    //The translator expects ".star" but we have things setup to
    //be ".param". One or the other should be changed for consistency.
    if( extension == "param" )
    {
        extension = "star";
    }
    if( extension == "case" )
    {
        extension = "ens";
    }

    vtkDataObject* translatedDataObject = 0;
    if( ( extension.find( "vtk" ) != std::string::npos ) ||
            ( extension.find( "vtu" ) != std::string::npos ) ||
            ( extension.find( "vtp" ) != std::string::npos ) ||
            ( extension.find( "vtm" ) != std::string::npos ) ||
            ( extension.find( "vti" ) != std::string::npos ) )
    {
        if( !_vtkFHndlr )
        {
            _vtkFHndlr = new lfx::vtk_utils::VTKFileHandler();
        }
        _vtkFHndlr->SetScalarsAndVectorsToRead( m_activeDataArrays );
        translatedDataObject = _vtkFHndlr->GetDataSetFromFile( fileName );
    }
    else
    {
        if( !m_externalFileLoader )
        {
            m_externalFileLoader = new lfx::vtk_translator::DataLoader();
        }
        m_externalFileLoader->SetInputData( "something", "somedir" );
        m_externalFileLoader->SetScalarsAndVectorsToRead( m_activeDataArrays );
        unsigned int nParams = 7;
        char** parameters = new char*[nParams];
        parameters[0] = new char[strlen( "loaderToVtk" ) + 1];
        std::strcpy( parameters[0], "loaderToVtk" );
        parameters[1] = new char[strlen( "-singleFile" ) + 1];
        std::strcpy( parameters[1], "-singleFile" );
        parameters[2] = new char[fileName.length()   + 1];
        std::strcpy( parameters[2], fileName.c_str() );
        parameters[3] = new char[strlen( "-loader" ) + 1];
        std::strcpy( parameters[3], "-loader" );
        parameters[4] = new char[extension.length() + 1];
        std::strcpy( parameters[4], extension.c_str() );
        parameters[5] = new char[strlen( "-o" ) + 1];
        std::strcpy( parameters[5], "-o" );
        parameters[6] = new char[strlen( "." ) + 1];
        std::strcpy( parameters[6], "." );
        //parameters[7] = new char[strlen( "-w" ) + 1];
        //strcpy(parameters[7], "-w" );
        //parameters[8] = new char[strlen( "stream" ) + 1];
        //strcpy(parameters[8], "stream" );

        translatedDataObject =
            m_externalFileLoader->GetVTKDataSet( nParams, parameters );

        for( unsigned int i = 0; i < nParams; ++i )
        {
            delete [] parameters[i];
        }
        delete parameters;
        if( !translatedDataObject )
        {
            //vprDEBUG( vesDBG, 1 ) << "|\tInvalid input file: " << fileName
            //                      << std::endl << vprDEBUG_FLUSH;
            return;
        }
    }

    //Now initialize the data based on the type of dataset
    if( translatedDataObject->IsA( "vtkTemporalDataSet" ) )
    {
        LoadTemporalDataSet( translatedDataObject );
    }
    else
    {
        InitializeVTKDataObject( translatedDataObject );
    }
}
////////////////////////////////////////////////////////////////////////////////
void DataSet::LoadData( vtkDataSet* tempDataset, bool isPartOfCompositeDataset )
{
    if( this->m_dataSet != NULL )
    {
        return;
    }

    m_isPartOfCompositeDataset = isPartOfCompositeDataset;

    m_dataSet = tempDataset;

    if( !m_dataObjectHandler )
    {
        m_dataObjectHandler = new lfx::vtk_utils::DataObjectHandler();
    }
    m_dataObjectHandler->OperateOnAllDatasetsInObject( m_dataSet );

    //Need to get number of pda
    this->numPtDataArrays = m_dataObjectHandler->GetNumberOfDataArrays();

    // count the number of scalars and store names and ranges...
    StoreScalarInfo();

    // count the number of vectors and store names ...
    this->numVectors = dynamic_cast<lfx::vtk_utils::CountNumberOfParametersCallback*>
                       ( m_dataObjectOps["Count Number Of Vectors And Scalars"] )->GetNumberOfParameters( true );
    if( this->numVectors )
    {
        this->vectorName = dynamic_cast<lfx::vtk_utils::CountNumberOfParametersCallback*>
                           ( m_dataObjectOps["Count Number Of Vectors And Scalars"] )->GetParameterNames( true );
    }

    // if there are point data, set the first scalar and vector as active...
    if( this->numPtDataArrays )
    {
        // set the first scalar and vector as active
        if( this->numScalars )
        {
            this->SetActiveScalar( 0 );
        }

        if( this->numVectors )
        {
            this->SetActiveVector( 0 );
            if( !this->vectorMagRange )
            {
                this->vectorMagRange = new double[2];
            }
            lfx::vtk_utils::ComputeVectorMagnitudeRangeCallback* vecMagRangeCbk =
                dynamic_cast<lfx::vtk_utils::ComputeVectorMagnitudeRangeCallback*>
                ( m_dataObjectOps["Compute Vector Magnitude Range"] );
            m_dataObjectHandler->SetDatasetOperatorCallback( vecMagRangeCbk );
            m_dataObjectHandler->OperateOnAllDatasetsInObject( this->m_dataSet );
            vecMagRangeCbk->GetVectorMagnitudeRange( this->vectorMagRange );
        }
    }

    SetType();

    //Register this dataset with the modeldatahandler
}
////////////////////////////////////////////////////////////////////////////////
unsigned int DataSet::GetNumberOfPoints()
{
    if( m_dataObjectHandler )
    {
        lfx::vtk_utils::GetNumberOfPointsCallback* numberOfPointsCallback =
            dynamic_cast<lfx::vtk_utils::GetNumberOfPointsCallback*>
            ( m_dataObjectOps["Number Of Grid Points"] );
        m_dataObjectHandler->SetDatasetOperatorCallback( numberOfPointsCallback );
        m_dataObjectHandler->OperateOnAllDatasetsInObject( this->m_dataSet );
        return numberOfPointsCallback->GetNumberOfPoints();

    }
    return 0;
}
////////////////////////////////////////////////////////////////////////////////
double* DataSet::GetBounds()
{
    GetBounds( m_bounds );
    return m_bounds;
}
////////////////////////////////////////////////////////////////////////////////
void DataSet::GetBounds( double* bounds )
{
    if( m_dataObjectHandler )
    {
        lfx::vtk_utils::ComputeDataObjectBoundsCallback* boundsCallback =
            dynamic_cast<lfx::vtk_utils::ComputeDataObjectBoundsCallback*>
            ( m_dataObjectOps["Compute Bounds"] );
        m_dataObjectHandler->SetDatasetOperatorCallback( boundsCallback );
        m_dataObjectHandler->OperateOnAllDatasetsInObject( this->m_dataSet );
        boundsCallback->GetDataObjectBounds( bounds );
        this->bbDiagonal = boundsCallback->GetDataObjectBoundsDiagonal();
    }
}
////////////////////////////////////////////////////////////////////////////////
void DataSet::SetActiveScalar( const std::string& tempActiveScalar )
{
    if( tempActiveScalar.empty() || scalarName.empty() )
    {
        this->activeScalar = -1;
        return;
    }

    int scalar = -1;
    for( int i = 0; i <  numScalars; ++i )
    {
        if( scalarName[ i ] == tempActiveScalar )
        {
            scalar = i;
            break;
        }
    }

    if( scalar < 0 || this->numScalars <= scalar )
    {
        std::cerr << "|\tWarning: SetActiveScalar: out-of-range scalar "
                  << scalar << ", will use first scalar " << this->scalarName[ 0 ]
                  << std::endl;
        this->activeScalar = 0;
    }
    else
    {
        this->activeScalar = scalar;
    }
    /*    lfx::vtk_utils::ActiveDataInformationCallback* activeDataInfoCbk =
            dynamic_cast<lfx::vtk_utils::ActiveDataInformationCallback*>
            ( m_dataObjectOps["Active Data Information"] );
        activeDataInfoCbk->SetActiveDataName( this->scalarName[ this->activeScalar ] );
        m_dataObjectHandler->SetDatasetOperatorCallback( activeDataInfoCbk );
        m_dataObjectHandler->OperateOnAllDatasetsInObject( this->m_dataSet );
    */
    /*for( int i = 0; i < 3; i++ )
    {
        int numPlanes = 0;
        if( this->GetPrecomputedSlices( i ) )
        {
            numPlanes = this->GetPrecomputedSlices( i )->GetNumberOfPlanes();
        }
        vprDEBUG( vesDBG, 1 ) << "\tnumPlanes = " << numPlanes
        << std::endl << vprDEBUG_FLUSH;

        if( numPlanes > 0 )
        {
            for( int j = 0; j < numPlanes; j++ )
            {
                this->GetPrecomputedSlices( i )->GetPlane( j )
                ->GetPointData()->SetActiveScalars(
                    this->scalarName[ this->activeScalar ].c_str() );
            }
        }
    }*/
    // Store the actual range of the active scalar...
    double* temp = this->GetActualScalarRange( this->activeScalar );
    this->range [ 0 ] = temp[ 0 ];
    this->range [ 1 ] = temp[ 1 ];
    //vprDEBUG( vesDBG, 1 ) << "|\t\trange[0] = " << this->range[0]
    //                      << ", range[1] = " << this->range[1]
    //                      << std::endl << vprDEBUG_FLUSH;

    temp = this->GetDisplayedScalarRange( this->activeScalar );
    this->definedRange[ 0 ] = temp[ 0 ];
    this->definedRange[ 1 ] = temp[ 1 ];

    // Step length for streamline integration
    this->stepLength = this->bbDiagonal / 5.0f ;
    //vprDEBUG( vesDBG, 1 ) << "|\t\tSetActiveScalar: stepLength = "
    //                      << this->stepLength << std::endl << vprDEBUG_FLUSH;

    // Maximum integration time for streamline integration
    this->maxTime = 5.0f * this->bbDiagonal /
                    ( ( this->range[1] - this->range[0] ) * 0.5f );
    //vprDEBUG( vesDBG, 1 ) << "|\t\tSetActiveScalar: maxTime = "
    //                      << this->maxTime << std::endl << vprDEBUG_FLUSH;

    // Time step for streamline integration
    this->timeStep = this->bbDiagonal / this->definedRange[1];
    //vprDEBUG( vesDBG, 1 ) << "|\t\tSetActiveScalar: timeStep = "
    //                      << this->timeStep << std::endl << vprDEBUG_FLUSH;
}
////////////////////////////////////////////////////////////////////////////////
void DataSet::SetActiveScalar( int scalar )
{
    if( this->numScalars == 0 )
    {
        this->activeScalar = -1;
        return;
    }

    if( scalar < 0 || this->numScalars <= scalar )
    {
        std::cerr << "|\tWarning: SetActiveScalar: out-of-range scalar "
                  << scalar << ", will use first scalar " << this->scalarName[ 0 ]
                  << std::endl;
        this->activeScalar = 0;
    }
    else
    {
        this->activeScalar = scalar;

        //vprDEBUG( vesDBG, 1 )
        //        << "|\t\tDataSet::SetActiveScalar: requested activeScalar = "
        //        << this->activeScalar << ", scalarName = "
        //        << this->scalarName[ this->activeScalar ]
        //        << std::endl << vprDEBUG_FLUSH;
    }

    SetActiveScalar( this->scalarName[ this->activeScalar ] );

    /*vprDEBUG(vesDBG,1) << "\tSetActiveScalar: Active scalar is \""
         << this->GetDataSet()->GetPointData()->GetScalars()->GetName()
         << "\"" << std::endl << vprDEBUG_FLUSH;*/

    /*for( int i = 0; i < 3; i++ )
    {
        int numPlanes = 0;
        if( this->GetPrecomputedSlices( i ) )
        {
            numPlanes = this->GetPrecomputedSlices( i )->GetNumberOfPlanes();
        }
        vprDEBUG( vesDBG, 1 ) << "\tnumPlanes = " << numPlanes
        << std::endl << vprDEBUG_FLUSH;

        if( numPlanes > 0 )
        {
            for( int j = 0; j < numPlanes; j++ )
            {
                this->GetPrecomputedSlices( i )->GetPlane( j )
                ->GetPointData()->SetActiveScalars(
                    this->scalarName[ this->activeScalar ].c_str() );
            }
        }
    }*/
    // Store the actual range of the active scalar...
    /*double * temp = this->GetActualScalarRange( this->activeScalar );
    this->range [ 0 ] = temp[ 0 ];
    this->range [ 1 ] = temp[ 1 ];
    vprDEBUG( vesDBG, 1 ) << "|\t\trange[0] = " << this->range[0]
    << ", range[1] = " << this->range[1]
    << std::endl << vprDEBUG_FLUSH;

    temp = this->GetDisplayedScalarRange( this->activeScalar );
    this->definedRange[ 0 ] = temp[ 0 ];
    this->definedRange[ 1 ] = temp[ 1 ];
    vprDEBUG( vesDBG, 1 ) << "|\t\tdefinedRange[0] = " << this->definedRange[0]
    << ", definedRange[1] = " << this->definedRange[1]
    << std::endl << vprDEBUG_FLUSH;

    vprDEBUG( vesDBG, 1 ) << "|\t\tactualScalarRange[0][0] = "
    << this->actualScalarRange[0][0]
    << ", actualScalarRange[0][1] = "
    << this->actualScalarRange[0][1]
    << std::endl << vprDEBUG_FLUSH;

    vprDEBUG( vesDBG, 1 ) << "|\t\tdisplayedScalarRange[0][0] = "
    << this->displayedScalarRange[0][0]
    << ", displayedScalarRange[0][1] = "
    << this->displayedScalarRange[0][1]
    << std::endl << vprDEBUG_FLUSH;

    // Step length for streamline integration
    this->stepLength = this->bbDiagonal / 5.0f ;
    vprDEBUG( vesDBG, 1 ) << "|\t\tSetActiveScalar: stepLength = "
    << this->stepLength << std::endl << vprDEBUG_FLUSH;

    // Maximum integration time for streamline integration
    this->maxTime = 5.0f * this->bbDiagonal /
                    (( this->range[1] - this->range[0] ) * 0.5f );
    vprDEBUG( vesDBG, 1 ) << "|\t\tSetActiveScalar: maxTime = "
    << this->maxTime << std::endl << vprDEBUG_FLUSH;

    // Time step for streamline integration
    this->timeStep = this->bbDiagonal / this->definedRange[1];
    vprDEBUG( vesDBG, 1 ) << "|\t\tSetActiveScalar: timeStep = "
    << this->timeStep << std::endl << vprDEBUG_FLUSH;

    // set up the vtkLookupTable
    this->lut->SetNumberOfColors( 256 );            //default is 256
    this->lut->SetHueRange( 2.0f / 3.0f, 0.0f );    //a blue-to-red scale
    this->lut->SetTableRange( this->definedRange );
    this->lut->Build();*/
}
////////////////////////////////////////////////////////////////////////////////
int DataSet::GetActiveScalar()
{
    // 0 <= activeScalar < numScalars
    return this->activeScalar;
}
////////////////////////////////////////////////////////////////////////////////
double* DataSet::GetVectorMagRange()
{
    return this->vectorMagRange;
}
////////////////////////////////////////////////////////////////////////////////
void DataSet::SetActiveVector( int vector )
{
    if( this->numVectors == 0 )
    {
        this->activeVector = -1;
        return;
    }

    if( vector < 0 || this->numVectors <= vector )
    {
        std::cerr << "|\tWarning: SetActiveVector: out-of-range vector "
                  << vector << ", will use first vector " << this->vectorName[ 0 ]
                  << std::endl;
        this->activeVector = 0;
    }
    else
    {
        this->activeVector = vector;

        //vprDEBUG( vesDBG, 1 )
        //        << "|\t\tDataSet::SetActiveVector: requested activeVector = "
        //        << this->activeVector << ", vectorName= "
        //        << this->vectorName[ this->activeVector ]
        //        << std::endl << vprDEBUG_FLUSH;
    }


    SetActiveVector( this->vectorName[ this->activeVector ] );
    /*for( int i = 0; i < 3; i++ )
    {
        int numPlanes = 0;
        if( this->GetPrecomputedSlices( i ) )
        {
            numPlanes = this->GetPrecomputedSlices( i )->GetNumberOfPlanes();
        }
        vprDEBUG( vesDBG, 1 ) << "|\t\tDataSet::SetActiveVector: numPlanes = " << numPlanes
        << std::endl << vprDEBUG_FLUSH;

        if( numPlanes > 0 )
        {
            for( int j = 0; j < numPlanes; j++ )
            {
                this->GetPrecomputedSlices( i )->GetPlane( j )
                ->GetPointData()->SetActiveVectors(
                    this->vectorName[ this->activeVector ].c_str() );
            }
        }
    }*/
}
////////////////////////////////////////////////////////////////////////////////
void DataSet::SetActiveVector( const std::string& tempVectorName )
{
    if( tempVectorName.empty() )
    {
        this->activeVector = -1;
        return;
    }

    if( vectorName.size() == 0 )
    {
        activeVector = -1;
        return;
    }

    int vector = -1;
    for( int i = 0; i < numVectors; ++i )
    {
        if( vectorName[ i ] == tempVectorName )
        {
            vector = i;
            break;
        }
    }

    if( vector < 0 || this->numVectors <= vector )
    {
        std::cerr << "|\tWarning: SetActiveVector: out-of-range vector "
                  << vector << ", will use first vector " << this->vectorName[ 0 ]
                  << std::endl;
        this->activeVector = 0;
    }
    else
    {
        this->activeVector = vector;

        //vprDEBUG( vesDBG, 1 )
        //        << "|\t\tDataSet::SetActiveVector: requested activeVector = "
        //        << this->activeVector << ", vectorName= "
        //        << this->vectorName[ this->activeVector ]
        //        << std::endl << vprDEBUG_FLUSH;
    }
    lfx::vtk_utils::ActiveDataInformationCallback* activeDataInfoCbk =
        dynamic_cast<lfx::vtk_utils::ActiveDataInformationCallback*>
        ( m_dataObjectOps["Active Data Information"] );
    activeDataInfoCbk->SetActiveDataName( this->vectorName[ this->activeVector ], true );
    m_dataObjectHandler->SetDatasetOperatorCallback( activeDataInfoCbk );
    m_dataObjectHandler->OperateOnAllDatasetsInObject( this->m_dataSet );

    /*for( int i = 0; i < 3; i++ )
    {
        int numPlanes = 0;
        if( this->GetPrecomputedSlices( i ) )
        {
            numPlanes = this->GetPrecomputedSlices( i )->GetNumberOfPlanes();
        }
        vprDEBUG( vesDBG, 1 ) << "|\t\tDataSet::SetActiveVector: numPlanes = " << numPlanes
        << std::endl << vprDEBUG_FLUSH;

        if( numPlanes > 0 )
        {
            for( int j = 0; j < numPlanes; j++ )
            {
                this->GetPrecomputedSlices( i )->GetPlane( j )
                ->GetPointData()->SetActiveVectors(
                    this->vectorName[ this->activeVector ].c_str() );
            }
        }
    }*/
}
////////////////////////////////////////////////////////////////////////////////
int DataSet::GetActiveVector()
{
    return this->activeVector;
}
////////////////////////////////////////////////////////////////////////////////
void DataSet::AutoComputeUserRange( const double rawRange[2],
                                    double prettyRange[2] )
{
    double highMinusLow = rawRange[1] - rawRange[0];
    //vprDEBUG( vesDBG, 1 ) << "|\t\thighMinusLow = " << highMinusLow
    //                      << std::endl << vprDEBUG_FLUSH;

    // if all scalar data is the same, then lower bound = upper bound.
    // Fix for this case...
    if( highMinusLow < 1e-15 )
    {
        prettyRange[ 0 ] = rawRange[ 0 ] - 2.0 * 0.1 * rawRange[ 0 ];
        prettyRange[ 1 ] = rawRange[ 0 ] + 2.0 * 0.1 * rawRange[ 0 ];
    }
    else
    {
        prettyRange[ 0 ] = rawRange[ 0 ];
        prettyRange[ 1 ] = rawRange[ 1 ];
    }
    //vprDEBUG( vesDBG, 1 ) << "|\t\tprettyRange: "
    //                      << prettyRange[ 0 ] << " : " << prettyRange[ 1 ]
    //                      << std::endl << vprDEBUG_FLUSH;
}
////////////////////////////////////////////////////////////////////////////////
void DataSet::ResetScalarBarRange( double min, double max )
{
    // converts percentile parameters into decimal values for a particular scalar
    if( this->numScalars == 0 )
    {
        return;
    }

    //Get the actual scalar range for the active scalar
    double rawRange[2];
    GetRange( rawRange );

    double newRawRange[2];
    newRawRange[0] = min;
    newRawRange[1] = max;

    double newPrettyRange[2];
    AutoComputeUserRange( newRawRange, newPrettyRange );

    this->SetUserRange( newPrettyRange );
}
////////////////////////////////////////////////////////////////////////////////
void DataSet::SetFileName( const std::string& newName )
{
    fileName.assign( newName );
    GetDCS()->setName( fileName );
}
////////////////////////////////////////////////////////////////////////////////
void DataSet::SetFileName_OnFly( int datasetindex )
{
    std :: ostringstream file_name;
    std :: string currentVtkFileName = "NewlyLoadedDataSet_000.vtk";
    file_name << "NewlyLoadedDataSet_" << datasetindex << ".vtk";
    currentVtkFileName = file_name.str();

    std::string newName;
    newName.assign( currentVtkFileName );

    this->SetFileName( newName.c_str() );
}
////////////////////////////////////////////////////////////////////////////////
const std::string& DataSet::GetFileName()
{
    return fileName;
}
////////////////////////////////////////////////////////////////////////////////
void DataSet::SetPrecomputedDataSliceDir( const std::string& newName )
{
    precomputedDataSliceDir.assign( newName );
}
////////////////////////////////////////////////////////////////////////////////
void DataSet::LoadPrecomputedDataSlices()
{
    /*if( x_planes )
    {
        delete x_planes;
    }

    if( y_planes )
    {
        delete y_planes;
    }

    if( z_planes )
    {
        delete z_planes;
    }*/

    if( !precomputedDataSliceDir.empty() )
    {
        double bounds[ 6 ];
        GetBounds( bounds );
        //vprDEBUG( vesDBG, 0 ) << "|\t\tDataset bounds xmin = " << bounds[ 0 ]
        //                      << " xmax " << bounds[ 1 ] << " ymin " << bounds[ 2 ]
        //                      << " ymax " << bounds[ 3 ] << " zmin " << bounds[ 4 ]
        //                      << " zmax " << bounds[ 5 ] << std::endl << vprDEBUG_FLUSH;
        //vprDEBUG( vesDBG, 0 ) << "|\t\tLoading precomputed planes from "
        //                      << precomputedDataSliceDir << std::endl << vprDEBUG_FLUSH;
        //x_planes = new cfdPlanes( 0, GetPrecomputedDataSliceDir().c_str(), bounds );
        //y_planes = new cfdPlanes( 1, this->GetPrecomputedDataSliceDir().c_str(), bounds );
        //z_planes = new cfdPlanes( 2, this->GetPrecomputedDataSliceDir().c_str(), bounds );
    }
}
////////////////////////////////////////////////////////////////////////////////
const std::string& DataSet::GetPrecomputedDataSliceDir()
{
    return this->precomputedDataSliceDir;
}
////////////////////////////////////////////////////////////////////////////////
void DataSet::SetPrecomputedSurfaceDir( const std::string& newName )
{
    precomputedSurfaceDir = newName;
}
///////////////////////////////////////////////////////////////
const std::string& DataSet::GetPrecomputedSurfaceDir()
{
    return this->precomputedSurfaceDir;
}
////////////////////////////////////////////////////////////
/*cfdPlanes* DataSet::GetPrecomputedXSlices()
{
    if( !x_planes )
    {
        return 0;
    }

    if( x_planes->GetNumberOfPlanes() == 0 )
    {
        return 0;
    }

    return this->x_planes;
}
/////////////////////////////////////////////////////////////
cfdPlanes* DataSet::GetPrecomputedYSlices()
{
    if( !y_planes )
    {
        return 0;
    }

    if( y_planes->GetNumberOfPlanes() == 0 )
    {
        return 0;
    }

    return this->y_planes;
}
/////////////////////////////////////////////////////////////
cfdPlanes* DataSet::GetPrecomputedZSlices()
{
    if( !z_planes )
    {
        return 0;
    }

    if( z_planes->GetNumberOfPlanes() == 0 )
    {
        return 0;
    }

    return this->z_planes;
}
/////////////////////////////////////////////////////////////////////
cfdPlanes* DataSet::GetPrecomputedSlices( int xyz )
{
    if( xyz == 0 )
    {
        return GetPrecomputedXSlices();
    }
    else if( xyz == 1 )
    {
        return GetPrecomputedYSlices();
    }
    else if( xyz == 2 )
    {
        return GetPrecomputedZSlices();
    }
    else
    {
        std::cerr << "ERROR: DataSet::GetPrecomputedSlices cannot "
                  << "handle index " << xyz << std::endl;
        return NULL; // to eliminate compile warning
    }
}*/
////////////////////////////////////////////////////////////
void DataSet::SetArrow( vtkPolyData* arrow )
{
    this->arrow = arrow;
}
///////////////////////////////////////////////
vtkPolyData* DataSet::GetArrow( )
{
    return this->arrow;
}
//////////////////////////////////////////////
int DataSet::GetNumberOfScalars()
{
    return this->numScalars;
}
///////////////////////////////////////////////////////
const std::string DataSet::GetScalarName( int i )
{
    if( 0 <= i && i < this->numScalars )
    {
        return this->scalarName[ i ];
    }
    else
    {
        std::cout << "|\tWarning: DataSet::GetScalarName cannot "
                  << "handle index " << i << std::endl;
        return m_nullScalarName;
    }
}
//////////////////////////////////////////////////////////////////////////
const std::vector< std::string > DataSet::GetScalarNames() const
{
    return scalarName;
}

const std::vector< std::string >& DataSet::GetTransientScalarNames() const
{
	return _transientScalarNames;
}

//////////////////////////////////////////////////////////////////////////
const std::vector< std::string > DataSet::GetVectorNames() const
{
    return vectorName;
}
///////////////////////////////////////////////
int DataSet::GetNumberOfVectors()
{
    return this->numVectors;
}
////////////////////////////////////////////////////////
const std::string DataSet::GetVectorName( int i )
{
    if( 0 <= i && i < this->numVectors )
    {
        return this->vectorName[ i ];
    }
    else
    {
        std::cerr << "|\tWarning: DataSet::GetVectorName cannot "
                  << "handle index " << i << std::endl;
        return m_nullVectorName;
    }
}
//////////////////////////////////////////////
void DataSet::SetNewlyActivated()
{
    this->isNewlyActivated = 1;
}
///////////////////////////////////////////////////
void DataSet::SetNotNewlyActivated()
{
    this->isNewlyActivated = 0;
}
//////////////////////////////////////////
int DataSet::IsNewlyActivated()
{
    return this->isNewlyActivated;
}
/////////////////////////////////////////////
DataSetPtr DataSet::GetParent()
{
    return parent.lock();
}
/////////////////////////////////////////////////////////////////
void DataSet::SetParent( DataSetPtr myParent )
{
    this->parent = myParent;
}
//////////////////////////////////////////////////////////////////////////
double* DataSet::GetActualScalarRange( std::string name )
{
    for( int i = 0; i < numScalars; ++i )
    {
        if( scalarName[i] == name )
        {
            return GetActualScalarRange( i );
        }
    }
    return 0;
}
//////////////////////////////////////////////////////////////////
double* DataSet::GetActualScalarRange( int index )
{
    return this->actualScalarRange[ index ];
}
/////////////////////////////////////////////////////////////////////////////////
void DataSet::GetActualScalarRange( int index, double* range )
{
    range[ 0 ] = this->actualScalarRange[ index ][ 0 ];
    range[ 1 ] = this->actualScalarRange[ index ][ 1 ];
}
///////////////////////////////////////////////////////////////////////////////////
void DataSet::SetActualScalarRange( int index, double* range )
{
    this->actualScalarRange[ index ][ 0 ] = range[ 0 ];
    this->actualScalarRange[ index ][ 1 ] = range[ 1 ];
}
//////////////////////////////////////////////////////////
double* DataSet::GetDisplayedScalarRange()
{
    return this->displayedScalarRange[ this->activeScalar ];
}
////////////////////////////////////////////////////////////////////////
double* DataSet::GetDisplayedScalarRange( int index )
{
    return this->displayedScalarRange[ index ];
}
/////////////////////////////////////////////////////////////////////////////////////
void DataSet::SetDisplayedScalarRange( int index, double* range )
{
    this->displayedScalarRange[ index ][ 0 ] = range[ 0 ];
    this->displayedScalarRange[ index ][ 1 ] = range[ 1 ];
}
//////////////////////////////////////////////////////////////////
osg::Switch* DataSet::GetSwitchNode()
{
    return switchNode.get();
}
/////////////////////////////////////////////////////
osg::PositionAttitudeTransform* DataSet::GetDCS()
{
    if( !dcs.valid() )
    {
        dcs = new osg::PositionAttitudeTransform();
    }

    return this->dcs.get();
}
/////////////////////////////////////////////////////////////////////
void DataSet::SetDCS( osg::PositionAttitudeTransform* myDCS )
{
    if( dcs == NULL )
    {
        this->dcs = myDCS;
    }
}
/////////////////////////////////////////////////
int DataSet::IsPartOfTransientSeries()
{
    return this->partOfTransientSeries;
}
////////////////////////////////////////////////////////
void DataSet::SetAsPartOfTransientSeries()
{
    this->partOfTransientSeries = 1;
}
/////////////////////////////////////////
void DataSet::StoreScalarInfo()
{
    //Get the scalar and vector information
    m_dataObjectHandler->SetDatasetOperatorCallback( m_dataObjectOps["Count Number Of Vectors And Scalars"] );
    m_dataObjectHandler->OperateOnAllDatasetsInObject( this->m_dataSet );


    // count the number of vectors and store names ...
    this->numScalars = dynamic_cast<lfx::vtk_utils::CountNumberOfParametersCallback*>
                       ( m_dataObjectOps["Count Number Of Vectors And Scalars"] )->GetNumberOfParameters();

    if( this->numScalars )
    {
        this->scalarName = dynamic_cast<lfx::vtk_utils::CountNumberOfParametersCallback*>
                           ( m_dataObjectOps["Count Number Of Vectors And Scalars"] )->GetParameterNames();

        this->actualScalarRange = new double * [ this->numScalars ];
        this->displayedScalarRange = new double * [ this->numScalars ];
        for( int i = 0; i < this->numScalars; i++ )
        {
            this->actualScalarRange[ i ]    = new double [ 2 ];
            this->displayedScalarRange[ i ] = new double [ 2 ];
        }
        lfx::vtk_utils::ProcessScalarRangeCallback* processScalarRangeCbk =
            dynamic_cast<lfx::vtk_utils::ProcessScalarRangeCallback*>
            ( m_dataObjectOps["Scalar Range Information"] );
        m_dataObjectHandler->SetDatasetOperatorCallback( processScalarRangeCbk );
        m_dataObjectHandler->OperateOnAllDatasetsInObject( this->m_dataSet );

        //Add the scalar info to actual scalar range and displayed scalar range
        for( int i = 0; i < this->numScalars; ++i )
        {
            processScalarRangeCbk->GetScalarRange( GetScalarName( i ), actualScalarRange[i] );
            AutoComputeUserRange( GetActualScalarRange( i ), displayedScalarRange[ i ] );
        }
    }
}
//////////////////////////////////////////////////////////
double* DataSet::GetScalarRange( const std::string& scalarName )
{
    for( int i = 0; i < this->numScalars; ++i )
    {
        if( this->scalarName[i] == scalarName )
        {
            return this->actualScalarRange[i];
        }
    }
    return NULL;
}
/////////////////////////////////////////
void DataSet::storeTransientInfo()
{
	
	lfx::vtk_utils::DataObjectHandler* dataObjectHandler = new lfx::vtk_utils::DataObjectHandler();

	lfx::vtk_utils::CountNumberOfParametersCallback* getNumParamsCbk = new lfx::vtk_utils::CountNumberOfParametersCallback();
    dataObjectHandler->SetDatasetOperatorCallback( getNumParamsCbk );

    for( size_t i = 0; i < m_transientDataSets.size(); ++i )
    {
        vtkDataObject* tempDataSet = m_transientDataSets.at( i )->GetDataSet();
        
        dataObjectHandler->OperateOnAllDatasetsInObject( tempDataSet );
    }

    _transientScalarNames = getNumParamsCbk->GetParameterNames( false );
    for( size_t i = 0; i < _transientScalarNames.size(); ++i )
    {
        std::cout << " scalar names " << _transientScalarNames.at( i ) << std::endl;
    }

	// set child datasets transient scalars to match parent
	for( size_t i = 0; i < m_childDataSets.size(); ++i )
	{
		m_childDataSets[i]->_transientScalarNames = _transientScalarNames;
	}

	delete dataObjectHandler;
    delete getNumParamsCbk;
}
/////////////////////////////
void DataSet::Print()
{
    std::ostringstream out;
    out << "filename = " << this->fileName << std::endl;
    out << "numScalars = " << this->numScalars << std::endl;
    for( int i = 0; i < this->numScalars; i++ )
    {
        out << "\tscalarName[" << i << "] = \"" << this->scalarName[ i ]
            << "\"\tactualScalarRange = "
            << this->actualScalarRange[ i ][ 0 ] << " : "
            << this->actualScalarRange[ i ][ 1 ]
            << ", displayedScalarRange = "
            << this->displayedScalarRange[ i ][ 0 ] << " : "
            << this->displayedScalarRange[ i ][ 1 ]
            << std::endl;
    }

    out << "numVectors = " << this->numVectors << std::endl;
    for( int i = 0; i < this->numVectors; i++ )
    {
        out << "\tvectorName[" << i << "] = \"" << this->vectorName[ i ]
            << "\"\tvectorMagRange = "
            << this->vectorMagRange[ 0 ] << " : "
            << this->vectorMagRange[ 1 ]
            << std::endl;
    }
    std::cout << out.str() << std::endl;
    //LOG_INFO( out.str() );
}
////////////////////////////////////////////////////////////////////////////////
void DataSet::SetUUID( const std::string& attribute, const std::string& uuid )
{
    dataSetUUIDMap[ attribute ] = uuid;
}
////////////////////////////////////////////////////////////////////////////////
const std::string DataSet::GetUUID( const std::string& attribute )
{
    std::map< std::string, std::string >::iterator iter;
    iter = dataSetUUIDMap.find( attribute );
    if( iter == dataSetUUIDMap.end() )
    {
        return std::string( "" );
    }
    else
    {
        return iter->second;
    }
}
////////////////////////////////////////////////////////////////////////////////
//create visual representation of bounding box
////////////////////////////////////////////////////////////////////////////////
void DataSet::CreateBoundingBoxGeode( void )
{
    if( !m_visualBBox.valid() )
    {
        m_visualBBox = new osg::Group();
    }
    lfx::vtk_utils::CreateDataObjectBBoxActorsCallback* bboxActorsCbk =
        dynamic_cast<lfx::vtk_utils::CreateDataObjectBBoxActorsCallback*>
        ( m_dataObjectOps["Create BBox Actors"] );

    m_dataObjectHandler->SetDatasetOperatorCallback( bboxActorsCbk );
    m_dataObjectHandler->OperateOnAllDatasetsInObject( this->m_dataSet );
    std::vector< vtkActor* >& bboxActors = bboxActorsCbk->GetBBoxActors();
    size_t nBBoxActors = bboxActors.size();
    for( size_t i = 0; i < nBBoxActors; ++i )
    {
        osg::ref_ptr<osg::Geode> bboxGeode = vtkActorToOSG( bboxActors.at( i ), 0, 0 );
        m_visualBBox->addChild( bboxGeode.get() );
        bboxActors.at( i )->Delete();
    }
    bboxActors.clear();
}
////////////////////////////////////////////////////////////////////////////////
//create wireframe to ensure accurate representation
////////////////////////////////////////////////////////////////////////////////
void DataSet::CreateWireframeGeode( void )
{
    vtkCellDataToPointData* c2p = vtkCellDataToPointData::New();
    if( GetDataSet()->IsA( "vtkCompositeDataSet" ) )
    {
        vtkCompositeDataGeometryFilter* wireframe = vtkCompositeDataGeometryFilter::New();
        wireframe->SetInput( GetDataSet() );

        c2p->SetInputConnection( wireframe->GetOutputPort() );

        wireframe->Delete();
    }
    else
    {
        vtkGeometryFilter* wireframe = vtkGeometryFilter::New();
        wireframe->SetInput( GetDataSet() );

        c2p->SetInputConnection( wireframe->GetOutputPort() );

        wireframe->Delete();
    }

    vtkPolyDataMapper* wireframeMapper = vtkPolyDataMapper::New();
    wireframeMapper->SetInputConnection( c2p->GetOutputPort() );
    //vtkPolyData* poly = lfx::vtk_utils::cfdGrid2Surface( this->GetDataSet(), 0.8f );
    wireframeMapper->SetScalarModeToUsePointFieldData();
    //mapper->SetScalarModeToDefault();
    wireframeMapper->UseLookupTableScalarRangeOn();
    wireframeMapper->SelectColorArray( GetActiveScalarName().c_str() );
    wireframeMapper->SetLookupTable( GetLookupTable() );
    //wireframeMapper->Update();
    c2p->Delete();

    vtkActor* wireframeActor = vtkActor::New();
    wireframeActor->SetMapper( wireframeMapper );
    //wireframeActor->GetProperty()->SetColor( 0, 0, 1 );
    wireframeActor->GetProperty()->SetOpacity( 0.7f );
    //wireframeActor->GetProperty()->SetRepresentationToWireframe();
    wireframeMapper->Delete();

    wireframeGeode = vtkActorToOSG( wireframeActor, 0, 0 );
    wireframeActor->Delete();
}
////////////////////////////////////////////////////////////////////////////////
void DataSet::SetBoundingBoxState( unsigned int state )
{
    if( !m_visualBBox.valid() )
    {
        CreateBoundingBoxGeode();
        GetDCS()->addChild( m_visualBBox.get() );
    }
    m_visualBBox->setNodeMask( ( state == 0 ) ? 0 : 1 );
}
////////////////////////////////////////////////////////////////////////////////
void DataSet::SetWireframeState( unsigned int state )
{
    if( ( state == 0 ) && wireframeGeode.valid() )
    {
        GetDCS()->removeChild( wireframeGeode.get() );
    }
    else if( state == 1 )
    {
        if( wireframeGeode.valid() )
        {
            GetDCS()->removeChild( wireframeGeode.get() );
        }

        //if( wireframeGeode == 0 )
        {
            CreateWireframeGeode();
        }
        GetDCS()->addChild( wireframeGeode.get() );
    }
}
////////////////////////////////////////////////////////////////////////////////
void DataSet::SetAxesState( unsigned int state )
{
    /*if( ( state == 0 ) && dataSetAxes )
    {
        GetDCS()->RemoveChild( dataSetAxes->GetAxis() );
    }
    else if( state == 1 )
    {
        if( dataSetAxes == 0 )
        {
            dataSetAxes = new DataSetAxis();
            dataSetAxes->SetBoundingBox( GetBounds() );
        }
        GetDCS()->RemoveChild( dataSetAxes->GetAxis() );
        dataSetAxes->CreateAxis();
        GetDCS()->AddChild( dataSetAxes->GetAxis() );
    }*/
}
////////////////////////////////////////////////////////////////////////////////
/*ves::xplorer::DataSetAxis* DataSet::GetDataSetAxes( void )
{
    return dataSetAxes;
}*/
////////////////////////////////////////////////////////////////////////////////
/*ves::xplorer::DataSetScalarBar* DataSet::GetDataSetScalarBar( void )
{
    return dataSetScalarBar;
}*/
////////////////////////////////////////////////////////////////////////////////
void DataSet::SetDataSetScalarState( unsigned int state )
{
    /*if( ( state == 0 ) && dataSetScalarBar )
    {
        GetDCS()->removeChild( dataSetScalarBar->GetScalarBar() );
    }
    else if( state == 1 )
    {
        if( dataSetScalarBar == 0 )
        {
            dataSetScalarBar = new DataSetScalarBar();
            dataSetScalarBar->SetBoundingBox( GetBounds() );
        }
        GetDCS()->removeChild( dataSetScalarBar->GetScalarBar() );
        dataSetScalarBar->AddScalarBarToGroup();
        GetDCS()->addChild( dataSetScalarBar->GetScalarBar() );
    }*/
}
////////////////////////////////////////////////////////////////////////////////
const std::string DataSet::GetActiveScalarName()
{
    return GetScalarName( GetActiveScalar() );
}
////////////////////////////////////////////////////////////////////////////////
const std::string DataSet::GetActiveVectorName()
{
    return GetVectorName( GetActiveVector() );
}
////////////////////////////////////////////////////////////////////////////////
void DataSet::CreateCompositeDataSets()
{
    ///Loop over all datasets
    ///Load datasets
    ///Register with model
    //If dataobject is a multi block dataset then load the individual blocks
    if( !m_dataSet->IsA( "vtkCompositeDataSet" ) )
    {
        return;
    }

    vtkDataSet* currentDataset = 0;
    vtkCompositeDataSet* mgd = static_cast< vtkCompositeDataSet* >( m_dataSet );
    vtkCompositeDataIterator* mgdIterator = vtkCompositeDataIterator::New();
    mgdIterator->SetDataSet( mgd );
    ///For traversal of nested multigroupdatasets
    mgdIterator->VisitOnlyLeavesOn();
    mgdIterator->GoToFirstItem();

    unsigned int num = 0;
    while( !mgdIterator->IsDoneWithTraversal() )
    {
        currentDataset =
            dynamic_cast<vtkDataSet*>( mgdIterator->GetCurrentDataObject() );

        DataSetPtr tempDataset = DataSetPtr( new DataSet() );
        tempDataset->SetParent( shared_from_this() );
        //set filename
        std::string subfilename;

        vtkCharArray* tempChar =
            dynamic_cast< vtkCharArray* >(
                currentDataset->GetFieldData()->GetArray( "Name" ) );
        if( tempChar )
        {
            subfilename = tempChar->WritePointer( 0, 0 );
        }
        else
        {
            std::ostringstream filenameStream;
            filenameStream << GetFileName() << "-" << num;
            subfilename = filenameStream.str();
        }
        tempDataset->SetFileName( subfilename );
        //set dcs
        tempDataset->SetDCS( this->GetDCS() );

        //set the vector arrow
        tempDataset->SetArrow( arrow );
        //Load Data sort of
        tempDataset->LoadData( currentDataset, true );
        m_childDataSets.push_back( tempDataset );

        tempDataset->WriteDatabaseEntry();

        mgdIterator->GoToNextItem();
        num++;
    }

    //Reset the filename to reset the dcs filename after
    //all of the files have set the names
    GetDCS()->setName( GetFileName() );

    mgdIterator->Delete();
    mgdIterator = 0;
}
////////////////////////////////////////////////////////////////////////////////
void DataSet::CreateSurfaceWrap()
{
    vtkCellDataToPointData* c2p = vtkCellDataToPointData::New();
    if( GetDataSet()->IsA( "vtkCompositeDataSet" ) )
    {
        vtkCompositeDataGeometryFilter* wireframe = vtkCompositeDataGeometryFilter::New();
        wireframe->SetInput( GetDataSet() );

        c2p->SetInputConnection( wireframe->GetOutputPort() );

        wireframe->Delete();
    }
    else
    {
        vtkGeometryFilter* wireframe = vtkGeometryFilter::New();
        wireframe->SetInput( GetDataSet() );

        c2p->SetInputConnection( wireframe->GetOutputPort() );

        wireframe->Delete();
    }

    c2p->Update();
    vtkDataSet* currentDataset = vtkPolyData::New();
    currentDataset->ShallowCopy( c2p->GetOutput() );
    c2p->Delete();

    DataSetPtr tempDataset = DataSetPtr( new DataSet() );
    tempDataset->SetParent( tempDataset );
    //set filename
    std::ostringstream filenameStream;
    filenameStream << GetFileName() << "-surface";
    std::string subfilename = filenameStream.str();
    tempDataset->SetFileName( subfilename );
    //set dcs
    tempDataset->SetDCS( GetDCS() );
    //set the vector arrow
    tempDataset->SetArrow( arrow );
    //Load Data sort of
    tempDataset->LoadData( currentDataset, false );
    m_childDataSets.push_back( tempDataset );
}
////////////////////////////////////////////////////////////////////////////////
void DataSet::SetActiveDataArrays( std::vector< std::string > activeArrays )
{
    m_activeDataArrays = activeArrays;
}
////////////////////////////////////////////////////////////////////////////////
void DataSet::LoadTransientData( const std::string& dirName, const std::string fileExtension )
{
    ///Get base file name or directory
    ///Scan directory for all of the files
    ///Read the files and create datasets for them
    if( !_vtkFHndlr )
    {
        _vtkFHndlr = new lfx::vtk_utils::VTKFileHandler();
    }

    std::vector<std::string> transientFile =
        lfx::vtk_utils::fileIO::GetFilesInDirectory( dirName, fileExtension );

    ///Load data for the file selected by the user for the transient series
    LoadData();

    SetAsPartOfTransientSeries();

    //not done with files in directory
    for( size_t i = 0; i < transientFile.size(); ++i )
    {
        //This could be a multi block dataset
        //Load in the dataset
        DataSetPtr tempDataset( new DataSet() );
        tempDataset->SetParent( shared_from_this() );
        //set filename
        tempDataset->SetFileName( transientFile.at( i ) );
        //set dcs
        tempDataset->SetDCS( GetDCS() );
        //set the vector arrow
        tempDataset->SetArrow( arrow );
        //Load Data sort of
        tempDataset->LoadData();

        tempDataset->SetAsPartOfTransientSeries();

        m_transientDataSets.push_back( tempDataset );
    }

    for( size_t i = 0; i < m_transientDataSets.size(); ++i )
    {
        m_transientDataSets.at( i )->
        SetTransientDataSetsList( m_transientDataSets );
    }

	storeTransientInfo();
}
////////////////////////////////////////////////////////////////////////////////
const std::vector< DataSetPtr >& DataSet::GetTransientDataSets()
{
    return m_transientDataSets;
}
////////////////////////////////////////////////////////////////////////////////
void DataSet::SetTransientDataSetsList( std::vector< DataSetPtr >& tempTransientData )
{
    m_transientDataSets = tempTransientData;
}
////////////////////////////////////////////////////////////////////////////////
void DataSet::WriteDatabaseEntry()
{
    /*xplorer::data::DatasetPropertySet set;

    //boost::filesystem::path tempPath( fileName );
    //std::string shortName = tempPath.filename().string();

    //set.LoadByKey( "Filename", shortName );
    set.LoadByKey( "Filename", fileName );

    osg::Node::DescriptionList descriptorsList;
    descriptorsList.push_back( "VE_DATA_NODE" );
    descriptorsList.push_back( set.GetUUIDAsString() );
    GetDCS()->setDescriptions( descriptorsList );

    //set.SetPropertyValue( "Filename", shortName );
    set.SetPropertyValue( "Filename", fileName );
    set.SetPropertyValue( "StepLength", stepLength );
    set.SetPropertyValue( "MaxTime", maxTime );
    set.SetPropertyValue( "TimeStep", timeStep );
    set.SetPropertyValue( "Type", datasetType );
    set.SetPropertyValue( "PrecomputedDataSliceDir", precomputedDataSliceDir );
    set.SetPropertyValue( "PrecomputedSurfaceDir", precomputedSurfaceDir );
    set.SetPropertyValue( "ScalarNames", scalarName );
    set.SetPropertyValue( "VectorNames", vectorName );

    std::vector< double > ScalarMins;
    std::vector< double > ScalarMaxes;
    for( int index = 0; index < GetNumberOfScalars(); ++index )
    {
        double* range = GetActualScalarRange( index );
        ScalarMins.push_back( range[0] );
        ScalarMaxes.push_back( range[1] );
    }
    set.SetPropertyValue( "ScalarMins", ScalarMins );
    set.SetPropertyValue( "ScalarMaxes", ScalarMaxes );

    Print();

    set.WriteToDatabase();*/
}
////////////////////////////////////////////////////////////////////////////////
void DataSet::LoadTemporalDataSet( vtkDataObject* temporalDataSet )
{
    vtkTemporalDataSet* tempVtkDataSet =
        vtkTemporalDataSet::SafeDownCast( temporalDataSet );
    //Set the first time step to this dataset
    vtkMultiBlockDataSet* firstTimeStep = vtkMultiBlockDataSet::New();
    firstTimeStep->ShallowCopy( tempVtkDataSet->GetTimeStep( 0 ) );
    firstTimeStep->Update();

    InitializeVTKDataObject( firstTimeStep );
    SetAsPartOfTransientSeries();
    m_transientDataSets.push_back( shared_from_this() );

    unsigned int numTimeSteps = tempVtkDataSet->GetNumberOfTimeSteps();
    //Loop over all the other time steps
    for( size_t i = 1; i < numTimeSteps; ++i )
    {
        //This could be a multi block dataset
        //Load in the dataset
        //Do work
        DataSetPtr tempDataset( new DataSet() );
        tempDataset->SetParent( shared_from_this() );
        //set filename
        std::ostringstream strm;
        strm << fileName
             << "_"
             << std::setfill( '0' )
             << std::setw( 6 )
             << i << ".vtm";

        tempDataset->SetFileName( strm.str() );
        //set dcs
        tempDataset->SetDCS( GetDCS() );

        //set the vector arrow
        tempDataset->SetArrow( arrow );
        //Load Data sort of
        vtkMultiBlockDataSet* timeStep = vtkMultiBlockDataSet::New();
        timeStep->ShallowCopy( tempVtkDataSet->GetTimeStep( i ) );
        timeStep->Update();
        tempDataset->InitializeVTKDataObject( timeStep );
        tempDataset->SetAsPartOfTransientSeries();

        m_transientDataSets.push_back( tempDataset );
    }
    tempVtkDataSet->Delete();

    for( size_t i = 1; i < m_transientDataSets.size(); ++i )
    {
        m_transientDataSets.at( i )->
        SetTransientDataSetsList( m_transientDataSets );
    }

	storeTransientInfo();
}
////////////////////////////////////////////////////////////////////////////////
void DataSet::InitializeVTKDataObject( vtkDataObject* tempDataObject )
{
    //Initialize the VTK dataset member variable
    m_dataSet = tempDataObject;

    if( !m_dataObjectHandler )
    {
        m_dataObjectHandler = new lfx::vtk_utils::DataObjectHandler();
    }
    //Now create vector mag and vector scalars
    {
        std::map<std::string, lfx::vtk_utils::DataObjectHandler::DatasetOperatorCallback* >::iterator iterTemp =
            m_dataObjectOps.find( "Compute Vector Mag and Scalars" );
        m_dataObjectHandler->SetDatasetOperatorCallback( iterTemp->second );
        m_dataObjectHandler->OperateOnAllDatasetsInObject( m_dataSet );
    }
    //Need to get number of pda
    m_dataObjectHandler->SetDatasetOperatorCallback( 0 );
    m_dataObjectHandler->OperateOnAllDatasetsInObject( m_dataSet );
    this->numPtDataArrays = m_dataObjectHandler->GetNumberOfDataArrays();

#ifdef USE_OMP
    char label[100];
    vtkUnstructuredGridReader* tableReader = vtkUnstructuredGridReader::New();
    tableReader->SetFileName( "./POST_DATA/octreeTable.vtk" );
    tableReader->Update();
    vtkUnstructuredGrid* table = ( vtkUnstructuredGrid* ) tableReader->GetOutput();
    if( this->noOfData == 0 )
    {
        this->noOfData = table->GetNumberOfCells();
    }

    tableReader->Delete();

    # pragma omp parallel for private(label,i)
    for( int i = 0; i < noOfData; i++ )
    {
        this->dataReader[i] = vtkUnstructuredGridReader::New();
        std::ostringstream dirStringStream;
        dirStringStream << "./POST_DATA/octant" << i << ".vtk";
        std::string dirString = dirStringStream.str();

        this->dataReader[i]->SetFileName( dirString.c_str() );
        this->dataReader[i]->Update();
        this->data[i] = ( vtkUnstructuredGrid* ) this->dataReader[i]->GetOutput();
    }
#endif

    // Compute the geometrical properties of the mesh
    /// Load the precomputed data
    LoadPrecomputedDataSlices();

    // count the number of scalars and store names and ranges...
    StoreScalarInfo();

    // count the number of vectors and store names ...
    this->numVectors = dynamic_cast<lfx::vtk_utils::CountNumberOfParametersCallback*>
                       ( m_dataObjectOps["Count Number Of Vectors And Scalars"] )->GetNumberOfParameters( true );
    if( this->numVectors )
    {
        this->vectorName = dynamic_cast<lfx::vtk_utils::CountNumberOfParametersCallback*>
                           ( m_dataObjectOps["Count Number Of Vectors And Scalars"] )->GetParameterNames( true );
    }

    // if there are point data, set the first scalar and vector as active...
    if( this->numPtDataArrays )
    {
        // set the first scalar and vector as active
        if( this->numScalars )
        {
            this->SetActiveScalar( 0 );
        }

        if( this->numVectors )
        {
            this->SetActiveVector( 0 );
            if( !this->vectorMagRange )
            {
                this->vectorMagRange = new double[2];
            }
            lfx::vtk_utils::ComputeVectorMagnitudeRangeCallback* vecMagRangeCbk =
                dynamic_cast<lfx::vtk_utils::ComputeVectorMagnitudeRangeCallback*>
                ( m_dataObjectOps["Compute Vector Magnitude Range"] );
            m_dataObjectHandler->SetDatasetOperatorCallback( vecMagRangeCbk );
            m_dataObjectHandler->OperateOnAllDatasetsInObject( m_dataSet );
            vecMagRangeCbk->GetVectorMagnitudeRange( this->vectorMagRange );
        }
    }

    SetType();

    //Register this dataset with the modeldatahandler
    //This assumes that the m_dataSet pointer is pointing to the correct
    //dataset.
    CreateCompositeDataSets();

    WriteDatabaseEntry();
}
////////////////////////////////////////////////////////////////////////////////
const std::vector< DataSetPtr > DataSet::GetChildDataSets() const
{
    return m_childDataSets;
}
////////////////////////////////////////////////////////////////////////////////
} // end xplorer
} // end ves
}

