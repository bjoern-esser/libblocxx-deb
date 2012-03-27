/*******************************************************************************
* Copyright (C) 2005, Vintela, Inc. All rights reserved.
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
*       Vintela, Inc., 
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

#ifndef BLOCXX_LOGGER_HPP_INCLUDE_GUARD_
#define BLOCXX_LOGGER_HPP_INCLUDE_GUARD_
#include "blocxx/BLOCXX_config.h"
#include "blocxx/CommonFwd.hpp"
#include "blocxx/String.hpp"
#include "blocxx/LogLevel.hpp"
#include "blocxx/IntrusiveCountableBase.hpp"
#include "blocxx/Exception.hpp"
#include "blocxx/LogAppender.hpp"
#include "blocxx/GlobalString.hpp"
#include <cerrno>


namespace BLOCXX_NAMESPACE
{

BLOCXX_DECLARE_APIEXCEPTION(Logger, BLOCXX_COMMON_API)


/**
 * Logging interface. Used to output log messages.
 * A Logger has a component, a log level and a LogAppenderRef.
 * The component will be used for all log messages generated, unless another component is explicitly
 * specified in a call to logMessage().
 * The AppenderRef is the target for log messages. The global LogAppenderRef will be retrieved via 
 * LogAppender::getCurrentLogAppender() if one is not passed to the constructor.
 * An application may call LogAppender::setDefaultLogAppender() to set the global LogAppenderRef.
 * The log level will be obtained from the appender by calling m_appender->getLogLevel().
 *
 * Invariants:
 * - m_defaultComponent != ""
 * - m_appender is not NULL.
 *
 * Responsibilities:
 * - Report log level.
 * - Provide interface to log messages.
 * - Filter messages based on log level.
 *
 * Collaborators:
 * - Any class which needs to log messages.
 *
 * Thread safety: read/write, except for setDefaultComponent() and setLogLevel() which should
 * only be called during initialization phase.
 *
 * Copy semantics: Value
 *
 * Exception safety: Strong
 */
class BLOCXX_COMMON_API Logger : public IntrusiveCountableBase
{
public:

	static const GlobalString STR_NONE_CATEGORY;
	static const GlobalString STR_FATAL_CATEGORY;
	static const GlobalString STR_ERROR_CATEGORY;
	static const GlobalString STR_WARNING_CATEGORY;
	static const GlobalString STR_INFO_CATEGORY;
	static const GlobalString STR_DEBUG_CATEGORY;
	static const GlobalString STR_DEBUG2_CATEGORY;
	static const GlobalString STR_DEBUG3_CATEGORY;
	static const GlobalString STR_ALL_CATEGORY;
	static const GlobalString STR_DEFAULT_COMPONENT; // "none"

	enum ELoggerErrorCodes
	{
		E_UNKNOWN_LOG_APPENDER_TYPE,
		E_INVALID_MAX_FILE_SIZE,
		E_INVALID_MAX_BACKUP_INDEX
	};

	/**
	 * @param defaultComponent The component used for log messages (can be overridden by logMessage())
	 * @param appender The Appender which will receive log messages. If NULL, the result of LogAppender::getCurrentLogAppender() will be used.
	 */
	Logger(const String& defaultComponent = STR_DEFAULT_COMPONENT, const LogAppenderRef& appender = LogAppenderRef());

	/**
	 * @param defaultComponent The component used for log messages (can be overridden my logMessage())
	 * @param logLevel The log level. All lower level log messages will be ignored.
	 **/
	Logger(const String& defaultComponent, const ELogLevel logLevel);

	Logger(const Logger&);
	Logger& operator=(const Logger&);
	void swap(Logger& x);
	virtual ~Logger();

	virtual LoggerRef clone() const BLOCXX_DEPRECATED; // in 4.0.0
	
