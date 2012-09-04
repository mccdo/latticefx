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

#ifndef __LFX_CORE_LOG_BASE_H__
#define __LFX_CORE_LOG_BASE_H__ 1


#include <Poco/Logger.h>
#include <Poco/LogStream.h>

#include <boost/shared_ptr.hpp>


namespace lfx {
namespace core {


typedef boost::shared_ptr< Poco::LogStream > PocoLogStreamPtr;


/** \addtogroup lfxCoreLogging Message Logging Utilities */
/*@{*/


/** \class LogBase LogBase.h <latticefx/core/LogBase.h>
\brief Enables message logging in Lfx classes.
\details Keeps a reference to a Poco::Logger and a smart pointer to a Poco LogStream.
Derived classes pass the Logger name as a parameter to the LogBase constructor. All
classes using the same Logger name share a reference to that Logger.

Classes that derive from LogBase can use the macros in latticefx/core/LogMacros.h. See
\ref lfxCoreLogging "Message Logging Utilities" for more information.
*/
class LogBase
{
public:
    LogBase( const std::string& loggerName )
#ifdef LFX_DISABLE_LOGGING
      : _logStream( PocoLogStreamPtr( NULL ) )
#else
      : _logger( Poco::Logger::get( loggerName ) ),
        _logStream( PocoLogStreamPtr( new Poco::LogStream( _logger ) ) )
#endif
    {}
    LogBase( const LogBase& rhs )
      :
#ifndef LFX_DISABLE_LOGGING
        _logger( rhs._logger ),
#endif
        _logStream( rhs._logStream )
    {}

    ~LogBase() {}

#ifndef LFX_DISABLE_LOGGING
    Poco::Logger& _logger;
#endif
    PocoLogStreamPtr _logStream;
};

/*@}*/


// core
}
// lfx
}


// __LFX_CORE_LOG_BASE_H__
#endif
