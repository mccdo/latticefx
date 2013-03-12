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
#ifndef __LATTICEFX_CHANNEL_DATA_VTKALGORITHMOUTPUT_H__
#define __LATTICEFX_CHANNEL_DATA_VTKALGORITHMOUTPUT_H__ 1


#include <latticefx/core/vtk/Export.h>
#include <latticefx/core/ChannelData.h>

#include <boost/shared_ptr.hpp>

class vtkAlgorithmOutput;

namespace lfx
{

namespace core
{

namespace vtk
{

/** \class ChannelDatavtkAlgorithmOutput ChannelDatavtkAlgorithmOutput.h <latticefx/core/vtk/ChannelDatavtkAlgorithmOutput.h>
\brief Container for ChannelDatavtkAlgorithmOutput objects
\details THis class holds ChannelDatavtkAlgorithmOutput */
class LATTICEFX_CORE_VTK_EXPORT ChannelDatavtkAlgorithmOutput : public lfx::core::ChannelData
{
public:

    ChannelDatavtkAlgorithmOutput( vtkAlgorithmOutput* const pd, const std::string& name = std::string( "" ) );
    ChannelDatavtkAlgorithmOutput( const ChannelDatavtkAlgorithmOutput& rhs );
    virtual ~ChannelDatavtkAlgorithmOutput();

    vtkAlgorithmOutput* GetAlgorithmOutput();

protected:
    vtkAlgorithmOutput* m_ao;
};

typedef boost::shared_ptr< ChannelDatavtkAlgorithmOutput > ChannelDatavtkAlgorithmOutputPtr;


}
}
// lfx
}


// __LATTICEFX_CHANNEL_DATA_VTKALGORITHMOUTPUT_H__
#endif