	/**
	 * Log message with a fatal error category and the default component.
	 * @param message The string to log.
	 * @param filename The file where the log statement was written.
	 * @param fileline The line number of the file where the log statement was written.
	 * @param methodname The method name where the log statement was written.
	 */
	void logFatalError(const String& message, const char* filename = 0, int fileline = -1, const char* methodname = 0) const;
	
	/**
	 * If getLogLevel() >= E_ERROR_LEVEL, Log message with an error category and the default component.
	 * @param message The string to log.
	 * @param filename The file where the log statement was written.
	 * @param fileline The line number of the file where the log statement was written.
	 * @param methodname The method name where the log statement was written.
	 */
	void logError(const String& message, const char* filename = 0, int fileline = -1, const char* methodname = 0) const;
	
	/**
	 * If getLogLevel() >= E_WARNING_LEVEL, Log info.
	 * @param message The string to log.
	 * @param filename The file where the log statement was written.
	 * @param fileline The line number of the file where the log statement was written.
	 * @param methodname The method name where the log statement was written.
	 */
	void logWarning(const String& message, const char* filename = 0, int fileline = -1, const char* methodname = 0) const;
	
	/**
	 * If getLogLevel() >= E_INFO_LEVEL, Log info.
	 * @param message The string to log.
	 * @param filename The file where the log statement was written.
	 * @param fileline The line number of the file where the log statement was written.
	 * @param methodname The method name where the log statement was written.
	 */
	void logInfo(const String& message, const char* filename = 0, int fileline = -1, const char* methodname = 0) const;
	
	/**
	 * If getLogLevel() >= E_DEBUG_LEVEL, Log debug info.
	 * @param message The string to log.
	 * @param filename The file where the log statement was written.
	 * @param fileline The line number of the file where the log statement was written.
	 * @param methodname The method name where the log statement was written.
	 */
	void logDebug(const String& message, const char* filename = 0, int fileline = -1, const char* methodname = 0) const;

	/**
	 * If getLogLevel() >= E_DEBUG2_LEVEL, Log debug info.
	 * @param message The string to log.
	 * @param filename The file where the log statement was written.
	 * @param fileline The line number of the file where the log statement was written.
	 * @param methodname The method name where the log statement was written.
	 */
	void logDebug2(const String& message, const char* filename = 0, int fileline = -1, const char* methodname = 0) const;

	/**
	 * If getLogLevel() >= E_DEBUG3_LEVEL, Log debug info.
	 * @param message The string to log.
	 * @param filename The file where the log statement was written.
	 * @param fileline The line number of the file where the log statement was written.
	 * @param methodname The method name where the log statement was written.
	 */
	void logDebug3(const String& message, const char* filename = 0, int fileline = -1, const char* methodname = 0) const;

	// Note that we don't use defaults on logMessage so the correct overload will be chosen.
	/**
	 * Log a message using the specified component and category
	 * The current log level is ignored.
	 * @param component The component generating the log message.
	 * @param category The category of the log message.
	 * @param message The message to log
	 */
	void logMessage(const String& component, const String& category, const String& message) const;
	/**
	 * Log a message using the specified component and category
	 * The current log level is ignored.
	 * @param component The component generating the log message.
	 * @param category The category of the log message.
	 * @param message The message to log
	 * @param filename The file where the log statement was written.
	 * @param fileline The line number of the file where the log statement was written.
	 * @param methodname The method name where the log statement was written.
	 */
	void logMessage(const String& component, const String& category, const String& message, const char* filename, int fileline, const char* methodname) const;

	/**
	 * Log a message using the default component and specified category.
	 * The current log level is ignored.
	 * @param category The category of the log message.
	 * @param message The message to log
	 */
	void logMessage(const String& category, const String& message) const;
	
	/**
	 * Log a message using the default component and specified category.
	 * The current log level is ignored.
	 * @param category The category of the log message.
	 * @param message The message to log
	 * @param filename The file where the log statement was written.
	 * @param fileline The line number of the file where the log statement was written.
	 * @param methodname The method name where the log statement was written.
	 */
	void logMessage(const String& category, const String& message, const char* filename, int fileline, const char* methodname) const;

