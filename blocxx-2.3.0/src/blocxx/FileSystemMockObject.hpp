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

#ifndef BLOCXX_FILE_SYSTEM_MOCK_OBJECT_HPP_INCLUDE_GUARD_
#define BLOCXX_FILE_SYSTEM_MOCK_OBJECT_HPP_INCLUDE_GUARD_
#include "blocxx/BLOCXX_config.h"
#include "blocxx/CommonFwd.hpp"
#include "blocxx/Types.hpp"
#include "blocxx/FileSystem.hpp"
#include <cstddef>
#include <utility>

namespace BLOCXX_NAMESPACE
{
	/**
	 * This class can be used to modify/replace the behavior of the FileSystem functions.
	 */
	class FileSystemMockObject
	{
	public:
		virtual ~FileSystemMockObject();
		virtual File openFile(const String& path);
		virtual File openFile(const String& path, FileSystem::EOpenModeFlag mode);
		virtual File createFile(const String& path);
		virtual File openOrCreateFile(const String& path);
		virtual File openForAppendOrCreateFile(const String& path);
		virtual File createTempFile(const String& dir);
		virtual File createTempFile(String& filePath, const String& dir);
		virtual int changeFileOwner(const String& filename,
			const UserId& userId);
		virtual bool exists(const String& path);
	#ifndef BLOCXX_WIN32
		virtual bool isExecutable(const String& path);
	#endif
		virtual bool canRead(const String& path);
		virtual bool canWrite(const String& path);
	#ifndef BLOCXX_WIN32
		virtual bool isLink(const String& path);
	#endif
		virtual bool isDirectory(const String& path);
		virtual bool changeDirectory(const String& path);
	#ifndef BLOCXX_WIN32
		virtual bool makeDirectory(const String& path, int mode=0777);
	#else
		virtual bool makeDirectory(const String& path, int mode=-1);
	#endif
		virtual bool getFileSize(const String& path, Int64& size);
		virtual UInt64 fileSize(FileHandle fh);
		virtual bool removeDirectory(const String& path);
		virtual bool removeFile(const String& path);
		virtual bool getDirectoryContents(const String& path,
			StringArray& dirEntries);
		virtual bool renameFile(const String& oldFileName,
			const String& newFileName);
		virtual size_t read(const FileHandle& hdl, void* bfr, size_t numberOfBytes,
			Int64 offset=-1L);
		virtual size_t write(FileHandle hdl, const void* bfr,
			size_t numberOfBytes, Int64 offset=-1L);
		virtual Int64 seek(const FileHandle& hdl, Int64 offset, int whence);
		virtual Int64 tell(const FileHandle& hdl);
		virtual void rewind(const FileHandle& hdl);
		virtual int close(const FileHandle& hdl);
		virtual int flush(FileHandle& hdl);
		virtual String getFileContents(const String& filename);
		virtual StringArray getFileLines(const String& filename);
		virtual String readSymbolicLink(const String& path);
		virtual String realPath(const String& path);

		virtual std::pair<FileSystem::Path::ESecurity, String>
			security(String const & path, UserId uid);

		virtual std::pair<FileSystem::Path::ESecurity, String> security(String const & path);

		virtual std::pair<FileSystem::Path::ESecurity, String>
			security(String const & base_dir, String const & rel_path, UserId uid);

		virtual std::pair<FileSystem::Path::ESecurity, String>
			security(String const & base_dir, String const & rel_path);

		virtual String dirname(const String& filename);
		virtual String basename(const String& filename);

		virtual String getCurrentWorkingDirectory();
		virtual FileSystem::FileInformation getFileInformation(const String& filename);


	};

} // end namespace BLOCXX_NAMESPACE


#endif


