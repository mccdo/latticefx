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

#ifndef __LFX_CORE_LOG_MACROS_H__
#define __LFX_CORE_LOG_MACROS_H__ 1


#include <Poco/Logger.h>
#include <Poco/LogStream.h>


/** \addtogroup lfxCoreLogging Message Logging Utilities
\details LatticeFX's message logging system is based on
<a href="http://pocoproject.org/slides/110-Logging.pdf" target=null>Poco</a>
and is modeled after
<a href="http://www.vesuite.org/" target=null>VE-Suite's</a> message logging system.

Lfx's modules and classes are arranged in a heirarchy to allow message destination (console
or log file) and verbosity on a per-module and per-class basis. The heirarchy is arranged
as follows:
<ul>
  <li>lfx
  <ul>
    <li>core (the lfx core module)
    <ul>
      <li>\link lfx::core::Log log \endlink
      <li>\link lfx::core::PluginManager plugmgr \endlink
      <li>\link lfx::core::VectorRenderer vec \endlink
    </ul>
    <li>db Database utilities
    <li>foo (the foo module)
    <ul>
      <li>\link foo::FooClass fooclass \endlink
    </ul>
    <li>apps (applications)
    <li>demo (demos, tests, examples)
    <ul>
      <li>various demos and tests
    </ul>
  </ul>
</ul>

Heirachy levels are separated by a period, so the name for the Lfx core ChannelData Poco::Logger is
\c lfx.core.channel.

Priority level match those used by Poco and are defined as follows:
\li 0 - Silent (no message logging)
\li 1 - Fatal
\li 2 - Critical
\li 3 - Error
\li 4 - Warning
\li 5 - Notice
\li 6 - Info
\li 7 - Debug
\li 8 - Trace

Set a Poco::Logger's priority and destination (console or log file) using the lfx::core::Log class.
\code
    // Set the global priority to 3 (error and above) and destination to Console.
    // There is an implicit 3rd parameter of "lfx".
    lfx::core::Log::instance()->setPriority( 3, lfx::core::Log::Console );

    // Set the lfx core module to priority 3 (error and above)
    lfx::core::Log::instance()->setPriority( 4, "lfx.core" );
\endcode

Poco Logger priority and destination inherit down the heirarchy. Your application should
set specific priority and destinations (as in the code above) at init time, before
Lfx class constructors obtain references to subordinate Loggers.

By default, the root Logger ("lfx") is configured with priority 0 (silent) and
destination lfx::core::Log::Console.

The default log file name is "lfx.log" and Lfx writes it to the current directory.
You can change the default log file name with either of the following methods:
\li Call lfx::core::Log::setLogFileName( const std::string& ).
\li Set the environment variable LFX_LOG_FILE_NAME to the path and file name prior
to invoking lfx::core::Log::instance().

Lfx internal code uses the macros defined in latticefx/core/logMatrix.h to display log messages at
verious levels. When using these macros, please observe the following rules of thumb:
\li Use the LFX_LOG_<prio> macros to avoid expensive message construction for messages that will not be logged due to their priority.
\li Use the LFX_<prio> macros from member functions of classes that derive from lfx::core::LogBase.
\li Use the LFX_<prio>_STATIC macros from static functions, or from member functions of classes that don't derive from lfx::core::LogBase.

Note that, even with priority set to 0, there is some non-zero overhead in handling
message logging. To eliminate all Lfx message logging, set the CMake variable
LFX_DISABLE_LOGGING to ON (or select its checkbox in cmake-gui). When this variable
is on, Lfx's message logging facilities become no-ops and have zero computational cost.

*/
/*@{*/


/** \def __LFX_LOG
\brief Shared internal logging macro.
\details Internal macro for efficient message logging for classes derived
from lfx::core::LogBase. (Assumes member or local variables \c _logger and \c _logStream.)

This function is a no-op if LFX_DISABLE_LOGGING is defined. The definition of
LFX_DISABLE_LOGGING is controlled using CMake.
\param prio Message priority based on Poco/Message.h.
\param msg Written to the \c _logStream if the \c _logger's priority is <= \c prio.
*/
/** \def __LFX_LOG_STATIC
\brief Shared internal logging macro.
\details Internal macro for message logging within static functions or classes
not derived from lfx::core::LogBase.

This function is a no-op if LFX_DISABLE_LOGGING is defined. The definition of
LFX_DISABLE_LOGGING is controlled using CMake.
\param prio Message priority based on Poco/Message.h.
\param name Poco::Logger name.
\param msg Written to named Logger if the Logger's priority is <= \c prio.
*/
#ifdef LFX_DISABLE_LOGGING
#  define __LFX_LOG( prio, msg )
#  define __LFX_LOG_STATIC( prio, name, msg )
#else
#  define __LFX_LOG( prio, msg ) \
    if( _logger.prio() ) \
        (*_logStream).prio() << ( msg ) << std::endl;
#  define __LFX_LOG_STATIC( prio, name, msg ) \
    { \
        Poco::Logger& logger = Poco::Logger::get( name ); \
        if( logger.prio() ) \
        { \
            Poco::LogStream logstream( logger ); \
            logstream.prio() << ( msg ) << std::endl; \
        } \
    }
#endif


