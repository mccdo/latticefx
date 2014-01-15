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
#include <latticefx/core/vtk/VTKStreamlineRenderer.h>

#include <latticefx/core/vtk/ChannelDatavtkPolyData.h>
#include <latticefx/core/vtk/ChannelDatavtkDataObject.h>
#include <latticefx/core/vtk/ChannelDatavtkPolyDataMapper.h>

#include <latticefx/core/ChannelDataOSGArray.h>
#include <latticefx/core/TransferFunctionUtils.h>
#include <latticefx/core/LogMacros.h>

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
osg::Node* VTKStreamlineRenderer::getSceneGraph( const lfx::core::ChannelDataPtr maskIn )
{
    vtkPolyData* tempVtkPD = boost::static_pointer_cast< lfx::core::vtk::ChannelDatavtkPolyData >( getInput( "vtkPolyData" ) )->GetPolyData();
    m_dataObject = boost::dynamic_pointer_cast< lfx::core::vtk::ChannelDatavtkDataObject >( getInput( "vtkDataObject" ) );

    if( !m_dataObject )
    {
		LFX_WARNING( "VTKStreamlineRenderer::getSceneGraph(): No vtkDataObject input was specified." );
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
        return( lfx::core::StreamlineRenderer::getSceneGraph( maskIn ) );
    }

    ExtractVTKPrimitives(tempVtkPD);

#if WRITE_IMAGE_DATA
    //osgDB::writeNodeFile( *(tempGeode.get()), "gpu_vector_field.ive" );
#endif

    return( lfx::core::StreamlineRenderer::getSceneGraph( maskIn ) );
}
////////////////////////////////////////////////////////////////////////////////
void VTKStreamlineRenderer::ExtractVTKPrimitives( vtkPolyData *polydata )
{
	std::vector< std::deque< Point > > streamlineList;
	ProcessStreamLines( polydata,  streamlineList );
	CreateStreamLines( polydata, streamlineList );
	SetupColorArrays( polydata, streamlineList );

	setNumTraces( streamlineList.size() );

	float perLen = ( ( 1. / (float)getNumTraces() ) * 0.2f );
	setTraceLengthPercent(perLen);
	setImageScale(.2f);
}

