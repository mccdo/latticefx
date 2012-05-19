/*************** <auto-copyright.rb BEGIN do not edit this line> **************
 *
 * VE-Suite is (C) Copyright 1998-2012 by Iowa State University
 *
 * Original Development Team:
 *   - ISU's Thermal Systems Virtual Engineering Group,
 *     Headed by Kenneth Mark Bryden, Ph.D., www.vrac.iastate.edu/~kmbryden
 *   - Reaction Engineering International, www.reaction-eng.com
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
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
 *************** <auto-copyright.rb END do not edit this line> ***************/

#include <ves/xplorer/util/CreateDataObjectBBoxActorsCallback.h>
#include <vtkDataSet.h>
#include <vtkActor.h>
#include <vtkOutlineFilter.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>

using namespace ves::xplorer::util;

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
