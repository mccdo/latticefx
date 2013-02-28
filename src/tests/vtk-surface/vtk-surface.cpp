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


void dumpUniformInfo( const RendererPtr renderOp )
{
    LFX_CRITICAL_STATIC( logstr, "Available uniforms:" );

    const Renderer::UniformInfoVector& infoVec( renderOp->getUniforms() );
    BOOST_FOREACH( const Renderer::UniformInfo& info, infoVec )
    {
        if( info._access == Renderer::UniformInfo::PUBLIC )
        {
            LFX_CRITICAL_STATIC( logstr, info._prototype->getName() + "\t" +
                Renderer::uniformTypeAsString( info._prototype->getType() ) + "\t" +
                info._description );

            // Display the default value.
            std::ostringstream ostr;
            ostr << "\tDefault: ";
            switch( info._prototype->getType() )
            {
            case osg::Uniform::FLOAT_MAT4:
            {
                osg::Matrix mat; info._prototype->get( mat );
                ostr << mat;
                break;
            }
            case osg::Uniform::FLOAT_VEC2:
            {
                osg::Vec2f vec2; info._prototype->get( vec2 );
                ostr << vec2;
                break;
            }
            case osg::Uniform::FLOAT_VEC3:
            {
                osg::Vec3f vec3; info._prototype->get( vec3 );
                ostr << vec3;
                break;
            }
            case osg::Uniform::FLOAT_VEC4:
            {
                osg::Vec4f vec4; info._prototype->get( vec4 );
                ostr << vec4;
                break;
            }
            case osg::Uniform::FLOAT:
            {
                float f; info._prototype->get( f );
                ostr << f;
                break;
            }
            case osg::Uniform::SAMPLER_1D:
            case osg::Uniform::SAMPLER_2D:
            case osg::Uniform::SAMPLER_3D:
            case osg::Uniform::INT:
            {
                int i; info._prototype->get( i );
                ostr << i;
                break;
            }
            case osg::Uniform::BOOL:
            {
                bool b; info._prototype->get( b );
                ostr << b;
                break;
            }
            default:
            {
                std::cout << "unsupported uniform type." << std::endl;
                break;
            }
            }
            LFX_CRITICAL_STATIC( logstr, ostr.str() );
        }
    }
}
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
    
    {
        //Create the DataSet for this visualization with VTK
        lfx::core::DataSetPtr dsp( new lfx::core::DataSet() );
        dsp->addChannel( dobjPtr );

        lfx::core::vtk::VTKContourSliceRTPPtr vectorRTP( new lfx::core::vtk::VTKContourSliceRTP() );
        vectorRTP->SetRequestedValue( 50.0 );
        vectorRTP->addInput( "vtkDataObject" );
        dsp->addOperation( vectorRTP );
        
        //Try the vtkActor renderer
        lfx::core::vtk::VTKSurfaceRendererPtr renderOp( new lfx::core::vtk::VTKSurfaceRenderer() );
        renderOp->SetActiveVector( "steve's_vector" );
        renderOp->SetActiveScalar( "200_to_1000" );
        renderOp->addInput( "vtkPolyDataMapper" );
        renderOp->addInput( "vtkDataObject" );
        dsp->setRenderer( renderOp );
        
        std::cout << "lfx...creating data..." << std::endl;
        tempGroup->addChild( dsp->getSceneData() );
        std::cout << "...finished creating data. " << std::endl;
    }
    
    {
        //Create the DataSet for this visualization with VTK
        lfx::core::DataSetPtr dsp( new lfx::core::DataSet() );
        dsp->addChannel( dobjPtr );

        lfx::core::vtk::VTKIsoSurfaceRTPPtr isosurfaceRTP( new lfx::core::vtk::VTKIsoSurfaceRTP() );
        isosurfaceRTP->SetRequestedValue( 500.0 );
        isosurfaceRTP->SetActiveScalar( "200_to_1000" );
        isosurfaceRTP->addInput( "vtkDataObject" );
        dsp->addOperation( isosurfaceRTP );
        
        //Try the vtkActor renderer
        lfx::core::vtk::VTKSurfaceRendererPtr renderOp2( new lfx::core::vtk::VTKSurfaceRenderer() );
        renderOp2->SetActiveVector( "steve's_vector" );
        renderOp2->SetActiveScalar( "200_to_1000" );
        renderOp2->addInput( "vtkPolyDataMapper" );
        renderOp2->addInput( "vtkDataObject" );
        dsp->setRenderer( renderOp2 );
        
        std::cout << "lfx...creating data..." << std::endl;
        tempGroup->addChild( dsp->getSceneData() );
        std::cout << "...finished creating data. " << std::endl;
    }
    
    //And do not forget to cleanup the algorithm executive prototype
    vtkAlgorithm::SetDefaultExecutivePrototype( 0 );
    
    osgViewer::Viewer viewer;
    viewer.setUpViewInWindow( 10, 30, 800, 440 );
    // Obtain the data set's scene graph and add it to the viewer.
    viewer.setSceneData( tempGroup.get() );
    
    return( viewer.run() );
}

