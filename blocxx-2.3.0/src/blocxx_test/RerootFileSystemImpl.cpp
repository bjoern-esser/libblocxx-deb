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
 * @author Alexander Furmanov
 */

#include "blocxx/BLOCXX_config.h"
#include "blocxx_test/RerootFileSystemImpl.hpp"
#include "blocxx_test/TextUtils.hpp"
#include "blocxx_test/LogUtils.hpp"

#include "blocxx/Logger.hpp"
#include "blocxx/Format.hpp"
#include "blocxx/File.hpp"
#include "blocxx/Map.hpp"
#include "blocxx/RegEx.hpp"

using namespace blocxx;

namespace BLOCXX_NAMESPACE
{
	namespace Test
	{
		namespace RerootFSImpl
		{
			namespace // anonymous
			{
				const GlobalString COMPONENT_NAME = BLOCXX_LAZY_GLOBAL_INIT("blocxx.test.RerootFileSystem");
			} // end anonymous namespace

			namespace aux
			{
				struct SkipMockupObjectScope
				{
					SkipMockupObjectScope()
					{
						m_oldMockupObject = FileSystem::g_fileSystemMockObject.set(0);
					}
					~SkipMockupObjectScope()
					{
						FileSystem::g_fileSystemMockObject.set(m_oldMockupObject);
					}
					FileSystemMockObject* m_oldMockupObject;
				};

				String cleanupDir(const String& path)
				{
					String cleaned(TextUtils::removeRedundantSeparators(path));

					// remove all "/./"s
					cleaned = TextUtils::searchAndReplace(cleaned, "/./", "/");

					// Remove sequences of "../"
					RegEx parentMatcher("(^|/)[^/]*/\\.\\.(/|$)");

					for( StringArray matches = parentMatcher.capture(cleaned);
						!matches.empty();
						matches = parentMatcher.capture(cleaned) )
					{
						cleaned = TextUtils::substringBefore(cleaned, *matches.begin()) + "/" + TextUtils::substringAfter(cleaned, *matches.begin());
					}

					// Remove leading dots (prevent escape).  Multiples were already removed above.
					if( cleaned.startsWith("../") )
					{
						cleaned = cleaned.substring(3);
					}

					// Remove a leading "./".  interior multiples were already removed.
					if( cleaned.startsWith("./") )
					{
						cleaned = cleaned.substring(2);
					}

					// Remove trailing "/." (not matched by the regex above).
					if( cleaned.endsWith("/.") )
					{
						cleaned = cleaned.substring(0, cleaned.length() - 2);
					}

					// If it ends with '/', remove it.
					if( cleaned.endsWith('/') )
					{
						cleaned = cleaned.substring(0, cleaned.length() - 1);
					}

					if( cleaned.empty() )
					{
						// The dots completely ate the directory, or it was just '/', which was just removed...
						cleaned = "/";
					}

					return cleaned;
				}
			} // aux

			// This class is not exposed.  It is just an implementation of the
			// FileSystemMockObject.  Functionality wil be added as needed.
			class RerootedFileSystemObject : public FileSystemMockObject
			{
			public:
				RerootedFileSystemObject(String const& newRoot);
				virtual ~RerootedFileSystemObject();

				void addFileMapping(const String& path, const String& realpath);
				bool removeFileMapping(const String& path);
			private:
				virtual File openFile(const String& path);
				virtual File openFile(const String& path, FileSystem::EOpenModeFlag);
				virtual File createFile(const String& path);
				virtual File openOrCreateFile(const String& path);
				virtual bool exists(const String& path);
				virtual bool canRead(const String& path);
				virtual bool canWrite(const String& path);
				virtual bool isDirectory(const String& path);
				virtual bool changeDirectory(const String& path);
				virtual bool makeDirectory(const String& path, int mode = 0777);
				virtual bool getFileSize(const String& path, Int64& size);
				virtual UInt64 fileSize(FileHandle fh);
				virtual bool removeDirectory(const String& path);
				virtual bool removeFile(const String& path);
				virtual bool getDirectoryContents(const String& path, StringArray& dirEntries);
				virtual bool renameFile(const String& oldFileName, const String& newFileName);
				virtual size_t read(const FileHandle& hdl, void* bfr, size_t numberOfBytes, Int64 offset = -1L);
				virtual size_t write(FileHandle hdl, const void* bfr, size_t numberOfBytes, Int64 offset = -1L);
				virtual Int64 seek(const FileHandle& hdl, Int64 offset, int whence);
				virtual Int64 tell(const FileHandle& hdl);
				virtual void rewind(const FileHandle& hdl);
				virtual int close(const FileHandle& hdl);
				virtual int flush(FileHandle& hdl);
				virtual String getFileContents(const String& filename);
				virtual StringArray getFileLines(const String& filename);
				virtual String readSymbolicLink(const String& path);
				virtual String realPath(const String& path);

