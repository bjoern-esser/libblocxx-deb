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
 * @author Kevin S. Van Horn
 */

#include "blocxx/BLOCXX_config.h"
#include "blocxx/MultiProcessFileAppender.hpp"
#include "blocxx/Format.hpp"
#include "blocxx/Logger.hpp"
#include "blocxx/LogMessage.hpp"
#include "blocxx/FileSystem.hpp"

#include <fcntl.h>
#ifdef BLOCXX_HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

namespace BLOCXX_NAMESPACE
{

namespace
{
	using namespace blocxx;

	class FileLock
	{
	public:
		FileLock(File & file)
		: m_file(file)
		{
			m_ok = file.getLock() == 0;
		}

		bool ok() const
		{
			return m_ok;
		}

		~FileLock()
		{
			if (m_ok)
			{
				m_file.unlock(); // nowhere to report error if unlock fails...
			}
		}

	private:
		File & m_file;
		bool m_ok;
	};

	typedef blocxx::MultiProcessFileAppender app_t;

	UInt64 kbytesToBytes(UInt64 max_size)
	{
		UInt64 const oneK = 1024;
		UInt64 const biggest = UInt64(-1); // largest possible UInt64
		return(
			max_size == app_t::NO_MAX_LOG_SIZE ? biggest
			: max_size > biggest / oneK ? biggest
			: max_size * oneK
		);
	}

}

MultiProcessFileAppender::MultiProcessFileAppender(
	const StringArray& components,
	const StringArray& categories,
	const String & filename,
	const String& pattern,
	UInt64 maxFileSize,
	UInt32 maxBackupIndex
)
	: LogAppender(components, categories, pattern)
	, m_filename(filename)
	, m_maxFileSize(kbytesToBytes(maxFileSize))
	, m_maxBackupIndex(maxBackupIndex)
{
	m_log = FileSystem::openForAppendOrCreateFile(filename);
	m_lock = FileSystem::openOrCreateFile(filename + ".lock");

	if (!m_log)
	{
		String msg = "Cannot create log file " + filename;
		BLOCXX_THROW(LoggerException, msg.c_str());
	}
	if (!m_lock)
	{
		String msg = "Cannot create lock file for log file " + filename + ".lock";
		BLOCXX_THROW(LoggerException, msg.c_str());
	}
}

MultiProcessFileAppender::~MultiProcessFileAppender()
{
}

void MultiProcessFileAppender::doProcessLogMessage(
	const String& formattedMessage, const LogMessage& message) const
{
	// On error, we just return, since doProcessLogMessage should not throw

	if (!m_log || !m_lock)
	{
		return;
	}

	FileLock lock(m_lock);
	if (!lock.ok())
	{
		return;
	}

	UInt64 size = m_log.size();

	if (size >= m_maxFileSize)
	{
		// Log file has been rotated
		File f = FileSystem::openForAppendOrCreateFile(m_filename);
		f.swap(m_log);
		if (!m_log)
		{
			return;
		}
		size = m_log.size();
	}

	String s = formattedMessage + "\n";
	m_log.write(s.c_str(), s.length());
	m_log.flush();
	size += s.length();

	// handle log rotation
	if (size >= m_maxFileSize)
	{
		// do the roll over
		m_log.close();

		if (m_maxBackupIndex > 0)
		{
			// delete the oldest file first, if it exists
			FileSystem::removeFile(m_filename + '.' + String(m_maxBackupIndex));

			// increment the numbers on all the files, if they exist
			for (UInt32 i = m_maxBackupIndex - 1; i >= 1; --i)
			{
				FileSystem::renameFile(m_filename + '.' + String(i), m_filename + '.' + String(i + 1));
			}

			if (!FileSystem::renameFile(m_filename, m_filename + ".1"))
			{
				// if we can't rename it, at least don't write to it anymore
				return;
			}
		}

		// open new log file
		File f = FileSystem::openForAppendOrCreateFile(m_filename);
		f.swap(m_log);
	}
}

} // end namespace BLOCXX_NAMESPACE




