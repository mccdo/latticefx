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
#include <latticefx/utils/vtk/ExtractGeometryCallback.h>

#include <vtkDataSet.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkUnstructuredGrid.h>
#include <vtkExtractGeometry.h>
#include <vtkBox.h>
#include <vtkPolyData.h>

#include <iostream>

using namespace lfx::vtk_utils;

//////////////////////////////////////////////////////////////////////
ExtractGeometryCallback::ExtractGeometryCallback()
    :
    m_dataset( 0 ),
    m_dataCounter( 0 ),
    m_surface( 0 )
{
    ;
}
//////////////////////////////////////////////////////////////////////
ExtractGeometryCallback::~ExtractGeometryCallback()
{
    m_dataset->Delete();
    m_dataset = 0;
}
//////////////////////////////////////////////////////////////
vtkDataObject* ExtractGeometryCallback::GetDataset()
{
    return m_dataset;
}
/////////////////////////////////////////////////////////////////////////////
void ExtractGeometryCallback::SetPolyDataSurface( vtkPolyData* surface )
{
    m_surface = surface;
    m_surface->GetBounds( m_bbox );
    m_bbox[ 0 ] -= 0.05;
    m_bbox[ 2 ] -= 0.05;
    m_bbox[ 4 ] -= 0.05;
    m_bbox[ 1 ] += 0.05;
    m_bbox[ 3 ] += 0.05;
    m_bbox[ 5 ] += 0.05;
    std::cout << "|\t\tThe bounding box dims " << m_bbox[ 0 ] << " " << m_bbox[ 1 ] << " " << m_bbox[ 2 ] << " " << m_bbox[ 3 ] << " " << m_bbox[ 4 ] << " " << m_bbox[ 5 ] << std::endl;
}
/////////////////////////////////////////////////////////////////////////////
void ExtractGeometryCallback::OperateOnDataset( vtkDataSet* dataset )
{
    if( !m_surface )
    {
        return;
    }
    vtkExtractGeometry* extractGrid = vtkExtractGeometry::New();
    extractGrid->SetInput( dataset );
    extractGrid->ExtractBoundaryCellsOn();

    //extractGrid->SetExtent( pdbbox );
    vtkBox* bbox = vtkBox::New();
    bbox->SetBounds( m_bbox );
    extractGrid->SetImplicitFunction( bbox );

    if( !m_dataset )
    {
        m_dataset = vtkMultiBlockDataSet::New();
    }

    extractGrid->Update();

    if( extractGrid->GetOutput() )
    {
        m_dataset->SetBlock( m_dataCounter, extractGrid->GetOutput() );
        m_dataCounter += 1;
    }

    bbox->Delete();
    extractGrid->Delete();
}