				virtual String dirname(const String& filename);
				virtual String basename(const String& filename);
				virtual FileSystem::FileInformation getFileInformation(const String& filename);
				virtual String getCurrentWorkingDirectory();
			private:
				String Translate(String const& path) const;

				String getMappedPath(const String& path) const;
			private:
				String m_root;
				Map<String,String> m_fileMapper;
				String m_currentDir;
			};

#define LOG_DEBUG3(X) STANDARD_LOG_DEBUG3( "RerootedFileSystemObject:  ", (X) )
#define LOG_ERROR(X) STANDARD_LOG_ERROR( "RerootedFileSystemObject:  ", (X) )

			RerootedFileSystemObject::RerootedFileSystemObject(String const& newRoot)
				: m_root(newRoot)
				, m_currentDir("/")
			{
			}

			RerootedFileSystemObject::~RerootedFileSystemObject()
			{
			}

			File RerootedFileSystemObject::openFile(const String& path)
			{
				Logger logger(COMPONENT_NAME);
				LOG_DEBUG3( Format("openFile() called for \"%1\"", path) );
				aux::SkipMockupObjectScope fileSystemScope;
				return FileSystem::openFile( Translate(path) );
			}

			File RerootedFileSystemObject::openFile(const String& path, FileSystem::EOpenModeFlag mode)
			{
				Logger logger(COMPONENT_NAME);
				LOG_DEBUG3( Format("openFile() called for \"%1\"", path) );
				aux::SkipMockupObjectScope fileSystemScope;
				return FileSystem::openFile(Translate(path), mode);
			}

			File RerootedFileSystemObject::createFile(const String& path)
			{
				Logger logger(COMPONENT_NAME);
				LOG_DEBUG3( Format("createFile() called for \"%1\"", path) );
				aux::SkipMockupObjectScope fileSystemScope;
				return FileSystem::createFile( Translate(path) );
			}

			File RerootedFileSystemObject::openOrCreateFile(const String& path)
			{
				Logger logger(COMPONENT_NAME);
				LOG_DEBUG3( Format("openOrCreateFile() called for \"%1\"", path) );
				aux::SkipMockupObjectScope fileSystemScope;
				return FileSystem::openOrCreateFile( Translate(path) );
			}

			bool RerootedFileSystemObject::exists(const String& path)
			{
				Logger logger(COMPONENT_NAME);
				LOG_DEBUG3( Format("exists() called for \"%1\"", path) );
				aux::SkipMockupObjectScope fileSystemScope;
				bool retval = FileSystem::exists( Translate(path) );
				LOG_DEBUG3( Format("exists(\"%1\") --> %2", path, retval) );
				return retval;
			}

			bool RerootedFileSystemObject::canRead(const String& path)
			{
				Logger logger(COMPONENT_NAME);
				LOG_DEBUG3( Format("canRead() called for \"%1\"", path) );
				aux::SkipMockupObjectScope fileSystemScope;
				bool retval = FileSystem::canRead( Translate(path) );
				LOG_DEBUG3( Format("canRead(\"%1\") --> %2", path, retval) );
				return retval;
			}

