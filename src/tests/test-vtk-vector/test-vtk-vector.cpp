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
#include <latticefx/DataSet.h>
#include <latticefx/ChannelData.h>
#include <latticefx/ChannelDataOSGArray.h>
#include <latticefx/RTPOperation.h>
#include <latticefx/Renderer.h>
#include <latticefx/TextureUtils.h>
#include <latticefx/BoundUtils.h>
#include <latticefx/VectorRenderer.h>
#include <latticefx/TransferFunctionUtils.h>

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Texture3D>
#include <osg/Shader>
#include <osg/Program>
#include <osg/Uniform>
#include <osgViewer/Viewer>
#include <osgDB/FileUtils>

#include <osgwTools/Shapes.h>

#include <vtk_utils/readWriteVtkThings.h>
#include <vtk_utils/DataSet.h>

#include <vtkDataObject.h>
#include <vtkCompositeDataGeometryFilter.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkCellDataToPointData.h>
#include <vtkMaskPoints.h>
#include <vtkCompositeDataPipeline.h>
#include <vtkAlgorithm.h>
#include <vtkAlgorithmOutput.h>

#include "OSGVectorStage.h"

lfx::DataSetPtr prepareDirectionVectors( vtkPolyData* tempVtkPD, std::string vectorName, std::string scalarName )
{
    vtkPoints* points = tempVtkPD->GetPoints();
    size_t dataSize = points->GetNumberOfPoints();

    vtkPointData* pointData = tempVtkPD->GetPointData();
    vtkDataArray* vectorArray = pointData->GetVectors(vectorName.c_str());
    vtkDataArray* scalarArray = pointData->GetScalars(scalarName.c_str());
    
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
        {
            //Get Position data
            points->GetPoint( i, x );
            (*vertArray)[ i ].set( x[0], x[1], x[2] );

            if( scalarArray )
            {
                //Setup the color array
                scalarArray->GetTuple( i, &val );
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
    }
    
    
    lfx::DataSetPtr dsp( new lfx::DataSet() );

    lfx::ChannelDataOSGArrayPtr vertData( new lfx::ChannelDataOSGArray( vertArray.get(), "positions" ) );
    dsp->addChannel( vertData );
    
    lfx::ChannelDataOSGArrayPtr dirData( new lfx::ChannelDataOSGArray( dirArray.get(), "directions" ) );
    dsp->addChannel( dirData );
    
    // Add RTP operation to create a depth channel to use as input to the transfer function.
    //DepthComputation* dc( new DepthComputation() );
    //dc->addInput( "positions" );
    //dsp->addOperation( lfx::RTPOperationPtr( dc ) );
    
    lfx::VectorRendererPtr renderOp( new lfx::VectorRenderer() );
    renderOp->setPointStyle( lfx::VectorRenderer::DIRECTION_VECTORS );
    renderOp->addInput( "positions" );
    renderOp->addInput( "directions" );
    //renderOp->addInput( "depth" ); // From DepthComputation channel creator
    
    // Configure transfer function.
    /*renderOp->setTransferFunctionInput( "depth" );
    renderOp->setTransferFunction( lfx::loadImageFromDat( "01.dat" ) );
    renderOp->setTransferFunctionDestination( lfx::Renderer::TF_RGBA );*/
    
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
/*lfx::DataSetPtr prepareDataSet()
{
    osg::ref_ptr< osg::Vec3Array > vertArray( new osg::Vec3Array );
    osg::ref_ptr< osg::Vec3Array > dirArray( new osg::Vec3Array );
    const unsigned int w( 4 ), h( 1 ), d( 1 );
    vertArray->resize( w*h*d );
    dirArray->resize( w*h*d );
    unsigned int wIdx, hIdx, dIdx, index( 0 );
    for( wIdx=0; wIdx<w; ++wIdx )
    {
        for( hIdx=0; hIdx<h; ++hIdx )
        {
            for( dIdx=0; dIdx<d; ++dIdx )
            {
                const float x( ((double)wIdx)/(w-1.) * 4. - 2. );
                const float y( 0. );
                const float z( 0. );
                (*vertArray)[ index ].set( x, y, z );
                (*dirArray)[ index ].set( sin(x), .5, .5 );
                ++index;
            }
        }
    }
    lfx::ChannelDataOSGArrayPtr vertData( new lfx::ChannelDataOSGArray( vertArray.get(), "positions" ) );
    lfx::ChannelDataOSGArrayPtr dirData( new lfx::ChannelDataOSGArray( dirArray.get(), "directions" ) );

    // Create a data set and add the vertex and direction data.
    lfx::DataSetPtr dsp( new lfx::DataSet() );
    dsp->addChannel( vertData );
    dsp->addChannel( dirData );

    lfx::RendererPtr renderOp( new InstancedVectors() );
    renderOp->addInput( vertData->getName() );
    renderOp->addInput( dirData->getName() );
    dsp->setRenderer( renderOp );

    return( dsp );
}*/
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
lfx::DataSetPtr CreatePolyData( vtkDataObject* tempVtkDataSet )
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
        /*OSGVectorStage* tempStage = new OSGVectorStage();
        
        osg::ref_ptr< osg::Geode > tempGeode = 
            tempStage->createInstanced( ptmask->GetOutput(), "Momentum", "Density", 1.0 );
        delete tempStage;*/
        
        lfx::DataSetPtr templfxDataSet = prepareDirectionVectors( ptmask->GetOutput(), "Momentum", "Density" );

        double scalarRange[ 2 ] = {0,1.0};
        //GetActiveDataSet()->GetUserRange( scalarRange );
        
        //geodes.push_back( tempGeode.get() );
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
    return lfx::DataSetPtr();     
}
////////////////////////////////////////////////////////////////////////////////
int main( int argc, char** argv )
{
    vtkCompositeDataPipeline* prototype = vtkCompositeDataPipeline::New();
    vtkAlgorithm::SetDefaultExecutivePrototype( prototype );
    prototype->Delete();

    lfx::vtk_utils::DataSet* tempDataSet = LoadDataSet( argv[ 1 ] );
    vtkDataObject* tempVtkDataSet = tempDataSet->GetDataSet();
    lfx::DataSetPtr testlfxDataSet = CreatePolyData( tempVtkDataSet );
    //tempVtkDataSet->Delete();
    delete tempDataSet;
    // Create an example data set.
    //lfx::DataSetPtr dsp( prepareDataSet() );

    osgViewer::Viewer viewer;
    viewer.setUpViewInWindow( 10, 30, 800, 440 );
    // Obtain the data set's scene graph and add it to the viewer.
    viewer.setSceneData( testlfxDataSet->getSceneData() );
    return( viewer.run() );
}
