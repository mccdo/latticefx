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

#ifndef __LFX_CORE_DB_OPERATIONS_H__
#define __LFX_CORE_DB_OPERATIONS_H__ 1

#include <latticefx/core/Export.h>
#include <latticefx/core/Preprocess.h>
#include <latticefx/core/RTPOperation.h>
#include <latticefx/core/LogBase.h>
#include <latticefx/core/DBBase.h>
#include <latticefx/core/types.h>

#include <boost/shared_ptr.hpp>


namespace lfx
{
namespace core
{


/** \class DBLoad DBOperations.h <latticefx/core/DBOperations.h>
\brief A Preprocess object to load a ChannelData from DB.
\details TBD */
class LATTICEFX_EXPORT DBLoad : public Preprocess, protected LogBase
{
public:
    DBLoad( const DBBasePtr db, const DBKey& key, const std::string& channelName );
    DBLoad( const DBLoad& rhs );
    virtual ~DBLoad();


    /** \brief TBD
    \details Override from Preprocess base class. */
    virtual ChannelDataPtr operator()();

protected:
    DBKey _key;
    std::string _channelName;

private:
};

typedef boost::shared_ptr< DBLoad > DBLoadPtr;



/** \class DBSave DBOperations.h <latticefx/core/DBOperations.h>
\brief A RTPOperation object to save a ChannelData to DB.
\details TBD */
class LATTICEFX_EXPORT DBSave : public RTPOperation, protected LogBase
{
public:
    DBSave( DBBasePtr db );
    DBSave( const DBSave& rhs );
    virtual ~DBSave();


    /** \brief TBD
    \details Override from RTPOperation base class. */
    virtual void filter( const ChannelDataPtr );

protected:

private:
};

typedef boost::shared_ptr< DBSave > DBSavePtr;


// core
}
// lfx
}


// __LFX_CORE_DB_OPERATIONS_H__
#endif
