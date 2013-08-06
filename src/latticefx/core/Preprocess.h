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

#ifndef __LFX_CORE_PREPROCESS_H__
#define __LFX_CORE_PREPROCESS_H__ 1


#include <latticefx/core/Export.h>
#include <latticefx/core/OperationBase.h>
#include <latticefx/core/ChannelData.h>

#include <boost/smart_ptr/shared_ptr.hpp>

#include <boost/serialization/access.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization//base_object.hpp>
#include <boost/serialization/nvp.hpp>

#include <list>



namespace lfx
{
namespace core
{


/** \class Preprocess Preprocess.h <latticefx/core/Preprocess.h>
\brief Base class for Preprocessing & Caching operations.
\details TBD
*/
class LATTICEFX_EXPORT Preprocess : public OperationBase
{
public:
    Preprocess();
    Preprocess( const Preprocess& rhs );
    virtual ~Preprocess();

	virtual std::string getClassName() const { return "Preprocess"; }

    /** \brief Indicates how DataSet should handle the newly created ChannelData.
    \details The operator() function returns a ChannelDataPtr of preprocessed data.
    The ActionType indicates how DataSet should handle the newly created data.
    \li ADD_DATA Add the new ChannelData to the DataSet.
    \li REPLACE_DATA Replace the Preprocess input with the new ChannelData. If the Preprocess has
    multiple inputs, the first input is replaced. Useful when creating a ChannelDataComposite.
    \li IGNORE_DATA Ignore the new ChannelData. This is useful if the Preprocess is simply storing
    the new data into the DB. */
    typedef enum
    {
        ADD_DATA,
        REPLACE_DATA,
        IGNORE_DATA
    } ActionType;

	std::string getEnumName( ActionType e ) const;
	ActionType getEnumFromName( const std::string &name ) const;

    /** \brief TBD
    \details The default is IGNORE_DATA. */
    void setActionType( const ActionType& action );
    /** \brief TBD
    \details TBD */
    ActionType getActionType() const;

    /** \brief Create and return a new ChannelData from inputs.
    \details The ReturnCode specifies how the owning DataSet should handle \c newData:
    add it to the DataSet, replace the first input with \c newData, or ignore
    \c newData. (IGNORE_DATA is useful if this function stores the data, in the DB for
    example.) */
    virtual ChannelDataPtr operator()()
    {
        return( ChannelDataPtr( ( ChannelData* )NULL ) );
    }

	virtual void dumpState( std::ostream &os );

protected:

	virtual void serializeData( JsonSerializer *json ) const;
	virtual bool loadData( JsonSerializer *json, IObjFactory *pfactory, std::string *perr=NULL );

protected:
    ActionType _action;


private:

	/*
    friend class boost::serialization::access;

    template< class Archive >
    void serialize( Archive& ar, const unsigned int version )
    {
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP( OperationBase );
        ar& BOOST_SERIALIZATION_NVP( _action );
    }
	*/
};

typedef boost::shared_ptr< Preprocess > PreprocessPtr;
typedef std::list< PreprocessPtr > PreprocessList;


// core
}
// lfx
}


BOOST_CLASS_VERSION( lfx::core::Preprocess, 0 );


// __LFX_CORE_PREPROCESS_H__
#endif
