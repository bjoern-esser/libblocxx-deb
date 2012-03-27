/*******************************************************************************
* Copyright (C) 2005, 2009, Quest Software, Inc. All rights reserved.
* Copyright (C) 2006, Novell, Inc. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*     * Redistributions of source code must retain the above copyright notice,
*       this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of
*       Quest Software, Inc.,
*       nor Novell, Inc.,
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
 * @author Anna Laguto // Windows part
 */

#include "blocxx/BLOCXX_config.h"
#include "blocxx/LogAppender.hpp"
#include "blocxx/String.hpp"
#include "blocxx/Array.hpp"
#include "blocxx/LogMessage.hpp"
#include "blocxx/Logger.hpp"
#include "blocxx/Assertion.hpp"
#include "blocxx/StringBuffer.hpp"
#include "blocxx/NullAppender.hpp"
#ifndef BLOCXX_WIN32
#include "blocxx/SyslogAppender.hpp"
#endif
#include "blocxx/CerrAppender.hpp"
#include "blocxx/FileAppender.hpp"
#include "blocxx/MultiProcessFileAppender.hpp"
#include "blocxx/Format.hpp"
#include "blocxx/SortedVectorMap.hpp"
#include "blocxx/NonRecursiveMutex.hpp"
#include "blocxx/NonRecursiveMutexLock.hpp"
#include "blocxx/ThreadOnce.hpp"
#include "blocxx/NullLogger.hpp"
#include "blocxx/GlobalPtr.hpp"


