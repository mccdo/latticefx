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
#ifndef _CFD_REI_TRANS_H_
#define _CFD_REI_TRANS_H_


#include <latticefx/translators/vtk/cfdTranslatorToVTK.h>
#include <set>

namespace lfx
{
namespace vtk_translator
{
class LATTICEFX_VTK_TRANSLATOR_EXPORT cfdREITranslator: public lfx::vtk_translator::cfdTranslatorToVTK
{
public:
    cfdREITranslator();
    virtual ~cfdREITranslator();
    ///Display help for the REI translator
    virtual void DisplayHelp( void );

    class LATTICEFX_VTK_TRANSLATOR_EXPORT REITranslatorCbk: public lfx::vtk_translator::cfdTranslatorToVTK::TranslateCallback
    {
    public:
        REITranslatorCbk()
        {};
        virtual ~REITranslatorCbk()
        {};
        //////////////////////////////////////////////////
        //ouputDataset should be populated              //
        //appropriately by the translate callback.      //
        //////////////////////////////////////////////////
        virtual void Translate( vtkDataObject*& outputDataset,
                                cfdTranslatorToVTK* toVTK,
                                vtkAlgorithm*& dataReader );

        int debug;
    protected:
    };
    class LATTICEFX_VTK_TRANSLATOR_EXPORT REIPreTranslatorCbk: public lfx::vtk_translator::cfdTranslatorToVTK::PreTranslateCallback
    {
    public:
        REIPreTranslatorCbk()
        {};
        virtual ~REIPreTranslatorCbk()
        {};
        //void Preprocess(int argc,char** argv,VE_Builder::cfdTranslatorToVTK* toVTK);
    protected:
    };
protected:
    REIPreTranslatorCbk _cmdParser;
    REITranslatorCbk _reiTranslator;
};
}
}
#endif//_CFD_REI_TRANS_H_
