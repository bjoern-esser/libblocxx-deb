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


#if      !defined (BLOCXX_TEST_CANNED_FILE_SYSTEM_NORMAL_FILE_HPP_INCLUDE_GUARD_)
#define            BLOCXX_TEST_CANNED_FILE_SYSTEM_NORMAL_FILE_HPP_INCLUDE_GUARD_

// This file defines a "normal file" type for the canned filesystem.  This is
// the file type you want to use 99% of the time.  Most of the time you won't
// even need to know that you are using it because there is a simple creation
// function in the CannedFileSystem to make one of these for you.
//
// If you need callbacks for access to files, you need to define your own class
// based on FileSystemEntry and use that instead of this.  The need for that
// should be rare, so you're looking in the right place.

#include "blocxx/BLOCXX_config.h"

#include "blocxx_test/CannedFileSystemEntryIFC.hpp"
#include "blocxx/String.hpp"

namespace BLOCXX_NAMESPACE
{
	namespace Test
	{
		// Permissions for use with "normal" files.
		enum PermissionFlags
		{
			E_FILE_EXECUTABLE = blocxx::FileSystem::FileInformation::E_FILE_USER_EXECUTE
			, E_FILE_WRITEABLE = blocxx::FileSystem::FileInformation::E_FILE_USER_WRITE
			, E_FILE_READABLE = blocxx::FileSystem::FileInformation::E_FILE_USER_READ
			, E_FILE_READWRITE = E_FILE_WRITEABLE | E_FILE_READABLE,

			// A few common chmod style constants...
			E_FILE_MODE_777 = E_FILE_READABLE | E_FILE_WRITEABLE | E_FILE_EXECUTABLE
			, E_FILE_MODE_666 = E_FILE_READABLE | E_FILE_WRITEABLE
			, E_FILE_MODE_555 = E_FILE_READABLE | E_FILE_EXECUTABLE
			, E_FILE_MODE_444 = E_FILE_READABLE
		};

		// Probably will be the most commonly used file type.  Maintains its own
		// contents and requires no external user functions.
		class NormalFile : public FileSystemEntry
		{
		public:
			NormalFile(const blocxx::String& path
				, const blocxx::String& contents = blocxx::String()
				, PermissionFlags mode = E_FILE_READABLE
				, FileEntryTypes type = FileInformation::E_FILE_REGULAR
				, blocxx::UserId owner = 0);
			NormalFile(const blocxx::String& path
				, const blocxx::String& contents = blocxx::String()
				, const FileInformation& stats = FileInformation());
			virtual ~NormalFile();

			virtual blocxx::String path() const;
			virtual FileEntryTypes getFileType() const;
			virtual bool readable() const;
			virtual bool writeable() const;
			virtual bool executable() const;
			virtual FileInformation getFileInfo() const;
			virtual blocxx::String contents() const;
			virtual bool setPath(const blocxx::String & location);
			virtual bool setContents(const blocxx::String & contents);

		private:
			blocxx::String m_path;
			blocxx::String m_contents;
			// The stats are mutable so that an atime can be tracked.
			mutable FileInformation m_stats;
		};

	} // end namespace Test
} // end namespace BLOCXX_NAMESPACE
#endif /* !defined(BLOCXX_TEST_CANNED_FILE_SYSTEM_NORMAL_FILE_HPP_INCLUDE_GUARD_) */
