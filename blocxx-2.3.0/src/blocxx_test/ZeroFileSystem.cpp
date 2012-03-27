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
#include "blocxx_test/ZeroFileSystem.hpp"
#include "blocxx/File.hpp"
#include "blocxx/String.hpp"
#include "blocxx/Array.hpp"

#include <cstring>

namespace BLOCXX_NAMESPACE
{
	namespace Test
	{
		class ZeroFileSystem : public blocxx::FileSystemMockObject
		{
		public:
			ZeroFileSystem();
			virtual ~ZeroFileSystem();
			virtual File openFile(const String& path);
			virtual File openFile(const String& path, FileSystem::EOpenModeFlag mode);
			virtual bool exists(const String& path);
			virtual bool isExecutable(const String& path);
			virtual bool canRead(const String& path);
			virtual bool canWrite(const String& path);
			virtual bool isLink(const String& path);
			virtual bool isDirectory(const String& path);
			virtual bool getFileSize(const String& path, Int64& size);
			virtual UInt64 fileSize(FileHandle fh);
			virtual bool getDirectoryContents(const String& path, StringArray& dirEntries);
			virtual bool renameFile(const String& oldFileName, const String& newFileName);
			virtual size_t read(const FileHandle& hdl, void* bfr, size_t numberOfBytes, Int64 offset=-1L);
			virtual size_t write(FileHandle hdl, const void* bfr, size_t numberOfBytes, Int64 offset=-1L);
			virtual Int64 seek(const FileHandle& hdl, Int64 offset, int whence);
			virtual Int64 tell(const FileHandle& hdl);
			virtual void rewind(const FileHandle& hdl);
			virtual int close(const FileHandle& hdl);
			virtual int flush(FileHandle& hdl);
			virtual String getFileContents(const String& filename);
			virtual StringArray getFileLines(const String& filename);
		};

		ZeroFileSystem::ZeroFileSystem()
		{
		}

		ZeroFileSystem::~ZeroFileSystem()
		{
		}

		File ZeroFileSystem::openFile(const String& path)
		{
			return FileHandle(1);
		}

		File ZeroFileSystem::openFile(const String& path, FileSystem::EOpenModeFlag mode)
		{
			return openFile(path);
		}

		bool ZeroFileSystem::exists(const String& path)
		{
			return true;
		}

		bool ZeroFileSystem::isExecutable(const String& path)
		{
			return false;
		}

		bool ZeroFileSystem::canRead(const String& path)
		{
			return true;
		}

		bool ZeroFileSystem::canWrite(const String& path)
		{
			return false;
		}

		bool ZeroFileSystem::isLink(const String& path)
		{
			return false;
		}

		bool ZeroFileSystem::isDirectory(const String& path)
		{
			return false;
		}

		bool ZeroFileSystem::getFileSize(const String& path, Int64& size)
		{
			size = 0;
			return true;
		}

		UInt64 ZeroFileSystem::fileSize(FileHandle fh)
		{
			return 0;
		}

		bool ZeroFileSystem::getDirectoryContents(const String& path, StringArray& dirEntries)
		{
			dirEntries = StringArray();
			return true;
		}

		bool ZeroFileSystem::renameFile(const String& oldFileName, const String& newFileName)
		{
			return true;
		}

		size_t ZeroFileSystem::read(const FileHandle& hdl, void* bfr, size_t numberOfBytes, Int64 offset)
		{
			memset(bfr, 0, numberOfBytes);
			return numberOfBytes;
		}

		size_t ZeroFileSystem::write(FileHandle hdl, const void* bfr, size_t numberOfBytes, Int64 offset)
		{
			errno = EBADF;
			return size_t(-1);
		}

		Int64 ZeroFileSystem::seek(const FileHandle& hdl, Int64 offset, int whence)
		{
			return 0;
		}

		Int64 ZeroFileSystem::tell(const FileHandle& hdl)
		{
			return 0;
		}

		void ZeroFileSystem::rewind(const FileHandle& hdl)
		{
		}

		int ZeroFileSystem::close(const FileHandle& hdl)
		{
			return 0;
		}

		int ZeroFileSystem::flush(FileHandle& hdl)
		{
			return 0;
		}

		String ZeroFileSystem::getFileContents(const String& filename)
		{
			return String();
		}

		StringArray ZeroFileSystem::getFileLines(const String& filename)
		{
			return StringArray();
		}

		blocxx::Reference<blocxx::FileSystemMockObject> createZeroFSObject()
		{
			return blocxx::Reference<blocxx::FileSystemMockObject>(new ZeroFileSystem);
		}

	} // end namespace Test
} // end namespace BLOCXX_NAMESPACE
