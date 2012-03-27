/*******************************************************************************
* Copyright (C) 2010, Quest Software, Inc. All rights reserved.
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

#include "blocxx/BLOCXX_config.h"
#include "blocxx_test/CannedFileSystemNormalFile.hpp"

namespace BLOCXX_NAMESPACE
{
	namespace Test
	{
		NormalFile::NormalFile(const String& path, const String& contents, PermissionFlags mode, FileEntryTypes type, UserId owner)
			: m_path(path)
			, m_contents(contents)
			, m_stats()
		{
			m_stats.ctime.setToCurrent();
			m_stats.mtime = m_stats.ctime;
			m_stats.size = contents.size();
			m_stats.type = type;
			m_stats.owner = owner;
			m_stats.permissions = FileSystem::FileInformation::EFilePerms(mode);
		}

		NormalFile::NormalFile(const blocxx::String& path, const blocxx::String& contents, const FileInformation& stats)
			: m_path(path)
			, m_contents(contents)
			, m_stats(stats)
		{
		}

		NormalFile::~NormalFile()
		{
		}

		bool NormalFile::readable() const
		{
			return m_stats.permissions & E_FILE_READABLE;
		}

		bool NormalFile::writeable() const
		{
			return m_stats.permissions & E_FILE_WRITEABLE;
		}

		bool NormalFile::executable() const
		{
			return m_stats.permissions & E_FILE_EXECUTABLE;
		}

		FileInformation NormalFile::getFileInfo() const
		{
			return m_stats;
		}

		String NormalFile::path() const
		{
			return m_path;
		}

		FileEntryTypes NormalFile::getFileType() const
		{
			return m_stats.type;
		}

		String NormalFile::contents() const
		{
			m_stats.atime.setToCurrent();
			return m_contents;
		}

		bool NormalFile::setPath(const String& path)
		{
			m_path = path;
			return true;
		}

		bool NormalFile::setContents(const String& contents)
		{
			if ( writeable() )
			{
				m_stats.mtime.setToCurrent();
				m_contents = contents;
				return true;
			}
			return false;
		}

	} // end namespace Test
} // end namespace BLOCXX_NAMESPACE