////////////////////////////////////////////////////////////////////////////////
void VTKStreamlineRenderer::ProcessStreamLines( vtkPolyData* polydata,  std::vector< std::deque< Point > > &streamlineList)
{
	/*
	std::stringstream ss;
	ss << "tNumber of Cells :: (" << brickNum << "), " << "depth: " << _depth << ", " << m_bricksDone << " of " << totbricks << ", cached: " << brickCached;
	LFX_INFO_STATIC( logname, ss.str() );

    vprDEBUG( vesDBG, 1 )
            << "|\tNumber of Cells : " << polydata->GetNumberOfCells()
            << std::endl << vprDEBUG_FLUSH;
    vprDEBUG( vesDBG, 1 )
            << "|\tNumber of Lines : " << polydata->GetNumberOfLines()
            << std::endl << vprDEBUG_FLUSH;
    vprDEBUG( vesDBG, 1 )
            << "|\tNumber of Points : " << polydata->GetNumberOfPoints()
            << std::endl << vprDEBUG_FLUSH;
			*/

    int numberOfStreamLines = polydata->GetNumberOfLines();
    if( numberOfStreamLines == 0 )
    {
        LFX_ERROR( "VTKStreamlineRenderer::ProcessStreamLines : Number of streamlines is 0 " );
        return;
    }

    std::vector< vtkIdList* > streamlineCells;
    for( vtkIdType cellId = 0; cellId < numberOfStreamLines; ++cellId )
    {
        vtkIdList* origVertList = polydata->GetCell( cellId )->GetPointIds();
        streamlineCells.push_back( vtkIdList::New() );
        streamlineCells.back()->DeepCopy( origVertList );
        vtkIdList* tempVertList = streamlineCells.back();
        //bool backwards = IsStreamlineBackwards( cellId, polydata );
        //std::cout << "Is a backwards streamline " << backwards << std::endl;
        //for( size_t i = 0; i < tempVertList->GetNumberOfIds(); ++i )
        //{
        //    std::cout << " " << tempVertList->GetId( i );
        //}
        //std::cout << std::endl;

        bool foundMatch = false;
        vtkIdList* oldVertList = 0;
        vtkIdList* newComboVertList = vtkIdList::New();
        for( size_t i = 0; i < streamlineCells.size() - 1; ++ i )
        {
            oldVertList = streamlineCells.at( i );
            vtkIdType matchId = oldVertList->IsId( tempVertList->GetId( 0 ) );
            if( matchId > -1 )
            {
                //std::cout << "orig ";
                //for( size_t j = 0; j < oldVertList->GetNumberOfIds(); ++j )
                //{
                //std::cout << " " << oldVertList->GetId( j );
                //}
                //std::cout << std::endl;
                foundMatch = true;
                vtkIdType numVerts = tempVertList->GetNumberOfIds();
                for( vtkIdType j = numVerts - 1; j >= 0; --j )
                {
                    //std::cout << " " << tempVertList->GetId( j );
                    //oldVertList->InsertId( 0, tempVertList->GetId( j ) );
                    newComboVertList->InsertNextId( tempVertList->GetId( j ) );
                }
                //std::cout << std::endl;
                for( vtkIdType j = 1; j < oldVertList->GetNumberOfIds(); ++j )
                {
                    //std::cout << " " << oldVertList->GetId( j );
                    newComboVertList->InsertNextId( oldVertList->GetId( j ) );
                }
                //std::cout << std::endl;

                //std::cout << "Streamline " <<  m_streamlineCells.size() - 1
                //    << " has a point in line " << i << " at index "
                //    << matchId << std::endl;
                break;
            }
        }

        if( foundMatch )
        {
            //std::cout << "Is a backwards streamline " << backwards << std::endl;
            //for( size_t i = 0; i < newComboVertList->GetNumberOfIds(); ++i )
            //{
            //    std::cout << " " << newComboVertList->GetId( i );
            //}
            //std::cout << std::endl;

            oldVertList->DeepCopy( newComboVertList );
            tempVertList->Delete();
            newComboVertList->Delete();
            streamlineCells.pop_back();
            continue;
        }

        newComboVertList->Delete();

        //If it is a standalone backwards line then reorder the points
        if( IsStreamlineBackwards( cellId, polydata ) )
        {
            vtkIdType numVerts = origVertList->GetNumberOfIds();
            for( vtkIdType i = 0; i < numVerts; ++i )
            {
                tempVertList->SetId( i, origVertList->GetId( numVerts - 1 - i ) );
            }
        }
    }
    //std::cout << "number of combined lines " << m_streamlineCells.size() << std::endl;

    double* x = 0;
    vtkIdList* tempVertList = 0;
    vtkPoints* points = polydata->GetPoints();
    for( size_t i = 0; i < streamlineCells.size(); ++i )
    {
        std::deque< Point > tempQueue;

        //cellId = m_streamlines.at( i ).first;
        // For forward integrated points
        //if( cellId != -1 )
        {
            tempVertList = streamlineCells.at( i );
            vtkIdType numVerts = tempVertList->GetNumberOfIds();
            //vprDEBUG( vesDBG, 1 )
              //      << "|\t\tNumber of Forward points = " << numVerts
                //    << std::endl << vprDEBUG_FLUSH;
            for( vtkIdType j = 0; j < numVerts; ++j )
            {
                x = points->GetPoint( tempVertList->GetId( j ) );
                //vprDEBUG( vesDBG, 3 )
                  //      << "|\t\tx[ " << j << " ] = " << x[ 0 ] << " : "
                    //    << x[ 1 ] << " : " << x[ 2 ] << std::endl << vprDEBUG_FLUSH;
                Point tempPoint;
                tempPoint.p[ 0 ] = x[ 0 ];
                tempPoint.p[ 1 ] = x[ 1 ];
                tempPoint.p[ 2 ] = x[ 2 ];
                tempPoint.vertId = tempVertList->GetId( j );
                tempQueue.push_back( tempPoint );
            }
        }
        streamlineList.push_back( tempQueue );
    }

    for( size_t i = 0; i < streamlineCells.size(); ++i )
    {
        streamlineCells.at( i )->Delete();
    }
    streamlineCells.clear();
    /*vprDEBUG( vesDBG, 1 ) << "|\t\tmaxNpts = "
     << m_maxNPts << std::endl << vprDEBUG_FLUSH;
     vprDEBUG( vesDBG, 1 ) << "|\t\tminNpts = "
     << minNpts << std::endl << vprDEBUG_FLUSH;*/
}

////////////////////////////////////////////////////////////////////////////////
bool VTKStreamlineRenderer::IsStreamlineBackwards( vtkIdType cellId, vtkPolyData* polydata )
{
    double x2[ 3 ];
    double x1[ 3 ];
    vtkPoints* points = polydata->GetCell( cellId )->GetPoints();
    points->GetPoint( 0, x1 );
    vtkIdType globalPointId1 = polydata->FindPoint( x1 );
    points->GetPoint( 1, x2 );

    //Create a vector along the streamline from point 0 to point 1
    double xComp = x2[ 0 ] - x1[ 0 ];
    double yComp = x2[ 1 ] - x1[ 1 ];
    double zComp = x2[ 2 ] - x1[ 2 ];

    polydata->GetPointData()->GetVectors( m_activeVector.c_str() )->GetTuple( globalPointId1, x1 );
    //GetVectors( GetActiveDataSet()->GetActiveVectorName().c_str() )->
    //GetTuple( globalPointId1, x1 );

    bool isBackwards = true;
    if( ( ( x1[ 0 ] * xComp ) >= 0 ) && ( ( x1[ 1 ] * yComp ) >= 0 ) && ( ( x1[ 2 ] * zComp ) >= 0 ) )
    {
        isBackwards = false;
    }

    return isBackwards;
}