	/**
	 * Log a message.
	 * The current log level is ignored.
	 * @param message The message to log
	 */
	void logMessage(const LogMessage& message) const;

	/**
	 * Sets the default component.
	 * This function is not thread safe.
	 * @param component The new default component
	 */
	void setDefaultComponent(const String& component);

	/**
	 * Gets the default component.
	 * @return The default component
	 */
	String getDefaultComponent() const;

	/**
	 * @return The current log level
	 */
	ELogLevel getLogLevel() const
	{
		return m_logLevel;
	}

	/**
	 * Set the log level. All lower level log messages will be ignored.
	 * This function is not thread safe.
	 * @param logLevel the level as an enumeration value.
	 */
	void setLogLevel(ELogLevel logLevel);

	/**
	 * Set the log level. All lower level log messages will be ignored.
	 * This function is not thread safe.
	 * @param logLevel The log level, valid values: { STR_FATAL_ERROR_CATEGORY, STR_ERROR_CATEGORY,
	 *   STR_INFO_CATEGORY, STR_DEBUG_CATEGORY, STR_DEBUG2_CATEGORY,
	 *   STR_DEBUG3_CATEGORY }. Case-insensitive. If logLevel is unknown, the
	 *   level will be set to E_FATAL_ERROR_LEVEL
	 */
	void setLogLevel(const String& logLevel);

	/**
	 * Convert a log level string to an enum value. E_FATAL_ERROR_LEVEL is returned if the string is unknown.
	 * @param logLevel
	 * 
	 * @return ELogLevel
	 */
	static ELogLevel stringToLogLevel(const String& logLevel);
	
	/**
	 * Convert a log level enum to a string
	 * @param logLevel
	 * 
	 * @return String
	 */
	static String logLevelToString(ELogLevel logLevel);

	/**
	 * Determine if the log category is enabled.
	 */
	bool categoryIsEnabled(const String& category) const;

	/**
	 * Check if the logger is enabled for given level.
	 */
	bool levelIsEnabled(const ELogLevel level) const;

	/**
	 * Determine if the component and category are both enabled.
	 */
	bool componentAndCategoryAreEnabled(const String& component, const String& category) const;

	/**
	 * Utility functions for backward compatibility with LoggerRef and the BLOCXX_LOG macros
	 */
	static inline const Logger& asLogger(const Logger& lgr)
	{
		return lgr;
	}
	static inline const Logger& asLogger(const LoggerRef& lgr)
	{
		return *lgr;
	}

private:
	void processLogMessage(const LogMessage& message) const;

protected: // data
	String m_defaultComponent;
	LogAppenderRef m_appender;
	ELogLevel m_logLevel;
};
BLOCXX_EXPORT_TEMPLATE(BLOCXX_COMMON_API, IntrusiveReference, Logger);

} // end namespace BLOCXX_NAMESPACE


#if defined(BLOCXX_HAVE_UUPRETTY_FUNCTIONUU)
#define BLOCXX_LOGGER_PRETTY_FUNCTION __PRETTY_FUNCTION__
#elif defined(BLOCXX_HAVE_C99_UUFUNCUU)
#define BLOCXX_LOGGER_PRETTY_FUNCTION __func__
#else
#define BLOCXX_LOGGER_PRETTY_FUNCTION ""
#endif

/**
 * Log message to logger with the Debug3 level.  message is only evaluated if logger->getLogLevel() >=
 * E_DEBUG3_LEVEL __FILE__ and __LINE__ are logged.
 * @param logger The logger to use.
 * @param message An expression that evaluates to a String which will be logged.
 */
