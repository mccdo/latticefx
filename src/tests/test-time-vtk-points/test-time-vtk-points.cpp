/*************** <auto-copyright.rb BEGIN do not edit this line> *************
*
* latticeFX is (C) Copyright 2011-2012 by Ames Laboratory
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Library General Public
* License as published by the Free Software Foundation; 
* version 2.1 of the License.
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
*************** <auto-copyright.rb END do not edit this line> **************/
#include <latticefx/core/DataSet.h>
#include <latticefx/core/ChannelData.h>
#include <latticefx/core/ChannelDataOSGArray.h>
#include <latticefx/core/RTPOperation.h>
#include <latticefx/core/Renderer.h>
#include <latticefx/core/TextureUtils.h>
#include <latticefx/core/BoundUtils.h>
#include <latticefx/core/VectorRenderer.h>
#include <latticefx/core/TransferFunctionUtils.h>

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Texture3D>
#include <osg/Shader>
#include <osg/Program>
#include <osg/Uniform>
#include <osgViewer/Viewer>
#include <osgDB/FileUtils>

#include <latticefx/utils/vtk/DataSet.h>

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

lfx::core::DataSetPtr prepareDirectionVectors( vtkPolyData* tempVtkPD, std::string vectorName, std::string scalarName )
{
    vtkPoints* points = tempVtkPD->GetPoints();
    size_t dataSize = points->GetNumberOfPoints();

    vtkPointData* pointData = tempVtkPD->GetPointData();
    vtkDataArray* vectorArray = pointData->GetVectors(vectorName.c_str());
    vtkDataArray* scalarArray = pointData->GetScalars(scalarName.c_str());
    
    double scalarRange[ 2 ] = {0,1.0};
    scalarArray->GetRange( scalarRange );
    
    double x[3];
    double val;
    //double rgb[3];
    
    osg::ref_ptr< osg::Vec3Array > vertArray( new osg::Vec3Array );
    vertArray->resize( dataSize );
    osg::ref_ptr< osg::Vec3Array > dirArray( new osg::Vec3Array );
    dirArray->resize( dataSize );
    osg::ref_ptr< osg::FloatArray > colorArray( new osg::FloatArray );
    colorArray->resize( dataSize );

    for( size_t i = 0; i < dataSize; ++i )
    {
        //Get Position data
        points->GetPoint( i, x );
        (*vertArray)[ i ].set( x[0], x[1], x[2] );

        if( scalarArray )
        {
            //Setup the color array
            scalarArray->GetTuple( i, &val );
            val = vtkMath::ClampAndNormalizeValue( val, scalarRange );
            (*colorArray)[ i ] = val;
            //lut->GetColor( val, rgb );
            //*scalarI++ = val;//rgb[0];
            //*scalarI++ = rgb[1];
            //*scalarI++ = rgb[2];
        }
        
        if( vectorArray )
        {
            //Get Vector data
            vectorArray->GetTuple( i, x );
            osg::Vec3 v( x[0], x[1], x[2] );
            v.normalize();
            (*dirArray)[ i ].set( v.x(), v.y(), v.z() );
        }
    }
    
    
    lfx::core::DataSetPtr dsp( new lfx::core::DataSet() );

    lfx::core::ChannelDataOSGArrayPtr vertData( new lfx::core::ChannelDataOSGArray( vertArray.get(), "positions" ) );
    dsp->addChannel( vertData );
    
    lfx::core::ChannelDataOSGArrayPtr dirData( new lfx::core::ChannelDataOSGArray( dirArray.get(), "directions" ) );
    dsp->addChannel( dirData );
    
    lfx::core::ChannelDataOSGArrayPtr colorData( new lfx::core::ChannelDataOSGArray( colorArray.get(), "scalar" ) );
    dsp->addChannel( colorData );

    // Add RTP operation to create a depth channel to use as input to the transfer function.
    //DepthComputation* dc( new DepthComputation() );
    //dc->addInput( "positions" );
    //dsp->addOperation( lfx::RTPOperationPtr( dc ) );
    
    lfx::core::VectorRendererPtr renderOp( new lfx::core::VectorRenderer() );
    renderOp->setPointStyle( lfx::core::VectorRenderer::DIRECTION_VECTORS );
    renderOp->addInput( "positions" );
    renderOp->addInput( "directions" );
    renderOp->addInput( "scalar" );
    
    // Configure transfer function.
    renderOp->setTransferFunctionInput( "scalar" );
    renderOp->setTransferFunction( lfx::core::loadImageFromDat( "01.dat" ) );
    renderOp->setTransferFunctionDestination( lfx::core::Renderer::TF_RGBA );
    
    dsp->setRenderer( renderOp );
    
    return( dsp );
}

