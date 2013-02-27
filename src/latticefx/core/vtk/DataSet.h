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
#ifndef LFX_VTK_DATA_SET_H
#define LFX_VTK_DATA_SET_H

#include <latticefx/core/vtk/DataSetPtr.h>

#include <latticefx/core/vtk/Export.h>

#include <latticefx/utils/vtk/DataObjectHandler.h>

#include <osg/PositionAttitudeTransform>
#include <osg/Group>
#include <osg/Switch>
#include <osg/Geode>

//#include <ves/xplorer/Logging.h>

#include <boost/enable_shared_from_this.hpp>

#include <osg/ref_ptr>

#ifdef USE_OMP
#define MAX_DATA 20
#endif

#include <string>
#include <map>
#include <vector>

class vtkLookupTable;
class vtkPolyData;
class vtkUnstructuredGrid;
class vtkUnstructuredGridReader;
class vtkDataSet;
class vtkDataObject;
class vtkAlgorithm;

namespace lfx
{
namespace vtk_utils
{
class VTKFileHandler;
}
}

namespace lfx
{
namespace vtk_translator
{
class DataLoader;
}
}

namespace lfx
{
namespace core
{
namespace vtk
{
/*!\file DataSet.h
 * DataSet API
 * \class ves::xplorer::DataSet
 * A class to load data set and pre-compute flow parameters
 * or properties for virtual environment interactive
 * computation.
 */
class LATTICEFX_CORE_VTK_EXPORT DataSet : public boost::enable_shared_from_this< DataSet >
{
public:
    // Construct vtkUnstructuredGrid and vtkLookupTable objects.
    DataSet();
    // Destruct vtkUnstructuredGrid and vtkLookupTable objects.
    ~DataSet();

    void CreateCompositeDataSets();

    // Initialize the number of data to load and parallel process.
    // By default, use the octree table.
    //void LoadData( const std::string fileName );
    void LoadData( vtkUnstructuredGrid*, int );
    void LoadData();
    void LoadData( vtkDataSet* data, bool isPartOfCompositeDataset = false );

    ///Set the filename for this dataset
    ///\post Now LoadData can be called
    void SetFileName( const std::string& filename );
    ///Get the filename used for this dataset
    const std::string& GetFileName();

    ///Load the precomputed data directory
    void LoadPrecomputedDataSlices();


    // Set/get the range of velocity based on the data set.
    void SetRange( double dataRange[2] );
    void SetRange( double dataMin, double dataMax );
    void GetRange( double dataRange[2] );
    void GetRange( double& dataMin, double& dataMax );
    double* GetRange();

    // Set/get the min/max velocity, used defined.
    void SetUserRange( double userRange[2] );
    void SetUserRange( double userMin, double userMax );
    void GetUserRange( double userRange[2] );
    void GetUserRange( double& userMin, double& userMax );
    double* GetUserRange();

    // Set/get the length of the diagonal of the bounding box for data set.
    void SetLength( float len );
    //void GetLength( float &len );
    //float GetLength();

    // Get the length of the diagonal of the bounding box of the average cell
    //void GetMeanCellLength( float &len );
    //float GetMeanCellLength();

    // Set/get the step length for streamline integration.
    void SetStepLength( float sLen );
    void GetStepLength( float& sLen );
    float GetStepLength();

    // Set/get the maximum streamline integration time.
    void SetMaxTime( float mT );
    void GetMaxTime( float& mT );
    float GetMaxTime();

    // Set/get time step for streamline integration
    void SetTimeStep( float tStep );
    void GetTimeStep( float& tStep );
    float GetTimeStep();

    // Get the vtk look up table.
    // Lookuptable will be either blue to red or greyscale based on state of greyscaleFlag.
    vtkLookupTable* GetLookupTable();

    // Get/Set greyscaleFlag for the lookuptable.
    void SetGreyscaleFlag( bool flag );
    bool GetGreyscaleFlag();

    // Get the single piece original data.
    vtkUnstructuredGrid* GetUnsData();
    vtkPolyData* GetPolyData();
    ///Get the data object for the dataset
    vtkDataObject* GetDataSet();

    void SetType();       // compute dataset type by looking at the file
    void SetType( int );  // manually set the dataset type
    int GetType();        // get the dataset type

    // SetActiveScalar and compute the actual scalar range and the pretty range for display purposes
    // 0 <= activeScalar < numScalars
    void SetActiveScalar( int );
    void SetActiveScalar( const std::string& scalarName );
    int GetActiveScalar();
    const std::string GetActiveScalarName();

    void SetActiveVector( int );
    void SetActiveVector( const std::string& vectorName );
    int GetActiveVector();

