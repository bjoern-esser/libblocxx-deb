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
 * @author Mat Bess
 */


#include "blocxx/BLOCXX_config.h"
#include "blocxx_test/FileUtils.hpp"
#include "blocxx_test/TextUtils.hpp"
#include "blocxx_test/LogUtils.hpp"

#include "blocxx/String.hpp"
#include "blocxx/Array.hpp"
#include "blocxx/Paths.hpp"
#include "blocxx/FileSystem.hpp"
#include "blocxx/File.hpp"
#include "blocxx/Types.hpp"
#include "blocxx/IFileStream.hpp"
#include "blocxx/StringStream.hpp"
#include "blocxx/Logger.hpp"
#include "blocxx/Format.hpp"


#define LOG_DEBUG(x) STANDARD_LOG_DEBUG("FileUtils: ", (x))
#define LOG_DEBUG2(x) STANDARD_LOG_DEBUG2("FileUtils: ", (x))
#define LOG_ERROR(x) STANDARD_LOG_ERROR("FileUtils: ", (x))

namespace BLOCXX_NAMESPACE
{
	namespace FileUtils
	{
		namespace // anonymous
		{
			const char* const COMPONENT_NAME = "blocxx.test.utils.file";
		}

		const blocxx::GlobalString EXECUTABLE_DEFAULT_PATH = BLOCXX_LAZY_GLOBAL_INIT(_PATH_STDPATH);

		namespace // anonymous
		{
			blocxx::AutoDescriptor openFileUnprivileged(const blocxx::String& path)
			{
				blocxx::Logger logger(COMPONENT_NAME);
				blocxx::AutoDescriptor fd;

				if( blocxx::FileSystem::canRead(path) && !blocxx::FileSystem::isDirectory(path) )
				{
					fd = blocxx::FileSystem::openFile(path, blocxx::FileSystem::E_READ).releaseDescriptor();
					if( !fd )
					{
						LOG_DEBUG(blocxx::Format("Failed to open file \"%1\": %2", path, strerror(errno)));
					}
				}
				else
				{
					LOG_DEBUG(blocxx::Format("File is not readable: \"%1\".", path));
				}
				return fd;
			}

			bool openFileUnprivileged(const blocxx::String& path, blocxx::IFileStream& target_stream)
			{
				blocxx::AutoDescriptor foo = openFileUnprivileged(path);
				if( foo )
				{
					if( target_stream.isOpen() )
					{
						target_stream.close();
					}

					if( target_stream.open(foo) )
					{
						return true;
					}
					else
					{
						blocxx::Logger logger(COMPONENT_NAME);
						LOG_DEBUG(blocxx::Format("Failed to open stream for opened file \"%1\"", path));
					}
				}
				return false;
			}
		} // end anonymous namespace

		blocxx::String locateExecutable(const blocxx::String& executable_name, const blocxx::String& valid_path)
		{
			blocxx::String result_path;
			blocxx::String searchpath(valid_path);
			result_path = searchPathForFile(searchpath, executable_name);

			if ( result_path.empty() )
			{
				const char* path = getenv("PATH");
				if ( !path )
				{
					path = getenv("path");
				}

				if ( path )
				{
					searchpath = path;
					result_path = searchPathForFile(searchpath, executable_name);
				}
			}
			return result_path;
		}


		blocxx::String searchPathForFile(const blocxx::String& path,
			const blocxx::String& fileName,
			const blocxx::String& pwd,
			const blocxx::String& separator)
		{
			blocxx::String result;

			blocxx::StringArray pathEntries = path.tokenize(separator.c_str());

			if ( !pwd.empty() )
			{
				blocxx::String fleep = pwd + BLOCXX_FILENAME_SEPARATOR + fileName;
				if ( blocxx::FileSystem::exists(fleep) )
				{
					result = fleep;
				}
			}
			if ( result.empty() )
			{
				for (blocxx::StringArray::const_iterator entry = pathEntries.begin();
					  entry != pathEntries.end();
					  ++entry)
				{
					if (!entry->empty())
					{
						blocxx::String fleep = *entry + BLOCXX_FILENAME_SEPARATOR + fileName;
						if (blocxx::FileSystem::exists(fleep))
						{
							result = fleep;
							break;
						}
					}
				}
			}
			return result;
		}

