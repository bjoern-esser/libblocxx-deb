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
#include "blocxx_test/TestFileSystem.hpp"
#include "blocxx/FileSystem.hpp"
#include "blocxx/File.hpp"
#include "blocxx/Logger.hpp"
#include "blocxx/Format.hpp"

#include "blocxx_test/LogUtils.hpp"
#include "blocxx_test/FileUtils.hpp"

#define LOG_DEBUG(x) STANDARD_LOG_DEBUG("TestFileSystem: ", (x))
#define LOG_DEBUG2(x) STANDARD_LOG_DEBUG2("TestFileSystem: ", (x))
#define LOG_ERROR(x) STANDARD_LOG_ERROR("TestFileSystem: ", (x))

namespace BLOCXX_NAMESPACE
{
	namespace Test
	{
		using namespace blocxx;

		namespace
		{
			const char* const COMPONENT_NAME = "blocxx.test.filesystem";
		}

		TestDirectory::TestDirectory(const String& testname)
			: ReferenceBase()
			, m_dirname(FileSystem::Path::getCurrentWorkingDirectory() + "/" + testname)
		{
			Logger logger(COMPONENT_NAME);
			LOG_DEBUG2(Format("Precleaning directory \"%1\"", m_dirname));
			FileUtils::recursiveDeleteFiles(m_dirname, FileUtils::E_I_REALLY_MEAN_TO_DELETE);
			FileSystem::makeDirectory(m_dirname);
		}

		TestDirectory::TestDirectory(const TestDirectory& dir)
			: ReferenceBase(dir)
			, m_dirname(dir.m_dirname)
		{
		}

		TestDirectory::~TestDirectory()
		{
			if( ReferenceBase::decRef() )
			{
				Logger logger(COMPONENT_NAME);
				LOG_DEBUG2(Format("Cleaning (final cleanup) directory \"%1\"", m_dirname));
				FileUtils::recursiveDeleteFiles(m_dirname, FileUtils::E_I_REALLY_MEAN_TO_DELETE);
			}
		}

		String TestDirectory::getDirectory() const
		{
			return m_dirname;
		}

		TestFile::TestFile(const TestDirectory& testdir, const String& filename)
			: m_testdir(testdir)
		{
			setfile(filename);
		}

		TestFile::TestFile(const TestDirectory& testdir, const String& filename, const String contents)
			: m_testdir(testdir)
		{
			setfile(filename);
			setContents(contents);
		}

		TestFile::~TestFile()
		{
			FileSystem::removeFile(m_filename);
		}

		String TestFile::getFilename() const
		{
			return m_filename;
		}

		void TestFile::setContents(const String& contents)
		{
			if( !contents.empty() )
			{
				File f = FileSystem::openFile(m_filename, FileSystem::E_WRITE);

				if( !f )
				{
					BLOCXX_THROW_ERRNO_MSG(FileSystemException, Format("Failed to open file \"%1\" for writing", m_filename));
				}


				size_t offset = 0;
				while( offset < contents.size() )
				{
					size_t amount = f.write(contents.c_str() + offset, contents.length() - offset);
					if( amount == size_t(-1) )
					{
						BLOCXX_THROW_ERRNO_MSG(FileSystemException, Format("Failed to write to file \"%1\" at position %2", m_filename, offset).c_str());
					}
					offset += amount;
				}
			}
		}

		String TestFile::getContents() const
		{
			return FileSystem::getFileContents(m_filename);
		}

		void TestFile::setfile(const String& filename)
		{
			m_filename = m_testdir.getDirectory() + "/" + filename;
			FileSystem::createFile(m_filename);
		}
	}
} // end namespace BLOCXX_NAMESPACE
