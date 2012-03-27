/*******************************************************************************
* Copyright (C) 2005, Quest Software, Inc. All rights reserved.
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
 * @author Kevin Harris
 */

/**
 * This file should not be confused with the LogAppenderScope.  This is a
 * logger that logs a message when created and destroyed.
 */

#ifndef BLOCXX_SCOPE_LOGGER_HPP
#define BLOCXX_SCOPE_LOGGER_HPP
#include "blocxx/BLOCXX_config.h"
#include <iosfwd>
#include "blocxx/String.hpp"
#include "blocxx/LogAppender.hpp"
#include "blocxx/Logger.hpp"
#include "blocxx/DelayedFormat.hpp"
#include "blocxx/Reference.hpp"

namespace BLOCXX_NAMESPACE
{

	/**
	 * This is a fairly simple class that will log a message when created and
	 * when destroyed.  This is intended to be used for logging the start/finish
	 * of a scope (eg. function).
	 */
	class BLOCXX_COMMON_API ScopeLogger
	{
	public:
		/**
		 * @param entrance   The message to log when created.
		 * @param exit   The message to log when destroyed.
		 * @param lgr   The logger to be used.
		 */
		ScopeLogger(const String& entrance, const String& exit, const Logger& lgr);
		/**
		 * @param entrance   The message to log when created.
		 * @param exit   The message to log when destroyed.
		 * @param component  The component to be used in creating the logger.
		 * @param appender  The log appender to pass to a logger.
		 */
		ScopeLogger(const String& entrance, const String& exit, const String& component = Logger::STR_DEFAULT_COMPONENT, const LogAppenderRef& appender = LogAppenderRef());

		/**
		 * @param entrance   The message to log when created.
		 * @param exit   The message to log when destroyed.
		 * @param format   A delayed formatter which will be used in formatting a
		 *     message directly appended to the enter/exit text.
		 * @param lgr   The logger to be used.
		 */
		ScopeLogger(const String& entrance, const String& exit, const Reference<DelayedFormat>& format, const Logger& lgr);
		/**
		 * @param entrance   The message to log when created.
		 * @param exit   The message to log when destroyed.
		 * @param format   A delayed formatter which will be used in formatting a
		 *     message directly appended to the enter/exit text.
		 * @param component  The component to be used in creating the logger.
		 * @param appender  The log appender to pass to a logger.
		 */
		ScopeLogger(const String& entrance, const String& exit, const Reference<DelayedFormat>& format, const String& component = Logger::STR_DEFAULT_COMPONENT, const LogAppenderRef& appender = LogAppenderRef());

		virtual ~ScopeLogger();

	private:
		// Not implemented because there is no reason to copy this class.  It is
		// intended for a single scope only.
		ScopeLogger(const ScopeLogger&);
		ScopeLogger& operator=(const ScopeLogger&);

		String enterMessage;
		String exitMessage;
		Logger logger;

		Reference<DelayedFormat> formatter;
	};

} // end namespace BLOCXX_NAMESPACE

#endif // BLOCXX_SCOPE_LOGGER_HPP
