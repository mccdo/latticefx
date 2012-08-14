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

#include <osgViewer/Viewer>

#include <latticefx/utils/vtk/DataSet.h>

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
            }
            LFX_CRITICAL_STATIC( logstr, ostr.str() );
        }
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
int main( int argc, char** argv )
{
    //Pre work specific to VTK
    vtkCompositeDataPipeline* prototype = vtkCompositeDataPipeline::New();
    vtkAlgorithm::SetDefaultExecutivePrototype( prototype );
    prototype->Delete();
    
    //Load the VTK data
    lfx::vtk_utils::DataSet* tempDataSet = LoadDataSet( argv[ 1 ] );
    
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
    lfx::core::vtk::VTKSurfaceRendererPtr renderOp( new lfx::core::vtk::VTKSurfaceRenderer() );
    renderOp->SetActiveVector( "Momentum" );
    renderOp->SetActiveScalar( "Density" );
    renderOp->addInput( "vtkPolyDataMapper" );
    dsp->setRenderer( renderOp );
    
    std::cout << "lfx...creating data..." << std::endl;
    osg::Node* sceneNode = dsp->getSceneData();
    std::cout << "...finished creating data. " << std::endl;
    
    //Clean up the raw vtk memory
    delete tempDataSet;
    
    //And do not forget to cleanup the algorithm executive prototype
    vtkAlgorithm::SetDefaultExecutivePrototype( 0 );
    
    osgViewer::Viewer viewer;
    viewer.setUpViewInWindow( 10, 30, 800, 440 );
    // Obtain the data set's scene graph and add it to the viewer.
    viewer.setSceneData( sceneNode );
    
    return( viewer.run() );
}

