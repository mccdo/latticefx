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
#include <latticefx/core/vtk/VTKPointRenderer.h>

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
bool VTKPointRenderer::setRenderSpheres( bool value )
{
	if( _renderSpheres != value )
	{
		_renderSpheres = value;
		return true;
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////
osg::Node* VTKPointRenderer::getSceneGraph( const lfx::core::ChannelDataPtr maskIn )
{
    vtkPolyData* tempVtkPD =
        boost::static_pointer_cast< lfx::core::vtk::ChannelDatavtkPolyData >( getInput( "vtkPolyData" ) )->GetPolyData();

    m_dataObject =
        boost::dynamic_pointer_cast< lfx::core::vtk::ChannelDatavtkDataObject >( getInput( "vtkDataObject" ) );
    if( !m_dataObject )
    {
        std::cout << "No vtkDataObject input was specified to VTKPointRenderer." << std::endl;
        return 0;
    }

	if( m_refresh )
	{
		m_scalarChannels.clear();
		m_refresh = false;
	}

	m_curScalar = m_activeScalar;

    if( !m_scalarChannels.empty() )
    {
        //Re-add all of the local inputs since they get removed in this call
        //void DataSet::setInputs( OperationBasePtr opPtr, ChannelDataList& currentData )
        for( std::map< std::string, lfx::core::ChannelDataPtr >::const_iterator iter = m_scalarChannels.begin(); iter != m_scalarChannels.end(); ++iter )
        {
            addInput( iter->second );
        }
        
		transferFuncRefresh( this );

        return( lfx::core::VectorRenderer::getSceneGraph( maskIn ) );
    }

    vtkPoints* points = tempVtkPD->GetPoints();
    vtkPointData* pointData = tempVtkPD->GetPointData();
    
    //Setup the position
    {

        double x[3];
        size_t dataSize = points->GetNumberOfPoints();
        osg::ref_ptr< osg::Vec3Array > vertArray( new osg::Vec3Array );
        vertArray->resize( dataSize );

        for( size_t i = 0; i < dataSize; ++i )
        {
            //Get Position data
            points->GetPoint( i, x );
            ( *vertArray )[ i ].set( x[0], x[1], x[2] );
        }
        //by this stage of the game the render has already had setInputs called
        //on it by lfx::core::DataSet therefore we can modify the _inputs array
        lfx::core::ChannelDataOSGArrayPtr vertData( new lfx::core::ChannelDataOSGArray( "positions", vertArray.get() ) );
        addInput( vertData );
        m_scalarChannels[ "positions" ] = vertData;

		if( _renderSpheres )
		{
			setPointStyle( lfx::core::VectorRenderer::SPHERES );
		}
		else
		{
			setPointStyle( lfx::core::VectorRenderer::SIMPLE_POINTS );
		}
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
        transferFuncInit( this );
    }


#if WRITE_IMAGE_DATA
    //osgDB::writeNodeFile( *(tempGeode.get()), "gpu_vector_field.ive" );
#endif

    return( lfx::core::VectorRenderer::getSceneGraph( maskIn ) );
}

////////////////////////////////////////////////////////////////////////////////
void VTKPointRenderer::serializeData( JsonSerializer *json ) const
{
	// let the parent write its data
	VectorRenderer::serializeData( json );

	json->insertObj( VTKPointRenderer::getClassName(), true);
	json->insertObjValue( "activeVector", m_activeVector );
	json->insertObjValue( "activeScalar", m_activeScalar );
	json->insertObjValue( "renderSpheres", _renderSpheres );

	json->popParent();
}

////////////////////////////////////////////////////////////////////////////////
bool VTKPointRenderer::loadData( JsonSerializer *json, IObjFactory *pfactory, std::string *perr )
{
	// let the parent load its data
	if ( !VectorRenderer::loadData( json, pfactory, perr )) return false;

	// get to this classes data
	if ( !json->getObj( VTKPointRenderer::getClassName() ) )
	{
		if (perr) *perr = "Json: Failed to get VTKPointRenderer data";
		return false;
	}

	json->getValue( "activeVector", &m_activeVector);
	json->getValue( "activeScalar", &m_activeScalar );
	json->getValue( "renderSpheres", &_renderSpheres );

	json->popParent();
	return true;
}

////////////////////////////////////////////////////////////////////////////////
void VTKPointRenderer::dumpState( std::ostream &os )
{
	VectorRenderer::dumpState( os );

	dumpStateStart( VTKPointRenderer::getClassName(), os );
	os << "_activeVector: " << m_activeVector << std::endl;
	os << "_activeScalar: " << m_activeScalar << std::endl;
	os << "_renderSpheres: " << _renderSpheres << std::endl;
	dumpStateEnd( VTKPointRenderer::getClassName(), os );
}

////////////////////////////////////////////////////////////////////////////////
}
}
}

