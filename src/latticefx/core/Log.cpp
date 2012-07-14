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

#include <latticefx/core/Log.h>
#include <latticefx/core/LogBase.h>
#include <latticefx/core/LogMacros.h>

#include <Poco/PatternFormatter.h>
#include <Poco/FormattingChannel.h>
#include <Poco/ConsoleChannel.h>
#include <Poco/FileChannel.h>


namespace lfx {
namespace core {


Log* Log::instance()
{
    // TBD Need singleton manager to cleanup/delete singletons.
    static Log* s_instance = new Log;
    return( s_instance );
}


Log::Log()
  : _logFileName( "lfx.log" ),
    _console( NULL ),
    _file( NULL )
{
    char* logFileName = getenv( "LFX_LOG_FILE_NAME" );
    if( logFileName != NULL )
        _logFileName = logFileName;
}
Log::~Log()
{
}

void Log::setLogFileName( const std::string& logFileName )
{
    if( _file != NULL )
    {
        LFX_WARNING_STATIC( "lfx.core.log", "Can't change log file name. Log file is already open." );
        return;
    }
    _logFileName = logFileName;
}

void Log::setPriority( int prio, const std::string& logName )
{
    Poco::Logger& logger( Poco::Logger::get( logName ) );
    logger.setLevel( prio );
}
void Log::setPriority( int prio, const DestinationType dest, const std::string& logName )
{
    Poco::Logger& logger( Poco::Logger::get( logName ) );
    logger.setLevel( prio );
    if( dest == Console )
    {
        if( _console == NULL )
        {
            _console = new Poco::FormattingChannel( new Poco::PatternFormatter("%s: %t") );
	        _console->setChannel( new Poco::ConsoleChannel );
	        _console->open();
        }
        logger.setChannel( _console );
    }
    else if( dest == LogFile )
    {
        if( _file == NULL )
        {
            _file = new Poco::FormattingChannel( new Poco::PatternFormatter("%Y-%m-%d %H:%M:%S.%c %N[%P]:%s:%q:%t") );
            _file->setChannel( new Poco::FileChannel( _logFileName ) );
            _file->open();
        }
        logger.setChannel( _file );
    }
}


// core
}
// lfx
}
