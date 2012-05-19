/*************** <auto-copyright.rb BEGIN do not edit this line> **************
 *
 * VE-Suite is (C) Copyright 1998-2012 by Iowa State University
 *
 * Original Development Team:
 *   - ISU's Thermal Systems Virtual Engineering Group,
 *     Headed by Kenneth Mark Bryden, Ph.D., www.vrac.iastate.edu/~kmbryden
 *   - Reaction Engineering International, www.reaction-eng.com
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
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
 *************** <auto-copyright.rb END do not edit this line> ***************/
#ifndef COMPUTE_MEAN_CELL_BBOX_LENGTH_CALLBACK
#define  COMPUTE_MEAN_CELL_BBOX_LENGTH_CALLBACK
/*!\file ComputeMeanCellBBoxLengthCallback.h
ComputeMeanCellBBoxLengthCallback API.
*/

/*!\class ves::xplorer::util::ComputeMeanCellBBoxLengthCallback
*
*/
class vtkDataSet;

#include <ves/VEConfig.h>
#include <ves/xplorer/util/DataObjectHandler.h>


namespace ves
{
namespace xplorer
{
namespace util
{
class VE_UTIL_EXPORTS ComputeMeanCellBBoxLengthCallback:
            public DataObjectHandler::DatasetOperatorCallback
{
public:
    ///Constructor
    ComputeMeanCellBBoxLengthCallback();
    ///Destructor
    virtual ~ComputeMeanCellBBoxLengthCallback()
    {};
    ///The operation to do on each vtkDataSet in the vtkDataObject
    ///\param dataset The vtkDataSet to operate on
    virtual void OperateOnDataset( vtkDataSet* dataset );

    ///Get the Mean Cell BBox length
    double GetMeanCellBBLength();
protected:
    double m_meanCellBBLength;///<The mean cell bbox length;
};
}// end of util namesapce
}// end of xplorer namesapce
}// end of ves namesapce
#endif

