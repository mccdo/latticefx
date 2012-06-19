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
#ifndef CFD_ACCESSORY_FUNCTIONS_H
#define CFD_ACCESSORY_FUNCTIONS_H

/*!file AccessoryFunctions.h
AccessoryFunctions API
*/
/*!class lfx::vtk_utils::AccessoryFunctions
*This class returns information on an input vector.
*/

class vtkDataArray;
class vtkDataSet;
#include <vtk_utils/Export.h>

namespace lfx
{
namespace vtk_utils
{
class LATTICEFX_VTK_UTILS_EXPORT AccessoryFunctions
{
public:
    ///Constructor
    AccessoryFunctions();
    ///Destructor
    ~AccessoryFunctions();
    ///Find the range of the magnitude of the vector for vector-based
    ///visualization when "scale by vector magnitude" is selected.
    ///\param dataArray The vector to be measured
    static double * ComputeVectorMagnitudeRange( vtkDataArray * dataArray );
    ///Find the Mean Cell Bounding Box (????) Length
    ///\param dataSet The values for the grid that is being analyzed.
    static double ComputeMeanCellBBLength( vtkDataSet * dataSet );


private:
    ///Find the magnitude of the input vector
    ///\param vectorComponents The three dimensional array holding the
    ///components of the vector being evaluated
    static double ComputeVectorMagnitude( double vectorComponents [ 3 ] );
};
}// end of util namesapce
}// end of xplorer namesapce
#endif
