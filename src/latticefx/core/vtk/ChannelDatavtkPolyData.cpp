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

#include <latticefx/core/vtk/ChannelDatavtkPolyData.h>

#include <vtkPolyData.h>

namespace lfx
{

namespace core
{

namespace vtk
{

////////////////////////////////////////////////////////////////////////////////
ChannelDatavtkPolyData::ChannelDatavtkPolyData( vtkPolyData* pd, const std::string& name, const std::string& logName )
    :
    ChannelData( name, logName ),
    m_pd( 0 )
{
    m_pd = vtkPolyData::New();
    m_pd->ShallowCopy( pd );
}
////////////////////////////////////////////////////////////////////////////////
ChannelDatavtkPolyData::ChannelDatavtkPolyData( const ChannelDatavtkPolyData& rhs )
    :
    ChannelData( rhs ),
    m_pd( rhs.m_pd )
{
    m_pd = vtkPolyData::New();
    m_pd->ShallowCopy( rhs.m_pd );
}
////////////////////////////////////////////////////////////////////////////////
ChannelDatavtkPolyData::~ChannelDatavtkPolyData()
{
    m_pd->Delete();
    m_pd = 0;
}
////////////////////////////////////////////////////////////////////////////////
vtkPolyData* ChannelDatavtkPolyData::GetPolyData()
{
    return m_pd;
}
////////////////////////////////////////////////////////////////////////////////
}
}
// lfx
}
