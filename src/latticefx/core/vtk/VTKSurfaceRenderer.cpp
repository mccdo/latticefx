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
#include <latticefx/core/vtk/VTKSurfaceRenderer.h>

#include <latticefx/core/vtk/ChannelDatavtkPolyData.h>
#include <latticefx/core/vtk/ChannelDatavtkPolyDataMapper.h>
#include <latticefx/core/vtk/VTKPrimitiveSetGenerator.h>

#include <latticefx/core/ChannelDataOSGArray.h>
#include <latticefx/core/TransferFunctionUtils.h>

#include <vtkMath.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkTriangleFilter.h>
#include <vtkStripper.h>
#include <vtkPolyDataNormals.h>
#include <vtkCellArray.h>
#include <vtkPolyDataMapper.h>

namespace lfx {
    
namespace core {

namespace vtk {

////////////////////////////////////////////////////////////////////////////////
void VTKSurfaceRenderer::SetActiveVector( const std::string& activeVector )
{
    m_activeVector = activeVector;
}
////////////////////////////////////////////////////////////////////////////////
void VTKSurfaceRenderer::SetActiveScalar( const std::string& activeScalar )
{
    m_activeScalar = activeScalar;
}
////////////////////////////////////////////////////////////////////////////////
void VTKSurfaceRenderer::SetScalarRange( double* array )
{
    m_scalarRange[ 0 ] = array[ 0 ];
    m_scalarRange[ 1 ] = array[ 1 ];
}
////////////////////////////////////////////////////////////////////////////////
osg::Node* VTKSurfaceRenderer::getSceneGraph( const lfx::core::ChannelDataPtr maskIn )
{
    m_pd = 
        boost::static_pointer_cast< lfx::core::vtk::ChannelDatavtkPolyDataMapper >( getInput( "vtkPolyDataMapper" ) )->GetPolyDataMapper()->GetInput();

    ExtractVTKPrimitives();
    
    vtkPoints* points = m_pd->GetPoints();
    size_t dataSize = points->GetNumberOfPoints();
    
    vtkPointData* pointData = m_pd->GetPointData();
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
            val = vtkMath::ClampAndNormalizeValue( val, m_scalarRange );
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
    
    //setPointStyle( lfx::core::VectorRenderer::DIRECTION_VECTORS );
    
    //by this stage of the game the render has already had setInputs called 
    //on it by lfx::core::DataSet therefore we can modify the _inputs array
    /*lfx::core::ChannelDataOSGArrayPtr vertData( new lfx::core::ChannelDataOSGArray( vertArray.get(), "positions" ) );
    addInput( vertData );
    
    lfx::core::ChannelDataOSGArrayPtr dirData( new lfx::core::ChannelDataOSGArray( dirArray.get(), "directions" ) );
    addInput( dirData );*/
    
    //lfx::core::ChannelDataOSGArrayPtr colorData( new lfx::core::ChannelDataOSGArray( colorArray.get(), "scalar" ) );
    //addInput( colorData );
    
    setInputNameAlias( SurfaceRenderer::VERTEX, "vertices" );
    setInputNameAlias( SurfaceRenderer::NORMAL, "normals" );

    // Configure transfer function.
    setTransferFunctionInput( "scalar" );
    setTransferFunction( lfx::core::loadImageFromDat( "01.dat" ) );
    setTransferFunctionDestination( lfx::core::Renderer::TF_RGBA );
    
#if WRITE_IMAGE_DATA            
    //osgDB::writeNodeFile( *(tempGeode.get()), "gpu_vector_field.ive" );
#endif
    
    return( lfx::core::SurfaceRenderer::getSceneGraph( maskIn ) );
}
////////////////////////////////////////////////////////////////////////////////
void VTKSurfaceRenderer::ExtractVTKPrimitives()
{
    m_pd->Update();
        
    vtkTriangleFilter* triangleFilter = vtkTriangleFilter::New();
    triangleFilter->SetInput( m_pd );
    triangleFilter->PassVertsOff();
    triangleFilter->PassLinesOff();
    //triangleFilter->Update();
    
    vtkStripper* triangleStripper = vtkStripper::New();
    triangleStripper->SetInput( triangleFilter->GetOutput() );
    int stripLength = triangleStripper->GetMaximumLength();
    triangleStripper->SetMaximumLength( stripLength * 1000 );
    //triangleStripper->Update();
    
    vtkPolyDataNormals* normalGen = vtkPolyDataNormals::New();
    normalGen->SetInput( triangleStripper->GetOutput() );
    normalGen->NonManifoldTraversalOn();
    normalGen->AutoOrientNormalsOn();
    normalGen->ConsistencyOn();
    normalGen->SplittingOn();
    
    vtkStripper* reTriangleStripper = vtkStripper::New();
    reTriangleStripper->SetInput( normalGen->GetOutput() );
    reTriangleStripper->SetMaximumLength( stripLength * 1000 );
    reTriangleStripper->Update();
    
    normalGen->Delete();
    triangleFilter->Delete();
    triangleStripper->Delete();
    
    m_pd = reTriangleStripper->GetOutput();
    
    vtkPointData* pointData = m_pd->GetPointData();
    vtkDataArray* normals = 0;
    
    //int numStrips = m_pd->GetNumberOfStrips();
    //int numPts = polydata->GetNumberOfPoints();
    //pointData = m_pd->GetPointData();
    normals = pointData->GetVectors( "Normals" );
    
    //vtkDataArray* vectorArray = pointData->GetVectors( disp.c_str() );
    vtkPoints* points = m_pd->GetPoints();
    
    //vtkDataArray* dataArray = pointData->GetScalars( colorScalar.c_str() );
    //double dataRange[2];
    //dataArray->GetRange( dataRange );
    
    //Call to lut tools
    
    double cVal;
    //double curColor[3];
    
    osg::ref_ptr< osg::Vec3Array > v = new osg::Vec3Array;
    //osg::ref_ptr< osg::Vec3Array> vDest = new osg::Vec3Array;
    //std::vector< double > scalarArray;
    osg::ref_ptr< osg::Vec3Array > n = new osg::Vec3Array;
    //osg::Vec3Array* colors = new osg::Vec3Array;
    //osg::Vec2Array* tc = new osg::Vec2Array;
    
    //int numCells = polydata->GetNumberOfCells();
    vtkCellArray* strips = m_pd->GetStrips();
    
    VTKPrimitiveSetGeneratorPtr primitiveGenerator = VTKPrimitiveSetGeneratorPtr( new VTKPrimitiveSetGenerator( strips ) );
    setPrimitiveSetGenerator( primitiveGenerator );
    
    //Number of vertex is potentially bigger than number of points,
    //Since same point can appear in different triangle strip.
    
    int numVetex = 0;
    vtkIdType* pts;
    vtkIdType cStripNp;
    int stripNum = 0;
    
    for( strips->InitTraversal();
        ( strips->GetNextCell( cStripNp, pts ) );
        ++stripNum )
    {
        numVetex += cStripNp;
    }
    
    
    vtkDataArray* scalarArray = pointData->GetScalars(m_activeScalar.c_str());
    //double scalarRange[ 2 ] = {0,1.0};
    //scalarArray->GetRange( scalarRange );

    double val;

    osg::ref_ptr< osg::FloatArray > colorArray( new osg::FloatArray );
    //colorArray->resize( dataSize );
    
    /*int am = mylog2( numVetex ) + 1;
    int mm = am / 2;
    int nn = am - am / 2;
    
    // Dimensions of the textures.
    unsigned int s = mypow2( mm );
    unsigned int t = mypow2( nn );
    
    double bounds[6];
    points->GetBounds( bounds );
    //VTK does bounds xmin, xmax,....
    //OSG does bounds xmin, ymin, zmin, xmax, ymax,...
    osg::BoundingBox bb( bounds[0] - 1, bounds[2] - 1, bounds[4] - 1, bounds[1] + 1, bounds[3] + 1, bounds[5] + 1 );
    */
    double x[3];
    double cnormal[3];
    double displacement[3];
    
    {
        //osg::Vec3 destVec;
        //osg::Vec3 ccolor;
        osg::Vec3 startVec;
        osg::Vec3 normal;
        //osg::Vec2 coord;
        
        stripNum = 0;
        
        for( strips->InitTraversal(); ( strips->GetNextCell( cStripNp, pts ) );
            stripNum++ )
        {
            for( int i = 0; i < cStripNp; ++i )
            {
               points->GetPoint( pts[i], x );
                startVec.set( x[0], x[1], x[2] );
                normals->GetTuple( pts[i], cnormal );
                normal.set( cnormal[0], cnormal[1], cnormal[2] );

                v->push_back( startVec );
                n->push_back( normal );
 
                scalarArray->GetTuple( pts[i], &val );
                val = vtkMath::ClampAndNormalizeValue( val, m_scalarRange );
                colorArray->push_back( val );
                //cVal = dataArray->GetTuple1( pts[i] );
                //scalarArray.push_back( cVal );
                //lut->GetColor(cVal,curColor);
                
                //ccolor.set(curColor[0],curColor[1],curColor[2]);
                //colors->push_back( ccolor);
                
                //coord is the cord in the texture for strip x and vertex y in the "scale term" of s and t
                
                /*int xx = ( v->size() - 1 ) % s;
                int yy = ( v->size() - 1 ) / s;
                coord.set( ( ( float )( xx ) / s ), ( ( float )( yy ) / t ) );
                
                tc->push_back( coord );
                
                if( vectorArray )
                {
                    vectorArray->GetTuple( pts[i], displacement );
                }
                else
                {
                    displacement[ 0 ] = 0.;
                    displacement[ 1 ] = 0.;
                    displacement[ 2 ] = 0.;
                }
                destVec.set( x[0] + displacement[0], x[1] + displacement[1], x[2] + displacement[2] );
                vDest->push_back( destVec );*/
            }
        }
    }
    
    ChannelDataOSGArrayPtr cdv( new ChannelDataOSGArray( v.get(), "vertices" ) );
    addInput( cdv );
    ChannelDataOSGArrayPtr cdn( new ChannelDataOSGArray( n.get(), "normals" ) );
    addInput( cdn );
    lfx::core::ChannelDataOSGArrayPtr colorData( new lfx::core::ChannelDataOSGArray( colorArray.get(), "scalar" ) );
    addInput( colorData );
}
            
}
}
}

