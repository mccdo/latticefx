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

#ifndef __LFX_CORE_OPERATION_BASE_H__
#define __LFX_CORE_OPERATION_BASE_H__ 1


#include <latticefx/core/Export.h>
#include <latticefx/core/OperationValue.h>
#include <latticefx/core/ChannelData.h>
#include <latticefx/core/DBBase.h>
#include <latticefx/core/ObjBase.h>

#include <osg/Vec3>

#include <boost/smart_ptr/shared_ptr.hpp>

#include <boost/serialization/access.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/map.hpp>

#include <list>
#include <map>


namespace lfx
{
namespace core
{


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
accessing those inputs. OperationBase also supports an input ChannelData
name alias mechanism; see below.
\li An enable/disable flag.
\li A generic name-value pair API for run-time configuration.

It would be impractical to require applications to set the name of input
ChannelData according to the demands of classes derived from OperationBase.
For this reason, OperationBase exposes an interface that allows applications
to specify arbitrarily name aliases for input ChannelData. The supported
(int or enum) names and their default name string values are defined by
each derived class.

For example, VectorRenderer supports input from a ChannelData named
POSITION. if your position ChannelData is named "MyXYZPositionData",
specify an aloas as follows:

\code
    // 'renderop' is a VectorRendererPtr
    renderop->setInputNameAlias( VectorRenderer::POSITION, "MyXYZPositionData" );
\endcode

*/
class LATTICEFX_EXPORT OperationBase : public ObjBase
{
public:
    typedef enum
    {
        UnspecifiedType,
        PreprocessCacheType,
        RunTimeProcessingType,
        RendererType
    } OperationType;

	std::string getEnumName( OperationType e ) const;
	OperationType getEnumFromName( const std::string &name ) const;

    /** \brief */
    OperationType getType() const
    {
        return( _opType );
    }

    OperationBase( const OperationType opType = UnspecifiedType );
    OperationBase( const OperationBase& rhs );
    virtual ~OperationBase();

	virtual std::string getClassName() const { return std::string( "OperationBase" ); }
	virtual void setPluginData( const std::string &pluginName, const std::string &pluginClassName );


    /** \brief Create and return a new instance.
    \details Called by PluginManager to create an instance of classes
    defined in plugin libraries. All classes loaded by plugins must
    override and implement this method. */
    virtual OperationBase* create()
    {
        return( NULL );
    }

    /**\name ChannelData input support.
    */
    /**@{*/

    typedef std::vector< std::string > StringList;
    /** \brief Add an input by name.
    \details \c name must match the name of a ChannelData added to the DataSet. */
    virtual void addInput( const std::string& name );
    /** \brief Retrieve the ChannelData corresponding to the named input.
    \details Before invoking the OperationBase, DataSet uses the input names to
    assign actual ChannelData inputs. Use this method to retrieve such an input. */
    virtual ChannelDataPtr getInput( const std::string& name );
    /** \overload ChannelDataPtr OperationBase::getInput( const std::string& ); */
    virtual const ChannelDataPtr getInput( const std::string& name ) const;
    /** \brief Add all inputs by name.
    \details All names in \c inputList must match the names of ChannelData objects added to the DataSet. */
    virtual void setInputs( StringList& inputList );
    /** \brief Get all input names. */
    virtual StringList getInputNames();
    /** \overload StringList OperationBase::getInputNames() */
    virtual const StringList& getInputNames() const;

    /** \brief Associate a ChannelData name with an integer value.
    \details This method allows the application to use arbitrarily named ChannelData
    with any OperationBase. Classes deriving from OperationBase should
    define an integer-valued enum, so that application developers can pass
    the enum name for \c inputType rather than a const int literal. */
    void setInputNameAlias( const int inputType, const std::string& alias );
    /** \brief Get the ChannelData name alias for the specified \c inputType. */
    std::string getInputNameAlias( const int inputType ) const;

    /**@}*/


    /** \brief Enable or disable the operation.
    \details Derived classes should adhere to this parameter and disable
    processing if requested to do so. */
    virtual void setEnable( const bool enable = true );
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


    /** \name DB Access
    \details For derived classes that access a database, the application must specify
    the database using setDB().
    
    Some Renderers, for example, create the scene graph differently when the DB in
    non-NULL. VolumeRenderer creates stub Texture3D objects when the DB is non-NULL,
    to be paged in at runtime by the PagingCallback. When the DB is NULL,
    VolumeRenderer stores the full Texture3D data in the scene graph and no paging
    occurs.

    Other classes have similar behavior, but how they respond to the presence or
    absence of a DB is claaa-dependent. */
    /**@{*/

    /** \brief Set the DB */
    void setDB( DBBasePtr db )
    {
        _db = db;
    }
    /** \brief Get the DB */
    DBBasePtr getDB() const
    {
        return( _db );
    }

	virtual void dumpState( std::ostream &os );

    /**@}*/

protected:
    friend class DataSet;
    virtual void addInput( ChannelDataPtr input );
    virtual void setInputs( ChannelDataList inputList );
    virtual bool validInputs() const
    {
        return( true );
    }
    virtual ChannelDataList getInputs();


	virtual void serializeData( JsonSerializer *json ) const;
	virtual bool loadData( JsonSerializer *json, IObjFactory *pfactory, std::string *perr=NULL );

    /** List of actual inputs. During data processing, DataSet assigns ChannelData
    objects to this array based on names stored in _inputNames. */
    ChannelDataList _inputs;
    /** List of input names assigned by the application. */
    StringList _inputNames;

    /** Map of integer aliases to input ChannelData names. */
    typedef std::map< int, std::string > InputTypeMap;
    /** Map of integer aliases to input ChannelData names. */
    InputTypeMap _inputTypeMap;

    OperationType _opType;
    bool _enable;
    NameValueMap _nameValueMap;

    DBBasePtr _db;

private:

	/*
    friend class boost::serialization::access;

    template< class Archive >
    void serialize( Archive& ar, const unsigned int version )
    {
        ar& BOOST_SERIALIZATION_NVP( _opType );
        ar& BOOST_SERIALIZATION_NVP( _enable );
        // TBD serialize - need to serialize OperationValue first.
        //ar & BOOST_SERIALIZATION_NVP( _nameValueMap );
    }
	*/
};

typedef boost::shared_ptr< OperationBase > OperationBasePtr;


// core
}
// lfx
}


BOOST_CLASS_VERSION( lfx::core::OperationBase, 0 );


// __LFX_CORE_OPERATION_BASE_H__
#endif