#define BLOCXX_LOG_DEBUG3(logger, message) \
do \
{ \
	int err = errno; \
	if (::BLOCXX_NAMESPACE::Logger::asLogger((logger)).getLogLevel() >= ::BLOCXX_NAMESPACE::E_DEBUG3_LEVEL) \
	{ \
		::BLOCXX_NAMESPACE::Logger::asLogger((logger)).logMessage(::BLOCXX_NAMESPACE::Logger::STR_DEBUG3_CATEGORY, (message), __FILE__, __LINE__, BLOCXX_LOGGER_PRETTY_FUNCTION); \
	} \
	errno = err; \
} while (0)


/**
 * Log message to logger with the Debug2 level.  message is only evaluated if logger->getLogLevel() >=
 * E_DEBUG2_LEVEL __FILE__ and __LINE__ are logged.
 * @param logger The logger to use.
 * @param message An expression that evaluates to a String which will be logged.
 */
#define BLOCXX_LOG_DEBUG2(logger, message) \
do \
{ \
	int err = errno; \
	if (::BLOCXX_NAMESPACE::Logger::asLogger((logger)).getLogLevel() >= ::BLOCXX_NAMESPACE::E_DEBUG2_LEVEL) \
	{ \
		::BLOCXX_NAMESPACE::Logger::asLogger((logger)).logMessage(::BLOCXX_NAMESPACE::Logger::STR_DEBUG2_CATEGORY, (message), __FILE__, __LINE__, BLOCXX_LOGGER_PRETTY_FUNCTION); \
	} \
	errno = err; \
} while (0)


/**
 * Log message to logger with the Debug level.  message is only evaluated if logger->getLogLevel() >= E_DEBUG_LEVEL
 * __FILE__ and __LINE__ are logged.
 * @param logger The logger to use.
 * @param message An expression that evaluates to a String which will be logged.
 */
#define BLOCXX_LOG_DEBUG(logger, message) \
do \
{ \
	int err = errno; \
	if (::BLOCXX_NAMESPACE::Logger::asLogger((logger)).getLogLevel() >= ::BLOCXX_NAMESPACE::E_DEBUG_LEVEL) \
	{ \
		::BLOCXX_NAMESPACE::Logger::asLogger((logger)).logMessage(::BLOCXX_NAMESPACE::Logger::STR_DEBUG_CATEGORY, (message), __FILE__, __LINE__, BLOCXX_LOGGER_PRETTY_FUNCTION); \
	} \
	errno = err; \
} while (0)


/**
 * Log message to logger with the Info level.  message is only evaluated if logger->getLogLevel() >= E_INFO_LEVEL
 * __FILE__ and __LINE__ are logged.
 * @param logger The logger to use.
 * @param message An expression that evaluates to a String which will be logged.
 */
#define BLOCXX_LOG_INFO(logger, message) \
do \
{ \
	int err = errno; \
	if (::BLOCXX_NAMESPACE::Logger::asLogger((logger)).getLogLevel() >= ::BLOCXX_NAMESPACE::E_INFO_LEVEL) \
	{ \
		::BLOCXX_NAMESPACE::Logger::asLogger((logger)).logMessage(::BLOCXX_NAMESPACE::Logger::STR_INFO_CATEGORY, (message), __FILE__, __LINE__, BLOCXX_LOGGER_PRETTY_FUNCTION); \
	} \
	errno = err; \
} while (0)

/**
 * Log message to logger with the Warning level.  message is only evaluated if logger->getLogLevel() >= E_WARNING_LEVEL
 * __FILE__ and __LINE__ are logged.
 * @param logger The logger to use.
 * @param message An expression that evaluates to a String which will be logged.
 */
#define BLOCXX_LOG_WARNING(logger, message) \
do \
{ \
	int err = errno; \
	if (::BLOCXX_NAMESPACE::Logger::asLogger((logger)).getLogLevel() >= ::BLOCXX_NAMESPACE::E_WARNING_LEVEL) \
	{ \
		::BLOCXX_NAMESPACE::Logger::asLogger((logger)).logMessage(::BLOCXX_NAMESPACE::Logger::STR_WARNING_CATEGORY, (message), __FILE__, __LINE__, BLOCXX_LOGGER_PRETTY_FUNCTION); \
	} \
	errno = err; \
} while (0)

