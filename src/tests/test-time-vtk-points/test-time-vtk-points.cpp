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
#include <latticefx/core/DataSet.h>
#include <latticefx/core/ChannelData.h>
#include <latticefx/core/ChannelDataOSGArray.h>
#include <latticefx/core/RTPOperation.h>
#include <latticefx/core/Renderer.h>
#include <latticefx/core/TextureUtils.h>
#include <latticefx/core/BoundUtils.h>
#include <latticefx/core/VectorRenderer.h>
#include <latticefx/core/TransferFunctionUtils.h>
#include <latticefx/core/PlayControl.h>
#include <latticefx/core/DBDisk.h>
#include <latticefx/core/Log.h>
#include <latticefx/core/LogMacros.h>

#include <latticefx/core/vtk/DataSet.h>
#include <latticefx/core/vtk/VTKSurfaceWrapRTP.h>
#include <latticefx/core/vtk/VTKActorRenderer.h>
#include <latticefx/core/vtk/ChannelDatavtkDataObject.h>


#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Texture3D>
#include <osg/Shader>
#include <osg/Program>
#include <osg/Uniform>
#include <osg/io_utils>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>
#include <osgDB/FileUtils>


#include <latticefx/utils/vtk/FindVertexCellsCallback.h>
#include <latticefx/utils/vtk/GetScalarDataArraysCallback.h>
#include <latticefx/utils/vtk/CountNumberOfParametersCallback.h>
#include <latticefx/utils/vtk/ProcessScalarRangeCallback.h>

#include <vtkDataObject.h>
#include <vtkCompositeDataGeometryFilter.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkCellDataToPointData.h>
#include <vtkMaskPoints.h>
#include <vtkCompositeDataPipeline.h>
#include <vtkAlgorithm.h>
#include <vtkAlgorithmOutput.h>
#include <vtkMath.h>
#include <vtkPointData.h>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
namespace fs = boost::filesystem;

