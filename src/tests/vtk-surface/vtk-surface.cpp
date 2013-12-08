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
#include <latticefx/core/SurfaceRenderer.h>
#include <latticefx/core/TransferFunctionUtils.h>
#include <latticefx/core/PluginManager.h>

#include <latticefx/core/Log.h>
#include <latticefx/core/LogMacros.h>

#include <osgViewer/Viewer>
#include <osgGA/TrackballManipulator>

#include <boost/foreach.hpp>

#include <osg/io_utils>
#include <sstream>

#include <latticefx/core/vtk/ChannelDatavtkPolyData.h>
#include <latticefx/core/vtk/ChannelDatavtkDataObject.h>
#include <latticefx/core/vtk/VTKVectorFieldRTP.h>
#include <latticefx/core/vtk/VTKVectorRenderer.h>
#include <latticefx/core/vtk/VTKActorRenderer.h>
#include <latticefx/core/vtk/VTKContourSliceRTP.h>
#include <latticefx/core/vtk/VTKSurfaceRenderer.h>
#include <latticefx/core/vtk/VTKIsoSurfaceRTP.h>
#include <latticefx/core/vtk/ObjFactoryVtk.h>

#include <osgViewer/Viewer>

#include <latticefx/core/vtk/DataSet.h>

#include <vtkCompositeDataPipeline.h>
#include <vtkAlgorithm.h>
#include <vtkAlgorithmOutput.h>
#include <vtkDataObject.h>


using namespace lfx::core;


const std::string logstr( "lfx.demo" );
////////////////////////////////////////////////////////////////////////////////
class MyKeyHandler : public osgGA::GUIEventHandler
{
public:
    MyKeyHandler( lfx::core::DataSetPtr dsp, osg::Group* group, const std::vector<std::string> &scalars, const std::vector<std::string> &vectors )
        :
        m_dsp( dsp ),
        m_group( group ),
        _mode( false ),
		_scalars( scalars ),
		_vectors( vectors ),
		_curScalar( 0 ),
		_curColorScalar( 0 )
    {
        ;
    }
    
    virtual bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa, osg::Object*, osg::NodeVisitor* )
    {
        bool handled( false );
        if( ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN )
        {
            if( ea.getKey() == 'a' )
            {
                boost::dynamic_pointer_cast< lfx::core::vtk::VTKSurfaceRenderer >( m_dsp->getRenderer() )->SetActiveScalar( "first-scalar" );
                m_dsp->setDirty( lfx::core::DataSet::RENDERER_DIRTY );
                m_dsp->getSceneData();
                handled = true;
            }
			else if( ea.getKey() == 's' )
			{
				// set the next channel
				if( _scalars.size() <= 1 ) return handled;

				 _curScalar++;
				if( _curScalar >= _scalars.size() ) _curScalar = 0;
				std::string name = _scalars[_curScalar];
				boost::dynamic_pointer_cast< lfx::core::vtk::VTKSurfaceRenderer >( m_dsp->getRenderer() )->SetActiveScalar( name );
                

				lfx::core::RTPOperationList& oplist = m_dsp->getOperations();
				BOOST_FOREACH( lfx::core::RTPOperationPtr op, oplist  )
				{
					lfx::core::vtk::VTKBaseRTP *prtp = dynamic_cast<lfx::core::vtk::VTKBaseRTP *>( op.get() );
					if( !prtp ) continue;
					prtp->SetActiveScalar( name );
				}

				//m_dsp->setDirty( lfx::core::DataSet::RENDERER_DIRTY | lfx::core::DataSet::RTPOPERATION_DIRTY );
				m_dsp->setDirty( lfx::core::DataSet::ALL_DIRTY );
				m_dsp->updateAll();
                handled = true;
			}
			else if( ea.getKey() == 'c' )
			{
				// set the next channel
				if( _scalars.size() <= 1 ) return handled;

				_curColorScalar++;
				if( _curColorScalar >= _scalars.size() ) _curColorScalar = 0;
				std::string name = _scalars[_curColorScalar];
				boost::dynamic_pointer_cast< lfx::core::vtk::VTKSurfaceRenderer >( m_dsp->getRenderer() )->SetColorByScalar( name );

				m_dsp->setDirty( lfx::core::DataSet::RENDERER_DIRTY | lfx::core::DataSet::RTPOPERATION_DIRTY );
                m_dsp->updateAll();
                handled = true;
			}
        }
        return( handled );
    }
    