    static void AutoComputeUserRange( const double rawRange[2],
                                      double prettyRange[2] );

    void ResetScalarBarRange( double min, double max );

    void SetFileName_OnFly( int );

    void SetPrecomputedDataSliceDir( const std::string& newDir );
    const std::string& GetPrecomputedDataSliceDir();

    void SetPrecomputedSurfaceDir( const std::string& newDir );
    const std::string& GetPrecomputedSurfaceDir();

    /*cfdPlanes* GetPrecomputedXSlices();
    cfdPlanes* GetPrecomputedYSlices();
    cfdPlanes* GetPrecomputedZSlices();
    cfdPlanes* GetPrecomputedSlices( int xyz );*/

    void StoreScalarInfo();

#ifdef USE_OMP
    vtkUnstructuredGrid* GetData( int i );
    int GetNoOfDataForProcs();       // Set/get number of data for parallel process.
#endif

    void SetArrow( vtkPolyData* );
    vtkPolyData* GetArrow();

    void SetNewlyActivated();
    void SetNotNewlyActivated();
    int IsNewlyActivated();

    int GetNumberOfScalars();
    const std::string GetScalarName( int );

    int GetNumberOfVectors();
    const std::string GetVectorName( int );
    const std::string GetActiveVectorName();

    DataSet* GetParent();
    void SetParent( DataSet* );

    void SetActualScalarRange( int, double* );
    void GetActualScalarRange( int, double* );
    double* GetActualScalarRange( int );
    double* GetActualScalarRange( std::string name );

    // returns displayed range of active scalar
    double* GetDisplayedScalarRange();

    // get/set displayed range of any scalar
    double* GetDisplayedScalarRange( int );
    void SetDisplayedScalarRange( int , double* );

    double* GetVectorMagRange();

    // get/set this dataset's DCS
    osg::PositionAttitudeTransform* GetDCS();
    void SetDCS( osg::PositionAttitudeTransform* );

    osg::Switch* GetSwitchNode( void );

    //ves::xplorer::scenegraph::cfdTempAnimation* GetAnimation( void );
    //void SetAnimation( ves::xplorer::scenegraph::cfdTempAnimation* );

    int IsPartOfTransientSeries();
    void SetAsPartOfTransientSeries();

    void Print();
    ///Accessor methods to store and query the uuids for specfic
    ///attributes of a DataSet
    void SetUUID( const std::string& attribute, const std::string& uuid );
    const std::string GetUUID( const std::string& attribute );

    ///Create the bbox geode for the dataset
    void CreateBoundingBoxGeode( void );
    ///Create the wireframe geode for the dataset
    void CreateWireframeGeode( void );
    ///Set the bounding box for this dataset
    ///\param state The state of the bounding box 0 or 1
    void SetBoundingBoxState( unsigned int state );
    ///Set the wireframe state for this dataset
    ///\param state The state of the wireframe 0 or 1
    void SetWireframeState( unsigned int state );
    ///Set the axes state for this dataset
    ///\param state The state of the axes state 0 or 1
    void SetAxesState( unsigned int state );
    ///Set the bounding box for this dataset
    //ves::xplorer::DataSetAxis* GetDataSetAxes( void );
    ///Set the scalar for this dataset
    ///\param state The state of the scalar bar 0 or 1
    void SetDataSetScalarState( unsigned int state );
    ///Get the scalar bar
    //ves::xplorer::DataSetScalarBar* GetDataSetScalarBar( void );

    ///Get the bounds of the vtkDataObject contained in the DataSet
    ///\param bounds xmin,xmax,ymin,ymax,zmin,zmax
    void GetBounds( double* bounds );

    ///Get the bounds of the vtkDataObject contained in the DataSet
    ///\param bounds xmin,xmax,ymin,ymax,zmin,zmax
    double* GetBounds();

    ///Get the scalar range by name
    ///\param scalarName The name of the scalar to get the range
    double* GetScalarRange( const std::string& scalarName );

    ///Get the number of points
    unsigned int GetNumberOfPoints();
    ///Set the model for this dataset
    //void SetModel( ves::xplorer::Model* model );

    ///Create a surface wrap of this dataset
    void CreateSurfaceWrap();
    ///Set the active data arrays to load
    void SetActiveDataArrays( std::vector< std::string > activeArrays );
    ///Load transient data based on a file prefix or directory scan.
    void LoadTransientData( const std::string& dirName, const std::string fileExtension = ".vtm" );

    ///Get Transient vectors for dataset
    ///\return The vector of datasets associated with this transient series
    const std::vector< DataSetPtr >& GetTransientDataSets();