////////////////////////////////////////////////////////////////////////////////
vtkAlgorithmOutput* ApplyGeometryFilterNew( vtkDataObject* tempVtkDataSet, vtkAlgorithmOutput* input )
{
    if( tempVtkDataSet->IsA( "vtkCompositeDataSet" ) )
    {
        vtkCompositeDataGeometryFilter* m_multiGroupGeomFilter = 
            vtkCompositeDataGeometryFilter::New();
        m_multiGroupGeomFilter->SetInputConnection( input );
        return m_multiGroupGeomFilter->GetOutputPort(0);
    }
    else
    {
        //m_geometryFilter->SetInputConnection( input );
        //return m_geometryFilter->GetOutputPort();
        vtkDataSetSurfaceFilter* m_surfaceFilter = 
            vtkDataSetSurfaceFilter::New();
        m_surfaceFilter->SetInputConnection( input );
        return m_surfaceFilter->GetOutputPort();
    }
}
////////////////////////////////////////////////////////////////////////////////
lfx::vtk_utils::DataSet* LoadDataSet( std::string filename )
{
    lfx::vtk_utils::DataSet* tempDataSet = new lfx::vtk_utils::DataSet();
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
lfx::core::DataSetPtr CreatePolyData( vtkDataObject* tempVtkDataSet )
{
    vtkCellDataToPointData* c2p = vtkCellDataToPointData::New();
    c2p->SetInput( tempVtkDataSet );
    //c2p->Update();
    
    vtkMaskPoints* ptmask = vtkMaskPoints::New();

    if( tempVtkDataSet->IsA( "vtkCompositeDataSet" ) )
    {
        vtkCompositeDataGeometryFilter* m_multiGroupGeomFilter = 
            vtkCompositeDataGeometryFilter::New();
        m_multiGroupGeomFilter->SetInputConnection( c2p->GetOutputPort() );
        //return m_multiGroupGeomFilter->GetOutputPort(0);
        ptmask->SetInputConnection( m_multiGroupGeomFilter->GetOutputPort(0) );
        m_multiGroupGeomFilter->Delete();
    }
    else
    {
        //m_geometryFilter->SetInputConnection( input );
        //return m_geometryFilter->GetOutputPort();
        vtkDataSetSurfaceFilter* m_surfaceFilter = 
            vtkDataSetSurfaceFilter::New();
        m_surfaceFilter->SetInputConnection( c2p->GetOutputPort() );
        //return m_surfaceFilter->GetOutputPort();
        ptmask->SetInputConnection( m_surfaceFilter->GetOutputPort() );
        m_surfaceFilter->Delete();
    }
    
    // get every nth point from the dataSet data
    ptmask->SetOnRatio( 1.0 );
    ptmask->Update();

    try
    {
        lfx::core::DataSetPtr templfxDataSet = prepareDirectionVectors( ptmask->GetOutput(), "Momentum", "Density" );

#if WRITE_IMAGE_DATA            
        osgDB::writeNodeFile( *(tempGeode.get()), "gpu_vector_field.ive" );
#endif
        c2p->Delete();
        ptmask->Delete();
        //tempAlgo->Delete();
        //this->updateFlag = true;
        return templfxDataSet;
    }
    catch( std::bad_alloc )
    {
        c2p->Delete();
        ptmask->Delete();
        //tempAlgo->Delete();
        //mapper->Delete();
        //mapper = vtkPolyDataMapper::New();
        //vprDEBUG( vesDBG, 0 ) << "|\tMemory allocation failure : cfdPresetVectors "
        //    << std::endl << vprDEBUG_FLUSH;
    }       
    return lfx::core::DataSetPtr();
}
////////////////////////////////////////////////////////////////////////////////
int main( int argc, char** argv )
{
    vtkCompositeDataPipeline* prototype = vtkCompositeDataPipeline::New();
    vtkAlgorithm::SetDefaultExecutivePrototype( prototype );
    prototype->Delete();

    lfx::vtk_utils::DataSet* tempDataSet = LoadDataSet( argv[ 1 ] );
    vtkDataObject* tempVtkDataSet = tempDataSet->GetDataSet();
    lfx::core::DataSetPtr testlfxDataSet = CreatePolyData( tempVtkDataSet );
    //tempVtkDataSet->Delete();
    delete tempDataSet;
    // Create an example data set.
    //lfx::core::ChannelDataOSGArrayPtr dsp( prepareDataSet() );

    osgViewer::Viewer viewer;
    viewer.setUpViewInWindow( 10, 30, 800, 440 );
    // Obtain the data set's scene graph and add it to the viewer.
    viewer.setSceneData( testlfxDataSet->getSceneData() );
    return( viewer.run() );
}