			bool RerootedFileSystemObject::canWrite(const String& path)
			{
				Logger logger(COMPONENT_NAME);
				LOG_DEBUG3( Format("canWrite() called for \"%1\"", path) );
				aux::SkipMockupObjectScope fileSystemScope;
				bool retval = FileSystem::canWrite( Translate(path) );
				LOG_DEBUG3( Format("canWrite(\"%1\") --> %2", path, retval) );
				return retval;
			}

			bool RerootedFileSystemObject::isDirectory(const String& path)
			{
				Logger logger(COMPONENT_NAME);
				LOG_DEBUG3( Format("isDirectory() called for \"%1\"", path) );
				aux::SkipMockupObjectScope fileSystemScope;
				bool retval = FileSystem::isDirectory( Translate(path) );
				LOG_DEBUG3( Format("isDirectory(\"%1\") --> %2", path, retval) );
				return retval;
			}

			bool RerootedFileSystemObject::changeDirectory(const String& path)
			{
				Logger logger(COMPONENT_NAME);
				LOG_DEBUG3( Format("changeDirectory() called for \"%1\"", path) );

				String fullPath = path;
				if ( !fullPath.startsWith('/') )
				{
					fullPath = m_currentDir + "/" + path;
				}

				fullPath = aux::cleanupDir(fullPath);

				String translated = Translate(fullPath);

				if( !exists(translated) )
				{
					LOG_DEBUG3(Format("changeDirectory attempted to non-existant location: \"%1\" (%2)", path, translated));
					return false;
				}
				m_currentDir = fullPath;

				LOG_DEBUG3( Format("changeDirectory(\"%1\") --> %2", path, true) );
				return true;
			}

			bool RerootedFileSystemObject::makeDirectory(const String& path, int mode)
			{
				Logger logger(COMPONENT_NAME);
				LOG_DEBUG3( Format("makeDirectory() called for \"%1\"", path) );
				aux::SkipMockupObjectScope fileSystemScope;
				bool retval = FileSystem::makeDirectory(Translate(path), mode);
				LOG_DEBUG3( Format("makeDirectory(\"%1\") --> %2", path, retval) );
				return retval;
			}

			bool RerootedFileSystemObject::getFileSize(const String& path, Int64& size)
			{
				Logger logger(COMPONENT_NAME);
				LOG_DEBUG3( Format("getFileSize() called for \"%1\"", path) );
				aux::SkipMockupObjectScope fileSystemScope;
				bool retval = FileSystem::getFileSize(Translate(path), size);
				LOG_DEBUG3( Format("getFileSize(\"%1\") --> %2 (%3)", path, retval, size) );
				return retval;
			}

			UInt64 RerootedFileSystemObject::fileSize(FileHandle fh)
			{
				Logger logger(COMPONENT_NAME);
				LOG_DEBUG3("fileSize() called.");
				aux::SkipMockupObjectScope fileSystemScope;
				return FileSystem::fileSize(fh);
			}

			bool RerootedFileSystemObject::removeDirectory(const String& path)
			{
				Logger logger(COMPONENT_NAME);
				LOG_DEBUG3( Format("removeDirectory() called for \"%1\"", path) );
				aux::SkipMockupObjectScope fileSystemScope;
				bool retval = FileSystem::removeDirectory( Translate(path) );
				LOG_DEBUG3( Format("removeDirectory(\"%1\") --> %2", path, retval) );
				return retval;
			}

			bool RerootedFileSystemObject::removeFile(const String& path)
			{
				Logger logger(COMPONENT_NAME);
				LOG_DEBUG3( Format("removeFile() called for \"%1\"", path) );
				aux::SkipMockupObjectScope fileSystemScope;
				bool retval = FileSystem::removeFile( Translate(path) );
				LOG_DEBUG3( Format("removeFile(\"%1\") --> %2", path, retval) );
				return retval;
			}

			bool RerootedFileSystemObject::getDirectoryContents(const String& path, StringArray& dirEntries)
			{
				Logger logger(COMPONENT_NAME);
				LOG_DEBUG3( Format("getDirectoryContents() called for \"%1\"", path) );
				aux::SkipMockupObjectScope fileSystemScope;
				bool retval = FileSystem::getDirectoryContents(Translate(path), dirEntries);
				LOG_DEBUG3( Format("getDirectoryContents(\"%1\") --> %2", path, retval) );
				return retval;
			}