/**
 * Log message to logger with the Error level.  message is only evaluated if logger->getLogLevel() >= E_ERROR_LEVEL
 * __FILE__ and __LINE__ are logged.
 * @param logger The logger to use.
 * @param message An expression that evaluates to a String which will be logged.
 */
#define BLOCXX_LOG_ERROR(logger, message) \
do \
{ \
	int err = errno; \
	if (::BLOCXX_NAMESPACE::Logger::asLogger((logger)).getLogLevel() >= ::BLOCXX_NAMESPACE::E_ERROR_LEVEL) \
	{ \
		::BLOCXX_NAMESPACE::Logger::asLogger((logger)).logMessage(::BLOCXX_NAMESPACE::Logger::STR_ERROR_CATEGORY, (message), __FILE__, __LINE__, BLOCXX_LOGGER_PRETTY_FUNCTION); \
	} \
	errno = err; \
} while (0)

/**
 * Log message to logger with the FatalError level.  message is always evaluated.
 * __FILE__ and __LINE__ are logged.
 * @param logger The logger to use.
 * @param message An expression that evaluates to a String which will be logged.
 */
#define BLOCXX_LOG_FATAL_ERROR(logger, message) \
do \
{ \
	int err = errno; \
	if (::BLOCXX_NAMESPACE::Logger::asLogger((logger)).getLogLevel() >= ::BLOCXX_NAMESPACE::E_FATAL_ERROR_LEVEL) \
	{ \
		::BLOCXX_NAMESPACE::Logger::asLogger((logger)).logMessage(::BLOCXX_NAMESPACE::Logger::STR_FATAL_CATEGORY, (message), __FILE__, __LINE__, BLOCXX_LOGGER_PRETTY_FUNCTION); \
	} \
	errno = err; \
} while (0)

/**
 * Log message to logger with the specified category.  message is only evaluated if logger->categoryIsEnabled(category) == true
 * __FILE__ and __LINE__ are logged.
 * @param logger The logger to use.
 * @param category The message category
 * @param message An expression that evaluates to a String which will be logged.
 */
#define BLOCXX_LOG(logger, category, message) \
do \
{ \
	int err = errno; \
	if (::BLOCXX_NAMESPACE::Logger::asLogger((logger)).categoryIsEnabled((category))) \
	{ \
		::BLOCXX_NAMESPACE::Logger::asLogger((logger)).logMessage((category), (message), __FILE__, __LINE__, BLOCXX_LOGGER_PRETTY_FUNCTION); \
	} \
	errno = err; \
} while (0)

/**
 * Log message to logger with the Debug3 level.
 * This macro variant allows the use of a stream operator<< to construct the message,
 * e.g. BLOCXX_SLOG_DEBUG3(logger, "Msg Nr" << 42);
 *
 * The message parameter is only evaluated if logger->getLogLevel() >=
 * E_DEBUG3_LEVEL, __FILE__ and __LINE__ are logged.
 * @param logger The logger to use.
 * @param message The message to log, using stream operator << parameters.
 */
#define BLOCXX_SLOG_DEBUG3(logger, message) \
do \
{ \
	int err = errno; \
	if (::BLOCXX_NAMESPACE::Logger::asLogger((logger)).getLogLevel() >= ::BLOCXX_NAMESPACE::E_DEBUG3_LEVEL) \
	{ \
		OStringStream buf; \
		buf << message; \
		::BLOCXX_NAMESPACE::Logger::asLogger((logger)).logMessage(::BLOCXX_NAMESPACE::Logger::STR_DEBUG3_CATEGORY, buf.toString(), __FILE__, __LINE__, BLOCXX_LOGGER_PRETTY_FUNCTION); \
	} \
	errno = err; \
} while (0)