////////////////////////////////////////////////////////////////////////////////
void VTKStreamlineRenderer::CreateStreamLines( vtkPolyData *polyData, const std::vector< std::deque< Point > > &streamlineList)
{
	osg::ref_ptr< osg::Vec3Array > posArray( new osg::Vec3Array );

    vtkPointData* pointData = polyData->GetPointData();

    size_t numStreamLines = streamlineList.size();
    for( size_t i = 0; i < numStreamLines; ++i )
    {
        std::deque< Point > tempLine = streamlineList.at( i );
		for( size_t p=0; p < tempLine.size(); p++ )
		{
			Point pt = tempLine[p];
			osg::Vec3f v;
			v.set( pt.p[0], pt.p[1], pt.p[2] );
			posArray->push_back( v );
        }
    }

	std::stringstream ss;
	ss << "StreamLine Count: " << numStreamLines << " Total Point Count: " << posArray->size();
	LFX_TRACE( ss.str().c_str() ); 

	ChannelDataOSGArrayPtr cdv( new ChannelDataOSGArray( getInputNameAlias( StreamlineRenderer::POSITION ), posArray.get() ) );
	addInput( cdv );
}

////////////////////////////////////////////////////////////////////////////////
void VTKStreamlineRenderer::SetupColorArrays( vtkPolyData* pd, const std::vector< std::deque< Point > > &streamlineList )
{
    vtkPointData* pointData = pd->GetPointData();
    size_t numDataArrays = pointData->GetNumberOfArrays();
    double val;
    double scalarRange[ 2 ];
    bool haveScalarData = false;
	int count = 0;
    
    for( size_t i = 0; i < numDataArrays; ++i )
    {
        vtkDataArray* scalarArray = pointData->GetArray( i );
        if( scalarArray->GetNumberOfComponents() != 1 ) continue;

		osg::ref_ptr< osg::FloatArray > colorArray( new osg::FloatArray );
        const std::string arrayName = scalarArray->GetName();
        m_dataObject->GetScalarRange( arrayName, scalarRange );

		
		size_t numStreamLines = streamlineList.size();
		for( size_t i = 0; i < numStreamLines; ++i )
		{
			std::deque< Point > tempLine = streamlineList.at( i );
			for( size_t p=0; p < tempLine.size(); p++ )
			{
				Point pt = tempLine[p];
				scalarArray->GetTuple( pt.vertId, &val ); 
				val = vtkMath::ClampAndNormalizeValue( val, scalarRange );
				colorArray->push_back( val );
			}
		}

		std::stringstream ss;
		ss << "SetupColorArrays ColorArray: " << arrayName << " count: " << colorArray->size();
		LFX_TRACE( ss.str().c_str() ); 

        lfx::core::ChannelDataOSGArrayPtr colorData( new lfx::core::ChannelDataOSGArray( arrayName, colorArray.get() ) );
        addInput( colorData );                       
        m_scalarChannels[ arrayName ] = colorData;                                
		haveScalarData = true;
    }

    if( haveScalarData )
    {
        // Configure transfer function.
        setTransferFunctionInput( m_curScalar );
        setTransferFunction( lfx::core::loadImageFromDat( "01.dat" ) );
        setTransferFunctionDestination( lfx::core::Renderer::TF_RGBA );
    }
}

////////////////////////////////////////////////////////////////////////////////
void VTKStreamlineRenderer::serializeData( JsonSerializer *json ) const
{
	// let the parent write its data
	StreamlineRenderer::serializeData( json );

	// TOOD: finish this
	json->insertObj( VTKStreamlineRenderer::getClassName(), true);
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
bool VTKStreamlineRenderer::loadData( JsonSerializer *json, IObjFactory *pfactory, std::string *perr )
{
	// let the parent load its data
	if ( !StreamlineRenderer::loadData( json, pfactory, perr )) return false;

	// get to this classes data
	if ( !json->getObj( VTKStreamlineRenderer::getClassName() ) )
	{
		if (perr) *perr = "Json: Failed to get VTKStreamlineRenderer data";
		return false;
	}

	// TOOD: finish this
	json->getValue( "activeVector", &m_activeVector);
	json->getValue( "activeScalar", &m_activeScalar );
	json->getValue( "colorByScalar", &m_colorByScalar );
	json->getValue( "curScalar", &m_curScalar );

	json->popParent();
	return true;
}

////////////////////////////////////////////////////////////////////////////////
void VTKStreamlineRenderer::dumpState( std::ostream &os )
{
	StreamlineRenderer::dumpState( os );

	// TOOD: finish this
	dumpStateStart( VTKStreamlineRenderer::getClassName(), os );
	os << "_activeVector: " << m_activeVector << std::endl;
	os << "_activeScalar: " << m_activeScalar << std::endl;
	os << "_colorByScalar: " << m_colorByScalar << std::endl;
	os << "_curScalar: " << m_curScalar << std::endl;
	dumpStateEnd( VTKStreamlineRenderer::getClassName(), os );
}

////////////////////////////////////////////////////////////////////////////////
}
}
}