namespace BLOCXX_NAMESPACE
{

#ifdef BLOCXX_WIN32
DWORD dwTlsIndex = 0;
#endif

char const LOG_1_LOCATION_opt[] = "log.%1.location";
char const LOG_1_MAX_FILE_SIZE_opt[] = "log.%1.max_file_size";
char const LOG_1_MAX_BACKUP_INDEX_opt[] = "log.%1.max_backup_index";
char const LOG_1_FLUSH_opt[] = "log.%1.flush";
char const LOG_1_SYSLOG_IDENTITY_opt[] = "log.%1.identity";
char const LOG_1_SYSLOG_FACILITY_opt[] = "log.%1.facility";


//////////////////////////////////////////////////////////////////////////////
LogAppender::~LogAppender()
{
}

/////////////////////////////////////////////////////////////////////////////
// we're passing a pointer to this to pthreads, it has to have C linkage.
extern "C"
{
static void freeThreadLogAppender(void *ptr)
{
	delete static_cast<LogAppenderRef *>(ptr);
}
} // end extern "C"

/////////////////////////////////////////////////////////////////////////////
namespace
{

OnceFlag g_onceGuard  = BLOCXX_ONCE_INIT;
NonRecursiveMutex* g_mutexGuard = NULL;

struct NullAppenderFactory
{
	static LogAppenderRef* create()
	{
		return new LogAppenderRef(new NullAppender());
	}
};
::BLOCXX_NAMESPACE::GlobalPtr<LogAppenderRef,NullAppenderFactory> g_defaultLogAppender = BLOCXX_GLOBAL_PTR_INIT;


/////////////////////////////////////////////////////////////////////////////
void initGuardAndKey()
{
	g_mutexGuard = new NonRecursiveMutex();
#ifdef BLOCXX_WIN32
	LPVOID thread_data = NULL;
	BOOL ret = TlsSetValue(dwTlsIndex, thread_data)
	BLOCXX_ASSERTMSG(ret, "failed create a thread specific key");
#elif BLOCXX_NCR
	int ret = pthread_keycreate(&g_loggerKey, freeThreadLogAppender);
	BLOCXX_ASSERTMSG(ret == 0, "failed create a thread specific key");
#else
	int ret = pthread_key_create(&g_loggerKey, freeThreadLogAppender);
	BLOCXX_ASSERTMSG(ret == 0, "failed create a thread specific key");
#endif
}


} // end unnamed namespace

/////////////////////////////////////////////////////////////////////////////
// STATIC
LogAppenderRef
LogAppender::getCurrentLogAppender()
{
	LogAppenderRef threadLogAppender = getThreadLogAppender();
	if(threadLogAppender)
	{
		return threadLogAppender;
	}
	else
	{
		return getDefaultLogAppender();
	}
}

/////////////////////////////////////////////////////////////////////////////
// STATIC
LogAppenderRef
LogAppender::getDefaultLogAppender()
{
	callOnce(g_onceGuard, initGuardAndKey);
	NonRecursiveMutexLock lock(*g_mutexGuard);

	// This looks unsafe, but the get() method (called indirectly by operator*),
	// if it has never been previously called, will allocate a new
	// LogAppenderRef wich will have a NullAppender inside it.
	return *g_defaultLogAppender;
}


/////////////////////////////////////////////////////////////////////////////
// STATIC
bool
LogAppender::setDefaultLogAppender(const LogAppenderRef &ref)
{
	if (ref)
	{
		callOnce(g_onceGuard, initGuardAndKey);
		NonRecursiveMutexLock lock(*g_mutexGuard);

		LogAppenderRef(ref).swap(*g_defaultLogAppender);
		return true;
	}
	return false;
}


/////////////////////////////////////////////////////////////////////////////
// STATIC
LogAppenderRef
LogAppender::getThreadLogAppender()
{
	callOnce(g_onceGuard, initGuardAndKey);
	LogAppenderRef *ptr = NULL;

#ifdef BLOCXX_WIN32
	ptr = static_cast<LogAppenderRef *>(TlsGetValue(dwTlsIndex));
#elif BLOCXX_NCR
	pthread_addr_t addr_ptr = NULL;
	int ret = pthread_getspecific(g_loggerKey, &addr_ptr);
	if (ret == 0)
	{
		ptr = static_cast<LogAppenderRef *>(addr_ptr);
	}
#else
	ptr = static_cast<LogAppenderRef *>(pthread_getspecific(g_loggerKey));
#endif

	if(ptr)
	{
		return *ptr;
	}
	else
	{
		return LogAppenderRef();
	}
}

/////////////////////////////////////////////////////////////////////////////
// STATIC
bool
LogAppender::setThreadLogAppender(const LogAppenderRef &ref)
{
	callOnce(g_onceGuard, initGuardAndKey);
	LogAppenderRef *ptr = 0;
	if (ref)
	{
		ptr = new LogAppenderRef(ref);
	}
#ifdef BLOCXX_WIN32
	LogAppenderRef *ptr_old = static_cast<LogAppenderRef *>(TlsGetValue(dwTlsIndex));
	if (ptr_old)
	{
		delete ptr_old;
	}

	BOOL ret = FALSE;
	if (!(ret = TlsSetValue(dwTlsIndex, ptr)))
	{
		if (ptr)
		{
			delete ptr;
		}
	}
	BLOCXX_ASSERTMSG(ret, "failed to set a thread specific logger");
#elif BLOCXX_NCR
	pthread_addr_t addr_ptr = NULL;
	pthread_getspecific(g_loggerKey, &addr_ptr);
	freeThreadLogAppender(addr_ptr);
	int ret = pthread_setspecific(g_loggerKey, ptr);
	BLOCXX_ASSERTMSG(ret == 0, "failed to set a thread specific logger");
#else
	freeThreadLogAppender(pthread_getspecific(g_loggerKey));

	int ret = pthread_setspecific(g_loggerKey, ptr);
	if (ret != 0)
	{
		delete ptr;
	}
	BLOCXX_ASSERTMSG(ret == 0, "failed to set a thread specific logger");
#endif

	return (ref != 0);
}


//////////////////////////////////////////////////////////////////////////////
void
LogAppender::logMessage(const LogMessage& message) const
{
	if (componentAndCategoryAreEnabled(message.component, message.category))
	{
		StringBuffer buf;
		m_formatter.formatMessage(message, buf);
		doProcessLogMessage(buf.releaseString(), message);
	}
}

//////////////////////////////////////////////////////////////////////////////
bool
LogAppender::categoryIsEnabled(const String& category) const
{
	return m_allCategories || m_categories.count(category) > 0;
}

//////////////////////////////////////////////////////////////////////////////
bool
LogAppender::componentAndCategoryAreEnabled(const String& component, const String& category) const
{
	return (m_allComponents || m_components.count(component) > 0) &&
		categoryIsEnabled(category);
}

/////////////////////////////////////////////////////////////////////////////
namespace
{
	String
	getConfigItem(const LoggerConfigMap& configItems, const String &itemName, const String& defRetVal = "")
	{
		LoggerConfigMap::const_iterator i = configItems.find(itemName);
		if (i != configItems.end())
		{
			return i->second;
		}
		else
		{
			return defRetVal;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
LogAppenderRef
LogAppender::createLogAppender(
	const String& name,
	const StringArray& components,
	const StringArray& categories,
	const String& messageFormat,
	const String& type,
	const LoggerConfigMap& configItems)
{
	LogAppenderRef appender;
	if (type.empty() || type.equalsIgnoreCase(TYPE_NULL))
	{
		appender = new NullAppender(components, categories, messageFormat);
	}
#ifndef BLOCXX_WIN32
	else if ( type == TYPE_SYSLOG )
	{
		String identity = getConfigItem(configItems, Format(LogConfigOptions::LOG_1_SYSLOG_IDENTITY_opt, name), BLOCXX_DEFAULT_LOG_1_SYSLOG_IDENTITY);
		String facility = getConfigItem(configItems, Format(LogConfigOptions::LOG_1_SYSLOG_FACILITY_opt, name), BLOCXX_DEFAULT_LOG_1_SYSLOG_FACILITY);

		appender = new SyslogAppender(components, categories, messageFormat, identity, facility);
	}
#endif
	else if (type == TYPE_STDERR || type == "cerr")
	{
		appender = new CerrAppender(components, categories, messageFormat);
	}
	else if (type == TYPE_FILE || type == TYPE_MPFILE)
	{
		String configItem = Format(LogConfigOptions::LOG_1_LOCATION_opt, name);
		String filename = getConfigItem(configItems, configItem);

		UInt64 maxFileSize(0);
		try
		{
			maxFileSize = getConfigItem(configItems, Format(LogConfigOptions::LOG_1_MAX_FILE_SIZE_opt, name),
				BLOCXX_DEFAULT_LOG_1_MAX_FILE_SIZE).toUInt64();
		}
		catch (StringConversionException& e)
		{
			BLOCXX_THROW_ERR_SUBEX(LoggerException,
				Format("%1: Invalid config value: %2", LogConfigOptions::LOG_1_MAX_FILE_SIZE_opt, e.getMessage()).c_str(),
				Logger::E_INVALID_MAX_FILE_SIZE, e);
		}

		unsigned int maxBackupIndex(0);
		try
		{
			maxBackupIndex = getConfigItem(configItems, Format(LogConfigOptions::LOG_1_MAX_BACKUP_INDEX_opt, name),
				BLOCXX_DEFAULT_LOG_1_MAX_BACKUP_INDEX).toUnsignedInt();
		}
		catch (StringConversionException& e)
		{
			BLOCXX_THROW_ERR_SUBEX(LoggerException,
				Format("%1: Invalid config value: %2", LogConfigOptions::LOG_1_MAX_BACKUP_INDEX_opt, e.getMessage()).c_str(),
				Logger::E_INVALID_MAX_BACKUP_INDEX, e);
		}

		if (type == TYPE_FILE)
		{
			bool flushLog =
				getConfigItem(
					configItems,
					Format(LogConfigOptions::LOG_1_FLUSH_opt, name),
					BLOCXX_DEFAULT_LOG_1_FLUSH
				).equalsIgnoreCase("true");
			appender = new FileAppender(
				components, categories, filename.c_str(), messageFormat,
				maxFileSize, maxBackupIndex, flushLog
			);
		}
		else // type == TYPE_MPFILE
		{
			appender = new MultiProcessFileAppender(
				components, categories, filename, messageFormat,
				maxFileSize, maxBackupIndex
			);
		}
	}
	else
	{
		BLOCXX_THROW_ERR(LoggerException, Format("Unknown log type: %1", type).c_str(), Logger::E_UNKNOWN_LOG_APPENDER_TYPE);
	}

	return appender;
}

StringArray LogAppender::getCategoriesForLevel(ELogLevel level)
{
	StringArray retval;
	while(level > E_NONE_LEVEL)
	{
		retval.append(Logger::logLevelToString(level));
		level = ELogLevel(level - 1);
	}
	return retval;
}

//////////////////////////////////////////////////////////////////////////////
const GlobalStringArray LogAppender::ALL_COMPONENTS = BLOCXX_GLOBAL_STRING_INIT("*");
const GlobalStringArray LogAppender::ALL_CATEGORIES = BLOCXX_GLOBAL_STRING_INIT("*");
const GlobalString LogAppender::STR_TTCC_MESSAGE_FORMAT = BLOCXX_GLOBAL_STRING_INIT("%r [%t] %-5p %c - %m");
const GlobalString LogAppender::TYPE_SYSLOG = BLOCXX_GLOBAL_STRING_INIT("syslog");
const GlobalString LogAppender::TYPE_STDERR = BLOCXX_GLOBAL_STRING_INIT("stderr");
const GlobalString LogAppender::TYPE_FILE = BLOCXX_GLOBAL_STRING_INIT("file");
const GlobalString LogAppender::TYPE_MPFILE = BLOCXX_GLOBAL_STRING_INIT("mpfile");
const GlobalString LogAppender::TYPE_NULL = BLOCXX_GLOBAL_STRING_INIT("null");

//////////////////////////////////////////////////////////////////////////////
LogAppender::LogAppender(const StringArray& components, const StringArray& categories, const String& pattern)
	: m_components(components.begin(), components.end())
	, m_categories(categories.begin(), categories.end())
	, m_formatter(pattern)
	, m_logLevel(E_NONE_LEVEL)
{
	m_allComponents = m_components.count("*") > 0;
	m_allCategories = m_categories.count("*") > 0;

	// set up the log level
	size_t numCategories = m_categories.size();
	size_t debug3Count = m_categories.count(Logger::STR_DEBUG3_CATEGORY);
	size_t debug2Count = m_categories.count(Logger::STR_DEBUG2_CATEGORY);
	size_t debugCount = m_categories.count(Logger::STR_DEBUG_CATEGORY);
	size_t infoCount = m_categories.count(Logger::STR_INFO_CATEGORY);
	size_t warningCount = m_categories.count(Logger::STR_WARNING_CATEGORY);
	size_t errorCount = m_categories.count(Logger::STR_ERROR_CATEGORY);
	size_t fatalCount = m_categories.count(Logger::STR_FATAL_CATEGORY);
	int nonLevelCategoryCount = numCategories - debug3Count - debug2Count - debugCount - infoCount - warningCount - errorCount - fatalCount;

	if (numCategories == 0)
	{
		m_logLevel = E_NONE_LEVEL;
	}
	else if (m_allCategories || nonLevelCategoryCount > 0)
	{
		m_logLevel = E_ALL_LEVEL;
	}
	else if (debug3Count > 0)
	{
		m_logLevel = E_DEBUG3_LEVEL;
	}
	else if (debug2Count > 0)
	{
		m_logLevel = E_DEBUG2_LEVEL;
	}
	else if (debugCount > 0)
	{
		m_logLevel = E_DEBUG_LEVEL;
	}
	else if (infoCount > 0)
	{
		m_logLevel = E_INFO_LEVEL;
	}
	else if (warningCount > 0)
	{
		m_logLevel = E_WARNING_LEVEL;
	}
	else if (errorCount > 0)
	{
		m_logLevel = E_ERROR_LEVEL;
	}
	else if (fatalCount > 0)
	{
		m_logLevel = E_FATAL_ERROR_LEVEL;
	}
	else
	{
		BLOCXX_ASSERTMSG(0, "Internal error. LogAppender unable to determine log level!");
	}
}



} // end namespace BLOCXX_NAMESPACE


