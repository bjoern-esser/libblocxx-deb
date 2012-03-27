/*******************************************************************************
* Copyright (C) 2005, 2010, Quest Software, Inc. All rights reserved.
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

#include "blocxx/BLOCXX_config.h"
#include "blocxx/Logger.hpp"
#include "blocxx/ExceptionIds.hpp"
#include "blocxx/LogMessage.hpp"
#include "blocxx/Assertion.hpp"
#include "blocxx/Array.hpp"
#include "blocxx/LogMessagePatternFormatter.hpp"
#include "blocxx/AppenderLogger.hpp"
#include "blocxx/LogAppender.hpp"
#include "blocxx/Format.hpp"

namespace BLOCXX_NAMESPACE
{

BLOCXX_DEFINE_EXCEPTION_WITH_ID(Logger);

const GlobalString Logger::STR_NONE_CATEGORY = BLOCXX_GLOBAL_STRING_INIT("NONE");
const GlobalString Logger::STR_FATAL_CATEGORY = BLOCXX_GLOBAL_STRING_INIT("FATAL");
const GlobalString Logger::STR_ERROR_CATEGORY = BLOCXX_GLOBAL_STRING_INIT("ERROR");
const GlobalString Logger::STR_WARNING_CATEGORY = BLOCXX_GLOBAL_STRING_INIT("WARNING");
const GlobalString Logger::STR_INFO_CATEGORY = BLOCXX_GLOBAL_STRING_INIT("INFO");
const GlobalString Logger::STR_DEBUG_CATEGORY = BLOCXX_GLOBAL_STRING_INIT("DEBUG");
const GlobalString Logger::STR_DEBUG2_CATEGORY = BLOCXX_GLOBAL_STRING_INIT("DEBUG2");
const GlobalString Logger::STR_DEBUG3_CATEGORY = BLOCXX_GLOBAL_STRING_INIT("DEBUG3");
const GlobalString Logger::STR_ALL_CATEGORY = BLOCXX_GLOBAL_STRING_INIT("ALL");
const GlobalString Logger::STR_DEFAULT_COMPONENT = BLOCXX_GLOBAL_STRING_INIT("none");

//////////////////////////////////////////////////////////////////////////////
Logger::Logger(const String& defaultComponent, const LogAppenderRef& appender)
	: m_defaultComponent(defaultComponent)
	, m_appender(appender ? appender : LogAppender::getCurrentLogAppender())
	, m_logLevel(m_appender->getLogLevel())
{
	BLOCXX_ASSERT(m_defaultComponent.length());
}

/////////////////////////////////////////////////////////////////////////////
Logger::Logger(const String& defaultComponent, const ELogLevel logLevel)
	: m_defaultComponent(defaultComponent)
	, m_appender(LogAppender::getCurrentLogAppender())
	, m_logLevel(logLevel)
{
	BLOCXX_ASSERT(m_defaultComponent.length());
}

/////////////////////////////////////////////////////////////////////////////
Logger::Logger(const Logger& x)
	: IntrusiveCountableBase(x)
	, m_defaultComponent(x.m_defaultComponent)
	, m_appender(x.m_appender)
	, m_logLevel(x.m_logLevel)
{
}

/////////////////////////////////////////////////////////////////////////////
Logger&
Logger::operator=(const Logger& x)
{
	m_defaultComponent = x.m_defaultComponent;
	m_appender = x.m_appender;
	m_logLevel = x.m_logLevel;

	return *this;
}

/////////////////////////////////////////////////////////////////////////////
void
Logger::swap(Logger& x)
{
	m_defaultComponent.swap(x.m_defaultComponent);
	m_appender.swap(x.m_appender);
	std::swap(m_logLevel, x.m_logLevel);
}

//////////////////////////////////////////////////////////////////////////////
Logger::~Logger()
{
}

//////////////////////////////////////////////////////////////////////////////
void
Logger::logFatalError(const String& message, const char* filename, int fileline, const char* methodname) const
{
	if (m_logLevel >= E_FATAL_ERROR_LEVEL)
	{
		processLogMessage( LogMessage(m_defaultComponent, STR_FATAL_CATEGORY, message, filename, fileline, methodname) );
	}
}

//////////////////////////////////////////////////////////////////////////////
void
Logger::logError(const String& message, const char* filename, int fileline, const char* methodname) const
{
	if (m_logLevel >= E_ERROR_LEVEL)
	{
		processLogMessage( LogMessage(m_defaultComponent, STR_ERROR_CATEGORY, message, filename, fileline, methodname) );
	}
}

//////////////////////////////////////////////////////////////////////////////
void
Logger::logWarning(const String& message, const char* filename, int fileline, const char* methodname) const
{
	if (m_logLevel >= E_WARNING_LEVEL)
	{
		processLogMessage( LogMessage(m_defaultComponent, STR_WARNING_CATEGORY, message, filename, fileline, methodname) );
	}
}

//////////////////////////////////////////////////////////////////////////////
void
Logger::logInfo(const String& message, const char* filename, int fileline, const char* methodname) const
{
	if (m_logLevel >= E_INFO_LEVEL)
	{
		processLogMessage( LogMessage(m_defaultComponent, STR_INFO_CATEGORY, message, filename, fileline, methodname) );
	}
}

//////////////////////////////////////////////////////////////////////////////
void
Logger::logDebug(const String& message, const char* filename, int fileline, const char* methodname) const
{
	if (m_logLevel >= E_DEBUG_LEVEL)
	{
		processLogMessage( LogMessage(m_defaultComponent, STR_DEBUG_CATEGORY, message, filename, fileline, methodname) );
	}
}

//////////////////////////////////////////////////////////////////////////////
void
Logger::logDebug2(const String& message, const char* filename, int fileline, const char* methodname) const
{
	if (m_logLevel >= E_DEBUG2_LEVEL)
	{
		processLogMessage( LogMessage(m_defaultComponent, STR_DEBUG2_CATEGORY, message, filename, fileline, methodname) );
	}
}

//////////////////////////////////////////////////////////////////////////////
void
Logger::logDebug3(const String& message, const char* filename, int fileline, const char* methodname) const
{
	if (m_logLevel >= E_DEBUG3_LEVEL)
	{
		processLogMessage( LogMessage(m_defaultComponent, STR_DEBUG3_CATEGORY, message, filename, fileline, methodname) );
	}
}

//////////////////////////////////////////////////////////////////////////////
void
Logger::logMessage(const String& component, const String& category, const String& message) const
{
	processLogMessage(LogMessage(component, category, message, 0, -1, 0));
}

//////////////////////////////////////////////////////////////////////////////
void
Logger::logMessage(const String& component, const String& category, const String& message, const char* filename, int fileline, const char* methodname) const
{
	processLogMessage(LogMessage(component, category, message, filename, fileline, methodname));
}

//////////////////////////////////////////////////////////////////////////////
void
Logger::logMessage(const String& category, const String& message) const
{
	processLogMessage(LogMessage(m_defaultComponent, category, message, 0, -1, 0));
}

//////////////////////////////////////////////////////////////////////////////
void
Logger::logMessage(const String& category, const String& message, const char* filename, int fileline, const char* methodname) const
{
	processLogMessage(LogMessage(m_defaultComponent, category, message, filename, fileline, methodname));
}

//////////////////////////////////////////////////////////////////////////////
void
Logger::logMessage(const LogMessage& message) const
{
	processLogMessage(message);
}

//////////////////////////////////////////////////////////////////////////////
void
Logger::setDefaultComponent(const String& component)
{
	BLOCXX_ASSERT(component != "");
	m_defaultComponent = component;
}

//////////////////////////////////////////////////////////////////////////////
String
Logger::getDefaultComponent() const
{
	return m_defaultComponent;
}

//////////////////////////////////////////////////////////////////////////////
void
Logger::setLogLevel(ELogLevel logLevel)
{
	m_logLevel = logLevel;
}

//////////////////////////////////////////////////////////////////////////////
void
Logger::setLogLevel(const String& l)
{
	setLogLevel(stringToLogLevel(l));
}

//////////////////////////////////////////////////////////////////////////////
ELogLevel
Logger::stringToLogLevel(const String& l)
{
	if (l.equalsIgnoreCase(STR_INFO_CATEGORY))
	{
		return E_INFO_LEVEL;
	}
	else if (l.equalsIgnoreCase(STR_DEBUG_CATEGORY))
	{
		return E_DEBUG_LEVEL;
	}
	else if (l.equalsIgnoreCase(STR_DEBUG2_CATEGORY))
	{
		return E_DEBUG2_LEVEL;
	}
	else if (l.equalsIgnoreCase(STR_DEBUG3_CATEGORY))
	{
		return E_DEBUG3_LEVEL;
	}
	else if (l.equalsIgnoreCase(STR_ERROR_CATEGORY))
	{
		return E_ERROR_LEVEL;
	}
	else if (l.equalsIgnoreCase(STR_WARNING_CATEGORY))
	{
		return E_WARNING_LEVEL;
	}
	else if (l.equalsIgnoreCase(STR_ALL_CATEGORY))
	{
		return E_ALL_LEVEL;
	}
	else if (l.equalsIgnoreCase(STR_NONE_CATEGORY))
	{
		return E_NONE_LEVEL;
	}
	else
	{
		return E_FATAL_ERROR_LEVEL;
	}
}

//////////////////////////////////////////////////////////////////////////////
String
Logger::logLevelToString(ELogLevel logLevel)
{
	switch (logLevel)
	{
		case E_ALL_LEVEL:
			return STR_ALL_CATEGORY;
		case E_DEBUG3_LEVEL:
			return STR_DEBUG3_CATEGORY;
		case E_DEBUG2_LEVEL:
			return STR_DEBUG2_CATEGORY;
		case E_DEBUG_LEVEL:
			return STR_DEBUG_CATEGORY;
		case E_WARNING_LEVEL:
			return STR_WARNING_CATEGORY;
		case E_INFO_LEVEL:
			return STR_INFO_CATEGORY;
		case E_ERROR_LEVEL:
			return STR_ERROR_CATEGORY;
		case E_FATAL_ERROR_LEVEL:
			return STR_FATAL_CATEGORY;
		default:
			return STR_NONE_CATEGORY;
	}
}

//////////////////////////////////////////////////////////////////////////////
bool
Logger::categoryIsEnabled(const String& category) const
{
	return m_appender->categoryIsEnabled(category);
}

/////////////////////////////////////////////////////////////////////////////
bool
Logger::levelIsEnabled(const ELogLevel level) const
{
	return (getLogLevel() >= level);
}

//////////////////////////////////////////////////////////////////////////////
bool
Logger::componentAndCategoryAreEnabled(const String& component, const String& category) const
{
	return m_appender->componentAndCategoryAreEnabled(component, category);
}

//////////////////////////////////////////////////////////////////////////////
void
Logger::processLogMessage(const LogMessage& message) const
{
	BLOCXX_ASSERT(!message.component.empty());
	BLOCXX_ASSERT(!message.category.empty());
	BLOCXX_ASSERT(!message.message.empty());

	m_appender->logMessage(message);
}

} // end namespace BLOCXX_NAMESPACE

