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
#include <latticefx/core/vtk/VTKVectorRenderer.h>

#include <latticefx/core/vtk/ChannelDatavtkPolyData.h>

#include <latticefx/ChannelDataOSGArray.h>
#include <latticefx/TransferFunctionUtils.h>

#include <vtkMath.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>

namespace lfx {
    
namespace core {

namespace vtk {

////////////////////////////////////////////////////////////////////////////////
void VTKVectorRenderer::SetActiveVector( const std::string& activeVector )
{
    m_activeVector = activeVector;
}
////////////////////////////////////////////////////////////////////////////////
void VTKVectorRenderer::SetActiveScalar( const std::string& activeScalar )
{
    m_activeScalar = activeScalar;
}
////////////////////////////////////////////////////////////////////////////////
osg::Node* VTKVectorRenderer::getSceneGraph( const lfx::ChannelDataPtr maskIn )
{
    vtkPolyData* tempVtkPD = 
    boost::static_pointer_cast< lfx::core::vtk::ChannelDatavtkPolyData >( getInput( "vtkPolyData" ) )->GetPolyData();
    
    vtkPoints* points = tempVtkPD->GetPoints();
    size_t dataSize = points->GetNumberOfPoints();
    
    vtkPointData* pointData = tempVtkPD->GetPointData();
    vtkDataArray* vectorArray = pointData->GetVectors(m_activeVector.c_str());
    vtkDataArray* scalarArray = pointData->GetScalars(m_activeScalar.c_str());
    
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
    
    setPointStyle( lfx::VectorRenderer::DIRECTION_VECTORS );
    
    //by this stage of the game the render has already had setInputs called 
    //on it by lfx::DataSet therefore we can modify the _inputs array
    lfx::ChannelDataOSGArrayPtr vertData( new lfx::ChannelDataOSGArray( vertArray.get(), "positions" ) );
    addInput( vertData );
    
    lfx::ChannelDataOSGArrayPtr dirData( new lfx::ChannelDataOSGArray( dirArray.get(), "directions" ) );
    addInput( dirData );
    
    lfx::ChannelDataOSGArrayPtr colorData( new lfx::ChannelDataOSGArray( colorArray.get(), "scalar" ) );
    addInput( colorData );
    
    // Configure transfer function.
    setTransferFunctionInput( "scalar" );
    setTransferFunction( lfx::loadImageFromDat( "01.dat" ) );
    setTransferFunctionDestination( lfx::Renderer::TF_RGBA );
    
#if WRITE_IMAGE_DATA            
    //osgDB::writeNodeFile( *(tempGeode.get()), "gpu_vector_field.ive" );
#endif
    
    return( lfx::VectorRenderer::getSceneGraph( maskIn ) );
}
////////////////////////////////////////////////////////////////////////////////    
}
}
}

