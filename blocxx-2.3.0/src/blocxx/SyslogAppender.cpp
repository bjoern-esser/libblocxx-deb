/*******************************************************************************
* Copyright (C) 2004-2005, Quest Software, Inc. All rights reserved.
* Copyright (C) 2005-2006, Novell, Inc. All rights reserved.
* 
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
* 
*     * Redistributions of source code must retain the above copyright notice,
*       this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of Quest Software, Inc., nor Novell, Inc., 
*       nor the names of its contributors or employees may be used to 
*       endorse or promote products derived from this software without 
*       specific prior written permission.
* 
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/


/**
 * @author Dan Nuffer
 */

#include "blocxx/BLOCXX_config.h"
#include "blocxx/SyslogAppender.hpp"
#include "blocxx/Logger.hpp"
#include "blocxx/LogMessage.hpp"
#include "blocxx/Mutex.hpp"
#include "blocxx/MutexLock.hpp"
#include "blocxx/Format.hpp"
#include "blocxx/GlobalMutex.hpp"

#if !defined(BLOCXX_WIN32)
#include <syslog.h>
#endif

#if defined(BLOCXX_WIN32)
#define snprintf _snprintf
#endif

namespace BLOCXX_NAMESPACE
{

namespace // anonymous
{
	GlobalMutex syslogGuard = BLOCXX_GLOBAL_MUTEX_INIT();

#if defined(NAME_MAX)
	static char log_ident[NAME_MAX];
#else
	static char log_ident[255];
#endif

	struct Facilities
	{
		const char * const name;
		const int          code;
	};

	static struct Facilities facilities[] =
	{
#ifdef LOG_AUTHPRIV
		{ "auth",	LOG_AUTH	},
#endif
#ifdef LOG_AUTHPRIV
		{ "authpriv",	LOG_AUTHPRIV	},
#endif
#ifdef LOG_CRON
		{ "cron",	LOG_CRON	},
#endif
#ifdef LOG_DAEMON
		{ "daemon",	LOG_DAEMON	},
#endif
#ifdef LOG_FTP
		{ "ftp",	LOG_FTP		},
#endif
#ifdef LOG_KERN
		{ "kern",	LOG_KERN	},
#endif
#ifdef LOG_LPR
		{ "lpr",	LOG_LPR		},
#endif
#ifdef LOG_MAIL
		{ "mail",	LOG_MAIL	},
#endif
#ifdef LOG_NEWS
		{ "news",	LOG_NEWS	},
#endif
#ifdef LOG_USER
		{ "user",	LOG_USER	},
#endif
#ifdef LOG_UUCP
		{ "uucp",	LOG_UUCP	},
#endif
#ifdef LOG_LOCAL0
		{ "local0",	LOG_LOCAL0	},
#endif
#ifdef LOG_LOCAL1
		{ "local1",	LOG_LOCAL1	},
#endif
#ifdef LOG_LOCAL2
		{ "local2",	LOG_LOCAL2	},
#endif
#ifdef LOG_LOCAL3
		{ "local3",	LOG_LOCAL3	},
#endif
#ifdef LOG_LOCAL4
		{ "local4",	LOG_LOCAL4	},
#endif
#ifdef LOG_LOCAL5
		{ "local5",	LOG_LOCAL5	},
#endif
#ifdef LOG_LOCAL6
		{ "local6",	LOG_LOCAL6	},
#endif
#ifdef LOG_LOCAL7
		{ "local7",	LOG_LOCAL7	},
#endif
		{ NULL,		0		}
	};

} // End of anonymous namespace


/////////////////////////////////////////////////////////////////////////////
SyslogAppender::SyslogAppender(const StringArray& components,
                               const StringArray& categories,
                               const String&      pattern,
                               const String&      identity,
                               const String&      facility)
	: LogAppender(components, categories, pattern)
{
	if( identity.empty() || identity.isSpaces())
	{
		BLOCXX_THROW(LoggerException,
			"SyslogAppender: Empty syslog identity name"
		);
	}
	if( facility.empty())
	{
		BLOCXX_THROW(LoggerException,
			"SyslogAppender: Empty syslog facility name"
		);
	}

	struct Facilities *f = facilities;
	for( ; f->name != NULL; f++)
	{
		if( facility.equals(f->name))
			break;
	}
	if( f->name == NULL)
	{
		BLOCXX_THROW(LoggerException,
			Format("SyslogAppender: Unknown syslog facility name: %1",
				facility).c_str()
		);
	}

	MutexLock lock(syslogGuard);
	if (!calledOpenLog)
	{
		/*
		 * Warning: openlog on linux remembers only the
		 *          pointer to log_ident ...
		 */
		::snprintf( log_ident, sizeof(log_ident), "%s", identity.c_str());
		openlog( log_ident, LOG_CONS, f->code);
		calledOpenLog = true;
	}
}

/////////////////////////////////////////////////////////////////////////////
SyslogAppender::~SyslogAppender() {}

/////////////////////////////////////////////////////////////////////////////
void
SyslogAppender::doProcessLogMessage(const String& formattedMessage, const LogMessage& message) const
{
	int syslogPriority;
	if (message.category.equalsIgnoreCase(Logger::STR_FATAL_CATEGORY))
	{
		syslogPriority = LOG_CRIT;
	}
	else if (message.category.equalsIgnoreCase(Logger::STR_ERROR_CATEGORY))
	{
		syslogPriority = LOG_ERR;
	}
	else if (message.category.equalsIgnoreCase(Logger::STR_WARNING_CATEGORY))
	{
		syslogPriority = LOG_WARNING;
	}
	else if (message.category.equalsIgnoreCase(Logger::STR_INFO_CATEGORY))
	{
		syslogPriority = LOG_INFO;
	}
	else if (message.category.equalsIgnoreCase(Logger::STR_DEBUG_CATEGORY) 
			 || message.category.equalsIgnoreCase(Logger::STR_DEBUG2_CATEGORY) 
			 || message.category.equalsIgnoreCase(Logger::STR_DEBUG3_CATEGORY))
	{
		syslogPriority = LOG_DEBUG;
	}
	else
	{
		syslogPriority = LOG_INFO;
	}

	StringArray a = formattedMessage.tokenize("\n");
	MutexLock lock(syslogGuard);
	for (size_t i = 0; i < a.size(); ++i)
	{
		char format[] = "%s";
		syslog( syslogPriority, format, a[i].c_str() );
	}
}

/////////////////////////////////////////////////////////////////////////////
bool SyslogAppender::calledOpenLog = false;
const GlobalString SyslogAppender::STR_DEFAULT_MESSAGE_PATTERN = BLOCXX_GLOBAL_STRING_INIT("[%t]%m");


} // end namespace BLOCXX_NAMESPACE




