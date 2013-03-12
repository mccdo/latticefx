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

#include <latticefx/core/vtk/ChannelDatavtkAlgorithmOutput.h>

#include <vtkAlgorithmOutput.h>

namespace lfx
{

namespace core
{

namespace vtk
{

////////////////////////////////////////////////////////////////////////////////
ChannelDatavtkAlgorithmOutput::ChannelDatavtkAlgorithmOutput( vtkAlgorithmOutput* ao, const std::string& name )
    :
    ChannelData( name ),
    m_ao( ao )
{
    //m_ao = vtkAlgorithmOutput::New();
    //m_ao->ShallowCopy( ao );
}
////////////////////////////////////////////////////////////////////////////////
ChannelDatavtkAlgorithmOutput::ChannelDatavtkAlgorithmOutput( const ChannelDatavtkAlgorithmOutput& rhs )
    :
    ChannelData( rhs ),
    m_ao( rhs.m_ao )
{
    //m_ao = vtkAlgorithmOutput::New();
    //m_ao->ShallowCopy( rhs.m_ao );
}
////////////////////////////////////////////////////////////////////////////////
ChannelDatavtkAlgorithmOutput::~ChannelDatavtkAlgorithmOutput()
{
    //m_ao->Delete();
    m_ao = 0;
}
////////////////////////////////////////////////////////////////////////////////
vtkAlgorithmOutput* ChannelDatavtkAlgorithmOutput::GetAlgorithmOutput()
{
    return m_ao;
}
////////////////////////////////////////////////////////////////////////////////
}
}
// lfx
}
