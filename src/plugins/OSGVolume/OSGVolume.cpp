/*************** <auto-copyright.rb BEGIN do not edit this line> *************
*
* latticeFX is (C) Copyright 2011-2012 by Ames Laboratory
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Library General Public
* License as published by the Free Software Foundation; 
* version 2.1 of the License.
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
*************** <auto-copyright.rb END do not edit this line> **************/

#include <latticefx/PluginManager.h>
#include <Poco/ClassLibrary.h>

#include <latticefx/Preprocess.h>
#include <latticefx/ChannelDataOSGImage.h>



class ReduceLOD : public lfx::Preprocess
{
public:
    ReduceLOD()
        : lfx::Preprocess()
    {}
    ReduceLOD( const ReduceLOD& rhs )
        : lfx::Preprocess( rhs )
    {}
    virtual ~ReduceLOD()
    {}

    virtual lfx::OperationBase* create()
    {
        return( new ReduceLOD );
    }

protected:
};

// Register the MyPreprocess operation with the PluginManager
// This declares a static object initialized when the plugin is loaded.
REGISTER_OPERATION(
    new ReduceLOD(),
    ReduceLOD,
    "Preprocess",
    "Reduce stp dimensions by 1/2, creating a volume data set 1/8th original size."
)


// Poco ClassLibrary manifest registration. Add a POCO_EXPORT_CLASS
// for each class (operation) in the plugin.
POCO_BEGIN_MANIFEST( lfx::OperationBase )
    POCO_EXPORT_CLASS( ReduceLOD )
POCO_END_MANIFEST
