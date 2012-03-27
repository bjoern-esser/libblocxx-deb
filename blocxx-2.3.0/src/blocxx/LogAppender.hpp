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
 */

#ifndef BLOCXX_LOG_APPENDER_HPP_INCLUDE_GUARD_
#define BLOCXX_LOG_APPENDER_HPP_INCLUDE_GUARD_
#include "blocxx/BLOCXX_config.h"
#include "blocxx/CommonFwd.hpp"
#include "blocxx/IntrusiveCountableBase.hpp"
#include "blocxx/SortedVectorSet.hpp"
#include "blocxx/LogConfig.hpp"
#include "blocxx/LogLevel.hpp"
#include "blocxx/LogMessagePatternFormatter.hpp"
#include "blocxx/GlobalString.hpp"
#include "blocxx/GlobalStringArray.hpp"


namespace BLOCXX_NAMESPACE
{

class BLOCXX_COMMON_API LogAppender : public IntrusiveCountableBase
{
public:

	virtual ~LogAppender();

	/**
	 * Get a copy of the per thread LogAppenderRef or if
	 * not set, the default one. If neither setDefaultLogAppender() or
	 * setThreadLogAppender() has been called, the default LogAppender will be set
	 * to a NullAppender, and then returned.
	 *
	 * This method is preferred over getDefaultLogAppender() or
	 * getThreadLogAppender().
	 */
	static LogAppenderRef getCurrentLogAppender();

	/**
	 * Returns a copy of default LogAppenderRef.
	 * If you want to get a log appender to log messages, you shouldn't call this function.  Use getCurrentLogAppender() instead.
	 * If setDefaultLogAppender() hasn't been called,
	 * the default LogAppender will be set to a NullAppender, and then returned.
	 */
	static LogAppenderRef getDefaultLogAppender();

	/**
	 * Set the default global LogAppenderRef.
	 * Each application that wants logging should call this function with the desired LogAppenderRef.
	 */
	static bool setDefaultLogAppender(const LogAppenderRef &ref);

	/**
	 * Returns a copy of the thread LogAppenderRef.
	 * If you want to get a log appender to log messages, you shouldn't call this function.  Use getCurrentLogAppender() instead.
	 * If setThreadLogAppender() hasn't been called by the current thread, a null LogAppenderRef is returned.
	 */
	static LogAppenderRef getThreadLogAppender();

	/**
	 * Set a per thread LogAppenderRef that overrides the default one.
	 * If a thread desires to use a different log appender than the default, it can call this function to set it.
	 * @param ref The appender that will be returned from subsequent calls to getCurrentLogAppender() or
	 *            getThreadLogAppender() by the same thread. Pass a null to cause subsequent calls to
	 *            getCurrentLogAppender() to return the global LogAppenderRef.
	 */
	static bool setThreadLogAppender(const LogAppenderRef &ref);

	/**
	 * Log a message using the specified component and category
	 * @param message The message to log
	 */
	void logMessage(const LogMessage& message) const;

	bool categoryIsEnabled(const String& category) const;
	bool componentAndCategoryAreEnabled(const String& component, const String& category) const;

	ELogLevel getLogLevel() const
	{
		return m_logLevel;
	}

	/**
	 * Create a concrete log appender depending on the type string passed in.
	 * If type == "syslog" (TYPE_SYSLOG) a logger the writes to the syslog
	 * will be returned.
	 * If type == "" || type == "null" (TYPE_NULL) a logger that doesn't do
	 * anything will be returned.
	 * If type == "cerr" || type == "stderr" (TYPE_STDERR) a logger that
	 * writes to stderr will be returned.
	 * If type == "file" (TYPE_FILE) then a logger that writes to a file --
	 * will be returned; the file name and other parameters are taken from
	 * @a configItems.
	 * If type == "mpfile" (TYPE_MPFILE) then a logger that writes to a file
	 * that may be shared with other process will be returned; the file name
	 * and other parameters are taken from @a configItems.
	 *
	 * @param name The name of the logger to create
	 * @param components The message components the logger will log.
	 *  "*" means all components.
	 * @param categories The message categories the logger will log.
	 *  "*" means all categories.
	 * @param type The type of logger to create
	 * @param configItems Additional config items the logger may use for
	 *  configuration.
	 * @return a class that implements the Logger interface.
	 * @throws LoggerException - E_UNKNOWN_LOG_APPENDER_TYPE - if type is unkonwn.
	 *                         - E_INVALID_MAX_FILE_SIZE     - if the max_file_size option is invalid
	 *                         - E_INVALID_MAX_BACKUP_INDEX  - if the max_backup_index option is invalid
	 */
	static LogAppenderRef createLogAppender(
		const String& name,
		const StringArray& components,
		const StringArray& categories,
		const String& messageFormat,
		const String& type,
		const LoggerConfigMap& configItems);

	/**
	 * Get the strings for the logging categories that should be enabled at the
	 * requested level.
	 *
	 * @param level The maximum log level to include. All lower log levels will
	 *        be included as well.
	 * @return The list of strings at or below the supplied log level.
	 */
	static StringArray getCategoriesForLevel(ELogLevel level);

	/// Pass to createLogAppender to indicate all components.
	static const GlobalStringArray ALL_COMPONENTS;
	/// Pass to createLogAppender to indicate all categories.
	static const GlobalStringArray ALL_CATEGORIES;
	/// The Log4j TTCC message format - TTCC is acronym for Time Thread Category Component.
	/// "%r [%t] %-5p %c - %m"
	static const GlobalString STR_TTCC_MESSAGE_FORMAT;
	/// String of the type of the syslog log appender
	static const GlobalString TYPE_SYSLOG;
	/// String of the type of the stderr log appender
	static const GlobalString TYPE_STDERR;
	/// String of the type of the file log appender
	static const GlobalString TYPE_FILE;
	/// String of the type of the multi-process file log appender
	static const GlobalString TYPE_MPFILE;
	/// String of the type of the null log appender
	static const GlobalString TYPE_NULL;

protected:

	LogAppender(const StringArray& components = ALL_COMPONENTS,
		const StringArray& categories = ALL_CATEGORIES,
		const String& pattern = STR_TTCC_MESSAGE_FORMAT);

private:
	virtual void doProcessLogMessage(const String& formattedMessage, const LogMessage& message) const = 0;

private: // data
	SortedVectorSet<String> m_components;
	bool m_allComponents;
	SortedVectorSet<String> m_categories;
	bool m_allCategories;

	LogMessagePatternFormatter m_formatter;

	ELogLevel m_logLevel;

};
BLOCXX_EXPORT_TEMPLATE(BLOCXX_COMMON_API, IntrusiveReference, LogAppender);

#ifdef BLOCXX_WIN32
extern DWORD dwTlsIndex;
#else
namespace
{
pthread_key_t g_loggerKey;
}
#endif

} // end namespace BLOCXX_NAMESPACE

#endif