		blocxx::UInt32 catToLogUnprivileged(const blocxx::String& pathname, blocxx::Logger& logger, const blocxx::String& prefix, bool debug)
		{
			// FIXME! EEP! Not mocked!

			blocxx::IFileStream input;
			blocxx::UInt32 lines_dumped = 0;
			bool fileOpened = openFileUnprivileged(pathname, input);

			if( fileOpened && input.good() )
			{
				while( input )
				{
					blocxx::String s = blocxx::String::getLine(input);

					if( debug )
					{
						BLOCXX_LOG_DEBUG(logger, prefix + s);
					}
					else
					{
						BLOCXX_LOG_ERROR(logger, prefix + s);
					}
					++lines_dumped;
				}
			}

			return lines_dumped;
		}

		bool readContentsUnprivileged(const blocxx::String& pathname, blocxx::StringArray& lines)
		{
			try
			{
				lines = blocxx::FileSystem::getFileLines(pathname);
				return true;
			}
			catch(const blocxx::FileSystemException& e)
			{
				blocxx::Logger logger(COMPONENT_NAME);
				BLOCXX_LOG_DEBUG3(logger, blocxx::Format("getFileLines() Failed to read from \"%1\": %2", pathname, e));
				return false;
			}
		}

		bool readContentsUnprivileged(const blocxx::String& pathname, blocxx::String& contents)
		{
			try
			{
				contents = blocxx::FileSystem::getFileContents(pathname);
				return true;
			}
			catch(const blocxx::FileSystemException& e)
			{
				blocxx::Logger logger(COMPONENT_NAME);
				BLOCXX_LOG_DEBUG3(logger, blocxx::Format("getFileContents() Failed to read from \"%1\": %2", pathname, e));
				return false;
			}
		}

		bool readHeadUnprivileged(
			const blocxx::String& pathname,
			blocxx::StringArray& lines,
			blocxx::UInt32 max_lines)
		{
			if( max_lines == 0 )
			{
				return readContentsUnprivileged(pathname, lines);
			}

			bool retval = false;
			blocxx::IFileStream input;
			bool fileOpened = openFileUnprivileged(pathname, input);

			if( fileOpened && input.good() )
			{
				lines.erase(lines.begin(), lines.end());

				for( blocxx::UInt32 lines_read = 0;
					  (lines_read < max_lines) && input;
					  ++lines_read )
				{
					lines.push_back(blocxx::String::getLine(input));
				}
				retval = true;
			}
			return retval;
		}

		blocxx::String followSymlink(const blocxx::String& pathname)
		{
			blocxx::String retVal = pathname;
			try
			{
				retVal = blocxx::FileSystem::readSymbolicLink(pathname);
			}
			catch(blocxx::FileSystemException& e)
			{
				if( !blocxx::FileSystem::exists(pathname) )
				{
					// the path was not a symlink
					return "";
				}
			}
			return retVal;
		}

		blocxx::String fixPathForMonitor(const blocxx::String& pathname)
		{
			blocxx::String fixedPath = TextUtils::removeRedundantSeparators(pathname);
			// Trim the trailing '/' so we don't have a chance of the monitor
			// up and quitting because of KSVH's decision to disallow trailing
			// slashes.

			if( fixedPath.endsWith('/') && (fixedPath.length() > 1) )
			{
				fixedPath = fixedPath.substring(0, fixedPath.length() - 1);
			}
			return fixedPath;
		}

