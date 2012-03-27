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
#include "blocxx/FileAppender.hpp"
#include "blocxx/Format.hpp"
#include "blocxx/Logger.hpp"
#include "blocxx/LogMessage.hpp"
#include "blocxx/Mutex.hpp"
#include "blocxx/MutexLock.hpp"
#include "blocxx/FileSystem.hpp"
#include "blocxx/GlobalMutex.hpp"

#include <fstream>

namespace BLOCXX_NAMESPACE
{

/////////////////////////////////////////////////////////////////////////////
FileAppender::FileAppender(const StringArray& components,
	const StringArray& categories,
	const char* filename,
	const String& pattern,
	UInt64 maxFileSize,
	unsigned int maxBackupIndex,
	bool flushLog)
	: LogAppender(components, categories, pattern)
	, m_filename(filename)
	, m_maxFileSize(maxFileSize)
	, m_maxBackupIndex(maxBackupIndex)
	, m_flushLog(flushLog)
{
	m_log.open(m_filename.c_str(), std::ios::out | std::ios::app);
	if (!m_log)
	{
		BLOCXX_THROW(LoggerException, Format("FileAppender: Unable to open file: %1", m_filename).toString().c_str() );
	}
}

/////////////////////////////////////////////////////////////////////////////
FileAppender::~FileAppender()
{
}

/////////////////////////////////////////////////////////////////////////////
namespace
{
	GlobalMutex fileGuard = BLOCXX_GLOBAL_MUTEX_INIT();
}
void
FileAppender::doProcessLogMessage(const String& formattedMessage, const LogMessage& message) const
{
	MutexLock lock(fileGuard);

	// take into account external log rotators, if the file we have open no longer exists, then reopen it.
	if (!FileSystem::exists(m_filename.c_str()))
	{
		// make sure we can re-open the log file before we close it
		std::ofstream temp;
		temp.open(m_filename.c_str(), std::ios::out | std::ios::app);
		if (temp)
		{
			temp.close();
			m_log.close();
			m_log.open(m_filename.c_str(), std::ios::out | std::ios::app);
		}
		else
		{
			m_log << "FileAppender::doProcessLogMessage(): " << m_filename << " no longer exists and re-opening it failed!\n";
		}
	}

	if (!m_log)
	{
		// hmm, not much we can do here.  doProcessLogMessage can't throw.
		return;
	}

	m_log.write(formattedMessage.c_str(), formattedMessage.length());
	m_log << '\n';

	if (m_flushLog)
	{
		m_log.flush();
	}

	// handle log rotation
	if (m_maxFileSize != NO_MAX_LOG_SIZE && m_log.tellp() >= static_cast<std::streampos>(m_maxFileSize * 1024))
	{
		// do the roll over

		if (m_maxBackupIndex > 0)
		{
			// Delete the oldest file first - this may or may not exist; we try anyway.
			FileSystem::removeFile(m_filename + '.' + String(m_maxBackupIndex));

			// increment the numbers on all the files - some may exist or not, but try anyway.
			for (unsigned int i = m_maxBackupIndex - 1; i >= 1; --i)
			{
				FileSystem::renameFile(m_filename + '.' + String(i), m_filename + '.' + String(i + 1));
			}

			if (!FileSystem::renameFile(m_filename, m_filename + ".1"))
			{
				// if we can't rename it, avoid truncating it.
				m_log << "FileAppender::doProcessLogMessage(): Failed to rename " << m_filename << " to " << m_filename + ".1! Logging to this file STOPPED!\n";
				m_log.close();
				return;
			}
		}

		// make sure we can re-open the log file before we close it
		std::ofstream temp;
		temp.open(m_filename.c_str(), std::ios::out | std::ios::app);
		if (temp)
		{
			temp.close();
			m_log.close();
			// truncate the existing one
			m_log.open(m_filename.c_str(), std::ios_base::out | std::ios_base::trunc);
		}
		else
		{
			m_log << "FileAppender::doProcessLogMessage(): Failed to open " << m_filename << "! Logging to this file STOPPED!\n";
			m_log.close();
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
const GlobalString FileAppender::STR_DEFAULT_MESSAGE_PATTERN = BLOCXX_GLOBAL_STRING_INIT("%d{%a %b %d %H:%M:%S %Y} [%t]: %m");

} // end namespace BLOCXX_NAMESPACE




