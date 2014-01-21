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
#include <latticefx/core/PluginManager.h>

#include <latticefx/core/vtk/ChannelDatavtkPolyData.h>
#include <latticefx/core/vtk/ChannelDatavtkDataObject.h>
#include <latticefx/core/vtk/VTKStreamlineRTP.h>
#include <latticefx/core/vtk/VTKStreamlineRenderer.h>
#include <latticefx/core/vtk/ObjFactoryVtk.h>

#include <osgViewer/Viewer>

#include <latticefx/core/vtk/DataSet.h>
#include <latticefx/core/Log.h>
#include <latticefx/core/LogMacros.h>

#include <vtkCompositeDataPipeline.h>
#include <vtkAlgorithm.h>
#include <vtkAlgorithmOutput.h>
#include <vtkDataObject.h>

////////////////////////////////////////////////////////////////////////////////
lfx::core::vtk::DataSetPtr LoadDataSet( std::string filename )
{
    lfx::core::vtk::DataSetPtr tempDataSet( new lfx::core::vtk::DataSet() );
    tempDataSet->SetFileName( filename );
    tempDataSet->SetUUID( "VTK_DATA_FILE", "test" );
    //ves::open::xml::DataValuePairPtr stringDVP =
    //    tempInfoPacket->GetProperty( "VTK_ACTIVE_DATA_ARRAYS" );
    /*std::vector< std::string > vecStringArray;
    if( stringDVP )
    {
    ves::open::xml::OneDStringArrayPtr stringArray =
    boost::dynamic_pointer_cast <
    ves::open::xml::OneDStringArray > (
    stringDVP->GetDataXMLObject() );
    vecStringArray = stringArray->GetArray();
    tempDataSet->SetActiveDataArrays( vecStringArray );
    }*/
    const std::string tempDataSetFilename = tempDataSet->GetFileName();
    std::cout << "|\tLoading data for file " << tempDataSetFilename << std::endl;
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
        tempDataSet->LoadData();
        tempDataSet->SetActiveVector( 0 );
        tempDataSet->SetActiveScalar( 0 );
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

lfx::core::DataSetPtr prepareVolume( const char *dsFile, bool serialize, bool loadPipeline,  lfx::core::vtk::DataSetPtr &tempDataSet )
{
	//Load the VTK data
    //lfx::core::vtk::DataSetPtr tempDataSet( LoadDataSet( dsFile ) );
	tempDataSet = LoadDataSet( dsFile );

    //Create the DataSet for this visualization with VTK
    lfx::core::DataSetPtr dsp( new lfx::core::DataSet() );

    //1st Step
    lfx::core::vtk::ChannelDatavtkDataObjectPtr dobjPtr( new lfx::core::vtk::ChannelDatavtkDataObject( tempDataSet->GetDataSet(), "vtkDataObject" ) );
    dsp->addChannel( dobjPtr );

	// load pipeline from json serialization
	if( loadPipeline )
	{
		std::string err;
		// Add additional plugin search paths.
		lfx::core::PluginManager* plug( lfx::core::PluginManager::instance() );
		if( plug == NULL )
		{
			std::cout << "Failure: NULL PluginManager.";
			return( lfx::core::DataSetPtr() );
		}
		plug->loadConfigFiles();

		lfx::core::vtk::ObjFactoryVtk objf( plug );
		if( !dsp->loadPipeline( &objf, "vtk-streamline.json", &err ) )
		{
			std::cout << "Serialization load failed" << err << endl;
			return( lfx::core::DataSetPtr() );
		} 

		return( dsp );
	}

	std::vector< std::string > vnames = tempDataSet->GetVectorNames();
	std::vector< std::string > snames = tempDataSet->GetScalarNames();
	if( vnames.size() <= 0 || snames.size() <= 0 )
	{
		std::cout << "Failure: missing a vector or scalar.";
		return( lfx::core::DataSetPtr() );
	}

	std::string vector = vnames[0];
	std::string scalar = snames[0];


#define ROI_TEST 0

	std::vector<double> bounds;
	bounds.resize(6);
	tempDataSet->GetBounds(&bounds[0]);

	float maxTime = tempDataSet->GetMaxTime();
	lfx::core::vtk::VTKStreamlineRTPPtr rtp( new lfx::core::vtk::VTKStreamlineRTP() );
	rtp->setMaxTime( maxTime );
	rtp->SetActiveVector( vector );
    rtp->SetActiveScalar( scalar );
	rtp->setDatasetBounds(&bounds[0]);
    rtp->addInput( "vtkDataObject" );

#if ROI_TEST
		// test roi
		std::vector<double> bounds;
		bounds.resize(6);
		tempDataSet->GetBounds(&bounds[0]);

		// left, top front
		
		bounds[1] = bounds[0] + fabs(bounds[1] - bounds[0])/2.;
		bounds[3] = bounds[2] + fabs(bounds[3] - bounds[2])/2.;
		//bounds[5] = bounds[4] + fabs(bounds[5] - bounds[4])/2.;
		

		/*
		// half box
		double quarterx = fabs(bounds[1] - bounds[0])/4.; 
		double quartery = fabs(bounds[3] - bounds[2])/4.; 
		double quarterz = fabs(bounds[5] - bounds[4])/4.;
	
		bounds[0] += quarterx;
		bounds[1] -= quarterx;
		bounds[2] += quartery;
		bounds[3] -= quartery;
		bounds[4] += quarterz;
		bounds[5] -= quarterz;
		*/	


		vectorRTP->SetRoiBox(bounds);
		vectorRTP->ExtractBoundaryCells(true);
#endif
    dsp->addOperation( rtp );


	lfx::core::vtk::VTKStreamlineRendererPtr renderOp( new lfx::core::vtk::VTKStreamlineRenderer() );
    renderOp->SetActiveVector( vector );
    renderOp->SetActiveScalar( scalar );
    renderOp->addInput( "vtkPolyData" );
    renderOp->addInput( "vtkDataObject" );
	renderOp->addInput( "positions" );
	renderOp->setHardwareMaskOperator( lfx::core::Renderer::HM_OP_OFF );
	renderOp->setAnimationEnable( false );
    dsp->setRenderer( renderOp );

	if( serialize )
	{
		std::string err;
		if( !dsp->savePipeline( "vtk-streamline.json", &err ) )
		{
			std::cout << "Serialization load failed" << err << endl;
			return( lfx::core::DataSetPtr() );
		}
	}

	return dsp;
}

////////////////////////////////////////////////////////////////////////////////
int main( int argc, char** argv )
{
    lfx::core::Log::instance()->setPriority( lfx::core::Log::PrioInfo, lfx::core::Log::Console );
	lfx::core::Log::instance()->setPriority( lfx::core::Log::PrioInfo, "lfx.core.hier" );
	lfx::core::Log::instance()->setPriority( lfx::core::Log::PrioDebug, lfx::core::Log::Console );
	lfx::core::Log::instance()->setPriority( lfx::core::Log::PrioDebug, "lfx.core.hier" );
	lfx::core::Log::instance()->setPriority( lfx::core::Log::PrioTrace, lfx::core::Log::Console );
	lfx::core::Log::instance()->setPriority( lfx::core::Log::PrioTrace, "lfx.core.hier" );
    

	bool serialize = false;
	if (argc < 2) return false;
	if (argc > 2)
	{
		if( !strcmp(argv[2], "-ser") )
		{
			serialize = true;
		}
	}

    //Pre work specific to VTK
    vtkCompositeDataPipeline* prototype = vtkCompositeDataPipeline::New();
    vtkAlgorithm::SetDefaultExecutivePrototype( prototype );
    prototype->Delete();
	 
	lfx::core::DataSetPtr dsp;
	lfx::core::vtk::DataSetPtr tempDataSet;

	if( serialize )
	{
		// debug
		std::ofstream osPre, osPst; 
		osPre.open( "DataSetDumpPre.txt" );
		osPst.open( "DataSetDumpPst.txt" );

		dsp = prepareVolume( argv[1], true, false, tempDataSet );
		if( dsp == NULL ) return -1;

		// debug
		dsp->dumpState( osPre );
		osPre.close();

		dsp = prepareVolume( argv[1], false, true, tempDataSet );
		if( dsp == NULL ) return -1;

		// debug
		dsp->dumpState( osPst );
		osPst.close();
	}
	else
	{
		dsp = prepareVolume( argv[1], false, false, tempDataSet );
	}

	
	if( dsp == NULL ) return -1;

    osg::Node* sceneNode = dsp->getSceneData();

    //And do not forget to cleanup the algorithm executive prototype
    // vtkAlgorithm::SetDefaultExecutivePrototype( 0 );

    osgViewer::Viewer viewer;
    viewer.setUpViewInWindow( 10, 30, 800, 440 );
    // Obtain the data set's scene graph and add it to the viewer.
    viewer.setSceneData( sceneNode );

    return( viewer.run() );
}
