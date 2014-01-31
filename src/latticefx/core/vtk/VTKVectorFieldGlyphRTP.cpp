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
#include <latticefx/core/vtk/VTKVectorFieldGlyphRTP.h>
#include <latticefx/core/vtk/ChannelDatavtkPolyData.h>
#include <latticefx/core/vtk/ChannelDatavtkDataObject.h>

#include <vtkDataObject.h>
#include <vtkCompositeDataGeometryFilter.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkCellDataToPointData.h>
#include <vtkMaskPoints.h>
#include <vtkAlgorithm.h>
#include <vtkAlgorithmOutput.h>

namespace lfx
{

namespace core
{

namespace vtk
{

////////////////////////////////////////////////////////////////////////////////
lfx::core::ChannelDataPtr VTKVectorFieldGlyphRTP::channel( const lfx::core::ChannelDataPtr maskIn )
{
    lfx::core::vtk::ChannelDatavtkDataObjectPtr cddoPtr =
        boost::static_pointer_cast< lfx::core::vtk::ChannelDatavtkDataObject >(
            getInput( "vtkDataObject" ) );

    vtkDataObject* tempVtkDO = cddoPtr->GetDataObject();
    vtkCellDataToPointData* c2p = vtkCellDataToPointData::New();
    c2p->SetInput( tempVtkDO );
    //c2p->Update();

    vtkMaskPoints* ptmask = vtkMaskPoints::New();
    //This is required for use with VTK 5.10
    ptmask->SetMaximumNumberOfPoints( cddoPtr->GetNumberOfPoints() );
#if ( VTK_MAJOR_VERSION >= 5 ) && ( VTK_MINOR_VERSION >= 10 )
    //New feature for selecting points at random in VTK 5.10
    ptmask->SetRandomModeType( 0 );
#else
    ptmask->RandomModeOn();
#endif
    // get every nth point from the dataSet data
    ptmask->SetOnRatio( m_mask );


    vtkPolyData* tempPd = 0;
    if( tempVtkDO->IsA( "vtkCompositeDataSet" ) )
    {
        vtkCompositeDataGeometryFilter* m_multiGroupGeomFilter =
            vtkCompositeDataGeometryFilter::New();
        m_multiGroupGeomFilter->SetInputConnection( c2p->GetOutputPort() );
        ptmask->SetInputConnection( m_multiGroupGeomFilter->GetOutputPort( 0 ) );
        m_multiGroupGeomFilter->Delete();
    }
    else
    {
        vtkDataSetSurfaceFilter* m_surfaceFilter =
            vtkDataSetSurfaceFilter::New();
        m_surfaceFilter->SetInputConnection( c2p->GetOutputPort() );
        ptmask->SetInputConnection( m_surfaceFilter->GetOutputPort() );
        m_surfaceFilter->Delete();
    }

    
	// set up roi extraction if needed
    ptmask->Update();

    //The rest of the pipeline goes here

    lfx::core::vtk::ChannelDatavtkPolyDataPtr cdpd(
        new lfx::core::vtk::ChannelDatavtkPolyData( ptmask->GetOutput(), "vtkPolyData" ) );

    ptmask->Delete();
    c2p->Delete();

    return( cdpd );
}
////////////////////////////////////////////////////////////////////////////////
void VTKVectorFieldGlyphRTP::SetMaskValue( double value )
{
    m_mask = value;
}

////////////////////////////////////////////////////////////////////////////////
void VTKVectorFieldGlyphRTP::serializeData( JsonSerializer *json ) const
{
	// let the parent write its data
	VTKBaseRTP::serializeData( json );

	json->insertObj( VTKVectorFieldGlyphRTP::getClassName(), true );
	json->insertObjValue( "mask", m_mask );
	json->popParent();
}

////////////////////////////////////////////////////////////////////////////////
bool VTKVectorFieldGlyphRTP::loadData( JsonSerializer *json, IObjFactory *pfactory, std::string *perr )
{
	// let the parent load its data
	if ( !VTKBaseRTP::loadData( json, pfactory, perr )) return false;

	// get to this classes data
	if ( !json->getObj( VTKVectorFieldGlyphRTP::getClassName() ) )
	{
		if (perr) *perr = "Json: Failed to get VTKVectorFieldGlyphRTP data";
		return false;
	}

	json->getValue( "mask", &m_mask );
	json->popParent();
	return true;
}

////////////////////////////////////////////////////////////////////////////////
void VTKVectorFieldGlyphRTP::dumpState( std::ostream &os )
{
	VTKBaseRTP::dumpState( os );

	dumpStateStart( VTKVectorFieldGlyphRTP::getClassName(), os );
	os << "_mask: " << m_mask << std::endl;
	dumpStateEnd( VTKVectorFieldGlyphRTP::getClassName(), os );
}

////////////////////////////////////////////////////////////////////////////////
}
}
}