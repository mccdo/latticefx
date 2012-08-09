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

#ifndef __LFX_CORE_OPERATION_BASE_H__
#define __LFX_CORE_OPERATION_BASE_H__ 1


#include <latticefx/core/Export.h>
#include <latticefx/core/OperationValue.h>
#include <latticefx/core/ChannelData.h>

#include <osg/Vec3>

#include <boost/smart_ptr/shared_ptr.hpp>

#include <list>
#include <map>


namespace lfx {
namespace core {


// Forward declaration.
class DataSet;


/** \class OperationBase OperationBase.h <latticefx/core/OperationBase.h>
\brief Base class for Preprocessing & Caching, Run-Time Processing, and Rendering Framework operations.
\details Common interface for all DataSet data operations, and the primary interface
for operations loaded via the PluginManager.

OperationBase contains the following features:
\li A OperationType to specify the class's usage as either Preprocessing & Caching, Run-TIme
Processing, or Rendering Framework.
\li A generic creation mechanism for use by plugin-loaded classes.
\li A set of ChannelData inputs used by the OperationBase-derived class, and methods for
accessing those inputs.
\li An enable/disable flag.
\li A generic name-value pair API for run-time configuration.
*/
class LATTICEFX_EXPORT OperationBase
{
public:
    typedef enum {
        UnspecifiedType,
        PreprocessCacheType,
        RunTimeProcessingType,
        RendererType
    } OperationType;
    /** \brief */
    OperationType getType() const { return( _opType ); }

    OperationBase( const OperationType opType=UnspecifiedType );
    OperationBase( const OperationBase& rhs );
    virtual ~OperationBase();

    /** \brief Create and return a new instance.
    \details Called by PluginManager to create an instance of classes
    defined in plugin libraries. All classes loaded by plugins must
    override and implement this method. */
    virtual OperationBase* create() { return( NULL ); }

    typedef std::vector< std::string > StringList;
    /** \brief Add an input by name.
    \details \c name must match the name of a ChannelData added to the DataSet. */
    virtual void addInput( const std::string& name );
    /** \brief Retrieve the ChannelData corresponding to the named input.
    \details Before invoking the OperationBase, DataSet uses the input names to
    assign actual ChannelData inputs. Use this method to retrieve such an input. */
    virtual ChannelDataPtr getInput( const std::string& name );
    /** \brief Add all inputs by name.
    \details All names in \c inputList must match the names of ChannelData objects added to the DataSet. */
    virtual void setInputs( StringList& inputList );
    /** \brief Get all input names. */
    virtual StringList getInputNames();
    /** \overload StringList OperationBase::getInputNames() */
    virtual const StringList& getInputNames() const;


    /** \brief Enable or disable the operation.
    \details Derived classes should adhere to this parameter and disable
    processing if requested to do so. */
    virtual void setEnable( const bool enable=true );
    /** \brief Get the enable/disable state. */
    virtual bool getEnable() const;


    /** \name Name-Value Pair Interface
    \details Allows run-time configuration of an OperationBase without compile-time
    knowledge of the derived class interface. The name-value pair API is particularly
    useful for passing parameters to classes loaded from plugins. */
    /**@{*/

    /** \brief Store a \c value associated with \c name.
    \details Stores the \c value in \c _nameValueMap indexed by \c name. */
    void setValue( const std::string& name, const OperationValue& value );
    /** \brief Check for existance of a name-value pair.
    \details If \c _nameValueMap has a value for \c name, return true. Otherwise,
    return false. */
    bool hasValue( const std::string& name ) const;
    /** \brief Get the value for a name. 
    \details Use \c name to look up a value in \c _nameValueMap and return the address
    of that value. If \c _nameValueMap doesn't contain \c name, return NULL. */
    const OperationValue* getValue( const std::string& name ) const;

    /**@}*/

protected:
    friend class DataSet;
    virtual void addInput( ChannelDataPtr input );
    virtual void setInputs( ChannelDataList inputList );
    virtual ChannelDataList getInputs();

    /** List of actual inputs. During data processing, DataSet assigns ChannelData
    objects to this array based on names stored in _inputNames. */
    ChannelDataList _inputs;
    /** List of input names assigned by the application. */
    StringList _inputNames;

    OperationType _opType;
    bool _enable;
    NameValueMap _nameValueMap;
};

typedef boost::shared_ptr< OperationBase > OperationBasePtr;


// core
}
// lfx
}


// __LFX_CORE_OPERATION_BASE_H__
#endif