/**
 * Log message to logger with the Debug2 level.
 * This macro variant allows the use of a stream operator<< to construct the message,
 * e.g. BLOCXX_SLOG_DEBUG2(logger, "Msg Nr" << 42);
 *
 * The message parameter is only evaluated if logger->getLogLevel() >=
 * E_DEBUG2_LEVEL, __FILE__ and __LINE__ are logged.
 * @param logger The logger to use.
 * @param message The message to log, using stream operator << parameters.
 */
#define BLOCXX_SLOG_DEBUG2(logger, message) \
do \
{ \
	int err = errno; \
	if (::BLOCXX_NAMESPACE::Logger::asLogger((logger)).getLogLevel() >= ::BLOCXX_NAMESPACE::E_DEBUG2_LEVEL) \
	{ \
		OStringStream buf; \
		buf << message; \
		::BLOCXX_NAMESPACE::Logger::asLogger((logger)).logMessage(::BLOCXX_NAMESPACE::Logger::STR_DEBUG2_CATEGORY, buf.toString(), __FILE__, __LINE__, BLOCXX_LOGGER_PRETTY_FUNCTION); \
	} \
	errno = err; \
} while (0)

/**
 * Log message to logger with the Debug level.
 * This macro variant allows the use of a stream operator<< to construct the message,
 * e.g. BLOCXX_SLOG_DEBUG(logger, "Msg Nr" << 42);
 *
 * The message parameter is only evaluated if logger->getLogLevel() >= E_DEBUG_LEVEL,
 * __FILE__ and __LINE__ are logged.
 * @param logger The logger to use.
 * @param message The message to log, using stream operator << parameters.
 */
#define BLOCXX_SLOG_DEBUG(logger, message) \
do \
{ \
	int err = errno; \
	if (::BLOCXX_NAMESPACE::Logger::asLogger((logger)).getLogLevel() >= ::BLOCXX_NAMESPACE::E_DEBUG_LEVEL) \
	{ \
		OStringStream buf; \
		buf << message; \
		::BLOCXX_NAMESPACE::Logger::asLogger((logger)).logMessage(::BLOCXX_NAMESPACE::Logger::STR_DEBUG_CATEGORY, buf.toString(), __FILE__, __LINE__, BLOCXX_LOGGER_PRETTY_FUNCTION); \
	} \
	errno = err; \
} while (0)

/**
 * Log message to logger with the Info level.
 * This macro variant allows the use of a stream operator<< to construct the message,
 * e.g. BLOCXX_SLOG_INFO(logger, "Msg Nr" << 42);.
 *
 * The message parameter is only evaluated if logger->getLogLevel() >= E_INFO_LEVEL,
 * __FILE__ and __LINE__ are logged.
 * @param logger The logger to use.
 * @param message The message to log, using stream operator << parameters.
 */
#define BLOCXX_SLOG_INFO(logger, message) \
do \
{ \
	int err = errno; \
	if (::BLOCXX_NAMESPACE::Logger::asLogger((logger)).getLogLevel() >= ::BLOCXX_NAMESPACE::E_INFO_LEVEL) \
	{ \
		OStringStream buf; \
		buf << message; \
		::BLOCXX_NAMESPACE::Logger::asLogger((logger)).logMessage(::BLOCXX_NAMESPACE::Logger::STR_INFO_CATEGORY, buf.toString(), __FILE__, __LINE__, BLOCXX_LOGGER_PRETTY_FUNCTION); \
	} \
	errno = err; \
} while (0)

/**
 * Log message to logger with the Warning level.
 * This macro variant allows the use of a stream operator<< to construct the message,
 * e.g. BLOCXX_SLOG_WARNING(logger, "Msg Nr" << 42);.
 *
 * The message parameter is only evaluated if logger->getLogLevel() >= E_WARNING_LEVEL,
 * __FILE__ and __LINE__ are logged.
 * @param logger The logger to use.
 * @param message The message to log, using stream operator << parameters.
 */