// Don't need normals and vertexs
/*
////////////////////////////////////////////////////////////////////////////////
void VTKStreamlineRenderer::SetupNormalAndVertexArrays( vtkPolyData* pd )
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
*/

/*
////////////////////////////////////////////////////////////////////////////////
void OSGStreamlineStage::ProcessStreamLines( vtkPolyData* polydata )
{
    vprDEBUG( vesDBG, 1 )
            << "|\tNumber of Cells : " << polydata->GetNumberOfCells()
            << std::endl << vprDEBUG_FLUSH;
    vprDEBUG( vesDBG, 1 )
            << "|\tNumber of Lines : " << polydata->GetNumberOfLines()
            << std::endl << vprDEBUG_FLUSH;
    vprDEBUG( vesDBG, 1 )
            << "|\tNumber of Points : " << polydata->GetNumberOfPoints()
            << std::endl << vprDEBUG_FLUSH;

    int numberOfStreamLines = polydata->GetNumberOfLines();
    if( numberOfStreamLines == 0 )
    {
        std::cout << "|\tVTKStreamlineTextureCreator::ProcessStreamLines : Number of streamlines is 0 " << std::endl;
        return;
    }

    std::vector< vtkIdList* > streamlineCells;
    for( vtkIdType cellId = 0; cellId < numberOfStreamLines; ++cellId )
    {
        vtkIdList* origVertList = polydata->GetCell( cellId )->GetPointIds();
        streamlineCells.push_back( vtkIdList::New() );
        streamlineCells.back()->DeepCopy( origVertList );
        vtkIdList* tempVertList = streamlineCells.back();
        //bool backwards = IsStreamlineBackwards( cellId, polydata );
        //std::cout << "Is a backwards streamline " << backwards << std::endl;
        //for( size_t i = 0; i < tempVertList->GetNumberOfIds(); ++i )
        //{
        //    std::cout << " " << tempVertList->GetId( i );
        //}
        //std::cout << std::endl;

        bool foundMatch = false;
        vtkIdList* oldVertList = 0;
        vtkIdList* newComboVertList = vtkIdList::New();
        for( size_t i = 0; i < streamlineCells.size() - 1; ++ i )
        {
            oldVertList = streamlineCells.at( i );
            vtkIdType matchId = oldVertList->IsId( tempVertList->GetId( 0 ) );
            if( matchId > -1 )
            {
                //std::cout << "orig ";
                //for( size_t j = 0; j < oldVertList->GetNumberOfIds(); ++j )
                //{
                //std::cout << " " << oldVertList->GetId( j );
                //}
                //std::cout << std::endl;
                foundMatch = true;
                vtkIdType numVerts = tempVertList->GetNumberOfIds();
                for( vtkIdType j = numVerts - 1; j >= 0; --j )
                {
                    //std::cout << " " << tempVertList->GetId( j );
                    //oldVertList->InsertId( 0, tempVertList->GetId( j ) );
                    newComboVertList->InsertNextId( tempVertList->GetId( j ) );
                }
                //std::cout << std::endl;
                for( vtkIdType j = 1; j < oldVertList->GetNumberOfIds(); ++j )
                {
                    //std::cout << " " << oldVertList->GetId( j );
                    newComboVertList->InsertNextId( oldVertList->GetId( j ) );
                }
                //std::cout << std::endl;

                //std::cout << "Streamline " <<  m_streamlineCells.size() - 1
                //    << " has a point in line " << i << " at index "
                //    << matchId << std::endl;
                break;
            }
        }

        if( foundMatch )
        {
            //std::cout << "Is a backwards streamline " << backwards << std::endl;
            //for( size_t i = 0; i < newComboVertList->GetNumberOfIds(); ++i )
            //{
            //    std::cout << " " << newComboVertList->GetId( i );
            //}
            //std::cout << std::endl;

            oldVertList->DeepCopy( newComboVertList );
            tempVertList->Delete();
            newComboVertList->Delete();
            streamlineCells.pop_back();
            continue;
        }

        newComboVertList->Delete();

        //If it is a standalone backwards line then reorder the points
        if( IsStreamlineBackwards( cellId, polydata ) )
        {
            vtkIdType numVerts = origVertList->GetNumberOfIds();
            for( vtkIdType i = 0; i < numVerts; ++i )
            {
                tempVertList->SetId( i, origVertList->GetId( numVerts - 1 - i ) );
            }
        }
    }
    //std::cout << "number of combined lines " << m_streamlineCells.size() << std::endl;

    double* x = 0;
    vtkIdList* tempVertList = 0;
    vtkPoints* points = polydata->GetPoints();
    for( size_t i = 0; i < streamlineCells.size(); ++i )
    {
        std::deque< ves::xplorer::scenegraph::VTKStreamlineTextureCreator::Point > tempQueue;

        //cellId = m_streamlines.at( i ).first;
        // For forward integrated points
        //if( cellId != -1 )
        {
            tempVertList = streamlineCells.at( i );
            vtkIdType numVerts = tempVertList->GetNumberOfIds();
            vprDEBUG( vesDBG, 1 )
                    << "|\t\tNumber of Forward points = " << numVerts
                    << std::endl << vprDEBUG_FLUSH;
            for( vtkIdType j = 0; j < numVerts; ++j )
            {
                x = points->GetPoint( tempVertList->GetId( j ) );
                vprDEBUG( vesDBG, 3 )
                        << "|\t\tx[ " << j << " ] = " << x[ 0 ] << " : "
                        << x[ 1 ] << " : " << x[ 2 ] << std::endl << vprDEBUG_FLUSH;
                ves::xplorer::scenegraph::VTKStreamlineTextureCreator::Point tempPoint;
                tempPoint.x[ 0 ] = x[ 0 ];
                tempPoint.x[ 1 ] = x[ 1 ];
                tempPoint.x[ 2 ] = x[ 2 ];
                tempPoint.vertId = tempVertList->GetId( j );
                tempQueue.push_back( tempPoint );
            }
        }
        m_streamlineList.push_back( tempQueue );
    }

    for( size_t i = 0; i < streamlineCells.size(); ++i )
    {
        streamlineCells.at( i )->Delete();
    }
    streamlineCells.clear();

    vprDEBUG( vesDBG, 1 ) << "|\tExiting cfdStreamers Update " << std::endl << vprDEBUG_FLUSH;
}

////////////////////////////////////////////////////////////////////////////////
void OSGStreamlineStage::createStreamLines( vtkPolyData* polyData,
        ves::xplorer::scenegraph::Geode* geode, int mult, const char* scalarName )
{
    vtkPointData* pointData = polyData->GetPointData();
    vtkDataArray* scalarArray = pointData->GetScalars( scalarName );

    double x[3];
    //We must set these to 0 becuase we do not want to doubly offset
    //our starting location of the vertecies
    x[ 0 ] = 0.;
    x[ 1 ] = 0.;
    x[ 2 ] = 0.;
    osg::Vec3 loc( x[0], x[1], x[2] );

    size_t numStreamLines = m_streamlineList.size();
    for( size_t i = 0; i < numStreamLines; ++i )
    {
        std::deque< ves::xplorer::scenegraph::VTKStreamlineTextureCreator::Point > tempLine = m_streamlineList.at( i );

        osg::ref_ptr< ves::xplorer::scenegraph::VTKStreamlineTextureCreator > rawVTKData =
            new ves::xplorer::scenegraph::VTKStreamlineTextureCreator();
        rawVTKData->SetPolyData( polyData );
        rawVTKData->SetActiveVectorAndScalar( m_activeVector, scalarName );
        rawVTKData->SetPointQueue( tempLine );
        rawVTKData->SetPointMultiplier( mult );
        rawVTKData->loadData();

        osg::Geometry* geom = new osg::Geometry();
        // Note:
        // Display Lists and draw instanced are mutually exclusive. Disable
        // display lists and use buffer objects instead.
        geom->setUseDisplayList( false );
        geom->setUseVertexBufferObjects( true );
        int totalNumberOfPoints = rawVTKData->getDataCount();
        createSLPoint( *geom, totalNumberOfPoints, loc, osg::Vec4( .5, 1., .6, 1. ) );
        geode->addDrawable( geom );

        // Note:
        // OSG has no idea where our vertex shader will render the points. For proper culling
        // and near/far computation, set an approximate initial bounding box.
        geom->setInitialBound( rawVTKData->getBoundingBox() );

        osg::StateSet* ss = geom->getOrCreateStateSet();

        {
            osg::ref_ptr< osg::Uniform > particlesizeUniform =
                new osg::Uniform( "particleSize", m_particleDiameter );
            ss->addUniform( particlesizeUniform.get() );
        }

        //osg::ref_ptr< osg::Shader > vertexShader = osg::Shader::readShaderFile(
        //    osg::Shader::VERTEX, osgDB::findDataFile( "streamline.vs" ) );

        //Apply the shader code here instead of calling it from a file as above
        std::string vertexSource =

            //Setup the color control textures
            "uniform vec2 scalarMinMax;\n"
            "uniform sampler1D texCS; \n"
            //streamline vars
            "uniform vec3 sizes; \n"
            "uniform sampler3D texPos; \n"
            "uniform sampler3D scalar; \n"

            "uniform float osg_SimulationTime; \n"
            "uniform float totalInstances; \n"
            "uniform float fadeTime; \n"
            "uniform float repeatTime; \n"
            "uniform float particleSize; \n"

            // Based on the global 'sizes' uniform that contains the 3D stp texture dimensions,
            // and the input parameter current instances, generate an stp texture coord that
            // indexes into a texture to obtain data for this instance.
            "vec3 \n"
            "generateTexCoord( in float fiid ) \n"
            "{ \n"
            "    float p1 = fiid / (sizes.x*sizes.y); \n"
            "    float t1 = fract( p1 ) * sizes.y; \n"

            "    vec3 tC; \n"
            "    tC.s = fract( t1 ); \n"
            "    tC.t = floor( t1 ) / sizes.y; \n"
            "    tC.p = floor( p1 ) / sizes.z; \n"

            "    return( tC ); \n"
            "} \n"
            //Main
            "void main() \n"
            "{ \n"
            // Using the instance ID, generate "texture coords" for this instance.
            "   float fInstance = gl_InstanceID; \n"
            "   vec3 tC = generateTexCoord( fInstance ); \n"

            // Get position from the texture.
            "   vec4 pos = texture3D( texPos, tC ); \n"
            //Set to 0 to have proper addition with gl_Vertex
            "   pos.w = 0.; \n"
            "   vec4 newPos = vec4( gl_Vertex.xyz + pos.xyz, 1.0 );\n"
            "   vec4 v = gl_ModelViewMatrix * newPos; \n"
            "   gl_Position = gl_ProjectionMatrix * v; \n"
            //" gl_Position = gl_ModelViewProjectionMatrix * ( gl_Vertex + pos ); \n"

            // TBD. Need to make this configurable from a uniform.
            "   gl_PointSize = (-50. / v.z) * particleSize; \n"

            // Compute a time offset from the InstanceID to
            // emulate motion.
            "   float timeOffset = ( fInstance / (totalInstances - 1) ) * repeatTime; \n"
            "   float repTimer = mod( ( osg_SimulationTime - timeOffset ), repeatTime ); \n"
            "   float alpha = (fadeTime - min( repTimer, fadeTime ))/fadeTime; \n"

            "   // Scalar texture containg key to color table. \n"
            "   vec4 activeScalar = texture3D( scalar, tC );\n"
            "   float normScalarVal = 0.;\n"
            "   normScalarVal = (activeScalar.a - scalarMinMax.x) / (scalarMinMax.y - scalarMinMax.x);\n"

            "   if( normScalarVal < 0. )\n"
            "   {\n"
            "       normScalarVal = 0.;\n"
            "   }\n"
            "   if( normScalarVal > 1. )\n"
            "   {\n"
            "       normScalarVal = 1.;\n"
            "   }\n"
            "   vec4 color = texture1D( texCS, normScalarVal );\n"
            "   color[3]=alpha; \n"
            "   gl_FrontColor = color; \n"
            "} \n";

        ///New shader for animation control

        std::string vertexSource2 =
            //Setup the color control textures
            "uniform vec2 scalarMinMax;\n"
            "uniform sampler1D texCS; \n"
            "uniform sampler3D scalar; \n"
            //Other uniforms
            "uniform vec3 sizes;\n"
            "uniform float totalInstances;\n"
            "uniform sampler3D texPos;\n"
            "\n"
            "uniform float osg_SimulationTime;\n"
            "\n"
            "\n"
            "// Total traces to draw on this streamline.\n"
            "// E.g., 1, 2, 3, etc.\n"
            "uniform int numTraces;\n"
            "\n"
            "// Time interval between traces. Total time for\n"
            "// a trace to run the length of all samples points:\n"
            "//     ( traceInterval * numTraces ) / osg_SimulationTime\n"
            "uniform float traceInterval;\n"
            "\n"
            "// Trace length, in number of sample points. Alpha fades\n"
            "// linearly over the traceLength.\n"
            "uniform int traceLength;\n"
            "\n"
            // Based on the global 'sizes' uniform that contains the 3D stp texture dimensions,
            // and the input parameter current instances, generate an stp texture coord that
            // indexes into a texture to obtain data for this instance.
            "vec3 \n"
            "generateTexCoord( in float fiid ) \n"
            "{ \n"
            "    float p1 = fiid / (sizes.x*sizes.y); \n"
            "    float t1 = fract( p1 ) * sizes.y; \n"

            "    vec3 tC; \n"
            "    tC.s = fract( t1 ); \n"
            "    tC.t = floor( t1 ) / sizes.y; \n"
            "    tC.p = floor( p1 ) / sizes.z; \n"

            "    return( tC ); \n"
            "} \n"
            "\n"
            "void main()\n"
            "{\n"
            //"    // Using the instance ID, generate \"texture coords\" for this instance.\n"
            //"    float fInstanceID = gl_InstanceID;\n"
            //"    float r = fInstanceID / sizes.x;\n"
            //"    vec2 tC;\n"
            //"    tC.s = fract( r ); tC.t = floor( r ) / sizes.y;\n"
            // Using the instance ID, generate "texture coords" for this instance.
            "    float fInstanceID = gl_InstanceID; \n"
            "    vec3 tC = generateTexCoord( fInstanceID ); \n"
            "    \n"
            "    // Get position from the texture.\n"
            "    vec4 pos = texture3D( texPos, tC );\n"
            "    pos.w = 0.; // w is 1.0 after lookup; do NOT add 1.0 to gl_Vertex.w\n"
            "    vec4 v = gl_ModelViewMatrix * ( gl_Vertex + pos );\n"
            "    gl_Position = gl_ProjectionMatrix * v;\n"
            "    \n"
            "    // TBD. Need to make this configurable from a uniform.\n"
            "    gl_PointSize = -2000. / v.z;\n"
            "    \n"
            "    \n"
            "    // Compute the length of a trace segment, in points.\n"
            "    float segLength = totalInstances / numTraces;\n"
            "    // Use current time to compute an offset in points for the animation.\n"
            "    float time = mod( osg_SimulationTime, traceInterval );\n"
            "    float pointOffset = ( time / traceInterval ) * segLength;\n"
            "    \n"
            "    // Find the segment tail for this point's relavant segment.\n"
            "    float segTail = floor( (fInstanceID - pointOffset) / segLength ) * segLength + pointOffset;\n"
            "    // ...and the head, which will have full intensity alpha.\n"
            "    float segHead = floor( segTail + segLength );\n"
            "    \n"
            "#if 1\n"
            "    // Use smoothstep to fade from the head to the traceLength.\n"
            "    float alpha = smoothstep( segHead-traceLength, segHead, fInstanceID );\n"
            "#else\n"
            "    // Alternative: Use step() instead for no fade.\n"
            "    float alpha = step( segHead-traceLength, fInstanceID );\n"
            "#endif\n"
            "    \n"
            //"    vec4 color = gl_Color;\n"
            //"    color.a *= alpha;\n"
            //"    gl_FrontColor = color;\n"
            "    // Scalar texture containg key to color table. \n"
            "    vec4 activeScalar = texture3D( scalar, tC );\n"
            "    float normScalarVal = 0.;\n"
            "    normScalarVal = (activeScalar.a - scalarMinMax.x) / (scalarMinMax.y - scalarMinMax.x);\n"

            "    if( normScalarVal < 0. )\n"
            "    {\n"
            "        normScalarVal = 0.;\n"
            "    }\n"
            "    if( normScalarVal > 1. )\n"
            "    {\n"
            "        normScalarVal = 1.;\n"
            "    }\n"
            "    vec4 color = texture1D( texCS, normScalarVal );\n"
            "    color[3]=alpha; \n"
            "    gl_FrontColor = color; \n"
            "}\n";

        osg::ref_ptr< osg::Program > program = new osg::Program();
        {
            osg::ref_ptr< osg::Shader > vertexShader = new osg::Shader();
            vertexShader->setType( osg::Shader::VERTEX );
            vertexShader->setShaderSource( vertexSource );

            program->addShader( vertexShader.get() );
        }

        {
            ///The uniform tex is bound up when the instances are created for
            ///the splotch texture
            osg::ref_ptr< osg::Program > program =
                ves::xplorer::scenegraph::SceneManager::instance()->
                GetNullGlowTextureProgram();

            ss->setAttributeAndModes( program.get(),
                                      osg::StateAttribute::ON | osg::StateAttribute::PROTECTED );
        }

        // Note:
        // We will render the streamline points with depth test on and depth write disabled,
        // with an order independent blend. This means we need to draw the streamlines last
        // (so use bin # 10) but we don't need the depth sort, so use bin name "RenderBin".
        ss->setRenderBinDetails( 10, "RenderBin" );

        // Note:
        // When using a vertex shader, point size is taken from glPointSize and _not_
        // distance-attenuated. However, set the following mode ON, and then our vertex
        // shader can do its own distance attenuation and emit gl_PointSize.
        ss->setMode( GL_VERTEX_PROGRAM_POINT_SIZE, osg::StateAttribute::ON );

        {
            // Pass the 3D texture dimensions to the shader as a "sizes" uniform.
            // Required to compute correct texture coordinates from the instance ID.
            osg::Vec3s ts( rawVTKData->getTextureSizes() );
            osg::ref_ptr< osg::Uniform > sizesUniform =
                new osg::Uniform( "sizes", osg::Vec3( float( ts.x() ), float( ts.y() ), float( ts.z() ) ) );
            ss->addUniform( sizesUniform.get() );
        }

        {
            // Tell the shader the total number of instances: tm * tn.
            // Required for animation based on the instance ID.
            osg::Vec3s ts( rawVTKData->getTextureSizes() );
            osg::ref_ptr< osg::Uniform > totalInstancesUniform =
                new osg::Uniform( "totalInstances", float( ts.x() * ts.y() * ts.z() ) );
            ss->addUniform( totalInstancesUniform.get() );
        }

        {
            // Specify the time in seconds for a given streamline point to fade
            // from full intensity to zero intensity.
            // (May be altered with simulation time.)
            osg::ref_ptr< osg::Uniform > fadeTimeUniform =
                new osg::Uniform( "fadeTime", 1.f );
            ss->addUniform( fadeTimeUniform.get() );
        }

        {
            // Specify the time in seconds for the animation to loop.
            // (May be altered with simulation time.)
            osg::ref_ptr< osg::Uniform > repeatTimeUniform =
                new osg::Uniform( "repeatTime", 3.f );
            ss->addUniform( repeatTimeUniform.get() );
        }

        // Note:
        // It turns out that SRC_ALPHA, ONE_MINUS_SRC_ALPHA actually is
        // non-saturating. Give it a color just shy of full intensity white,
        // and the result will never saturate to white no matter how many
        // times it is overdrawn.
        osg::ref_ptr< osg::BlendFunc > bf = new osg::BlendFunc(
            GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        ss->setAttributeAndModes( bf.get() );

        // Note:
        // Leave the depth test enabled, but mask off depth writes (4th param is false).
        // This allows us to render the streamline points in any order, front to back
        // or back to front, and not lose any points by depth testing against themselves.
        osg::ref_ptr< osg::Depth > depth =
            new osg::Depth( osg::Depth::LESS, 0., 1., false );
        ss->setAttributeAndModes( depth.get() );

        // Note:
        // After drawing opaque objects, translucency can be order-independent only if
        // certain criteria are met:
        // 1. The alpha values of all pixels must be the same, OR
        //    The RGB valuues of all pixels must be the same.
        // 2. Depth write must be disabled so that far translucent pixels don't lose the
        //    depth test to near translucent pixels.
        // 3. The blend function must not reference destination alpha.


        {
            // specify the position texture. The vertex shader will index into
            // this texture to obtain position values for each streamline point.
            //NOTE: Texture slot 1 is used by the splotch.png image
            ss->setTextureAttribute( 0, rawVTKData->getPositionTexture() );
            osg::ref_ptr< osg::Uniform > texPosUniform =
                new osg::Uniform( "texPos", 0 );
            ss->addUniform( texPosUniform.get() );
        }

        {
            //send down rgb using texture
            //NOTE: Texture slot 1 is used by the splotch.png image
            ss->setTextureAttribute( 2, rawVTKData->getScalarTexture() );
            osg::ref_ptr< osg::Uniform > texScaUniform =
                new osg::Uniform( "scalar", 2 );
            ss->addUniform( texScaUniform.get() );
        }

        {
            double dataRange[2];
            scalarArray->GetRange( dataRange );

            // Pass the min/max for the scalar range into the shader as a uniform.
            osg::Vec2 ts( dataRange[ 0 ], dataRange[ 1 ] );
            osg::ref_ptr< osg::Uniform > scalarMinMaxUniform =
                new osg::Uniform( "scalarMinMax",
                                  osg::Vec2( ( float )ts.x(), ( float )ts.y() ) );
            ss->addUniform( scalarMinMaxUniform.get() );

            // Set up the color spectrum.
            osg::Texture1D* texCS =
                new osg::Texture1D( rawVTKData->CreateColorTextures( dataRange ) );
            texCS->setFilter( osg::Texture::MIN_FILTER, osg::Texture2D::LINEAR );
            texCS->setFilter( osg::Texture::MAG_FILTER, osg::Texture2D::LINEAR );
            texCS->setWrap( osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE );

            ss->setTextureAttribute( 3, texCS );
            osg::ref_ptr< osg::Uniform > texCSUniform =
                new osg::Uniform( "texCS", 3 );
            ss->addUniform( texCSUniform.get() );
        }
    }

}
*/