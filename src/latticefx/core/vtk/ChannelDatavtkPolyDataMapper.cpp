/*************** <auto-copyright.rb BEGIN do not edit this line> *************
*
* latticeFX is (C) Copyright 2011-2012 by Ames Laboratory
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Library General Public
* License as published by the Free Software Foundation; 
* version 2.1 of the License.
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
*************** <auto-copyright.rb END do not edit this line> **************/

#include <latticefx/core/vtk/ChannelDatavtkPolyDataMapper.h>

#include <vtkPolyDataMapper.h>

namespace lfx {

namespace core {

namespace vtk {
    
////////////////////////////////////////////////////////////////////////////////
ChannelDatavtkPolyDataMapper::ChannelDatavtkPolyDataMapper( vtkAlgorithmOutput* ao, const std::string& name )
    : 
    ChannelData( name ),
    m_pdm( vtkPolyDataMapper::New() )
{
    m_pdm->SetInputConnection( ao );
    //m_pdm->DebugOn();
    //mapper->SetScalarModeToDefault();
    //mapper->SetColorModeToDefault();
    //mapper->SetColorModeToMapScalars();
    //mapper->InterpolateScalarsBeforeMappingOff();
    m_pdm->SetScalarModeToUsePointFieldData();
    m_pdm->UseLookupTableScalarRangeOn();
}
////////////////////////////////////////////////////////////////////////////////
ChannelDatavtkPolyDataMapper::ChannelDatavtkPolyDataMapper( const ChannelDatavtkPolyDataMapper& rhs )
    : 
    ChannelData( rhs ),
    m_pdm( rhs.m_pdm )
{
    ;
}
////////////////////////////////////////////////////////////////////////////////
ChannelDatavtkPolyDataMapper::~ChannelDatavtkPolyDataMapper()
{
    m_pdm->Delete();
    m_pdm = 0;
}
////////////////////////////////////////////////////////////////////////////////
vtkPolyDataMapper* ChannelDatavtkPolyDataMapper::GetPolyDataMapper()
{
    return m_pdm;
}
////////////////////////////////////////////////////////////////////////////////
}
}
// lfx
}