			bool RerootedFileSystemObject::renameFile(const String& oldFileName, const String& newFileName)
			{
				Logger logger(COMPONENT_NAME);
				LOG_DEBUG3( Format("renameFile() called, from \"%1\" to \"%2\"", oldFileName, newFileName) );
				aux::SkipMockupObjectScope fileSystemScope;
				bool retval = FileSystem::renameFile( Translate(oldFileName), Translate(newFileName) );
				LOG_DEBUG3( Format("renameFile(\"%1\",\"%2\") --> %3", oldFileName, newFileName, retval) );
				return retval;
			}

			size_t RerootedFileSystemObject::read(const FileHandle& hdl, void* bfr, size_t numberOfBytes, Int64 offset)
			{
				Logger logger(COMPONENT_NAME);
				LOG_DEBUG3(Format("read(%1, #bytes=%2, offset=%3) called.", hdl, numberOfBytes, offset));
				aux::SkipMockupObjectScope fileSystemScope;
				return FileSystem::read(hdl, bfr, numberOfBytes, offset);
			}

			size_t RerootedFileSystemObject::write(FileHandle hdl, const void* bfr, size_t numberOfBytes, Int64 offset)
			{
				Logger logger(COMPONENT_NAME);
				LOG_DEBUG3("write() called.");
				aux::SkipMockupObjectScope fileSystemScope;
				return FileSystem::write(hdl, bfr, numberOfBytes, offset);
			}

			Int64 RerootedFileSystemObject::seek(const FileHandle& hdl, Int64 offset, int whence)
			{
				Logger logger(COMPONENT_NAME);
				LOG_DEBUG3(Format("seek(%1,%2,%3) called.", hdl, offset, whence));
				aux::SkipMockupObjectScope fileSystemScope;
				return FileSystem::seek(hdl, offset, whence);
			}

			Int64 RerootedFileSystemObject::tell(const FileHandle& hdl)
			{
				Logger logger(COMPONENT_NAME);
				LOG_DEBUG3(Format("tell(%1) called.", hdl));
				aux::SkipMockupObjectScope fileSystemScope;
				return FileSystem::tell(hdl);
			}

			void RerootedFileSystemObject::rewind(const FileHandle& hdl)
			{
				Logger logger(COMPONENT_NAME);
				LOG_DEBUG3(Format("rewind(%1) called.", hdl));
				aux::SkipMockupObjectScope fileSystemScope;
				return FileSystem::rewind(hdl);
			}

			int RerootedFileSystemObject::close(const FileHandle& hdl)
			{
				Logger logger(COMPONENT_NAME);
				LOG_DEBUG3("close() called.");
				aux::SkipMockupObjectScope fileSystemScope;
				return FileSystem::close(hdl);
			}

			int RerootedFileSystemObject::flush(FileHandle& hdl)
			{
				Logger logger(COMPONENT_NAME);
				LOG_DEBUG3("flush() called.");
				aux::SkipMockupObjectScope fileSystemScope;
				return FileSystem::flush(hdl);
			}

			String RerootedFileSystemObject::getFileContents(const String& filename)
			{
				Logger logger(COMPONENT_NAME);
				LOG_DEBUG3("getFileContent() called.");
				aux::SkipMockupObjectScope fileSystemScope;
				return FileSystem::getFileContents( Translate(filename) );
			}

			StringArray RerootedFileSystemObject::getFileLines(const String& filename)
			{
				Logger logger(COMPONENT_NAME);
				LOG_DEBUG3( Format("getFileLines() called for \"%1\"", filename) );
				aux::SkipMockupObjectScope fileSystemScope;
				return FileSystem::getFileLines( Translate(filename) );
			}

			String RerootedFileSystemObject::readSymbolicLink(const String& path)
			{
				Logger logger(COMPONENT_NAME);
				LOG_DEBUG3( Format("readSymbolicLink() called for \"%1\"", path) );
				aux::SkipMockupObjectScope fileSystemScope;
				return FileSystem::readSymbolicLink( Translate(path) );
			}

