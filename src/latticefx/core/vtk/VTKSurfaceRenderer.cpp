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
#include <latticefx/core/vtk/VTKSurfaceRenderer.h>

#include <latticefx/core/vtk/ChannelDatavtkPolyData.h>
#include <latticefx/core/vtk/ChannelDatavtkDataObject.h>
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

//#include <vtkXMLPolyDataWriter.h>

namespace lfx
{

namespace core
{

namespace vtk
{

////////////////////////////////////////////////////////////////////////////////
osg::Node* VTKSurfaceRenderer::getSceneGraph( const lfx::core::ChannelDataPtr maskIn )
{
    lfx::core::ChannelDataPtr channelDataTemp = getInput( "vtkPolyDataMapper" );
    if( !channelDataTemp )
    {
        std::cout << "VTKSurfaceRenderer::getSceneGraph : The vtkPolyDataMapper was not set as an input." << std::endl;
        return 0;
    }
    m_pd =
        boost::static_pointer_cast< lfx::core::vtk::ChannelDatavtkPolyDataMapper >( channelDataTemp )->GetPolyDataMapper()->GetInput();
    /*{
        vtkXMLPolyDataWriter* writer = vtkXMLPolyDataWriter::New();
        writer->SetInput( m_pd );
        writer->SetDataModeToAscii();
        writer->SetFileName( "testpd_surface.vtp" );
        writer->Write();
        writer->Delete();
    }*/

    if( !m_pd )
    {
        std::cout << "VTKSurfaceRenderer::getSceneGraph : The PolyData was not set as an input." << std::endl;
        return 0;
    }

    m_dataObject =
        boost::dynamic_pointer_cast< lfx::core::vtk::ChannelDatavtkDataObject >( getInput( "vtkDataObject" ) );
    if( !m_dataObject )
    {
        std::cout << "No vtkDataObject input was specified to VTKSurfaceRenderer." << std::endl;
        return 0;
    }

    vtkPoints* points = m_pd->GetPoints();
    if( !points )
    {
        std::cout << "VTKSurfaceRenderer::getSceneGraph : There are no points in this surface." << std::endl;
        return 0;
    }

	m_curScalar = m_activeScalar;
    if( !m_colorByScalar.empty() )
    { 
        m_curScalar = m_colorByScalar;
    }

	if( m_refresh )
	{
		m_scalarChannels.clear();
		m_refresh = false;
	}

    if( !m_scalarChannels.empty() )
    {
        //Re-add all of the local inputs since they get removed in this call
        //void DataSet::setInputs( OperationBasePtr opPtr, ChannelDataList& currentData )
        for( std::map< std::string, lfx::core::ChannelDataPtr >::const_iterator iter = m_scalarChannels.begin(); iter != m_scalarChannels.end(); ++iter )
        {
            addInput( iter->second );
        }
        setTransferFunctionInput( m_curScalar );
        return( lfx::core::SurfaceRenderer::getSceneGraph( maskIn ) );
    }

    ExtractVTKPrimitives();

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

    vtkPolyData* pd = reTriangleStripper->GetOutput();

    {
        VTKPrimitiveSetGeneratorPtr primitiveGenerator =
            VTKPrimitiveSetGeneratorPtr( new VTKPrimitiveSetGenerator( pd->GetStrips() ) );
        setPrimitiveSetGenerator( primitiveGenerator );
    }

    //Number of vertex is potentially bigger than number of points,
    //Since the same points can appear in different triangle strip.

    /*int numVetex = 0;
    vtkIdType* pts = 0;
    vtkIdType cStripNp = 0;
    int stripNum = 0;

    for( strips->InitTraversal(); strips->GetNextCell( cStripNp, pts ); ++stripNum )
    {
        numVetex += cStripNp;
    }*/

    //Setup normals and verts
    SetupNormalAndVertexArrays( pd );

    //Setup the scalar arrays
    SetupColorArrays( pd );

    reTriangleStripper->Delete();
}
////////////////////////////////////////////////////////////////////////////////
void VTKSurfaceRenderer::SetupNormalAndVertexArrays( vtkPolyData* pd )
{
    vtkPointData* pointData = pd->GetPointData();
    vtkPoints* points = pd->GetPoints();
    vtkCellArray* strips = pd->GetStrips();
    double x[3];
    double cnormal[3];
    osg::Vec3 startVec;
    osg::Vec3 normal;
    vtkIdType* pts = 0;
    vtkIdType cStripNp = 0;
    int stripNum = 0;
    vtkDataArray* normals = pointData->GetVectors( "Normals" );
    osg::ref_ptr< osg::Vec3Array > v = new osg::Vec3Array;
    osg::ref_ptr< osg::Vec3Array > n = new osg::Vec3Array;
    
    for( strips->InitTraversal(); strips->GetNextCell( cStripNp, pts ); ++stripNum )
    {
        for( vtkIdType i = 0; i < cStripNp; ++i )
        {
            points->GetPoint( pts[i], x );
            startVec.set( x[0], x[1], x[2] );
            normals->GetTuple( pts[i], cnormal );
            normal.set( cnormal[0], cnormal[1], cnormal[2] );
            
            v->push_back( startVec );
            n->push_back( normal );
        }
    }
    ChannelDataOSGArrayPtr cdv( new ChannelDataOSGArray( "vertices", v.get() ) );
    addInput( cdv );
    ChannelDataOSGArrayPtr cdn( new ChannelDataOSGArray( "normals", n.get() ) );
    addInput( cdn );
    m_scalarChannels[ "vertices" ] = cdv;
    m_scalarChannels[ "normals" ] = cdn;

    setInputNameAlias( SurfaceRenderer::VERTEX, "vertices" );
    setInputNameAlias( SurfaceRenderer::NORMAL, "normals" );
}
////////////////////////////////////////////////////////////////////////////////
void VTKSurfaceRenderer::SetupColorArrays( vtkPolyData* pd )
{
    vtkPointData* pointData = pd->GetPointData();
    vtkCellArray* strips = pd->GetStrips();
    size_t numDataArrays = pointData->GetNumberOfArrays();
    double val;
    double scalarRange[ 2 ];
    vtkIdType* pts = 0;
    vtkIdType cStripNp = 0;
    int stripNum = 0;
    bool haveScalarData = false;
    
    for( size_t i = 0; i < numDataArrays; ++i )
    {
        vtkDataArray* scalarArray = pointData->GetArray( i );
        if( scalarArray->GetNumberOfComponents() == 1 )
        {
            osg::ref_ptr< osg::FloatArray > colorArray( new osg::FloatArray );
            const std::string arrayName = scalarArray->GetName();
            m_dataObject->GetScalarRange( arrayName, scalarRange );
            for( strips->InitTraversal(); strips->GetNextCell( cStripNp, pts ); ++stripNum )
            {
                for( vtkIdType j = 0; j < cStripNp; ++j )
                {
                    scalarArray->GetTuple( pts[j], &val );
                    val = vtkMath::ClampAndNormalizeValue( val, scalarRange );
                    colorArray->push_back( val );
                }
            }
            lfx::core::ChannelDataOSGArrayPtr colorData( new lfx::core::ChannelDataOSGArray( arrayName, colorArray.get() ) );
            addInput( colorData );
            m_scalarChannels[ arrayName ] = colorData;
            haveScalarData = true;
        }
    }

    if( haveScalarData )
    {
        // Configure transfer function.
        setTransferFunctionInput( m_curScalar );
        setTransferFunction( lfx::core::loadImageFromDat( "01.dat" ) );
        setTransferFunctionDestination( lfx::core::Renderer::TF_RGBA );
    }
	else
	{
		setTransferFunctionDestination( TF_DISABLE );
		setTransferFunction( NULL ); // disable the transfer function.
	}
}

////////////////////////////////////////////////////////////////////////////////
void VTKSurfaceRenderer::serializeData( JsonSerializer *json ) const
{
	// let the parent write its data
	SurfaceRenderer::serializeData( json );

	json->insertObj( VTKSurfaceRenderer::getClassName(), true);
	json->insertObjValue( "activeVector", m_activeVector ); 
	json->insertObjValue( "activeScalar", m_activeScalar );
	json->insertObjValue( "colorByScalar", m_colorByScalar );
	json->insertObjValue( "curScalar", m_curScalar );
    //vtkPolyData* m_pd;
    //std::map< std::string, lfx::core::ChannelDataPtr > m_scalarChannels;
    //lfx::core::vtk::ChannelDatavtkDataObjectPtr m_dataObject;
	json->popParent();
}

////////////////////////////////////////////////////////////////////////////////
bool VTKSurfaceRenderer::loadData( JsonSerializer *json, IObjFactory *pfactory, std::string *perr )
{
	// let the parent load its data
	if ( !SurfaceRenderer::loadData( json, pfactory, perr )) return false;

	// get to this classes data
	if ( !json->getObj( VTKSurfaceRenderer::getClassName() ) )
	{
		if (perr) *perr = "Json: Failed to get VTKSurfaceRenderer data";
		return false;
	}

	json->getValue( "activeVector", &m_activeVector);
	json->getValue( "activeScalar", &m_activeScalar );
	json->getValue( "colorByScalar", &m_colorByScalar );
	json->getValue( "curScalar", &m_curScalar );

	json->popParent();
	return true;
}

////////////////////////////////////////////////////////////////////////////////
void VTKSurfaceRenderer::dumpState( std::ostream &os )
{
	SurfaceRenderer::dumpState( os );

	dumpStateStart( VTKSurfaceRenderer::getClassName(), os );
	os << "_activeVector: " << m_activeVector << std::endl;
	os << "_activeScalar: " << m_activeScalar << std::endl;
	os << "_colorByScalar: " << m_colorByScalar << std::endl;
	os << "_curScalar: " << m_curScalar << std::endl;
	dumpStateEnd( VTKSurfaceRenderer::getClassName(), os );
}

////////////////////////////////////////////////////////////////////////////////
}
}
}

