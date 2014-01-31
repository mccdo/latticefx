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
#include <latticefx/core/vtk/VTKVectorRenderer.h>

#include <latticefx/core/vtk/ChannelDatavtkPolyData.h>

#include <latticefx/core/ChannelDataOSGArray.h>
#include <latticefx/core/TransferFunctionUtils.h>

#include <vtkMath.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>

namespace lfx
{

namespace core
{

namespace vtk
{

////////////////////////////////////////////////////////////////////////////////
osg::Node* VTKVectorRenderer::getSceneGraph( const lfx::core::ChannelDataPtr maskIn )
{
    vtkPolyData* tempVtkPD =
        boost::static_pointer_cast< lfx::core::vtk::ChannelDatavtkPolyData >( getInput( "vtkPolyData" ) )->GetPolyData();

    m_dataObject =
        boost::dynamic_pointer_cast< lfx::core::vtk::ChannelDatavtkDataObject >( getInput( "vtkDataObject" ) );
    if( !m_dataObject )
    {
        std::cout << "No vtkDataObject input was specified to VTKSurfaceRenderer." << std::endl;
        return 0;
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
        setTransferFunctionInput( m_activeScalar );
        return( lfx::core::VectorRenderer::getSceneGraph( maskIn ) );
    }

    vtkPoints* points = tempVtkPD->GetPoints();
    vtkPointData* pointData = tempVtkPD->GetPointData();
    
    //Setup the position and direction arrays
    {
        vtkDataArray* vectorArray = pointData->GetVectors( m_activeVector.c_str() );

        double x[3];
        size_t dataSize = points->GetNumberOfPoints();
        osg::ref_ptr< osg::Vec3Array > vertArray( new osg::Vec3Array );
        vertArray->resize( dataSize );
        osg::ref_ptr< osg::Vec3Array > dirArray( new osg::Vec3Array );
        dirArray->resize( dataSize );
        //osg::ref_ptr< osg::FloatArray > colorArray( new osg::FloatArray );
        //colorArray->resize( dataSize );

        for( size_t i = 0; i < dataSize; ++i )
        {
            //Get Position data
            points->GetPoint( i, x );
            ( *vertArray )[ i ].set( x[0], x[1], x[2] );

            //if( vectorArray )
            {
                //Get Vector data
                vectorArray->GetTuple( i, x );
                osg::Vec3 v( x[0], x[1], x[2] );
                v.normalize();
                ( *dirArray )[ i ].set( v.x(), v.y(), v.z() );
            }
        }
        //by this stage of the game the render has already had setInputs called
        //on it by lfx::core::DataSet therefore we can modify the _inputs array
        lfx::core::ChannelDataOSGArrayPtr vertData( new lfx::core::ChannelDataOSGArray( "positions", vertArray.get() ) );
        addInput( vertData );
        m_scalarChannels[ "positions" ] = vertData;
        
        lfx::core::ChannelDataOSGArrayPtr dirData( new lfx::core::ChannelDataOSGArray( "directions", dirArray.get() ) );
        addInput( dirData );
        m_scalarChannels[ "directions" ] = dirData;

        setPointStyle( lfx::core::VectorRenderer::DIRECTION_VECTORS );
    }

    //Setup the scalar arrays
    {
        double scalarRange[ 2 ] = {0, 1.0};
        double val = 0;
        size_t numDataArrays = pointData->GetNumberOfArrays();
        size_t dataSize = points->GetNumberOfPoints();
        for( size_t i = 0; i < numDataArrays; ++i )
        {
            vtkDataArray* scalarArray = pointData->GetArray( i );
            if( scalarArray->GetNumberOfComponents() == 1 )
            {
                osg::ref_ptr< osg::FloatArray > colorArray( new osg::FloatArray );
                const std::string arrayName = scalarArray->GetName();
                m_dataObject->GetScalarRange( arrayName, scalarRange );
                for( vtkIdType j = 0; j < dataSize; ++j )
                {
                    scalarArray->GetTuple( j, &val );
                    val = vtkMath::ClampAndNormalizeValue( val, scalarRange );
                    colorArray->push_back( val );
                }
                lfx::core::ChannelDataOSGArrayPtr colorData( new lfx::core::ChannelDataOSGArray( arrayName, colorArray.get() ) );
                addInput( colorData );
                m_scalarChannels[ arrayName ] = colorData;
                //haveScalarData = true;
            }
        }

        // Configure transfer function.
        setTransferFunctionInput( m_activeScalar );
        setTransferFunction( lfx::core::loadImageFromDat( "01.dat" ) );
        setTransferFunctionDestination( lfx::core::Renderer::TF_RGBA );
    }


#if WRITE_IMAGE_DATA
    //osgDB::writeNodeFile( *(tempGeode.get()), "gpu_vector_field.ive" );
#endif

    return( lfx::core::VectorRenderer::getSceneGraph( maskIn ) );
}

////////////////////////////////////////////////////////////////////////////////
void VTKVectorRenderer::serializeData( JsonSerializer *json ) const
{
	// let the parent write its data
	VectorRenderer::serializeData( json );

	json->insertObj( VTKVectorRenderer::getClassName(), true);
	json->insertObjValue( "activeVector", m_activeVector );
	json->insertObjValue( "activeScalar", m_activeScalar );
    //vtkPolyData* m_pd;
    //std::map< std::string, lfx::core::ChannelDataPtr > m_scalarChannels;
    //lfx::core::vtk::ChannelDatavtkDataObjectPtr m_dataObject;
	json->popParent();
}

////////////////////////////////////////////////////////////////////////////////
bool VTKVectorRenderer::loadData( JsonSerializer *json, IObjFactory *pfactory, std::string *perr )
{
	// let the parent load its data
	if ( !VectorRenderer::loadData( json, pfactory, perr )) return false;

	// get to this classes data
	if ( !json->getObj( VTKVectorRenderer::getClassName() ) )
	{
		if (perr) *perr = "Json: Failed to get VTKVectorRenderer data";
		return false;
	}

	json->getValue( "activeVector", &m_activeVector);
	json->getValue( "activeScalar", &m_activeScalar );

	json->popParent();
	return true;
}

////////////////////////////////////////////////////////////////////////////////
void VTKVectorRenderer::dumpState( std::ostream &os )
{
	VectorRenderer::dumpState( os );

	dumpStateStart( VTKVectorRenderer::getClassName(), os );
	os << "_activeVector: " << m_activeVector << std::endl;
	os << "_activeScalar: " << m_activeScalar << std::endl;
	dumpStateEnd( VTKVectorRenderer::getClassName(), os );
}

////////////////////////////////////////////////////////////////////////////////
}
}
}

