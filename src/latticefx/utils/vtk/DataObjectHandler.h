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
#ifndef LFX_DATA_OBJECT_HANDLER_H
#define LFX_DATA_OBJECT_HANDLER_H
/*!\file DataObjectHandler.h
 * DataObjectHandler API. At a minimum, converts cell data to point data in each dataset of
 * the data object.
 * \class lfx::vtk_utils::DataObjectHandler
 *
 */
class vtkDataObject;
class vtkDataSet;

#include <latticefx/utils/vtk/Export.h>


namespace lfx
{
namespace vtk_utils
{
class LATTICEFX_VTK_UTILS_EXPORT DataObjectHandler
{
public:
    ///Constructor
    DataObjectHandler();
    ///Destructor
    virtual ~DataObjectHandler();

    /*!\class lfx::vtk_utils::DataObjectHandler::DatasetOperatorCallback
    *
    */
    class LATTICEFX_VTK_UTILS_EXPORT DatasetOperatorCallback
    {
    public:
        ///Constructor
        DatasetOperatorCallback()
        {
            m_isMultiBlock = false;
        }
        ///Destructor
        virtual ~DatasetOperatorCallback(){;}

        ///Set the type of dataset
        void SetIsMultiBlockDataset( bool multiBlock )
        {
            m_isMultiBlock = multiBlock;
        }
        
        ///The operation to do on each vtkDataSet in the vtkDataObject
        ///\param dataset The vtkDataSet to operate on
        virtual void OperateOnDataset( vtkDataSet* dataset ) = 0;
    protected:
        bool m_isMultiBlock;
    };
    ///Do the operation specified in the DataObjectHandlerCallback\n
    ///on a vtkDataObject
    void OperateOnAllDatasetsInObject( vtkDataObject* dataObject );
    ///Set the operation to perform on each dataset in the vtkDataObject
    ///Caller is responsible for cleaning up DatasetOperatorCallback memory
    ///\param dsoCbk The DataSetOperatorCallback
    void SetDatasetOperatorCallback( DatasetOperatorCallback* dsoCbk );

    ///Get the number of cell or point data arrays
    ///\param isPointData Return point or cell data array count
    unsigned int GetNumberOfDataArrays( bool isPointData = true );
protected:
    ///Convert the cell data to point data if it exists
    ///\param dataset The vtkDataSet
    void _convertCellDataToPointData( vtkDataSet* dataset );

    ///Update the variables for PD arrays for the respective dataset
    ///\param dataSet The current dataset
    void UpdateNumberOfPDArrays( vtkDataSet* dataSet );

    ///<Number of point data arrays
    unsigned int m_numberOfPointDataArrays;
    ///<Number of cell data arrays
    unsigned int m_numberOfCellDataArrays;
    DatasetOperatorCallback* m_datasetOperator;///<The DatasetOperatorCallback.
};
}// end of util namesapce
}// end of xplorer namesapce
#endif
