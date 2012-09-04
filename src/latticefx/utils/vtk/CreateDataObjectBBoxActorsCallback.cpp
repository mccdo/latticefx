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

#include <latticefx/utils/vtk/CreateDataObjectBBoxActorsCallback.h>
#include <vtkDataSet.h>
#include <vtkActor.h>
#include <vtkOutlineFilter.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>

using namespace lfx::vtk_utils;

//////////////////////////////////////////////////////////////////////
CreateDataObjectBBoxActorsCallback::CreateDataObjectBBoxActorsCallback()
{}
///////////////////////////////////////////////////////////////////////
CreateDataObjectBBoxActorsCallback::~CreateDataObjectBBoxActorsCallback()
{
    size_t nActors = m_bboxActors.size();
    for( size_t i = 0; i < nActors; ++i )
    {
        m_bboxActors.at( i )->Delete();
    }
    m_bboxActors.clear();
}
/////////////////////////////////////////////////////////////////////////////
void CreateDataObjectBBoxActorsCallback::OperateOnDataset( vtkDataSet* dataset )
{
    if( dataset->GetNumberOfCells() == 0 )
    {
        return;
    }

    vtkOutlineFilter* outlineData = vtkOutlineFilter::New();

    outlineData->SetInput( dataset );

    vtkPolyDataMapper* mapOutline = vtkPolyDataMapper::New();
    mapOutline->SetInput( outlineData->GetOutput() );

    m_bboxActors.push_back( vtkActor::New() );

    m_bboxActors.back()->SetMapper( mapOutline );
    m_bboxActors.back()->GetProperty()->SetColor( 1, 0, 0 );

    outlineData->Delete();
    mapOutline->Delete();
}
////////////////////////////////////////////////////////////////////////////
std::vector< vtkActor* >& CreateDataObjectBBoxActorsCallback::GetBBoxActors()
{
    return m_bboxActors;
}
