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
#include <latticefx/core/vtk/VTKVectorFieldRTP.h>
#include <latticefx/core/vtk/ChannelDatavtkPolyData.h>
#include <latticefx/core/vtk/ChannelDatavtkDataObject.h>
#include <latticefx/core/MiscUtils.h>

#include <vtkDataObject.h>
#include <vtkCompositeDataGeometryFilter.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkCellDataToPointData.h>
#include <vtkMaskPoints.h>
#include <vtkAlgorithm.h>
#include <vtkAlgorithmOutput.h>
#include <vtkPlane.h>
#include <vtkCutter.h>
#include <vtkThresholdPoints.h>
#include <vtkPolyDataMapper.h>

namespace lfx
{

namespace core
{

namespace vtk
{

////////////////////////////////////////////////////////////////////////////////
VTKVectorFieldRTP::VTKVectorFieldRTP() : VTKBaseRTP( lfx::core::RTPOperation::Channel ),
	_vectorRatioFactor( 1 ),
	_numSteps( 10 )
        
{
	_vectorThreshHold[ 0 ] = 0.0;
    _vectorThreshHold[ 1 ] = 100.0;

	_planeOrigin[0] = 0;
	_planeOrigin[1] = 0;
	_planeOrigin[2] = 0;
	_planeNormal[0] = 1;
	_planeNormal[1] = 0;
	_planeNormal[2] = 0;
}

////////////////////////////////////////////////////////////////////////////////
bool VTKVectorFieldRTP::setVectorRatioFactor( double value )
{
	if( MiscUtils::isnot_close( _vectorRatioFactor , value, .001 ) )
	{
		_vectorRatioFactor = value;
		return true;
	}
	
	return false;
}

////////////////////////////////////////////////////////////////////////////////
bool VTKVectorFieldRTP::setNumberOfSteps( int steps )
{
	if( _numSteps != steps )
	{
		_numSteps = steps;
		return true;
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////
bool VTKVectorFieldRTP::setVectorThreshHold( double min, double max )
{
	bool modified = false;
	if( MiscUtils::isnot_close( _vectorThreshHold[ 0 ] , min, .001 ) )
	{
		_vectorThreshHold[ 0 ] = min;
		modified = true;
	}

	if( MiscUtils::isnot_close( _vectorThreshHold[ 1 ] , max, .001 ) )
	{
		_vectorThreshHold[ 1 ] = max;
		modified = true;
	}
    
	return modified;
}

////////////////////////////////////////////////////////////////////////////////
bool VTKVectorFieldRTP::setPlaneOrigin( double o[3] )
{
	bool modified = false;

	for( int i=0; i<3; i++ )
	{
		if( MiscUtils::isnot_close( _planeOrigin[ i ] , o[ i ], .001 ) )
		{
			_planeOrigin[ i ] = o[ i ];
			modified = true;
		}
	}

	return modified;
}

////////////////////////////////////////////////////////////////////////////////
bool VTKVectorFieldRTP::setPlaneNormal( double n[3] )
{
	bool modified = false;

	for( int i=0; i<3; i++ )
	{
		if( MiscUtils::isnot_close( _planeNormal[ i ] , n[ i ], .001 ) )
		{
			_planeNormal[ i ] = n[ i ];
			modified = true;
		}
	}

	return modified;
}

////////////////////////////////////////////////////////////////////////////////
lfx::core::ChannelDataPtr VTKVectorFieldRTP::channel( const lfx::core::ChannelDataPtr maskIn )
{
	lfx::core::vtk::ChannelDatavtkDataObjectPtr cddoPtr = boost::static_pointer_cast< lfx::core::vtk::ChannelDatavtkDataObject >( getInput( "vtkDataObject" ) );
	return createPresetVector( cddoPtr );
}

/*
// ORIGINAL CODE HERE
////////////////////////////////////////////////////////////////////////////////
lfx::core::ChannelDataPtr VTKVectorFieldRTP::channel( const lfx::core::ChannelDataPtr maskIn )
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

    ptmask->Update();

    lfx::core::vtk::ChannelDatavtkPolyDataPtr cdpd(
        new lfx::core::vtk::ChannelDatavtkPolyData( ptmask->GetOutput(), "vtkPolyData" ) );

    ptmask->Delete();
    c2p->Delete();

    return( cdpd );
}
*/

////////////////////////////////////////////////////////////////////////////////
void VTKVectorFieldRTP::serializeData( JsonSerializer *json ) const
{
	// let the parent write its data
	VTKBaseRTP::serializeData( json );

	json->insertObj( VTKVectorFieldRTP::getClassName(), true );
	json->insertObjValue( "mask", m_mask );
	json->popParent();
}

////////////////////////////////////////////////////////////////////////////////
bool VTKVectorFieldRTP::loadData( JsonSerializer *json, IObjFactory *pfactory, std::string *perr )
{
	// let the parent load its data
	if ( !VTKBaseRTP::loadData( json, pfactory, perr )) return false;

	// get to this classes data
	if ( !json->getObj( VTKVectorFieldRTP::getClassName() ) )
	{
		if (perr) *perr = "Json: Failed to get VTKVectorFieldRTP data";
		return false;
	}

	json->getValue( "mask", &m_mask );
	json->popParent();
	return true;
}

////////////////////////////////////////////////////////////////////////////////
void VTKVectorFieldRTP::dumpState( std::ostream &os )
{
	VTKBaseRTP::dumpState( os );

	dumpStateStart( VTKVectorFieldRTP::getClassName(), os );
	os << "_mask: " << m_mask << std::endl;
	dumpStateEnd( VTKVectorFieldRTP::getClassName(), os );
}

////////////////////////////////////////////////////////////////////////////////
lfx::core::ChannelDataPtr VTKVectorFieldRTP::createPresetVector( ChannelDatavtkDataObjectPtr cddoPtr )
{
	vtkPolyDataMapper*   mapper = vtkPolyDataMapper::New(); // TODO: 
	vtkCellDataToPointData* c2p = vtkCellDataToPointData::New();
	vtkDataObject* tempVtkDO = cddoPtr->GetDataObject();

	double bounds[6];
	cddoPtr->GetBounds( bounds );
	

	if( m_planeDirection == CuttingPlane::SliceDirection::CUSTOM)
	{
		vtkPlane *plane = vtkPlane::New();
		plane->SetOrigin( _planeOrigin );
		plane->SetNormal( _planeNormal );

		vtkCutter* cutter = vtkCutter::New();
        cutter->SetInput( tempVtkDO );
        cutter->SetCutFunction( plane );
        cutter->Update();
        c2p->SetInputConnection( cutter->GetOutputPort() );
		//c2p->Update();
        cutter->Delete();
		plane->Delete();
	}
    else if( m_planeDirection < 3 )
    {
		CuttingPlane cuttingPlane(bounds, m_planeDirection, _numSteps);
                
		// insure that we are using correct bounds for the given data set...
        cuttingPlane.SetBounds( bounds );
		cuttingPlane.Advance( m_requestedValue );
		vtkCutter* cutter = vtkCutter::New();
        cutter->SetInput( tempVtkDO );
        cutter->SetCutFunction( cuttingPlane.GetPlane() );
        cutter->Update();
        c2p->SetInputConnection( cutter->GetOutputPort() );
		//c2p->Update();
        cutter->Delete();
	}
    else if( m_planeDirection == 3 )
    {
		c2p->SetInput( tempVtkDO );
        //c2p->Update();
	}

	// get every nth point from the dataSet data
	vtkMaskPoints *ptmask = createMaskPoints( cddoPtr );
	ConnectGeometryFilter( tempVtkDO, c2p->GetOutputPort(), ptmask );
	ptmask->Update();


	vtkThresholdPoints* tfilter = filterByThreshHold( cddoPtr, ptmask );
    //setGlyphAttributes();        

	vtkPolyData *polyData;
	if( tfilter != NULL ) 
	{
		tfilter->Update();
		polyData = tfilter->GetOutput();
	}
	else
	{
		polyData = ptmask->GetOutput();
	}

	lfx::core::vtk::ChannelDatavtkPolyDataPtr cdpd( new lfx::core::vtk::ChannelDatavtkPolyData( polyData, "vtkPolyData" ) );

	// clean up
	if( tfilter ) tfilter->Delete();
	ptmask->Delete();
	c2p->Delete();

	return cdpd;
}
////////////////////////////////////////////////////////////////////////////////
vtkMaskPoints* VTKVectorFieldRTP::createMaskPoints( ChannelDatavtkDataObjectPtr cddoPtr )
{
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
    ptmask->SetOnRatio( _vectorRatioFactor );

	return ptmask;
}

////////////////////////////////////////////////////////////////////////////////
vtkThresholdPoints* VTKVectorFieldRTP::filterByThreshHold( ChannelDatavtkDataObjectPtr cddoPtr, vtkMaskPoints *ptmask )
{
	vtkThresholdPoints* tfilter = NULL;
	double currentScalarRange[ 2 ] = { m_minScalarValue, m_maxScalarValue } ;
    //cddoPtr->GetScalarRange( m_activeScalar, currentScalarRange );

    if( _vectorThreshHold[ 0 ] > currentScalarRange[ 0 ] && _vectorThreshHold[ 1 ] < currentScalarRange[ 1 ] )
    {
		// ThresholdBetween
		tfilter = vtkThresholdPoints::New();
        tfilter->SetInputConnection( ptmask->GetOutputPort() );
        tfilter->ThresholdBetween( _vectorThreshHold[ 0 ], _vectorThreshHold[ 1 ] );
        tfilter->SetInputArrayToProcess( 0, 0, 0,
                                         vtkDataObject::FIELD_ASSOCIATION_POINTS,
                                         m_activeScalar.c_str() );

        //this->tris->SetInputConnection( this->tfilter->GetOutputPort() );
        //this->strip->SetInputConnection( this->tris->GetOutputPort() );
        //this->glyph->SetInputConnection( this->strip->GetOutputPort() );
    }
    else if( _vectorThreshHold[ 0 ] > currentScalarRange[ 0 ] )
    {
        // ThresholdByUpper
                          
		tfilter = vtkThresholdPoints::New();
        tfilter->SetInputConnection( ptmask->GetOutputPort() );
        tfilter->ThresholdByUpper( _vectorThreshHold[ 0 ] );
        tfilter->SetInputArrayToProcess( 0, 0, 0,
                                         vtkDataObject::FIELD_ASSOCIATION_POINTS,
                                         m_activeScalar.c_str() );
        //this->tris->SetInputConnection( this->tfilter->GetOutputPort() );
        //this->strip->SetInputConnection( this->tris->GetOutputPort() );
        //this->glyph->SetInputConnection( this->strip->GetOutputPort() );
    }
    else if( _vectorThreshHold[ 1 ] < currentScalarRange[ 1 ] )
    {
        // ThresholdByLower
		tfilter = vtkThresholdPoints::New();
        tfilter->SetInputConnection( ptmask->GetOutputPort() );
        tfilter->ThresholdByLower( _vectorThreshHold[ 1 ] );
        tfilter->SetInputArrayToProcess( 0, 0, 0,
                                         vtkDataObject::FIELD_ASSOCIATION_POINTS,
                                         m_activeScalar.c_str() );
        //this->tris->SetInputConnection( this->tfilter->GetOutputPort() );
        //this->strip->SetInputConnection( this->tris->GetOutputPort() );
        //this->glyph->SetInputConnection( this->strip->GetOutputPort() );
    }
    else
    {
		// NO Threshold
    }

	return tfilter;
}

}
}
}

/*
vprDEBUG( vesDBG, 1 ) << "|\t\tcfdPresetVector " << this->cursorType
                              << " : " << usePreCalcData
                              << std::endl << vprDEBUG_FLUSH;
        {
            vtkCellDataToPointData* c2p = vtkCellDataToPointData::New();
            if( xyz < 3 )
            {
                this->cuttingPlane =
                    new cfdCuttingPlane( GetActiveDataSet()->GetBounds(),
                                         xyz, numSteps );
                // insure that we are using correct bounds for the given data set...
                this->cuttingPlane->SetBounds(
                    this->GetActiveDataSet()->GetBounds() );
                this->cuttingPlane->Advance( requestedValue );
                vtkCutter* cutter = vtkCutter::New();
                cutter->SetInput( GetActiveDataSet()->GetDataSet() );
                cutter->SetCutFunction( this->cuttingPlane->GetPlane() );
                cutter->Update();
                delete this->cuttingPlane;
                this->cuttingPlane = NULL;
                c2p->SetInputConnection( cutter->GetOutputPort() );
                //c2p->Update();
                cutter->Delete();
            }
            else if( xyz == 3 )
            {
                c2p->SetInput( GetActiveDataSet()->GetDataSet() );
                //c2p->Update();
            }

            // get every nth point from the dataSet data
            this->ptmask->SetInputConnection( ApplyGeometryFilterNew( c2p->GetOutputPort() ) );
            //std::cout << this->GetVectorRatioFactor() << std::endl;
            this->ptmask->SetOnRatio( this->GetVectorRatioFactor() );
            this->ptmask->Update();


            this->SetGlyphWithThreshold();
            this->SetGlyphAttributes();
            //this->glyph->Update();


            mapper->SetInputConnection( glyph->GetOutputPort() );
            mapper->SetScalarModeToUsePointFieldData();
            mapper->UseLookupTableScalarRangeOn();
            mapper->SelectColorArray( GetActiveDataSet()->
                                      GetActiveScalarName().c_str() );
            mapper->SetLookupTable( GetActiveDataSet()->GetLookupTable() );
            mapper->Update();


            c2p->Delete();
            vprDEBUG( vesDBG, 1 )
                    << "|\t\tNo Precalc : " << this->cursorType << " : " << usePreCalcData
                    << " : " << GetVectorRatioFactor() << std::endl << vprDEBUG_FLUSH;
        }
*/


// original
	/*
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
*/