/** \def LFX_LOG_FATAL
\brief Boolean for quick abort of log message construction.
\details Constructing anything other than a trivial log message is typically an expensive
operation involving string concatenation, std::ostringstream writes, and other string
manipulation. Use this macro to check the Poco::Logger priority and avoid log message
construction when the log level would prevent display of the message.

If LFX_DISABLE_LOGGING is defined, LFX_LOG_FATAL is a runtime constane \c false. An optimizing
compiler will recognize and eliminate log message construction as dead code. For example:
\code
    if( LFX_LOG_FATAL )
    {
        // Log message construction
        LFX_FATAL( msg );
    }
\endcode
becomes this:
\code
    if( false )
    {
        // Dead code block, optimized away at compile time.
    }
\endcode
*/
/** \def LFX_LOG_CRITICAL
\see LFX_LOG_FATAL */
/** \def LFX_LOG_ERROR
\see LFX_LOG_FATAL */
/** \def LFX_LOG_WARNING
\see LFX_LOG_FATAL */
/** \def LFX_LOG_NOTICE
\see LFX_LOG_FATAL */
/** \def LFX_LOG_INFO
\see LFX_LOG_FATAL */
/** \def LFX_LOG_DEBUG
\see LFX_LOG_FATAL */
/** \def LFX_LOG_TRACE
\see LFX_LOG_FATAL */
#ifdef LFX_DISABLE_LOGGING

#  define LFX_LOG_FATAL     false
#  define LFX_LOG_CRITICAL  false
#  define LFX_LOG_ERROR     false
#  define LFX_LOG_WARNING   false
#  define LFX_LOG_NOTICE    false
#  define LFX_LOG_INFO      false
#  define LFX_LOG_DEBUG     false
#  define LFX_LOG_TRACE     false

#else

/** \def __LFX_LOG_PRIO_CHECK
\brief Internal Boolean macro for checking \c _logger member variable logging priority.
*/
#  define __LFX_LOG_PRIO_CHECK( __prio )    ( _logger.__prio() )

#  define LFX_LOG_FATAL     __LFX_LOG_PRIO_CHECK( fatal )
#  define LFX_LOG_CRITICAL  __LFX_LOG_PRIO_CHECK( critical )
#  define LFX_LOG_ERROR     __LFX_LOG_PRIO_CHECK( error )
#  define LFX_LOG_WARNING   __LFX_LOG_PRIO_CHECK( warning )
#  define LFX_LOG_NOTICE    __LFX_LOG_PRIO_CHECK( notice )
#  define LFX_LOG_INFO      __LFX_LOG_PRIO_CHECK( information )
#  define LFX_LOG_DEBUG     __LFX_LOG_PRIO_CHECK( debug )
#  define LFX_LOG_TRACE     __LFX_LOG_PRIO_CHECK( trace )

#endif


/** \def LFX_FATAL
\brief Efficient message logging macros.
\details Efficient message logging for classes derived from lfx::core::LogBase.
(Assumes member or local variables \c _logger and \c _logStream.)
*/
/** \def LFX_CRITICAL
\see LFX_FATAL */
/** \def LFX_ERROR
\see LFX_FATAL */
/** \def LFX_WARNING
\see LFX_FATAL */
/** \def LFX_NOTICE
\see LFX_FATAL */
/** \def LFX_INFO
\see LFX_FATAL */
/** \def LFX_DEBUG
\see LFX_FATAL */
/** \def LFX_TRACE
\see LFX_FATAL */
#define LFX_FATAL( __msg )     __LFX_LOG( fatal, __msg )
#define LFX_CRITICAL( __msg )  __LFX_LOG( critical, __msg )
#define LFX_ERROR( __msg )     __LFX_LOG( error, __msg )
#define LFX_WARNING( __msg )   __LFX_LOG( warning, __msg )
#define LFX_NOTICE( __msg )    __LFX_LOG( notice, __msg )
#define LFX_INFO( __msg )      __LFX_LOG( information, __msg )
#define LFX_DEBUG( __msg )     __LFX_LOG( debug, __msg )
#define LFX_TRACE( __msg )     __LFX_LOG( trace, __msg )

/** \def LFX_FATAL_STATIC
\brief Message logging macros.
\details Message logging for static functions or classes not derived from lfx::core::LogBase.
These macros obtain a Logger from Poco's global map and construct a Poco::LogStream
on the fly. */
/** \def LFX_CRITICAL_STATIC
\see LFX_FATAL_STATIC */
/** \def LFX_ERROR_STATIC
\see LFX_FATAL_STATIC */
/** \def LFX_WARNING_STATIC
\see LFX_FATAL_STATIC */
/** \def LFX_NOTICE_STATIC
\see LFX_FATAL_STATIC */
/** \def LFX_INFO_STATIC
\see LFX_FATAL_STATIC */
/** \def LFX_DEBUG_STATIC
\see LFX_FATAL_STATIC */
/** \def LFX_TRACE_STATIC
\see LFX_FATAL_STATIC */

#define LFX_FATAL_STATIC( __name, __msg )     __LFX_LOG_STATIC( fatal, __name, __msg )
#define LFX_CRITICAL_STATIC( __name, __msg )  __LFX_LOG_STATIC( critical, __name, __msg )
#define LFX_ERROR_STATIC( __name, __msg )     __LFX_LOG_STATIC( error, __name, __msg )
#define LFX_WARNING_STATIC( __name, __msg )   __LFX_LOG_STATIC( warning, __name, __msg )
#define LFX_NOTICE_STATIC( __name, __msg )    __LFX_LOG_STATIC( notice, __name, __msg )
#define LFX_INFO_STATIC( __name, __msg )      __LFX_LOG_STATIC( information, __name, __msg )
#define LFX_DEBUG_STATIC( __name, __msg )     __LFX_LOG_STATIC( debug, __name, __msg )
#define LFX_TRACE_STATIC( __name, __msg )     __LFX_LOG_STATIC( trace, __name, __msg )

/*@}*/


// __LFX_CORE_LOG_MACROS_H__
#endif
