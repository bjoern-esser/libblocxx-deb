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

#include "blocxx/BLOCXX_config.h"
#include "blocxx/FileSystemMockObject.hpp"
#include "blocxx/Exception.hpp"
#include "blocxx/File.hpp"
#include "blocxx/String.hpp"
#include "blocxx/Array.hpp"

namespace BLOCXX_NAMESPACE
{

BLOCXX_DECLARE_EXCEPTION(FileSystemMockObjectUnimplemented);
BLOCXX_DEFINE_EXCEPTION(FileSystemMockObjectUnimplemented);

FileSystemMockObject::~FileSystemMockObject()
{
}

File
FileSystemMockObject::openFile(const String& path)
{
	BLOCXX_THROW(FileSystemMockObjectUnimplementedException, "openFile");
}
File
FileSystemMockObject::openFile(const String& path, FileSystem::EOpenModeFlag mode)
{
	BLOCXX_THROW(FileSystemMockObjectUnimplementedException, "openFile");
}
File
FileSystemMockObject::createFile(const String& path)
{
	BLOCXX_THROW(FileSystemMockObjectUnimplementedException, "createFile");
}
File
FileSystemMockObject::openOrCreateFile(const String& path)
{
	BLOCXX_THROW(FileSystemMockObjectUnimplementedException, "openOrCreateFile");
}
File
FileSystemMockObject::openForAppendOrCreateFile(const String& path)
{
	BLOCXX_THROW(FileSystemMockObjectUnimplementedException, "openForAppendOrCreateFile");
}
File
FileSystemMockObject::createTempFile(String& filePath, const String& dir)
{
	BLOCXX_THROW(FileSystemMockObjectUnimplementedException, "createTempFile");
}
File
FileSystemMockObject::createTempFile(const String& dir)
{
	BLOCXX_THROW(FileSystemMockObjectUnimplementedException, "createTempFile");
}
int
FileSystemMockObject::changeFileOwner(const String& filename,	const UserId& userId)
{
	BLOCXX_THROW(FileSystemMockObjectUnimplementedException, "changeFileOwner");
}
bool
FileSystemMockObject::exists(const String& path)
{
	BLOCXX_THROW(FileSystemMockObjectUnimplementedException, "exists");
}
#ifndef BLOCXX_WIN32
bool
FileSystemMockObject::isExecutable(const String& path)
{
	BLOCXX_THROW(FileSystemMockObjectUnimplementedException, "isExecutable");
}
#endif
bool
FileSystemMockObject::canRead(const String& path)
{
	BLOCXX_THROW(FileSystemMockObjectUnimplementedException, "canRead");
}
bool
FileSystemMockObject::canWrite(const String& path)
{
	BLOCXX_THROW(FileSystemMockObjectUnimplementedException, "canWrite");
}
#ifndef BLOCXX_WIN32
bool
FileSystemMockObject::isLink(const String& path)
{
	BLOCXX_THROW(FileSystemMockObjectUnimplementedException, "isLink");
}
#endif
bool
FileSystemMockObject::isDirectory(const String& path)
{
	BLOCXX_THROW(FileSystemMockObjectUnimplementedException, "isDirectory");
}
bool
FileSystemMockObject::changeDirectory(const String& path)
{
	BLOCXX_THROW(FileSystemMockObjectUnimplementedException, "changeDirectory");
}
bool
FileSystemMockObject::makeDirectory(const String& path, int mode)
{
	BLOCXX_THROW(FileSystemMockObjectUnimplementedException, "makeDirectory");
}
bool
FileSystemMockObject::getFileSize(const String& path, Int64& size)
{
	BLOCXX_THROW(FileSystemMockObjectUnimplementedException, "getFileSize");
}
UInt64
FileSystemMockObject::fileSize(FileHandle fh)
{
	BLOCXX_THROW(FileSystemMockObjectUnimplementedException, "fileSize");
}
bool
FileSystemMockObject::removeDirectory(const String& path)
{
	BLOCXX_THROW(FileSystemMockObjectUnimplementedException, "removeDirectory");
}
bool
FileSystemMockObject::removeFile(const String& path)
{
	BLOCXX_THROW(FileSystemMockObjectUnimplementedException, "removeFile");
}
bool
FileSystemMockObject::getDirectoryContents(const String& path, StringArray& dirEntries)
{
	BLOCXX_THROW(FileSystemMockObjectUnimplementedException, "getDirectoryContents");
}
bool
FileSystemMockObject::renameFile(const String& oldFileName, const String& newFileName)
{
	BLOCXX_THROW(FileSystemMockObjectUnimplementedException, "renameFile");
}
size_t
FileSystemMockObject::read(const FileHandle& hdl, void* bfr, size_t numberOfBytes, Int64 offset)
{
	BLOCXX_THROW(FileSystemMockObjectUnimplementedException, "read");
}
size_t
FileSystemMockObject::write(FileHandle hdl, const void* bfr, size_t numberOfBytes, Int64 offset)
{
	BLOCXX_THROW(FileSystemMockObjectUnimplementedException, "write");
}
Int64
FileSystemMockObject::seek(const FileHandle& hdl, Int64 offset, int whence)
{
	BLOCXX_THROW(FileSystemMockObjectUnimplementedException, "seek");
}
Int64
FileSystemMockObject::tell(const FileHandle& hdl)
{
	BLOCXX_THROW(FileSystemMockObjectUnimplementedException, "tell");
}
void
FileSystemMockObject::rewind(const FileHandle& hdl)
{
	BLOCXX_THROW(FileSystemMockObjectUnimplementedException, "rewind");
}
int
FileSystemMockObject::close(const FileHandle& hdl)
{
	BLOCXX_THROW(FileSystemMockObjectUnimplementedException, "close");
}
int
FileSystemMockObject::flush(FileHandle& hdl)
{
	BLOCXX_THROW(FileSystemMockObjectUnimplementedException, "flush");
}
String
FileSystemMockObject::getFileContents(const String& filename)
{
	BLOCXX_THROW(FileSystemMockObjectUnimplementedException, "getFileContents");
}
StringArray
FileSystemMockObject::getFileLines(const String& filename)
{
	BLOCXX_THROW(FileSystemMockObjectUnimplementedException, "getFileLines");
}
String
FileSystemMockObject::readSymbolicLink(const String& path)
{
	BLOCXX_THROW(FileSystemMockObjectUnimplementedException, "readSymbolicLink");
}
String
FileSystemMockObject::realPath(const String& path)
{
	BLOCXX_THROW(FileSystemMockObjectUnimplementedException, "realPath");
}

std::pair<FileSystem::Path::ESecurity, String>
FileSystemMockObject::security(String const & path, UserId uid)
{
	BLOCXX_THROW(FileSystemMockObjectUnimplementedException, "security");
}

std::pair<FileSystem::Path::ESecurity, String>
FileSystemMockObject::security(String const & path)
{
	BLOCXX_THROW(FileSystemMockObjectUnimplementedException, "security");
}

std::pair<FileSystem::Path::ESecurity, String>
FileSystemMockObject::security(String const & base_dir, String const & rel_path, UserId uid)
{
	BLOCXX_THROW(FileSystemMockObjectUnimplementedException, "security");
}

std::pair<FileSystem::Path::ESecurity, String>
FileSystemMockObject::security(String const & base_dir, String const & rel_path)
{
	BLOCXX_THROW(FileSystemMockObjectUnimplementedException, "security");
}

String
FileSystemMockObject::dirname(const String& filename)
{
	BLOCXX_THROW(FileSystemMockObjectUnimplementedException, "dirname");
}

String
FileSystemMockObject::basename(const String& filename)
{
	BLOCXX_THROW(FileSystemMockObjectUnimplementedException, "basename");
}

String
FileSystemMockObject::getCurrentWorkingDirectory()
{
	BLOCXX_THROW(FileSystemMockObjectUnimplementedException, "getCurrentWorkingDirectory");
}

FileSystem::FileInformation
FileSystemMockObject::getFileInformation(const String& filename)
{
	BLOCXX_THROW(FileSystemMockObjectUnimplementedException, "getFileInformation");
}

} // end namespace BLOCXX_NAMESPACE




