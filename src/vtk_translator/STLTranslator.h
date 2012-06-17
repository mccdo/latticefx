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
#ifndef STL_TRANSLATOR_H_
#define STL_TRANSLATOR_H_

#include <ves/builder/cfdTranslatorToVTK/cfdTranslatorToVTK.h>

class vtkDataSet;

namespace ves
{
namespace builder
{
namespace DataLoader
{
class VE_USER_BUILDER_EXPORTS STLTranslator:
    public ves::builder::cfdTranslatorToVTK::cfdTranslatorToVTK
{
public:
    STLTranslator();
    virtual ~STLTranslator();
    ///Display help for the Fluent translator
    virtual void DisplayHelp( void );
    //////////////////////////////////////////////////////
    class VE_USER_BUILDER_EXPORTS STLTranslateCbk: public ves::builder::cfdTranslatorToVTK::cfdTranslatorToVTK::TranslateCallback
    {
    public:
        STLTranslateCbk()
        {
            ;
        }
        virtual ~STLTranslateCbk()
        {
            ;
        }

        //////////////////////////////////////////////////
        //ouputDataset should be populated              //
        //appropriately by the translate callback.      //
        //////////////////////////////////////////////////
        virtual void Translate( vtkDataObject*& outputDataset,
                                cfdTranslatorToVTK* toVTK,
                                vtkAlgorithm*& dataReader );
    };
    //////////////////////////////////////////////////////
    class VE_USER_BUILDER_EXPORTS STLPreTranslateCbk:
        public ves::builder::cfdTranslatorToVTK::cfdTranslatorToVTK::PreTranslateCallback
    {
    public:
        STLPreTranslateCbk()
        {
            ;
        }
        virtual ~STLPreTranslateCbk()
        {
            ;
        }
        void Preprocess( int argc, char** argv, ves::builder::cfdTranslatorToVTK::cfdTranslatorToVTK* toVTK );
    };
protected:
    STLPreTranslateCbk cmdParser;
    STLTranslateCbk stlToVTK;
};
}
}
}
#endif//STL_TRANSLATOR_H_
