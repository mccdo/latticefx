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
#ifndef __LATTICEFX_CHANNEL_DATA_VTKPOLYDATAMAPPER_H__
#define __LATTICEFX_CHANNEL_DATA_VTKPOLYDATAMAPPER_H__ 1


#include <latticefx/core/vtk/Export.h>
#include <latticefx/core/ChannelData.h>

#include <boost/shared_ptr.hpp>

class vtkPolyDataMapper;
class vtkAlgorithmOutput;

namespace lfx {

namespace core {
    
namespace vtk {

/** \class ChannelDatavtkPolyDataMapper ChannelDatavtkPolyDataMapper.h <latticefx/core/vtk/ChannelDatavtkPolyDataMapper.h>
\brief Container for ChannelDatavtkPolyDataMapper objects
\details THis class holds ChannelDatavtkPolyDataMapper */
class LATTICEFX_CORE_VTK_EXPORT ChannelDatavtkPolyDataMapper : public lfx::core::ChannelData
{
public:

    ChannelDatavtkPolyDataMapper( vtkAlgorithmOutput* const pd, const std::string& name=std::string( "" ) );
    ChannelDatavtkPolyDataMapper( const ChannelDatavtkPolyDataMapper& rhs );
    virtual ~ChannelDatavtkPolyDataMapper();

    vtkPolyDataMapper* GetPolyDataMapper();
    
protected:
    vtkPolyDataMapper* m_pdm;
};

typedef boost::shared_ptr< ChannelDatavtkPolyDataMapper > ChannelDatavtkPolyDataMapperPtr;


}
}
// lfx
}


// __LATTICEFX_CHANNEL_DATA_VTKPOLYDATAMAPPER_H__
#endif