		bool linkIsAcceptableForRecursion(const String& start, const String& link)
		{
			Logger logger(COMPONENT_NAME);

			// Since any random paths can be passed in, we must clean them before comparison.
			String startDirectory = TextUtils::removeRedundantSeparators(start);
			String symlinkPath = TextUtils::removeRedundantSeparators(link);

			LOG_DEBUG2(Format("Checking symlink \"%1\" to see if it should be followed.  Start directory=\"%2\"", symlinkPath, startDirectory));

			// Symlink matching startDirectory file is no good.
			if( symlinkPath == startDirectory )
			{
				LOG_DEBUG2(Format("Ignored link of startDirectory directory \"%1\"", startDirectory));
				return false;
			}

			try
			{
				// StartDirectory directory starting with real path of symlink is no good.
				String linkDest = FileSystem::Path::realPath(symlinkPath);
				if( startDirectory.startsWith(TextUtils::removeRedundantSeparators(linkDest + "/")) )
				{
					LOG_DEBUG2(Format("Ignored symlink link \"%1\": realpath (%2) is a parent of start directory: \"%3\"", symlinkPath, linkDest, startDirectory));
					return false;
				}

				// Real path of startDirectory directory matching real path of link is no good.
				String fullDest = FileSystem::Path::realPath(startDirectory);
				if( linkDest == fullDest )
				{
					LOG_DEBUG2(Format("Ignored symlink \"%1\": realpath (%2) matches startDirectory directory.", symlinkPath, linkDest));
					return false;
				}

				// Real path of startDirectory directory starting with real path of link is no good.
				if( fullDest.startsWith(TextUtils::removeRedundantSeparators(linkDest + "/")) )
				{
					LOG_DEBUG2(Format("Ignored link \"%1\": realpath (%2) is a parent of the real path (%3) of the startDirectory dir: \"%4\"", symlinkPath, linkDest, fullDest, startDirectory));
					return false;
				}

				LOG_DEBUG2(Format("link \"%1\" looks ok.  It points to \"%2\".", symlinkPath, linkDest));
			}
			catch(const FileSystemException& e)
			{
				LOG_DEBUG2(Format("Caught a filesystem exception while checking link: %1", e));
				return false;
			}
			catch(const Exception& e)
			{
				LOG_DEBUG2(Format("Caught an exception while checking link: %1", e));
				return false;
			}

			return true;
		}

		void recursiveDeleteFiles(const String& basePath, FileDeletionTypes reallyRemove)
		{
			Logger logger(COMPONENT_NAME);
			if (FileSystem::isDirectory(basePath))
			{
				LOG_DEBUG(Format("Removing all files in %1", basePath));
				StringArray filesInDirectory;
				if (FileSystem::getDirectoryContents(basePath, filesInDirectory))
				{
					for (
						StringArray::const_iterator fileIter = filesInDirectory.begin();
						fileIter != filesInDirectory.end();
						++fileIter
					)
					{
						if (*fileIter == "." || *fileIter == "..")
						{
							continue;
						}
						String sub_path = basePath + BLOCXX_FILENAME_SEPARATOR + *fileIter;

						if (FileSystem::isDirectory(sub_path))
						{
							LOG_DEBUG2(Format("Entering directory %1", sub_path));
							recursiveDeleteFiles(sub_path, reallyRemove);
							LOG_DEBUG2(Format("Leaving directory %1", sub_path));
						}
						else
						{
							LOG_DEBUG2(Format("Removing file %1", sub_path));
							if ((reallyRemove == E_I_REALLY_MEAN_TO_DELETE) && !FileSystem::removeFile(sub_path))
							{
								LOG_DEBUG2(Format("Failed to remove file: %1", sub_path));
							}
						}
					}
					LOG_DEBUG2(Format("Removing directory: %1", basePath));
					if ((reallyRemove == E_I_REALLY_MEAN_TO_DELETE) && !FileSystem::removeDirectory(basePath))
					{
						LOG_DEBUG2(Format("Failed to remove directory: %1", basePath));
					}
				}
			}
			else
			{
				LOG_DEBUG2(Format("Removing file: %1", basePath));
				if ((reallyRemove == E_I_REALLY_MEAN_TO_DELETE) && !FileSystem::removeFile(basePath))
				{
					LOG_DEBUG2(Format("Failed to remove file: %1", basePath));
				}
			}
		}

	} // end namespace FileUtils
} // end namespace BLOCXX_NAMESPACE
