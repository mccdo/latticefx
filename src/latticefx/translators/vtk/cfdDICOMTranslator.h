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
#ifndef _CFD_DICOM_TRANSLATOR_H_
#define _CFD_DICOM_TRANSLATOR_H_


#include <latticefx/translators/vtk/cfdTranslatorToVTK.h>

namespace lfx
{
namespace vtk_translator
{
class LATTICEFX_VTK_TRANSLATOR_EXPORT cfdDICOMTranslator:
    public lfx::vtk_translator::cfdTranslatorToVTK
{

public:
    cfdDICOMTranslator();
    virtual ~cfdDICOMTranslator();
    ///Display help for the DICOM translator
    virtual void DisplayHelp( void );

    class LATTICEFX_VTK_TRANSLATOR_EXPORT DICOMTranslateCbk: public lfx::vtk_translator::cfdTranslatorToVTK::TranslateCallback
    {
    public:
        DICOMTranslateCbk()
        {};
        virtual ~DICOMTranslateCbk()
        {};
        //////////////////////////////////////////////////
        //ouputDataset should be populated              //
        //appropriately by the translate callback.      //
        //////////////////////////////////////////////////
        virtual void Translate( vtkDataObject*& outputDataset,
                                cfdTranslatorToVTK* toVTK,
                                vtkAlgorithm*& dataReader );

    protected:
    };
    class LATTICEFX_VTK_TRANSLATOR_EXPORT DICOMPreTranslateCbk: public lfx::vtk_translator::cfdTranslatorToVTK::PreTranslateCallback
    {
    public:
        DICOMPreTranslateCbk()
        {};
        virtual ~DICOMPreTranslateCbk()
        {};
    protected:
    };
protected:
    DICOMPreTranslateCbk _cmdParser;
    DICOMTranslateCbk _dicomToVTK;
};
}
}
#endif//_CFD_DICOM_TRANSLATOR_H_