protected:
    lfx::core::DataSetPtr m_dsp;
    osg::ref_ptr< osg::Group > m_group;
	std::vector<std::string> _scalars;
	std::vector<std::string> _vectors;
	int _curScalar;
	int _curColorScalar;
    bool _mode;
};

////////////////////////////////////////////////////////////////////////////////
lfx::core::vtk::DataSetPtr LoadDataSet( std::string filename )
{
    lfx::core::vtk::DataSetPtr tempDataSet( new lfx::core::vtk::DataSet() );
    tempDataSet->SetFileName( filename );
    tempDataSet->SetUUID( "VTK_DATA_FILE", "test" );
    const std::string tempDataSetFilename = tempDataSet->GetFileName();
    std::cout << "|\tLoading data for file " << tempDataSetFilename << std::endl;

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
        tempDataSet->Print();
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
bool loadPipeline( lfx::core::DataSetPtr dsp, const std::string &jsonfile )
{
	// load pipeline from json serialization

	std::string err;
	// Add additional plugin search paths.
	lfx::core::PluginManager* plug( lfx::core::PluginManager::instance() );
	if( plug == NULL )
	{
		std::cout << "Failure: NULL PluginManager.";
		return( false );
	}
	plug->loadConfigFiles();

	lfx::core::vtk::ObjFactoryVtk objf( plug );
	if( !dsp->loadPipeline( &objf, jsonfile, &err ) )
	{
		std::cout << "Serialization load failed for file: " << jsonfile << endl;
		std::cout << "Error: " << err << endl;
		return( false );
	} 

	return( true );
}

////////////////////////////////////////////////////////////////////////////////
bool savePipeline( lfx::core::DataSetPtr dsp, const std::string &jsonfile )
{
	std::string err;
	if( !dsp->savePipeline( jsonfile, &err ) )
	{
		std::cout << "Serialization load failed" << err << endl;
		return( false );
	}

	return( true );
}

////////////////////////////////////////////////////////////////////////////////
lfx::core::DataSetPtr prepareVolume2(  osg::ref_ptr< osg::Group > tempGroup, 
									  lfx::core::vtk::ChannelDatavtkDataObjectPtr dobjPtr, 
									  lfx::core::vtk::DataSetPtr tempDataSet,
									  bool serialize, 
									  bool loadPipeLine )
{
	//Create the DataSet for this visualization with VTK
	lfx::core::DataSetPtr dsp( new lfx::core::DataSet() );
	dsp->addChannel( dobjPtr );

	// load pipeline from json serialization and return
	if( loadPipeLine )
	{
		if( !loadPipeline( dsp, "vtk-surface-vol2.json" ) ) return lfx::core::DataSetPtr();

		std::cout << "lfx...creating data from serialization..." << std::endl;
		tempGroup->addChild( dsp->getSceneData() );
		std::cout << "...finished creating data from serialization. " << std::endl;

		return dsp;
	}

	lfx::core::vtk::VTKIsoSurfaceRTPPtr isosurfaceRTP( new lfx::core::vtk::VTKIsoSurfaceRTP() );


#if 0
	// test roi
	std::vector<double> bounds;
	bounds.resize(6);
	tempDataSet->GetBounds(&bounds[0]);

	bounds[1] = bounds[0] + fabs(bounds[1] - bounds[0])/2;
	bounds[3] = bounds[2] + fabs(bounds[3] - bounds[2])/2.;
	//bounds[5] = bounds[4] + fabs(bounds[5] - bounds[4])/2.;
	isosurfaceRTP->SetRoiBox(bounds);
	isosurfaceRTP->ExtractBoundaryCells(true);
#endif
#if 0
	isosurfaceRTP->SetRequestedValue( 500.0 );
	isosurfaceRTP->SetActiveScalar( "200_to_1000" );
#else
	isosurfaceRTP->SetRequestedValue( 150.0 );
	isosurfaceRTP->SetActiveScalar( "Momentum_magnitude" );
#endif
	isosurfaceRTP->addInput( "vtkDataObject" );
	dsp->addOperation( isosurfaceRTP );

	//Try the vtkActor renderer
	lfx::core::vtk::VTKSurfaceRendererPtr renderOp2( new lfx::core::vtk::VTKSurfaceRenderer() );
#if 0
	renderOp2->SetActiveVector( "steve's_vector" );
	renderOp2->SetActiveScalar( "200_to_1000" );
#else
	renderOp2->SetActiveVector( "Momentum" );
	renderOp2->SetActiveScalar( "Momentum_magnitude" );
#endif
	renderOp2->addInput( "vtkPolyDataMapper" );
	renderOp2->addInput( "vtkDataObject" );
	dsp->setRenderer( renderOp2 );

	std::cout << "lfx...creating iso surface data..." << std::endl;
	tempGroup->addChild( dsp->getSceneData() );
	std::cout << "...finished creating data. " << std::endl;
	renderOp2->dumpUniformInfo( std::cout );

	if( serialize )
	{
		if( !savePipeline( dsp, "vtk-surface-vol2.json" ) ) return lfx::core::DataSetPtr();
		return dsp;
	}

	return( dsp );
}

////////////////////////////////////////////////////////////////////////////////
lfx::core::DataSetPtr prepareVolume1(  osg::ref_ptr< osg::Group > tempGroup, 
									  lfx::core::vtk::ChannelDatavtkDataObjectPtr dobjPtr, 
									  lfx::core::vtk::DataSetPtr tempDataSet,
									  bool serialize, 
									  bool loadPipeLine )
{
	lfx::core::DataSetPtr dsp( new lfx::core::DataSet() );

	//Create the DataSet for this visualization with VTK
	dsp->addChannel( dobjPtr );

	// load pipeline from json serialization and return
	if( loadPipeLine )
	{
		if( !loadPipeline( dsp, "vtk-surface-vol1.json" ) ) return lfx::core::DataSetPtr();

		std::cout << "lfx...creating data from serialization..." << std::endl;
		tempGroup->addChild( dsp->getSceneData() );
		std::cout << "...finished creating data from serialization. " << std::endl;

		return dsp;
	}
	
	lfx::core::vtk::VTKContourSliceRTPPtr vectorRTP( new lfx::core::vtk::VTKContourSliceRTP() );
	vectorRTP->SetPlaneDirection( lfx::core::vtk::CuttingPlane::Y_PLANE );
	vectorRTP->SetRequestedValue( 50.0 );
	vectorRTP->addInput( "vtkDataObject" );

#if 0
	// test roi
	std::vector<double> bounds;
	bounds.resize(6);
	tempDataSet->GetBounds(&bounds[0]);

	bounds[1] = bounds[0] + fabs(bounds[1] - bounds[0])/2.;
	//bounds[3] = bounds[2] + fabs(bounds[3] - bounds[2])/5.;
	//bounds[5] = bounds[4] + fabs(bounds[5] - bounds[4])/5.;
	vectorRTP->SetRoiBox(bounds);
	vectorRTP->ExtractBoundaryCells(true);
#endif

	dsp->addOperation( vectorRTP );

	//Try the vtkActor renderer
	lfx::core::vtk::VTKSurfaceRendererPtr renderOp( new lfx::core::vtk::VTKSurfaceRenderer() );
#if 0
	renderOp->SetActiveVector( "steve's_vector" );
	renderOp->SetActiveScalar( "200_to_1000" );
#else
	renderOp->SetActiveVector( "Momentum" );
	renderOp->SetActiveScalar( "Momentum_magnitude" );
#endif
	renderOp->addInput( "vtkPolyDataMapper" );
	renderOp->addInput( "vtkDataObject" );
	dsp->setRenderer( renderOp );

	std::cout << "lfx...creating contour data..." << std::endl;
	tempGroup->addChild( dsp->getSceneData() );
	std::cout << "...finished creating data. " << std::endl;
	renderOp->dumpUniformInfo( std::cout );

	if( serialize )
	{
		if( !savePipeline( dsp, "vtk-surface-vol1.json" ) ) return lfx::core::DataSetPtr();
		return dsp;
	}

	return( dsp );
}

////////////////////////////////////////////////////////////////////////////////
int main( int argc, char** argv )
{
    Log::instance()->setPriority( Log::PrioInfo, Log::Console );
    Log::instance()->setPriority( Log::PrioInfo, "lfx.core.hier" );

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

    osg::ref_ptr< osg::Group > tempGroup = new osg::Group();

    //Load the VTK data
    lfx::core::vtk::DataSetPtr tempDataSet( LoadDataSet( argv[ 1 ] ) );

	std::vector< std::string > scalars, vectors;
	scalars = tempDataSet->GetScalarNames();
    vectors = tempDataSet->GetVectorNames();

    //1st Step
    //Since we are not modifying the original channel data we can create
    //just one instance of it
    lfx::core::vtk::ChannelDatavtkDataObjectPtr dobjPtr( new lfx::core::vtk::ChannelDatavtkDataObject( tempDataSet->GetDataSet(), "vtkDataObject" ) );

	lfx::core::DataSetPtr dsp1;
    lfx::core::DataSetPtr dsp2;
	if( serialize )
	{
		// debug
		std::ofstream osPre1, osPst1, osPre2, osPst2;
		osPre1.open( "DataSetDumpPreVol1.txt" );
		osPst1.open( "DataSetDumpPstVol1.txt" );
		osPre2.open( "DataSetDumpPreVol2.txt" );
		osPst2.open( "DataSetDumpPstVol2.txt" );

		//lfx::core::DataSetPtr dsp2;
		osg::ref_ptr< osg::Group > grp = new osg::Group();
		//lfx::core::DataSetPtr dsp2;
		dsp1 = prepareVolume1( grp, dobjPtr, tempDataSet, true, false );
		if( dsp1 == NULL ) return -1;
		dsp2 = prepareVolume2( grp, dobjPtr, tempDataSet, true, false );
		if( dsp2 == NULL ) return -1;

		// debug
		dsp1->dumpState( osPre1 );
		dsp2->dumpState( osPre2 );
		

		dsp1 = prepareVolume1( tempGroup, dobjPtr, tempDataSet, false, true );
		if( dsp1 == NULL ) return -1;
		dsp2 = prepareVolume2( tempGroup, dobjPtr, tempDataSet, false, true );
		if( dsp2 == NULL ) return -1;

		// debug
		dsp1->dumpState( osPst1 );
		dsp2->dumpState( osPst2 );
	}
	else
	{
		dsp2 = prepareVolume1( tempGroup, dobjPtr, tempDataSet, false, false );
		dsp1 = prepareVolume2( tempGroup, dobjPtr, tempDataSet, false, false );
	}

    //And do not forget to cleanup the algorithm executive prototype
    vtkAlgorithm::SetDefaultExecutivePrototype( 0 );

    osgViewer::Viewer viewer;
    viewer.setUpViewInWindow( 10, 30, 800, 440 );
    // Obtain the data set's scene graph and add it to the viewer.
    viewer.setSceneData( tempGroup.get() );
    viewer.addEventHandler( new MyKeyHandler( dsp1, tempGroup.get(), scalars, vectors ) );

    return( viewer.run() );
}