    ///Set the transient dataset vector so that sub transient datasets
    ///can tell other pipelines what the other datasets in the series are.
    void SetTransientDataSetsList( std::vector< DataSetPtr >& tempTransientData );

protected:
//#ifdef QT_ON
    void WriteDatabaseEntry();
//#endif // QT_ON

private:
    ///Load a VTK Temporal data set
    ///\param temporalDataSet The temporal data object
    void LoadTemporalDataSet( vtkDataObject* temporalDataSet );

    ///Initialize a VTK data object and run all of our preprocessing filters
    ///on the data object.
    ///\param tempDataObject The data set to initialize
    void InitializeVTKDataObject( vtkDataObject* tempDataObject );

    ///Sets the lookuptable to greyscale.
    void SetGreyscale();
    ///Sets the lookuptable to a blue to red scale.
    void SetColorscale();

    ///Model pointer to the model that is holding this dataset
    //ves::xplorer::Model* m_tempModel;

    ///Operator callbacks for DataObjectHandler
    std::map<std::string, lfx::vtk_utils::DataObjectHandler::DatasetOperatorCallback* > m_dataObjectOps;
    std::map< std::string, std::string > dataSetUUIDMap;

    double** actualScalarRange;
    double** displayedScalarRange;

    DataSet* parent;
    double m_bounds[6];///The bounding box data;
    int isNewlyActivated;

    double* range;          // Range of scalar.

    double* definedRange;   // 'prettied' range of scalar that is automatically computed or user-defined.

    double* vectorMagRange; // assumes just one vector

    float bbDiagonal;        // length of the diagonal of the bounding box.

    double meanCellBBLength; // length of diagonal of average cell bounding box.

    float stepLength;        // Step length for streamline integration.

    float maxTime;           // Maximum time of integration for streamline.

    float timeStep;          // Time step for streamline integration.

    bool m_greyscaleFlag;         // Flag for whether lookuptable being returned is in greyscale or blue to red.

    ///Lookup table.
    vtkLookupTable* lut;
    ///Original piece of vtk data
    vtkDataObject* m_dataSet;

    ///used by gui to place in appropriate column
    int datasetType;

    int activeScalar;
    int activeVector;

    std::string fileName;
    std::string precomputedDataSliceDir;
    std::string precomputedSurfaceDir;

    //cfdPlanes* x_planes;
    //cfdPlanes* y_planes;
    //cfdPlanes* z_planes;

    vtkPolyData* arrow;

    int numPtDataArrays;
    int numScalars;
    int numVectors;
    ///Scalar names
    std::vector< std::string > scalarName;
    ///Null scalar name
    std::string m_nullScalarName;
    ///Vector names
    std::vector< std::string > vectorName;
    ///Null vector name
    std::string m_nullVectorName;

    osg::ref_ptr< osg::Geode > wireframeGeode;
    osg::ref_ptr< osg::Group > m_visualBBox;

    osg::ref_ptr< osg::PositionAttitudeTransform > dcs;
    ///Switch node used to control the display of TBET or classic viz
    osg::ref_ptr< osg::Switch > switchNode;
    osg::ref_ptr< osg::Group > classic;
    osg::ref_ptr< osg::Group > textureBased;

    //ves::xplorer::DataSetAxis* dataSetAxes;
    //ves::xplorer::DataSetScalarBar* dataSetScalarBar;
    lfx::vtk_utils::VTKFileHandler* _vtkFHndlr;
    ///Handle vtkDataObjects
    lfx::vtk_utils::DataObjectHandler* m_dataObjectHandler;
    ///Tell other classes that this dataset is part of a transient domain
    int partOfTransientSeries;
    ///Translator interface
    lfx::vtk_translator::DataLoader* m_externalFileLoader;

    ///Easy way to tell if this dataset is a child of a composite dataset
    bool m_isPartOfCompositeDataset;
    ///List of child datasets for this dataset
    std::vector< DataSet* > m_childDataSets;
    ///Set the active data arrays to load
    std::vector< std::string > m_activeDataArrays;
    ///List of transient datasets associated with this dataset
    std::vector< DataSetPtr > m_transientDataSets;
#ifdef USE_OMP
    unsigned int noOfData;   // Total no. of octants.
    vtkUnstructuredGridReader* dataReader[MAX_DATA];
    vtkUnstructuredGrid* data[MAX_DATA];
#endif
    ///Logger reference
    //Poco::Logger& m_logger;
    ///Actual stream for this class
    //ves::xplorer::LogStreamPtr m_logStream;

};
}
}
}
#endif
