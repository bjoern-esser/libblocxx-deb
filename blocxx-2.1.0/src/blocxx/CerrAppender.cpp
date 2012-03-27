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
#include "blocxx/CerrAppender.hpp"
#include "blocxx/String.hpp"
#include "blocxx/NonRecursiveMutex.hpp"
#include "blocxx/NonRecursiveMutexLock.hpp"

#include <iostream>

namespace BLOCXX_NAMESPACE
{

/////////////////////////////////////////////////////////////////////////////
CerrAppender::CerrAppender(const StringArray& components,
	const StringArray& categories,
	const String& pattern)
	: LogAppender(components, categories, pattern)
{
}

/////////////////////////////////////////////////////////////////////////////
CerrAppender::CerrAppender()
	: LogAppender(LogAppender::ALL_COMPONENTS, LogAppender::ALL_CATEGORIES, STR_DEFAULT_MESSAGE_PATTERN)
{
}

/////////////////////////////////////////////////////////////////////////////
CerrAppender::~CerrAppender()
{
}

/////////////////////////////////////////////////////////////////////////////
namespace
{
	NonRecursiveMutex cerrGuard;
}

void
CerrAppender::doProcessLogMessage(const String& formattedMessage, const LogMessage& message) const
{
	NonRecursiveMutexLock lock(cerrGuard);
	std::clog << formattedMessage << std::endl;
}

/////////////////////////////////////////////////////////////////////////////
// const char* red = "\x1b[1;31;40m";
// const char* darkRed = "\x1b[0;31;40m";
// const char* green = "\x1b[1;32;40m";
// const char* darkGreen = "\x1b[0;32;40m";
// const char* yellow = "\x1b[1;33;40m";
// const char* darkYellow = "\x1b[0;33;40m";
// const char* blue = "\x1b[1;34;40m";
// const char* darkBlue = "\x1b[0;34;40m";
// const char* purple = "\x1b[1;35;40m";
// const char* darkPurple = "\x1b[0;35;40m";
// const char* cyan = "\x1b[1;36;40m";
// const char* darkCyan = "\x1b[0;36;40m";
// const char* white = "\x1b[1;37;40m";
// const char* darkWhite = "\x1b[0;37;40m";
// const char* gray = "\x1b[0;37;40m";
// const char* resetColor = "\x1b[0;37;40m";

// colorized version: const String CerrAppender::STR_DEFAULT_MESSAGE_PATTERN("\x1b[1;37;40m[\x1b[1;31;40m%-.6t\x1b[1;37;40m]\x1b[1;32;40m %m\x1b[0;37;40m");
const GlobalString CerrAppender::STR_DEFAULT_MESSAGE_PATTERN = BLOCXX_GLOBAL_STRING_INIT("[%-.6t] %m");


} // end namespace BLOCXX_NAMESPACE



