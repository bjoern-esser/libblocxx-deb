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

#include "blocxx/BLOCXX_config.h"
#include "blocxx/MultiAppender.hpp"
#include "blocxx/LogMessage.hpp"
#include "blocxx/LogAppender.hpp"
#include "blocxx/MultiAppender.hpp"

#include <algorithm>

namespace BLOCXX_NAMESPACE
{

/////////////////////////////////////////////////////////////////////////////
MultiAppender::MultiAppender(const LogAppenderRef& appender)
	: LogAppender(LogAppender::ALL_COMPONENTS, LogAppender::ALL_CATEGORIES, String())
	, m_appenders(1, appender)
{
}

/////////////////////////////////////////////////////////////////////////////
MultiAppender::MultiAppender(const Array<LogAppenderRef>& appenders)
	: LogAppender(LogAppender::ALL_COMPONENTS, LogAppender::ALL_CATEGORIES, String())
	, m_appenders(appenders)
{
}

/////////////////////////////////////////////////////////////////////////////
MultiAppender::~MultiAppender()
{
}

/////////////////////////////////////////////////////////////////////////////
void
MultiAppender::addLogAppender(const LogAppenderRef& appender)
{
    m_appenders.append(appender);
}

/////////////////////////////////////////////////////////////////////////////
void
MultiAppender::doProcessLogMessage(const String& formattedMessage, const LogMessage& message) const
{
	for (size_t i = 0; i < m_appenders.size(); ++i)
	{
		m_appenders[i]->logMessage(message);
	}
}

} // end namespace BLOCXX_NAMESPACE