#define BLOCXX_SLOG_WARNING(logger, message) \
do \
{ \
	int err = errno; \
	if (::BLOCXX_NAMESPACE::Logger::asLogger((logger)).getLogLevel() >= ::BLOCXX_NAMESPACE::E_WARNING_LEVEL) \
	{ \
		OStringStream buf; \
		buf << message; \
		::BLOCXX_NAMESPACE::Logger::asLogger((logger)).logMessage(::BLOCXX_NAMESPACE::Logger::STR_WARNING_CATEGORY, buf.toString(), __FILE__, __LINE__, BLOCXX_LOGGER_PRETTY_FUNCTION); \
	} \
	errno = err; \
} while (0)

/**
 * Log message to logger with the Error level.
 * This macro variant allows the use of a stream operator<< to construct the message,
 * e.g. BLOCXX_SLOG_ERROR(logger, "Msg Nr" << 42);.
 *
 * The message parameter is only evaluated if logger->getLogLevel() >= E_ERROR_LEVEL,
 * __FILE__ and __LINE__ are logged.
 * @param logger The logger to use.
 * @param message The message to log, using stream operator << parameters.
 */
#define BLOCXX_SLOG_ERROR(logger, message) \
do \
{ \
	int err = errno; \
	if (::BLOCXX_NAMESPACE::Logger::asLogger((logger)).getLogLevel() >= ::BLOCXX_NAMESPACE::E_ERROR_LEVEL) \
	{ \
		OStringStream buf; \
		buf << message; \
		::BLOCXX_NAMESPACE::Logger::asLogger((logger)).logMessage(::BLOCXX_NAMESPACE::Logger::STR_ERROR_CATEGORY, buf.toString(), __FILE__, __LINE__, BLOCXX_LOGGER_PRETTY_FUNCTION); \
	} \
	errno = err; \
} while (0)

/**
 * Log message to logger with the FatalError level.
 * This macro variant allows the use of a stream operator<< to construct the message,
 * e.g. BLOCXX_SLOG_FATAL_ERROR(logger, "Msg Nr" << 42);.
 *
 * The message parameter is always evaluated, __FILE__ and __LINE__ are logged.
 * @param logger The logger to use.
 * @param message The message to log, using stream operator << parameters.
 */
#define BLOCXX_SLOG_FATAL_ERROR(logger, message) \
do \
{ \
	int err = errno; \
	if (::BLOCXX_NAMESPACE::Logger::asLogger((logger)).getLogLevel() >= ::BLOCXX_NAMESPACE::E_FATAL_ERROR_LEVEL) \
	{ \
		OStringStream buf; \
		buf << message; \
		::BLOCXX_NAMESPACE::Logger::asLogger((logger)).logMessage(::BLOCXX_NAMESPACE::Logger::STR_FATAL_CATEGORY, buf.toString(), __FILE__, __LINE__, BLOCXX_LOGGER_PRETTY_FUNCTION); \
	} \
	errno = err; \
} while (0)

/**
 * Log message to logger with the specified category.
 * This macro variant allows the use of a stream operator<< to construct the message,
 * e.g. BLOCXX_SLOG(logger, "INFO", "Msg Nr" << 42);
 * 
 * The message parameter is only evaluated if logger->categoryIsEnabled(category) == true,
 * __FILE__ and __LINE__ are logged.
 * @param logger The logger to use.
 * @param category The message category
 * @param message The message to log, using stream operator << parameters.
 */
#define BLOCXX_SLOG(logger, category, message) \
do \
{ \
	int err = errno; \
	if (::BLOCXX_NAMESPACE::Logger::asLogger((logger)).categoryIsEnabled((category))) \
	{ \
		OStringStream buf; \
		buf << message; \
		::BLOCXX_NAMESPACE::Logger::asLogger((logger)).logMessage((category), buf.toString(), __FILE__, __LINE__, BLOCXX_LOGGER_PRETTY_FUNCTION); \
	} \
	errno = err; \
} while (0)

#endif	// BLOCXX_LOGGER_HPP_INCLUDE_GUARD_