			String RerootedFileSystemObject::realPath(const String& path)
			{
				Logger logger(COMPONENT_NAME);
				LOG_DEBUG3( Format("realPath() called for \"%1\"", path) );
				aux::SkipMockupObjectScope fileSystemScope;
				return FileSystem::Path::realPath( Translate (path) );
			}

			String RerootedFileSystemObject::dirname(const String& filename)
			{
				Logger logger(COMPONENT_NAME);
				LOG_DEBUG3( Format("dirname() called for \"%1\"", filename) );
				aux::SkipMockupObjectScope fileSystemScope;
				return FileSystem::Path::dirname( Translate (filename) );
			}

			String RerootedFileSystemObject::basename(const String& filename)
			{
				Logger logger(COMPONENT_NAME);
				LOG_DEBUG3( Format("basename() called for \"%1\"", filename) );
				aux::SkipMockupObjectScope fileSystemScope;
				return FileSystem::Path::basename( Translate (filename) );
			}

			FileSystem::FileInformation RerootedFileSystemObject::getFileInformation(const String& filename)
			{
				Logger logger(COMPONENT_NAME);
				LOG_DEBUG3( Format("getFileInformation() called for \"%1\"", filename) );
				aux::SkipMockupObjectScope fileSystemScope;
				return FileSystem::getFileInformation( Translate (filename) );
			}

			String RerootedFileSystemObject::getCurrentWorkingDirectory()
			{
				Logger logger(COMPONENT_NAME);
				LOG_DEBUG3( Format("getCurrentWorkingDirectory() called --> \"%1\"", m_currentDir) );
				return m_currentDir;
			}

			String RerootedFileSystemObject::getMappedPath(const String& path) const
			{
				// This probably defeats the purpose of having a map... Some freaky
				// combination of lower_bound and substring() could be used...
				// This could be fixed later.  For now, the mapping list should be
				// sufficiently small to ignore this.
				for( Map<String,String>::const_iterator iter = m_fileMapper.begin();
					  iter != m_fileMapper.end();
					  ++iter )
				{
					if( iter->first == path )
					{
						return iter->second;
					}
					else if( path.startsWith(iter->first + "/") )
					{
						// This substring() leaves the "/", so the join will have it.
						return iter->second + path.substring(iter->first.length());
					}
				}
				return String();
			}

			String RerootedFileSystemObject::Translate(const String& path) const
			{
				Logger logger(COMPONENT_NAME);

				String fullPath = path;
				if ( !fullPath.startsWith('/') )
				{
					fullPath = m_currentDir + "/" + path;
				}

				fullPath = aux::cleanupDir(fullPath);

				String result = m_root + fullPath;
				String mapped = getMappedPath(fullPath);

				if( !mapped.empty() )
				{
					result = mapped;
				}

				LOG_DEBUG3( Format("Translate() called for \"%1\" -> \"%2\"", path, result) );
				return result;
			}

			void RerootedFileSystemObject::addFileMapping(const String& path, const String& realpath)
			{
				Logger logger(COMPONENT_NAME);

				LOG_DEBUG3( Format("Adding mapping from virtual path \"%1\" to \"%2\"", path, realpath));

				m_fileMapper[path] = realpath;
			}

#undef LOG_DEBUG3
#undef LOG_ERROR

			FSMockObjectRef createRerootedFSObject(const String& newRoot)
			{
				return Reference<RerootedFileSystemObject>( new RerootedFileSystemObject (newRoot) );
			}


			bool remapFileName(const FSMockObjectRef& obj, const String& path, const String& realpath)
			{
				Reference<RerootedFileSystemObject> foo = obj.cast_to<RerootedFileSystemObject>();

				if( foo )
				{
					foo->addFileMapping(path, realpath);
					return true;
				}
				return false;
			}
		} // end namespace RerootFSImpl
	} // end namespace Test
} // end namespace BLOCXX_NAMESPACE
