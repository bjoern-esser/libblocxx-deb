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
 * @author Dan Nuffer
 */

#ifndef BLOCXX_APPENDER_LOGGER_HPP_INCLUDE_GUARD_
#define BLOCXX_APPENDER_LOGGER_HPP_INCLUDE_GUARD_
#include "blocxx/BLOCXX_config.h"
#include "blocxx/CommonFwd.hpp"
#include "blocxx/LogLevel.hpp"
#include "blocxx/Logger.hpp"
#include "blocxx/Array.hpp"

namespace BLOCXX_NAMESPACE
{

/**
 * This implementation of Logger is used to send a Log message
 * to multiple LogAppenders.
 */
class BLOCXX_COMMON_API AppenderLogger : public Logger
{
public:
	/**
	 * Create an AppenderLogger with one LogAppender.
	 * The log level that will be used will be the lowest priority
	 * (i.e. debug) found in the given log apppenders.
	 * @param defaultComponent The default component for logging
	 *        used when no component was passed to in logMessage().
	 * @param appender The LogAppender messages will be send to.
	 */
	AppenderLogger(const String& defaultComponent,
	               const LogAppenderRef& appender);

	/**
	 * Create an AppenderLogger with multiple LogAppenders.
	 * The log level that will be used will be the lowest priority
	 * (i.e. debug) found in the given log apppenders.
	 * @param defaultComponent The default component for logging
	 *        used when no component was passed to in logMessage().
	 * @param appenders The LogAppenders the messages will be routed to.
	 */
	AppenderLogger(const String& defaultComponent,
	               const Array<LogAppenderRef>& appenders);

	/**
	 * Create an AppenderLogger with one LogAppender.
	 * @param defaultComponent The default component for logging
	 *        used when no component was passed to in logMessage().
	 * @param logLevel The default log level to use.
	 * @param appender The LogAppender messages will be send to.
	 */
	AppenderLogger(const String& defaultComponent, ELogLevel logLevel,
	               const LogAppenderRef& appender);

	/**
	 * Create an AppenderLogger with multiple LogAppenders.
	 * @param defaultComponent The default component for logging
	 *        used when no component was passed to in logMessage().
	 * @param logLevel The default log level to use.
	 * @param appenders The LogAppenders the messages will be routed to.
	 */
	AppenderLogger(const String& defaultComponent, ELogLevel logLevel,
	               const Array<LogAppenderRef>& appenders);

	/**
	 * Destroy this AppenderLogger.
	 */
	virtual ~AppenderLogger();

	void addLogAppender(const LogAppenderRef& appender);

	static ELogLevel getLevel(const Array<LogAppenderRef>& appenders);
};

} // end namespace BLOCXX_NAMESPACE

#endif



