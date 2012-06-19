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
#ifndef OPENVTKTHING_H
#define OPENVTKTHING_H
/*!\file readWriteVtkThings.h
readWriteVtkThings API
*/
class vtkDataSet;
class vtkDataObject;
#include <latticefx/utils/vtk/Export.h>

namespace lfx
{
namespace vtk_utils
{
///Prints out (cout) the class of the object passed in.
///\param readerOutput The data set printWhatItIs identifies.
LATTICEFX_VTK_UTILS_EXPORT void printWhatItIs( vtkDataObject* readerOutput );
///Reads in a VTK data file and has the option to print what the data file is.
///\param vtkFilename The name of the VTK file to be read in.
///\param printFlag Flag to print class type. Default (0) is to not print, 1 is to pring.
LATTICEFX_VTK_UTILS_EXPORT vtkDataObject* readVtkThing( std::string vtkFilename, int printFlag = 0 );  //default is not to print information
///(????) Writes vtkThing out as vtkFilename.  Default is to print ASCII, binary is an option.
///\param vtkThing Dataset to be printed.
///\param vtkFilename The name of the file to be generated.
///\param binaryFlag Toggle between binary (1) and ASCII (0) output modes.
LATTICEFX_VTK_UTILS_EXPORT bool writeVtkThing( vtkDataObject* vtkThing, std::string vtkFilename, int binaryFlag = 0 );// default is to print ascii file
///Outputs the cartesian bounds of the data object passed in.
///\param dataObject The data set whose bounds are to be identified.
LATTICEFX_VTK_UTILS_EXPORT void printBounds( vtkDataObject* dataObject );//double bounds[6] );
}// end of util namesapce
}// end of xplorer namesapce
#endif
