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
#ifndef PROCESS_SCALAR_RANGE_CALLBACK
#define PROCESS_SCALAR_RANGE_CALLBACK
/*!\file ProcessScalarRangeCallback.h
ProcessScalarRangeCallback API.
*/

/*!\class lfx::vtk_utils::ProcessScalarRangeCallback
*
*/
class vtkDataSet;

#include <latticefx/utils/vtk/Export.h>
#include <latticefx/utils/vtk/DataObjectHandler.h>

#include <vector>
#include <string>
#include <map>
namespace lfx
{
namespace vtk_utils
{
class LATTICEFX_VTK_UTILS_EXPORT ProcessScalarRangeCallback:
            public DataObjectHandler::DatasetOperatorCallback
{
public:
    ///Constructor
    ProcessScalarRangeCallback();
    ///Destructor
    virtual ~ProcessScalarRangeCallback();
    ///The operation to do on each vtkDataSet in the vtkDataObject
    ///\param dataset The vtkDataSet to operate on
    virtual void OperateOnDataset( vtkDataSet* dataset );

    ///Get scalar range for a specified scalar
    ///\param scalarName The name of the scalar to find the range
    ///\param  range The scalar range
    void GetScalarRange( const std::string& scalarName, double*& range );
protected:
    std::map<std::string, double* > m_scalarRanges;///<The scalar names
};
}// end of util namesapce
}// end of xplorer namesapce
#endif

