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

#include <latticefx/core/vtk/ChannelDatavtkPolyData.h>
#include <latticefx/core/vtk/ChannelDatavtkDataObject.h>
#include <latticefx/core/vtk/VTKVectorFieldRTP.h>
#include <latticefx/core/vtk/VTKVectorRenderer.h>
#include <latticefx/core/vtk/VTKActorRenderer.h>
#include <latticefx/core/vtk/VTKContourSliceRTP.h>

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
////////////////////////////////////////////////////////////////////////////////
int main( int argc, char** argv )
{
    lfx::core::Log::instance()->setPriority( lfx::core::Log::PrioInfo, lfx::core::Log::Console );
    lfx::core::Log::instance()->setPriority( lfx::core::Log::PrioInfo, "lfx.core.hier" );

    //Pre work specific to VTK
    vtkCompositeDataPipeline* prototype = vtkCompositeDataPipeline::New();
    vtkAlgorithm::SetDefaultExecutivePrototype( prototype );
    prototype->Delete();

    //Load the VTK data
    lfx::core::vtk::DataSetPtr tempDataSet( LoadDataSet( argv[ 1 ] ) );
    
    //Create the DataSet for this visualization with VTK
    lfx::core::DataSetPtr dsp( new lfx::core::DataSet() );
    
    //1st Step
    lfx::core::vtk::ChannelDatavtkDataObjectPtr dobjPtr( new lfx::core::vtk::ChannelDatavtkDataObject( tempDataSet->GetDataSet(), "vtkDataObject" ) );
    dsp->addChannel( dobjPtr );
    
    lfx::core::vtk::VTKContourSliceRTPPtr vectorRTP( new lfx::core::vtk::VTKContourSliceRTP() );
    vectorRTP->SetRequestedValue( 50.0 );
    vectorRTP->addInput( "vtkDataObject" );
    dsp->addOperation( vectorRTP );

    //2nd Step - the output of this is a ChannelData containing vtkPolyData
    //lfx::core::vtk::VTKVectorFieldRTPPtr vectorRTP( new lfx::core::vtk::VTKVectorFieldRTP() );
    //vectorRTP->addInput( "vtkDataObject" );
    //dsp->addOperation( vectorRTP );
    
    //3rd Step - now lets use out generic Renderer for vtkPolyData-to-an-instance-vector-field
    /*lfx::core::vtk::VTKVectorRendererPtr renderOp( new lfx::core::vtk::VTKVectorRenderer() );
    renderOp->SetActiveVector( "Momentum" );
    renderOp->SetActiveScalar( "Density" );
    renderOp->addInput( "vtkPolyData" );
    dsp->setRenderer( renderOp );*/

    //Try the vtkActor renderer
    lfx::core::vtk::VTKActorRendererPtr renderOp( new lfx::core::vtk::VTKActorRenderer() );
    renderOp->SetActiveVector( "Momentum" );
    renderOp->SetActiveScalar( "Density" );
    renderOp->addInput( "vtkPolyDataMapper" );
    dsp->setRenderer( renderOp );
    
    std::cout << "lfx...creating data..." << std::endl;
    osg::Node* sceneNode = dsp->getSceneData();
    std::cout << "...finished creating data. " << std::endl;

    //And do not forget to cleanup the algorithm executive prototype
    vtkAlgorithm::SetDefaultExecutivePrototype( 0 );

    osgViewer::Viewer viewer;
    viewer.setUpViewInWindow( 10, 30, 800, 440 );
    // Obtain the data set's scene graph and add it to the viewer.
    viewer.setSceneData( sceneNode );

    return( viewer.run() );
}
