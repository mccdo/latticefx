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

#include <latticefx/RTPOperation.h>
#include <latticefx/Preprocess.h>
#include <latticefx/ChannelDataOSGArray.h>



class MyMask : public lfx::RTPOperation
{
public:
    MyMask()
      : lfx::RTPOperation( lfx::RTPOperation::Mask )
    {}
    virtual ~MyMask()
    {}

    virtual lfx::OperationBase* create()
    {
        return( new MyMask );
    }

    virtual lfx::ChannelDataPtr mask( const lfx::ChannelDataPtr maskIn )
    {
        lfx::ChannelDataOSGArrayPtr cdp( new lfx::ChannelDataOSGArray() );
        return( cdp );
    }

protected:
};

// Register the MyMask operation with the PluginManager
// This declares a static object initialized when the plugin is loaded.
REGISTER_OPERATION(
    new MyMask(),    // Create an instance of MyMask.
    MyMask,          // Class name -- NOT a string.
    "RTPOperation",  // Base class name as a string.
    "Test mask."     // Description text.
)



class MyPreprocess : public lfx::Preprocess
{
public:
    MyPreprocess()
        : lfx::Preprocess()
    {}
    virtual ~MyPreprocess()
    {}

    virtual lfx::OperationBase* create()
    {
        return( new MyPreprocess );
    }

protected:
};

// Register the MyPreprocess operation with the PluginManager
// This declares a static object initialized when the plugin is loaded.
REGISTER_OPERATION(
    new MyPreprocess(), // Create an instance of MyPreprocess.
    MyPreprocess,       // Class name -- NOT a string.
    "Preprocess",       // Base class name as a string.
    "Test preprocess."  // Description text.
)


// Poco ClassLibrary manifest registration. Add a POCO_EXPORT_CLASS
// for each class (operation) in the plugin.
POCO_BEGIN_MANIFEST( lfx::OperationBase )
    POCO_EXPORT_CLASS( MyMask )
    POCO_EXPORT_CLASS( MyPreprocess )
POCO_END_MANIFEST
