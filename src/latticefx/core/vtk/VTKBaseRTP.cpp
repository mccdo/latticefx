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
#include <latticefx/core/vtk/VTKBaseRTP.h>

namespace lfx {

namespace core {

namespace vtk {

////////////////////////////////////////////////////////////////////////////////
void VTKBaseRTP::SetMinMaxScalarRangeValue( double const minVal, double const maxVal )
{
    m_minScalarValue = minVal;
    m_maxScalarValue = maxVal;
}
////////////////////////////////////////////////////////////////////////////////
void VTKBaseRTP::SetRequestedValue( double const value )
{
    m_requestedValue = value;
}
////////////////////////////////////////////////////////////////////////////////
void VTKBaseRTP::SetActiveScalar( std::string const scalarName )
{
    m_activeScalar = scalarName;
}
////////////////////////////////////////////////////////////////////////////////
void VTKBaseRTP::SetActiceVector( std::string const vectorName )
{
    m_activeVector = vectorName;
}
////////////////////////////////////////////////////////////////////////////////
void VTKBaseRTP::SetMaskValue( double const value )
{
    m_mask = value;
}
////////////////////////////////////////////////////////////////////////////////
}
}
}
