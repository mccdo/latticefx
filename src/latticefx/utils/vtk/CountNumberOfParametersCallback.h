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
#ifndef COUNT_NUMBER_OF_PARAMETERS_CALLBACK
#define  COUNT_NUMBER_OF_PARAMETERS_CALLBACK
/*!\file CountNumberOfParametersCallback.h
CountNumberOfParametersCallback API.
*/

/*!\class lfx::vtk_utils::CountNumberOfParametersCallback
*
*/
class vtkDataSet;

#include <latticefx/utils/vtk/Export.h>
#include <latticefx/utils/vtk/DataObjectHandler.h>

#include <vector>
#include <string>
namespace lfx
{
namespace vtk_utils
{
class LATTICEFX_VTK_UTILS_EXPORT CountNumberOfParametersCallback:
            public DataObjectHandler::DatasetOperatorCallback
{
public:
    ///Constructor
    CountNumberOfParametersCallback();
    ///Destructor
    virtual ~CountNumberOfParametersCallback();
    ///The operation to do on each vtkDataSet in the vtkDataObject
    ///\param dataset The vtkDataSet to operate on
    virtual void OperateOnDataset( vtkDataSet* dataset );

    ///Get the number of vectors or scalars
    ///\param  isVector The determining which type to count
    unsigned int GetNumberOfParameters( bool isVector = false );

    ///Get the names of the scalars and vectors
    ///\param  isVector The determining which type to count
    std::vector<std::string> GetParameterNames( bool isVector = false );
protected:
    unsigned int m_numberOfParameters[2];///<The number of parameters in the vtkDataObject;
    std::vector<std::string> m_scalarNames;///<The scalar names
    std::vector<std::string> m_vectorNames;///<The vector names
};
}// end of util namesapce
}// end of xplorer namesapce
#endif

