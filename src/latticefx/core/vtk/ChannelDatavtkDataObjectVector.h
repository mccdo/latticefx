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

#ifndef __LATTICEFX_CHANNEL_DATA_VTKDATAOBJECTVECTOR_H__
#define __LATTICEFX_CHANNEL_DATA_VTKDATAOBJECTVECTOR_H__ 1


#include <latticefx/core/vtk/Export.h>
#include <latticefx/core/ChannelData.h>

#include <boost/shared_ptr.hpp>

class vtkDataObject;

namespace lfx
{

namespace core
{

namespace vtk
{

class LATTICEFX_CORE_VTK_EXPORT ChannelDatavtkDataObjectVector : public lfx::core::ChannelData
{
public:
    ChannelDatavtkDataObjectVector( std::vector< vtkDataObject* > dobj, const std::string& name = std::string( "" ) );
    ChannelDatavtkDataObjectVector( const ChannelDatavtkDataObjectVector& rhs );
    virtual ~ChannelDatavtkDataObjectVector();

    std::vector< vtkDataObject* > GetDataObjectVector();

    /*unsigned int GetNumberOfPoints();
    double* GetBounds();
    void GetBounds( double* bounds );
    void GetScalarRange( std::string const scalarName, double* scalarRange );*/

protected:
    std::vector< vtkDataObject* > m_dobj;
    double m_bounds[ 6 ];
};

typedef boost::shared_ptr< ChannelDatavtkDataObjectVector > ChannelDatavtkDataObjectVectorPtr;

}
}
// lfx
}


// __LATTICEFX_CHANNEL_DATA_VTKDATAOBJECTVECTOR_H__
#endif
