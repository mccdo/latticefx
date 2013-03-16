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
    MyKeyHandler( lfx::core::DataSetPtr dsp, osg::Group* group )
        :
        m_dsp( dsp ),
        m_group( group ),
        _mode( false )
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
        }
        return( handled );
    }
    
protected:
    lfx::core::DataSetPtr m_dsp;
    osg::ref_ptr< osg::Group > m_group;
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
int main( int argc, char** argv )
{
    Log::instance()->setPriority( Log::PrioInfo, Log::Console );
    Log::instance()->setPriority( Log::PrioInfo, "lfx.core.hier" );

    //Pre work specific to VTK
    vtkCompositeDataPipeline* prototype = vtkCompositeDataPipeline::New();
    vtkAlgorithm::SetDefaultExecutivePrototype( prototype );
    prototype->Delete();

    osg::ref_ptr< osg::Group > tempGroup = new osg::Group();

    //Load the VTK data
    lfx::core::vtk::DataSetPtr tempDataSet( LoadDataSet( argv[ 1 ] ) );

    //1st Step
    //Since we are not modifying the original channel data we can create
    //just one instance of it
    lfx::core::vtk::ChannelDatavtkDataObjectPtr dobjPtr( new lfx::core::vtk::ChannelDatavtkDataObject( tempDataSet->GetDataSet(), "vtkDataObject" ) );

    lfx::core::DataSetPtr dsp1( new lfx::core::DataSet() );
    {
        //Create the DataSet for this visualization with VTK
        dsp1->addChannel( dobjPtr );

        lfx::core::vtk::VTKContourSliceRTPPtr vectorRTP( new lfx::core::vtk::VTKContourSliceRTP() );
        vectorRTP->SetPlaneDirection( lfx::core::vtk::CuttingPlane::Y_PLANE );
        vectorRTP->SetRequestedValue( 50.0 );
        vectorRTP->addInput( "vtkDataObject" );
        dsp1->addOperation( vectorRTP );

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
        dsp1->setRenderer( renderOp );

        std::cout << "lfx...creating data..." << std::endl;
        tempGroup->addChild( dsp1->getSceneData() );
        std::cout << "...finished creating data. " << std::endl;
        renderOp->dumpUniformInfo( std::cout );
    }

    {
        //Create the DataSet for this visualization with VTK
        lfx::core::DataSetPtr dsp( new lfx::core::DataSet() );
        dsp->addChannel( dobjPtr );

        lfx::core::vtk::VTKIsoSurfaceRTPPtr isosurfaceRTP( new lfx::core::vtk::VTKIsoSurfaceRTP() );
#if 0
        isosurfaceRTP->SetRequestedValue( 500.0 );
        isosurfaceRTP->SetActiveScalar( "200_to_1000" );
#else
        isosurfaceRTP->SetRequestedValue( 0.0 );
        isosurfaceRTP->SetActiveScalar( "StagnationEnergy" );
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
        renderOp2->SetActiveScalar( "StagnationEnergy" );
#endif
        renderOp2->addInput( "vtkPolyDataMapper" );
        renderOp2->addInput( "vtkDataObject" );
        dsp->setRenderer( renderOp2 );

        std::cout << "lfx...creating data..." << std::endl;
        tempGroup->addChild( dsp->getSceneData() );
        std::cout << "...finished creating data. " << std::endl;
        renderOp2->dumpUniformInfo( std::cout );
    }

    //And do not forget to cleanup the algorithm executive prototype
    vtkAlgorithm::SetDefaultExecutivePrototype( 0 );

    osgViewer::Viewer viewer;
    viewer.setUpViewInWindow( 10, 30, 800, 440 );
    // Obtain the data set's scene graph and add it to the viewer.
    viewer.setSceneData( tempGroup.get() );
    viewer.addEventHandler( new MyKeyHandler( dsp1, tempGroup.get() ) );

    return( viewer.run() );
}

