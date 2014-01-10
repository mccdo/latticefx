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
#include <latticefx/core/vtk/VTKStreamlineRTP.h>
#include <latticefx/core/vtk/ChannelDatavtkPolyData.h>
#include <latticefx/core/vtk/ChannelDatavtkDataObject.h>

#include <latticefx/core/LogMacros.h>

#include <vtkDataObject.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkCellDataToPointData.h>
#include <vtkAlgorithm.h>
#include <vtkAlgorithmOutput.h>
#include <vtkPolyData.h>
#include <vtkPolyDataNormals.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataAlgorithm.h>
#include <vtkPointData.h>
#include <vtkStreamTracer.h>
#include <vtkRungeKutta4.h>
#include <vtkCleanPolyData.h>
#include <vtkConeSource.h>
#include <vtkGlyph3D.h>
#include <vtkAppendPolyData.h>
#include <vtkMaskPoints.h>
#include <vtkRibbonFilter.h>
#include <vtkLookupTable.h>

//#include <vtkSmartPointer.h>

namespace lfx
{

namespace core
{

namespace vtk
{

VTKStreamlineRTP::VTKStreamlineRTP() : VTKBaseRTP( lfx::core::RTPOperation::Channel ),
	_dsBounds(6, 0),
	_bbox( 6, 0 ),
	_numPts( 3, 4 ),
	_integrationDirection( 0 ),
    _streamArrows( 0 ),
    _streamRibbons( 0 ),
    _propagationTime( -1 ),
    _integrationStepLength( -1 ),
    _lineDiameter( 1.0f ),
    _arrowDiameter( 1 ),
    _particleDiameter( 1.0f ),
	_maxTime( 1 )
{
	_bbox[1] = 1;
	_bbox[3] = 1;
	_bbox[5] = 1;
}

////////////////////////////////////////////////////////////////////////////////
VTKStreamlineRTP::~VTKStreamlineRTP()
{
}

////////////////////////////////////////////////////////////////////////////////
void VTKStreamlineRTP::setDatasetBounds(double *bounds)
{
	for( int i=0; i<6; i++ )
	{
		_dsBounds[i] = bounds[i];
	}
}

////////////////////////////////////////////////////////////////////////////////
void VTKStreamlineRTP::setMaxTime( float time )
{
	_maxTime = time;
}

////////////////////////////////////////////////////////////////////////////////
lfx::core::ChannelDataPtr VTKStreamlineRTP::channel( const lfx::core::ChannelDataPtr maskIn )
{
	if( m_activeVector.empty() )
    {
        LFX_ERROR( "VTKStreamlineRTP::channel : The vector name for the streamline is empty." );
    }

	if( m_activeScalar.empty() )
    {
        LFX_ERROR( "VTKStreamlineRTP::channel : The scalar name for the streamline is empty." );
    }

	lfx::core::vtk::ChannelDatavtkDataObjectPtr cddoPtr = boost::static_pointer_cast< lfx::core::vtk::ChannelDatavtkDataObject >(getInput( "vtkDataObject" ) );
    vtkDataObject *tempVtkDO = cddoPtr->GetDataObject();

	vtkPolyData *seedPoints = createSeedPoints(_dsBounds, _bbox, _numPts);

	vtkPolyDataMapper *mapper = vtkPolyDataMapper::New();
	// TODO: caller needs to set this up
    if( _propagationTime == -1 )
    {
        _propagationTime = 10.0f * _maxTime;
    }

    if( _integrationStepLength == -1 )
    {
        _integrationStepLength = 0.050f;
    }

    vtkCellDataToPointData* c2p = vtkCellDataToPointData::New();
    c2p->SetInput( tempVtkDO );

	vtkStreamTracer *streamTracer = vtkStreamTracer::New();
    streamTracer->SetInputConnection( c2p->GetOutputPort() );
    //overall length of streamline
    streamTracer->SetMaximumPropagation( _propagationTime );

    // typically < 1
    streamTracer->SetMaximumIntegrationStep( _integrationStepLength );

    if( _integrationDirection == 0 )
    {
        streamTracer->SetIntegrationDirectionToBoth();
    }
    else if( _integrationDirection == 1 )
    {
        streamTracer->SetIntegrationDirectionToForward();
    } 
    else if( _integrationDirection == 2 )
    {
        streamTracer->SetIntegrationDirectionToBackward();
    }

	vtkRungeKutta4 *integ = vtkRungeKutta4::New();
    streamTracer->SetSource( seedPoints );
    streamTracer->SetIntegrator( integ );
	integ->Delete();
    streamTracer->SetComputeVorticity( true );
    streamTracer->SetInputArrayToProcess( 0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS, m_activeVector.c_str() );

    vtkCleanPolyData* cleanPD = vtkCleanPolyData::New();
    cleanPD->PointMergingOn();
    cleanPD->SetInputConnection( streamTracer->GetOutputPort() );

    vtkRibbonFilter* ribbon = 0;

	// DON"T NEED TO WORRY ABOUT THIS FOR NOW.. 
    if( _streamRibbons )
    {
        ribbon = vtkRibbonFilter::New();
        //ribbon->SetWidthFactor( arrowDiameter * 0.25);
        ribbon->SetWidth( _arrowDiameter );
        ribbon->SetInputConnection( cleanPD->GetOutputPort() );
        ribbon->SetInputArrayToProcess( 0, 0, 0,
                                        vtkDataObject::FIELD_ASSOCIATION_POINTS,
                                        "Vorticity" );
    }


	// DON"T NEED TO WORRY ABOUT THIS FOR NOW
    if( _streamArrows )
    {
        // Stream Points Section
        vtkConeSource* cone = 0;
        vtkGlyph3D* cones = 0;
        vtkAppendPolyData* append = 0;
        vtkPolyDataNormals* normals = 0;

        /*{
            vtkXMLPolyDataWriter* writer = vtkXMLPolyDataWriter::New();
            writer->SetInput( ( vtkPolyData* ) cleanPD->GetOutput() );
            //writer->SetDataModeToAscii();
            writer->SetFileName( "teststreamersclean.vtk" );
            writer->Write();
            writer->Delete();
        }*/

        vtkMaskPoints* ptmask = vtkMaskPoints::New();
        ptmask->RandomModeOff();
        ptmask->SetInputConnection( cleanPD->GetOutputPort() );
        ptmask->SetOnRatio( 2 );

        cone = vtkConeSource::New();
        cone->SetResolution( 5 );

        cones = vtkGlyph3D::New();
        cones->SetInputConnection( 0, ptmask->GetOutputPort() );
        cones->SetInputConnection( 1, cone->GetOutputPort() );
        //cones->SetSource( cone->GetOutput() );
        cones->SetScaleFactor( _arrowDiameter );
        //cones->SetScaleModeToScaleByVector();
        cones->SetScaleModeToDataScalingOff();
        //cones->SetColorModeToColorByScalar();
        cones->SetVectorModeToUseVector();
        //cones->GetOutput()->ReleaseDataFlagOn();
        cones->SetInputArrayToProcess( 1, 0, 0,
                                       vtkDataObject::FIELD_ASSOCIATION_POINTS,
                                       m_activeVector.c_str() );
        cones->SetInputArrayToProcess( 0, 0, 0,
                                       vtkDataObject::FIELD_ASSOCIATION_POINTS,
                                       m_activeScalar.c_str() );
        cones->SetInputArrayToProcess( 3, 0, 0,
                                       vtkDataObject::FIELD_ASSOCIATION_POINTS,
                                       m_activeScalar.c_str() );
        ptmask->Delete();

        normals = vtkPolyDataNormals::New();
        normals->SetInputConnection( cones->GetOutputPort() );
        normals->SplittingOff();
        normals->ConsistencyOn();
        normals->AutoOrientNormalsOn();
        normals->ComputePointNormalsOn();
        normals->ComputeCellNormalsOff();
        normals->NonManifoldTraversalOff();
        normals->Update();
        /*{
            vtkXMLPolyDataWriter* writer = vtkXMLPolyDataWriter::New();
            writer->SetInput( ( vtkPolyData* ) normals->GetOutput() );
            //writer->SetDataModeToAscii();
            writer->SetFileName( "teststreamercones.vtk" );
            writer->Write();
            writer->Delete();
        }*/
        normals->GetOutput()->GetPointData()->SetActiveNormals( "Normals" );
        normals->GetOutput()->GetPointData()->SetActiveScalars( m_activeScalar.c_str() );
        normals->GetOutput()->GetPointData()->SetActiveVectors( m_activeVector.c_str() );

        streamTracer->GetOutput()->GetPointData()->SetActiveNormals( "Normals" );
        streamTracer->GetOutput()->GetPointData()->SetActiveScalars( m_activeScalar.c_str() );
        streamTracer->GetOutput()->GetPointData()->SetActiveVectors( m_activeVector.c_str() );

        append = vtkAppendPolyData::New();
        //append->DebugOn();

        if( ribbon )
        {
            append->AddInput( ribbon->GetOutput() );
            ribbon->Delete();
            ribbon = 0;
        }
        else
        {
            append->AddInput( cleanPD->GetOutput() );
        }

        /*{
            vtkXMLPolyDataWriter* writer = vtkXMLPolyDataWriter::New();
            writer->SetInput( ( vtkPolyData* ) streamTracer->GetOutput() );
            //writer->SetDataModeToAscii();
            writer->SetFileName( "teststreamersstream.vtk" );
            writer->Write();
            writer->Delete();
        }*/
        append->AddInput( normals->GetOutput() );
        /*{
            vtkXMLPolyDataWriter* writer = vtkXMLPolyDataWriter::New();
            writer->SetInput( ( vtkPolyData* ) append->GetOutput() );
            //writer->SetDataModeToAscii();
            writer->SetFileName( "teststreamersapp.vtk" );
            writer->Write();
            writer->Delete();
        }*/

        vtkPolyDataNormals* overallNormals = vtkPolyDataNormals::New();
        overallNormals->SetInputConnection( append->GetOutputPort() );
        overallNormals->SplittingOff();
        overallNormals->ConsistencyOn();
        overallNormals->AutoOrientNormalsOn();
        overallNormals->ComputePointNormalsOn();
        overallNormals->ComputeCellNormalsOff();
        overallNormals->NonManifoldTraversalOff();
        //overallNormals->Update();

        mapper->SetInputConnection( overallNormals->GetOutputPort() );

        // Stream Points Section
        cone->Delete();
        cones->Delete();
        append->Delete();
        normals->Delete();
        overallNormals->Delete();
    }
    else
    {
		// no need to worry about RIBBON
        if( ribbon )
        {
            mapper->SetInputConnection( ribbon->GetOutputPort() );
        }
        else
        {
            /*{
             vtkXMLPolyDataWriter* writer = vtkXMLPolyDataWriter::New();
             writer->SetInput( cleanPD->GetOutput() );
             //writer->SetDataModeToAscii();
             writer->SetFileName( "streamline_output.vtp" );
             writer->Write();
             writer->Delete();
             }*/




			
            mapper->SetInputConnection( cleanPD->GetOutputPort() );
        }
    }

    mapper->SetColorModeToMapScalars();
    mapper->ImmediateModeRenderingOn();
    mapper->SetScalarModeToUsePointFieldData();
    mapper->UseLookupTableScalarRangeOn();
    // mapper->SelectColorArray( m_activeScalar.c_str() );

	mapper->Update();

	// this is from OSGStreamlineStage::createInstanced.. 
	vtkCleanPolyData* cleanPD2 = vtkCleanPolyData::New();
    cleanPD2->PointMergingOn();
    cleanPD2->SetTolerance( 0.0f );
    cleanPD2->SetInput( cleanPD->GetOutput() );
    cleanPD2->Update();
    vtkPolyData* streamlinePD = cleanPD2->GetOutput();

    vtkPointData* pointData = streamlinePD->GetPointData();
    if( pointData == NULL )
    {
		LFX_ERROR( "VTKStreamlineRTP::channel : pd point data is null." );
        return NULL;
    }
    //pointData->Update();

    vtkPoints* points = streamlinePD->GetPoints();
    if( points == NULL )
    {
		LFX_ERROR( "VTKStreamlineRTP::channel : points are null." );
        return NULL;
    }


	cleanPD->Update();
	lfx::core::ChannelDataPtr cdpd = lfx::core::vtk::ChannelDatavtkPolyDataPtr(
                   new lfx::core::vtk::ChannelDatavtkPolyData( streamlinePD, "vtkPolyData" ) );

	cleanPD2->Delete();
	cleanPD->Delete();
    if( ribbon )
    {
        ribbon->Delete();
    }

	return cdpd;


	// start of the orignal method pulled from cfdStreamers.cxx in ves
	/*
	if( m_activeVector.empty() )
    {
        LFX_ERROR( "VTKStreamlineRTP::channel : The vector name for the streamline is empty." );
    }

    if( m_activeScalar.empty() )
    {
        LFX_ERROR( "VTKStreamlineRTP::channel : The scalar name for the streamline is empty." );
    }

    lfx::core::vtk::ChannelDatavtkDataObjectPtr cddoPtr =
        boost::static_pointer_cast< lfx::core::vtk::ChannelDatavtkDataObject >(
            getInput( "vtkDataObject" ) );
    vtkDataObject* tempVtkDO = cddoPtr->GetDataObject();
    vtkCellDataToPointData* c2p = vtkCellDataToPointData::New();

	c2p->SetInput( tempVtkDO );

	c2p->SetInputArrayToProcess(0, 0, 0,
                                           vtkDataObject::FIELD_ASSOCIATION_POINTS,
                                           m_activeVector.c_str() );

	c2p->Update();
	vtkAlgorithmOutput *pout = c2p->GetOutputPort();
	vtkSmartPointer<vtkExtractPolyDataGeometry> roi = GetRoiPoly(pout);
	if (roi) 
	{
		roi->SetInputConnection( pout );
		roi->Update();
		pout = roi->GetOutputPort();
	}

	pout->
    //c2p->Update();

    vtkContourFilter* contourFilter = vtkContourFilter::New();
    contourFilter->UseScalarTreeOn();
    contourFilter->SetInputConnection( 0, c2p->GetOutputPort( 0 ) );
    contourFilter->SetValue( 0, m_requestedValue );
    contourFilter->ComputeNormalsOff();
    contourFilter->SetInputArrayToProcess( 0, 0, 0,
                                           vtkDataObject::FIELD_ASSOCIATION_POINTS,
                                           m_activeScalar.c_str() );
    //contourFilter->Update();

    vtkPolyDataNormals* normals = vtkPolyDataNormals::New();

    if( tempVtkDO->IsA( "vtkCompositeDataSet" ) )
    {
        vtkCompositeDataGeometryFilter* m_multiGroupGeomFilter =
            vtkCompositeDataGeometryFilter::New();
        m_multiGroupGeomFilter->SetInputConnection( contourFilter->GetOutputPort( 0 ) );
        normals->SetInputConnection( m_multiGroupGeomFilter->GetOutputPort( 0 ) );
        m_multiGroupGeomFilter->Delete();
    }
    else
    {
        vtkDataSetSurfaceFilter* m_surfaceFilter =
            vtkDataSetSurfaceFilter::New();
        m_surfaceFilter->SetInputConnection( contourFilter->GetOutputPort( 0 ) );

        normals->SetInputConnection( m_surfaceFilter->GetOutputPort() );

        m_surfaceFilter->Delete();
    }

	// set up roi extraction if needed
    normals->Update();
	vtkAlgorithmOutput *pout =  normals->GetOutputPort();
	vtkSmartPointer<vtkExtractPolyDataGeometry> roi = GetRoiPoly(pout);
	if (roi) 
	{
		roi->SetInputConnection( pout );
		roi->Update();
		pout = roi->GetOutputPort();
	}
 
    lfx::core::vtk::ChannelDatavtkPolyDataMapperPtr cdpd(
        new lfx::core::vtk::ChannelDatavtkPolyDataMapper( pout, "vtkPolyDataMapper" ) );
    //cdpd->GetPolyDataMapper()->SetScalarModeToUsePointFieldData();
    //cdpd->GetPolyDataMapper()->UseLookupTableScalarRangeOn();
    //cdpd->GetPolyDataMapper()->SelectColorArray( m_colorByScalar.c_str() );

    normals->Delete();
    c2p->Delete();
    contourFilter->Delete();

    return( cdpd );
	*/
}

////////////////////////////////////////////////////////////////////////////////
vtkPolyData* VTKStreamlineRTP::createSeedPoints( const std::vector<double> &bounds, const std::vector<double> &bbox, const std::vector<int> &numPts )
{
    //double bounds[ 6 ];
    //ds->GetBounds( bounds );

    double xDiff = bounds[ 1 ] - bounds[ 0 ];
    double yDiff = bounds[ 3 ] - bounds[ 2 ];
    double zDiff = bounds[ 5 ] - bounds[ 4 ];

    double xMin = bounds[ 0 ] + ( xDiff * bbox[0] );
    double xMax = bounds[ 0 ] + ( xDiff * bbox[1] );
    double yMin = bounds[ 2 ] + ( yDiff * bbox[2] );
    double yMax = bounds[ 2 ] + ( yDiff * bbox[3] );
    double zMin = bounds[ 4 ] + ( zDiff * bbox[4] );
    double zMax = bounds[ 4 ] + ( zDiff * bbox[5] );

    double xLoc = 0;
    double yLoc = 0;
    double zLoc = 0;
    int number = 0;

    //insert evenly spaced points inside bounding box
    vtkPoints *points = vtkPoints::New();

    double deltaX = ( numPts[0] == 1 ) ? 0 : ( xMax - xMin ) / double( numPts[0] - 1 );
    double deltaY = ( numPts[1] == 1 ) ? 0 : ( yMax - yMin ) / double( numPts[1] - 1 );
    double deltaZ = ( numPts[2] == 1 ) ? 0 : ( zMax - zMin ) / double( numPts[2] - 1 );

    for( unsigned int i = 0; i < numPts[0]; ++i )
    {
        xLoc = xMin + ( i * deltaX );
        for( unsigned int j = 0; j < numPts[1]; ++j )
        {
            yLoc = yMin + ( j * deltaY );
            for( unsigned int k = 0; k < numPts[2]; ++k )
            {
                //points added in ptMin + length*iteration/(number of equal segments)
                //where (number of equal segments) = ptValue+1
                zLoc = zMin + ( k * deltaZ );
                //std::cout << xLoc << " " <<  yLoc << " " <<  zLoc << std::endl;
                points->InsertPoint( number, xLoc, yLoc, zLoc );
                number = number + 1;
            }
        }
    }

    //create polydata to be glyphed
    vtkPolyData *seedPoints = vtkPolyData::New();
    seedPoints->SetPoints( points );
    points->Delete();
	return seedPoints;
}

////////////////////////////////////////////////////////////////////////////////
void VTKStreamlineRTP::serializeData( JsonSerializer *json ) const
{
	// let the parent write its data
	VTKBaseRTP::serializeData( json );

	json->insertObj( VTKStreamlineRTP::getClassName(), true);
	// store any class specific data here
	json->popParent();
}

////////////////////////////////////////////////////////////////////////////////
bool VTKStreamlineRTP::loadData( JsonSerializer *json, IObjFactory *pfactory, std::string *perr )
{
	// let the parent load its data
	if ( !VTKBaseRTP::loadData( json, pfactory, perr )) return false;

	// get to this classes data
	if ( !json->getObj( VTKStreamlineRTP::getClassName() ) )
	{
		if (perr) *perr = "Json: Failed to get VTKStreamlineRTP data";
		return false;
	}

	json->popParent();
	return true;
}

////////////////////////////////////////////////////////////////////////////////
void VTKStreamlineRTP::dumpState( std::ostream &os )
{
	VTKBaseRTP::dumpState( os );

	dumpStateStart( VTKStreamlineRTP::getClassName(), os );
	// no data
	dumpStateEnd( VTKStreamlineRTP::getClassName(), os );
}

////////////////////////////////////////////////////////////////////////////////
}
}
}