std::vector< lfx::core::vtk::DataSetPtr > transientSeries;
#if 0
std::string diameterNameString = "Diameter";
std::string vmagNameString = "VelocityMagnitude";
double conversionFactor = 1.0;
#else
std::string diameterNameString = "RepDiam";
std::string vmagNameString = "MotionVector_magnitude";
double conversionFactor = 0.00328084;
#endif
////////////////////////////////////////////////////////////////////////////////
lfx::core::DataSetPtr createInstanced( const std::vector< lfx::core::vtk::DataSetPtr >& transData,
                                                                   const std::string& activeScalar,
                                                                   const std::string& activeVector,
                                                                   lfx::core::DBBasePtr dbBase )
{
    std::vector< std::vector< std::pair< vtkIdType, double* > > >  m_pointCollection;
    ///The raw data for the respective points
    std::vector< std::vector< std::pair< std::string, std::vector< double > > > >  m_dataCollection;
    ///Streamline ordered raw data for the respective lines
    std::vector< std::vector< std::pair< std::string, std::vector< double > > > >  m_lineDataCollection;

    m_pointCollection.clear();
    
    std::vector< lfx::core::vtk::DataSetPtr > m_transientDataSet;
    m_transientDataSet = transData;
    std::string m_activeVector = activeVector;
    std::string m_activeScalar = activeScalar;
    
    lfx::vtk_utils::FindVertexCellsCallback* findVertexCellsCbk =
        new lfx::vtk_utils::FindVertexCellsCallback();
    lfx::vtk_utils::DataObjectHandler* dataObjectHandler =
        new lfx::vtk_utils::DataObjectHandler();
    dataObjectHandler->SetDatasetOperatorCallback( findVertexCellsCbk );
    
    size_t maxNumPoints = 0;
    for( size_t i = 0; i < m_transientDataSet.size(); ++i )
    {
        vtkDataObject* tempDataSet = m_transientDataSet.at( i )->GetDataSet();
        
        dataObjectHandler->OperateOnAllDatasetsInObject( tempDataSet );
        std::vector< std::pair< vtkIdType, double* > > tempCellGroups =
        findVertexCellsCbk->GetVertexCells();
        m_pointCollection.push_back( tempCellGroups );
        findVertexCellsCbk->ResetPointGroup();
        if( maxNumPoints < tempCellGroups.size() )
        {
            maxNumPoints = tempCellGroups.size();
        }
    }
    delete findVertexCellsCbk;
    std::cout << maxNumPoints << std::endl;
    lfx::vtk_utils::CountNumberOfParametersCallback* getNumParamsCbk =
        new lfx::vtk_utils::CountNumberOfParametersCallback();
    dataObjectHandler->SetDatasetOperatorCallback( getNumParamsCbk );
    for( size_t i = 0; i < m_transientDataSet.size(); ++i )
    {
        vtkDataObject* tempDataSet = m_transientDataSet.at( i )->GetDataSet();
        
        dataObjectHandler->OperateOnAllDatasetsInObject( tempDataSet );
    }
    std::vector< std::string > scalarNames = getNumParamsCbk->GetParameterNames( false );
    for( size_t i = 0; i < scalarNames.size(); ++i )
    {
        std::cout << " scalar names " << scalarNames.at( i ) << std::endl;
    }

    lfx::vtk_utils::ProcessScalarRangeCallback* processScalarRangeCbk =
        new lfx::vtk_utils::ProcessScalarRangeCallback();
    double* diamRange = new double[ 2 ];
    double* vmagRange = new double[ 2 ];
    double diamRangeF[ 2 ] = {10000000.,-10000000.};
    double vmagRangeF[ 2 ] = {10000000.,-10000000.};
    for( size_t j = 0; j < m_transientDataSet.size(); ++j )
    {
        dataObjectHandler->SetDatasetOperatorCallback( processScalarRangeCbk );
        vtkDataObject* tempDataSet = m_transientDataSet.at( j )->GetDataSet();
        dataObjectHandler->OperateOnAllDatasetsInObject( tempDataSet );
        processScalarRangeCbk->GetScalarRange( diameterNameString, diamRange ); 
        processScalarRangeCbk->GetScalarRange( vmagNameString, vmagRange );
        if( diamRange[ 0 ] < diamRangeF[ 0 ] )
        {
            diamRangeF[ 0 ] = diamRange[ 0 ];
        }

        if( diamRange[ 1 ] > diamRangeF[ 1 ] )
        {
            diamRangeF[ 1 ] = diamRange[ 1 ];
        }

        if( vmagRange[ 0 ] < vmagRangeF[ 0 ] )
        {
            vmagRangeF[ 0 ] = vmagRange[ 0 ];
        }

        if( vmagRange[ 1 ] > vmagRangeF[ 1 ] )
        {
            vmagRangeF[ 1 ] = vmagRange[ 1 ];
        }
    }    
    delete [] diamRange;
    diamRange = 0;
    delete [] vmagRange;
    vmagRange = 0;

    std::cout << " range " << diamRangeF[ 0 ] << " " << diamRangeF[ 1 ] << std::endl;
    std::cout << " range " << vmagRangeF[ 0 ] << " " << vmagRangeF[ 1 ] << std::endl;

    lfx::vtk_utils::GetScalarDataArraysCallback* getScalarDataArrayCbk =
        new lfx::vtk_utils::GetScalarDataArraysCallback();
    dataObjectHandler->SetDatasetOperatorCallback( getScalarDataArrayCbk );
    getScalarDataArrayCbk->SetScalarNames( scalarNames );
    for( size_t i = 0; i < m_transientDataSet.size(); ++i )
    {
        vtkDataObject* tempDataSet = m_transientDataSet.at( i )->GetDataSet();
        
        dataObjectHandler->OperateOnAllDatasetsInObject( tempDataSet );
        std::vector< std::pair< std::string, std::vector< double > > > tempCellGroups =
        getScalarDataArrayCbk->GetCellData();
        m_dataCollection.push_back( tempCellGroups );
        getScalarDataArrayCbk->ResetPointGroup();
    }
    
    delete getScalarDataArrayCbk;
    delete dataObjectHandler;
    delete getNumParamsCbk;
    delete processScalarRangeCbk;

    lfx::core::DataSetPtr dsp( new lfx::core::DataSet() );    
    for( size_t i = 0; i < transData.size(); ++i )
    {
        std::vector< std::pair< std::string, std::vector< double > > >* tempScalarData = &m_dataCollection.at( i );
        osg::ref_ptr< osg::FloatArray > radArray( new osg::FloatArray );
        //radArray->resize( samplesPerTime );
        osg::ref_ptr< osg::FloatArray > depthArray( new osg::FloatArray );
        //depthArray->resize( samplesPerTime );
        std::vector< double >* diamArray = 0;
        std::vector< double >* vmagArray = 0;

        for( size_t j = 0; j < tempScalarData->size(); ++j )
        {
            if( tempScalarData->at( j ).first == diameterNameString && tempScalarData->at( j ).second.size() > 0 )
            {
                //std::cout << tempScalarData->at( j ).second.size() << std::endl;
                diamArray = &tempScalarData->at( j ).second;
            }
            else if( tempScalarData->at( j ).first == vmagNameString && tempScalarData->at( j ).second.size() > 0 )
            {
                //std::cout << tempScalarData->at( j ).second.size() << std::endl;
                vmagArray = &tempScalarData->at( j ).second;
            }
            //std::cout << " scalar name " << tempScalarData->at( j ).first << " " << tempScalarData->at( j ).second.size() << std::endl;
        }


        std::vector< std::pair< vtkIdType, double* > >* tempCellGroups = 
            &m_pointCollection.at( i );
        osg::ref_ptr< osg::Vec3Array > posArray( new osg::Vec3Array ); 
        double val=0;
        for( size_t j = 0; j < tempCellGroups->size(); ++j )
        {
            //std::cout << tempCellGroups->size() << std::endl;
            double* tempData = tempCellGroups->at( j ).second;
            osg::Vec3d position( tempData[ 0 ], tempData[ 1 ], tempData[ 2 ] );
            position *= conversionFactor;
            posArray->push_back( position );
            //std::cout << tempCellGroups->at( j ).first << " " << tempData[ 0 ] << " " << tempData[ 1 ] << " " << tempData[ 2 ] << std::endl;

            //radArray->push_back( 0.02 );
            if( diamArray )
            {
                val = diamArray->at( j ) * 0.5 * conversionFactor;
                //val = vtkMath::ClampAndNormalizeValue( val, diamRange );

                radArray->push_back( val );
            }
            else
            {
                std::cout << " no diameter " << j << std::endl;
            }
            
            //depthArray->push_back( float(j%6)/5. );
            if( vmagArray )
            {
                val = vmagArray->at( j );
                val = vtkMath::ClampAndNormalizeValue( val, vmagRangeF );

                depthArray->push_back( val );
            }
            else
            {
                std::cout << " no vmag " << j << std::endl;
            }
        }

        lfx::core::ChannelDataOSGArrayPtr posData( new lfx::core::ChannelDataOSGArray( "positions", posArray.get() ) );
        dsp->addChannel( posData, i * 0.25 );
        lfx::core::ChannelDataOSGArrayPtr radData( new lfx::core::ChannelDataOSGArray( "radii", radArray.get() ) );
        dsp->addChannel( radData, i * 0.25 );
        lfx::core::ChannelDataOSGArrayPtr depthData( new lfx::core::ChannelDataOSGArray( "depth", depthArray.get() ) );
        dsp->addChannel( depthData, i * 0.25 );
    }
    //341 - 343
    lfx::core::VectorRendererPtr renderOp( new lfx::core::VectorRenderer() );
    renderOp->setPointStyle( lfx::core::VectorRenderer::SPHERES );
    renderOp->addInput( "positions" );
    renderOp->addInput( "radii" );
    renderOp->addInput( "depth" ); // From DepthComputation channel creator
    
    // Configure transfer function.
    renderOp->setTransferFunctionInput( "depth" );
    renderOp->setTransferFunction( lfx::core::loadImageFromDat( "01.dat" ) );
    renderOp->setTransferFunctionDestination( lfx::core::Renderer::TF_RGBA );
    
    // Configure hardware mask.
    renderOp->setHardwareMaskInputSource( lfx::core::Renderer::HM_SOURCE_RED );
    renderOp->setHardwareMaskOperator( lfx::core::Renderer::HM_OP_OFF );
    renderOp->setHardwareMaskReference( 0.f );
    
    renderOp->setDB( dbBase );
    
    dsp->setRenderer( renderOp );
    dsp->setDB( dbBase );
   
    /*m_bb[0] = 1000000;
    m_bb[1] = -1000000;
    m_bb[2] = 1000000;
    m_bb[3] = -1000000;
    m_bb[4] = 1000000;
    m_bb[5] = -1000000;*/
    
    /*ves::xplorer::scenegraph::VTKParticleTextureCreator::Point tempPoint;
    for( size_t i = 0; i < maxNumPoints; ++i )
    {
        //Iterate through all points
        std::deque< ves::xplorer::scenegraph::VTKParticleTextureCreator::Point > tempQueue;
        ///Add a pair for each scalar for each line
        std::vector< std::pair< std::string, std::vector< double > > > tempLineData;
        for( size_t k = 0; k < m_dataCollection.at( 0 ).size(); ++k )
        {
            std::vector< double > tempVec;
            tempLineData.push_back( std::make_pair< std::string, std::vector< double > >( m_dataCollection.at( 0 ).at( k ).first, tempVec ) );
        }
        
        for( size_t j = 0; j < m_transientDataSet.size(); ++j )
        {
            std::vector< std::pair< vtkIdType, double* > >* activeCellGroups =
            &m_pointCollection.at( j );
            std::vector< std::pair< std::string, std::vector< double > > >* dataCollection =
            &m_dataCollection.at( j );
            
            if( i < activeCellGroups->size() )
            {
                vtkIdType cellid = activeCellGroups->at( i ).first;
                double* pointid = activeCellGroups->at( i ).second;
                tempPoint.x[ 0 ] = pointid[ 0 ];
                tempPoint.x[ 1 ] = pointid[ 1 ];
                tempPoint.x[ 2 ] = pointid[ 2 ];
                tempPoint.vertId = cellid;
                //We can delete these hear because i never repeats and
                //is always moving forward
                delete [] pointid;
                activeCellGroups->at( i ).second = 0;
                for( size_t k = 0; k < dataCollection->size(); ++k )
                {
                    tempLineData.at( k ).second.push_back( dataCollection->at( k ).second.at( i ) );
                }
            }
            else
            {
                tempPoint.vertId = i;
                tempPoint.x[ 0 ] = 0.;
                tempPoint.x[ 1 ] = 0.;
                tempPoint.x[ 2 ] = 0.;
                for( size_t k = 0; k < dataCollection->size(); ++k )
                {
                    tempLineData.at( k ).second.push_back( 0.0 );
                }
            }
            tempQueue.push_back( tempPoint );
            //std::cout << tempCellGroups->at( i ).first << " "
            //    << tempCellGroups->at( i ).second << std::endl;
            //DataSet* tempData = transData.at( j );
            //vtkDataObject* tempDataObject = tempData->GetDataSet();
            if( tempPoint.x[0] < m_bb[0] )
            {
                m_bb[0] = tempPoint.x[0];
            }
            if( tempPoint.x[0] > m_bb[1] )
            {
                m_bb[1] = tempPoint.x[0];
            }
            if( tempPoint.x[1] < m_bb[2] )
            {
                m_bb[2] = tempPoint.x[1];
            }
            if( tempPoint.x[1] > m_bb[3] )
            {
                m_bb[3] = tempPoint.x[1];
            }
            if( tempPoint.x[2] < m_bb[4] )
            {
                m_bb[4] = tempPoint.x[2];
            }
            if( tempPoint.x[2] > m_bb[5] )
            {
                m_bb[5] = tempPoint.x[2];
            }
        }
        m_streamlineList.push_back( tempQueue );
        m_lineDataCollection.push_back( tempLineData );
        //std::cout << std::endl;
        //std::cout << std::endl;
        //std::cout << std::endl;
    }*/
    
    /*for( size_t i = 0; i < m_pointCollection.size(); ++i )
     {
     std::vector< std::pair< vtkIdType, double* > >* activeCellGroups =
     &m_pointCollection.at( i );
     size_t numParticleTracks = activeCellGroups->size();
     for( size_t j = 0; j < numParticleTracks; ++j )
     {
     delete [] activeCellGroups->at( j ).second;
     }
     }*/
    ///Clean up memory now that we have transferred it to the streamline list
    m_pointCollection.clear();
    m_dataCollection.clear();
    //osg::Geode* geode = new osg::Geode();
    
    //createStreamLines( geode );
    
    //m_streamlineList.clear();
    m_lineDataCollection.clear();
    
    return dsp;
}
////////////////////////////////////////////////////////////////////////////////
lfx::core::vtk::DataSetPtr LoadDataSet( std::string filename )
{
    lfx::core::vtk::DataSetPtr tempDataSet( new lfx::core::vtk::DataSet() );
    tempDataSet->SetFileName( filename );
    tempDataSet->SetUUID( "VTK_DATA_FILE", "test" );

    //tempDataSet->SetArrow(
    //                        ves::xplorer::ModelHandler::instance()->GetArrow() );
    //Check and see if the data is part of a transient series
    /*if( tempInfoPacket->GetProperty( "VTK_TRANSIENT_SERIES" ) )
    {
    std::string precomputedSurfaceDir =
    tempInfoPacket->GetProperty( "VTK_TRANSIENT_SERIES" )->
    GetDataString();
    lastDataAdded->LoadTransientData( precomputedSurfaceDir );
    }
    else*/
    {
        fs::path pathName( filename );
        if( !fs::exists( pathName ) )
        {
            return lfx::core::vtk::DataSetPtr();
        }

        fs::path parentDir = pathName.parent_path();

        tempDataSet->LoadTransientData( parentDir.string(), pathName.extension().string() );
    }

    /*{
        //ves::open::xml::DataValuePairPtr stringDVP =
        //    tempInfoPacket->GetProperty( "VTK_ACTIVE_DATA_ARRAYS" );
        //std::vector< std::string > vecStringArray;
        //if( stringDVP )
        //{
        //ves::open::xml::OneDStringArrayPtr stringArray =
        //boost::dynamic_pointer_cast <
        //ves::open::xml::OneDStringArray > (
        //stringDVP->GetDataXMLObject() );
        //vecStringArray = stringArray->GetArray();
        //tempDataSet->SetActiveDataArrays( vecStringArray );
        //}
        tempDataSet->LoadData();
    }*/
    tempDataSet->SetActiveVector( 0 );
    tempDataSet->SetActiveScalar( 0 );

    const std::string tempDataSetFilename = tempDataSet->GetFileName();
    std::cout << "|\tLoading data for file " << tempDataSetFilename << std::endl;

    if( tempDataSet->IsPartOfTransientSeries() )
    {
        transientSeries =
            tempDataSet->GetTransientDataSets();
    }
    
    vtkDataObject* tempVtkDataSet = tempDataSet->GetDataSet();
    //If the data load failed
    if( !tempVtkDataSet )
    {
        std::cout << "|\tData failed to load." << std::endl;
        //_activeModel->DeleteDataSet( tempDataSetFilename );
    }
    else
    {
        std::cout << "|\tData is loaded for file "
            << tempDataSetFilename
            << std::endl;
        //if( lastDataAdded->GetParent() == lastDataAdded )
        //{
        //_activeModel->GetDCS()->
        //AddChild( lastDataAdded->GetDCS() );
        //_activeModel->SetActiveDataSet( 0 );
        //}
        //m_datafileLoaded( tempDataSetFilename );
    }
    return tempDataSet;
}
////////////////////////////////////////////////////////////////////////////////
int main( int argc, char** argv )
{
    lfx::core::Log::instance()->setPriority( lfx::core::Log::PrioInfo, lfx::core::Log::Console );
    
    vtkCompositeDataPipeline* prototype = vtkCompositeDataPipeline::New();
    vtkAlgorithm::SetDefaultExecutivePrototype( prototype );
    prototype->Delete();


    lfx::core::DBBasePtr dbBase;
/*#ifdef LFX_USE_CRUNCHSTORE
    if( !( csFile.empty() ) )
    {
        DBCrunchStorePtr cs( DBCrunchStorePtr( new DBCrunchStore() ) );

        crunchstore::DataManagerPtr manager( crunchstore::DataManagerPtr( new crunchstore::DataManager() ) );
        crunchstore::DataAbstractionLayerPtr cache( new crunchstore::NullCache );
        crunchstore::DataAbstractionLayerPtr buffer( new crunchstore::NullBuffer );
        manager->SetCache( cache );
        manager->SetBuffer( buffer );
        crunchstore::SQLiteStorePtr sqstore( new crunchstore::SQLiteStore );
        sqstore->SetStorePath( csFile );
        manager->AttachStore( sqstore, crunchstore::Store::BACKINGSTORE_ROLE );
        try {
            cs->setDataManager( manager );
        }
        catch( std::exception exc ) {
            LFX_FATAL_STATIC( logstr, std::string(exc.what()) );
            LFX_FATAL_STATIC( logstr, "Unable to set DataManager." );
            exit( 1 );
        }

        dbBase = (DBBasePtr)cs;
    }
#endif*/
    //if( csFile.empty() )
    {
        lfx::core::DBDiskPtr disk( lfx::core::DBDiskPtr( new lfx::core::DBDisk() ) );
        std::string filePath( "." );
        disk->setRootPath( filePath );
        dbBase = disk;
    }



    lfx::core::vtk::DataSetPtr tempDataSet = LoadDataSet( argv[ 1 ] );
    //vtkDataObject* tempVtkDataSet = tempDataSet->GetDataSet();
    lfx::core::DataSetPtr dsp = createInstanced( transientSeries, "test", "test", dbBase );
    //tempVtkDataSet->Delete();
    //delete tempDataSet;
    // Create an example data set.
    //lfx::core::ChannelDataOSGArrayPtr dsp( prepareDataSet() );
    // Play the time series animation
    lfx::core::PlayControlPtr playControl( new lfx::core::PlayControl( dsp->getSceneData() ) );
    playControl->setTimeRange( dsp->getTimeRange() );

    osg::ref_ptr< osg::Group > rootGroup = new osg::Group();
    rootGroup->addChild( dsp->getSceneData() );

    /*{
        //Create the DataSet for this visualization with VTK
        lfx::core::DataSetPtr dsp2( new lfx::core::DataSet() );

        lfx::core::vtk::ChannelDatavtkDataObjectPtr dobjPtr( new lfx::core::vtk::ChannelDatavtkDataObject( tempDataSet->GetDataSet(), "vtkDataObject" ) );
        dsp2->addChannel( dobjPtr );
        
        lfx::core::vtk::VTKSurfaceWrapRTPPtr surfaceRTP( new lfx::core::vtk::VTKSurfaceWrapRTP() );
        surfaceRTP->addInput( "vtkDataObject" );
        dsp2->addOperation( surfaceRTP );

        //Try the vtkActor renderer
        lfx::core::vtk::VTKActorRendererPtr renderOp( new lfx::core::vtk::VTKActorRenderer() );
        //renderOp->SetActiveVector( "Momentum" );
        //renderOp->SetActiveScalar( "Density" );
        renderOp->addInput( "vtkPolyDataMapper" );
        dsp2->setRenderer( renderOp );

        rootGroup->addChild( dsp2->getSceneData() );
    }*/

    osgViewer::Viewer viewer;
    viewer.setUpViewInWindow( 10, 30, 800, 440 );
    viewer.setCameraManipulator( new osgGA::TrackballManipulator() );
    viewer.addEventHandler( new osgViewer::StatsHandler() );
    // add the window size toggle handler
    viewer.addEventHandler(new osgViewer::WindowSizeHandler);
    // Obtain the data set's scene graph and add it to the viewer.
    viewer.setSceneData( rootGroup.get() );

    double prevClockTime( 0. );
    while( !( viewer.done() ) )
    {
        const double clockTime( viewer.getFrameStamp()->getReferenceTime() );
        const double elapsed( clockTime - prevClockTime );
        prevClockTime = clockTime;
        playControl->elapsedClockTick( elapsed );

        viewer.frame();
    }

    return( 0 );
}
