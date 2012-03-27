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
 * @author Kevin Harris
 */

#include "blocxx/BLOCXX_config.h"

#define PROVIDE_AUTO_TEST_MAIN
#include "AutoTest.hpp"

#include "blocxx_test/LogUtils.hpp"
#include "blocxx_test/FileSystemMockObjectScope.hpp"
#include "blocxx_test/CannedFileSystem.hpp"
#include "blocxx_test/ZeroFileSystem.hpp"
#include "blocxx_test/TestHackery.hpp"
#include "blocxx_test/FileUtils.hpp"
#include "blocxx_test/TextUtils.hpp"

#include "blocxx/Reference.hpp"
#include "blocxx/File.hpp"
#include "blocxx/String.hpp"
#include "blocxx/Logger.hpp"
#include "blocxx/LogAppenderScope.hpp"
#include "blocxx/CerrAppender.hpp"

#include <utility>
#include <iostream>
#include <map>

using namespace blocxx;

namespace // anonymous
{
	const char* const COMPONENT_NAME("test");
} // end anonymous namespace

AUTO_UNIT_TEST(test_filesystem_basic)
{
	//	LogAppenderScope lgr(new CerrAppender);

	blocxx::Reference<blocxx::FileSystemMockObject> mockfiles = blocxx::Test::createCannedFSObject();
	String foo_bar_text = "/foo/bar is a very lonely file\nAnd this is its second line...";
	blocxx::Test::addNormalFile(mockfiles, "/foo/bar", foo_bar_text);
	blocxx::Test::addNormalFile(mockfiles, "/i/need/a/long/path/name", "I exist");

	{
		Logger logger(COMPONENT_NAME);
		blocxx::Test::FileSystemMockObjectScope mos(mockfiles);

		StringArray lines;
		unitAssert( FileUtils::readContentsUnprivileged("/foo/bar", lines) );
		LogUtils::dumpToLog(lines, logger, "foobar lines: ");

		unitAssertEquals( lines, foo_bar_text.tokenize("\n") );

		String output;
		unitAssert( !FileUtils::readContentsUnprivileged("/bar", output) ); // Doesn't exist.

		unitAssert( !FileUtils::readContentsUnprivileged("/foo", output) ); // Is a directory.

		unitAssert( FileUtils::readContentsUnprivileged("/i/need/a/long/path/name", output) );
		unitAssertEquals(output, "I exist");

		unitAssert( !FileSystem::canRead("/nothere") );
		unitAssert( FileSystem::canRead("/foo") );
		unitAssert( FileSystem::canRead("/foo/bar") );
		unitAssert( !FileSystem::canRead("/foo/missing") );
	}
}

AUTO_UNIT_TEST(test_filesystem_directory)
{
	//	LogAppenderScope lgr(new CerrAppender);

	blocxx::Reference<blocxx::FileSystemMockObject> mockfiles = blocxx::Test::createCannedFSObject();
	blocxx::Test::addNormalFile(mockfiles, "/foo", "<empty foo>");
	blocxx::Test::addNormalFile(mockfiles, "/bar", "<empty bar>");
	blocxx::Test::addNormalFile(mockfiles, "/zzyzx", "<empty zzyzx>");
	blocxx::Test::addNormalFile(mockfiles, "/ick/quux/flump/barf", "<empty barf>");
	blocxx::Test::addNormalFile(mockfiles, "/ick/quux/trash/junk", "<empty junk>");
	blocxx::Test::addDirectory(mockfiles, "/ick/quux/trash/empty");

	{
		Logger logger(COMPONENT_NAME);
		blocxx::Test::FileSystemMockObjectScope mos(mockfiles);

		StringArray files;
		unitAssert( FileSystem::getDirectoryContents("/", files) );
		// These will come out of the directory listing in sorted order.
		unitAssertEquals( files, String("bar|foo|ick|zzyzx").tokenize("|") );

		unitAssert( !FileSystem::getDirectoryContents("/not/there", files) );
		unitAssert( !FileSystem::getDirectoryContents("/bar", files) );

		unitAssert( FileSystem::getDirectoryContents("/ick", files) );
		unitAssertEquals( files, String("quux").tokenize() );

		unitAssert( FileSystem::getDirectoryContents("/ick/quux", files) );
		unitAssertEquals( files, String("flump|trash").tokenize("|") );

		unitAssert( FileSystem::getDirectoryContents("/ick/quux/trash/empty", files) );
		unitAssertEquals( files, StringArray() );
	}
}

AUTO_UNIT_TEST(test_directory_contents)
{
	//	LogAppenderScope lgr(new CerrAppender);

	blocxx::Reference<blocxx::FileSystemMockObject> mockfiles = blocxx::Test::createCannedFSObject();
	blocxx::Test::addNormalFile(mockfiles, "/foo", "<empty foo>");
	blocxx::Test::addNormalFile(mockfiles, "/bar", "<empty bar>");
	blocxx::Test::addNormalFile(mockfiles, "/zzyzx", "<empty zzyzx>");
	blocxx::Test::addNormalFile(mockfiles, "/ick/quux/flump/barf", "<empty barf>");
	blocxx::Test::addNormalFile(mockfiles, "/ick/quux/trash/junk", "<empty junk>");
	blocxx::Test::addDirectory(mockfiles, "/ick/quux/trash/empty");

	{
		Logger logger(COMPONENT_NAME);
		blocxx::Test::FileSystemMockObjectScope mos(mockfiles);

		StringArray files;
		unitAssert( FileSystem::getDirectoryContents("/", files) );
		// These will come out of the directory listing in sorted order.
		unitAssertEquals( files, String("bar|foo|ick|zzyzx").tokenize("|") );

		unitAssert( !FileSystem::getDirectoryContents("/not/there", files) );
		unitAssert( !FileSystem::getDirectoryContents("/bar", files) );

		unitAssert( FileSystem::getDirectoryContents("/ick", files) );
		unitAssertEquals( files, String("quux").tokenize() );

		unitAssert( FileSystem::getDirectoryContents("/ick/quux", files) );
		unitAssertEquals( files, String("flump|trash").tokenize("|") );

		unitAssert( FileSystem::getDirectoryContents("/ick/quux/trash/empty", files) );
		unitAssertEquals( files, StringArray() );
	}
}

AUTO_UNIT_TEST(test_mock_symlink)
{
	//	LogAppenderScope lgr(new CerrAppender);

	blocxx::Reference<blocxx::FileSystemMockObject> mockfiles = blocxx::Test::createCannedFSObject();

	blocxx::Test::addNormalFile(mockfiles, "/nonlink", "<empty junk>");
	blocxx::Test::addSymlink(mockfiles, "/foo", "/bar");
	blocxx::Test::addSymlink(mockfiles, "/infinite", "/infinite");
	blocxx::Test::addSymlink(mockfiles, "/junk", "/nonlink");
	blocxx::Test::addSymlink(mockfiles, "/trash", "/junk");
	blocxx::Test::addDirectory(mockfiles, "/barf");

	{
		Logger logger(COMPONENT_NAME);
		blocxx::Test::FileSystemMockObjectScope mos(mockfiles);

		// Regular file (not a link)
		unitAssert( FileSystem::exists("/nonlink") );
		unitAssert( !FileSystem::isLink("/nonlink") );
		unitAssert( !FileSystem::isDirectory("/nonlink") );

		// Directory (not a link)
		unitAssert( FileSystem::exists("/barf") );
		unitAssert( !FileSystem::isLink("/barf") );
		unitAssert( FileSystem::isDirectory("/barf") );

		// Dangling symlink
		unitAssert( FileSystem::exists("/foo") );
		unitAssert( FileSystem::isLink("/foo") );
		unitAssert( !FileSystem::isDirectory("/foo") );
		unitAssertEquals( FileSystem::readSymbolicLink("/foo"), "/bar" );
		unitAssertThrowsEx( FileSystem::getFileContents("/foo"), blocxx::FileSystemException );

		// This symlink references itself.  Attempting to read the
		// contents from it (not the link itself) should result in a
		// filesystem exception.
		unitAssert( FileSystem::exists("/infinite") );
		unitAssert( FileSystem::isLink("/infinite") );
		unitAssert( !FileSystem::isDirectory("/infinite") );
		unitAssertEquals( FileSystem::readSymbolicLink("/infinite"), "/infinite" );
		unitAssertThrowsEx( FileSystem::getFileContents("/infinite"), blocxx::FileSystemException );


		// A valid symlink pointing to a non link.
		unitAssert( FileSystem::exists("/junk") );
		unitAssert( FileSystem::isLink("/junk") );
		unitAssert( !FileSystem::isDirectory("/junk") );
		unitAssertEquals( FileSystem::readSymbolicLink("/junk"), "/nonlink" );
		String contents;
		unitAssertNoThrow( contents = FileSystem::getFileContents("/junk") );
		unitAssertEquals( contents, "<empty junk>" );

		// A valid symlink pointing to a symlink to a non link.
		unitAssert( FileSystem::exists("/trash") );
		unitAssert( FileSystem::isLink("/trash") );
		unitAssert( !FileSystem::isDirectory("/trash") );
		unitAssertEquals( FileSystem::readSymbolicLink("/trash"), "/junk" );
		String contents2;
		unitAssertNoThrow( contents2 = FileSystem::getFileContents("/trash") );
		unitAssertEquals( contents2, "<empty junk>" );
	}
}

AUTO_UNIT_TEST(test_file_deletion)
{
	//	LogAppenderScope lgr(new CerrAppender);

	blocxx::Reference<blocxx::FileSystemMockObject> mockfiles = blocxx::Test::createCannedFSObject();

	blocxx::Test::addNormalFile(mockfiles, "/foo/bar", "<empty bar>");
	blocxx::Test::addNormalFile(mockfiles, "/foo/baz", "<empty baz>");

	{
		Logger logger(COMPONENT_NAME);
		blocxx::Test::FileSystemMockObjectScope mos(mockfiles);

		// Can't remove a directory with removeFile...
		unitAssert( !FileSystem::removeFile("/foo") );

		StringArray files;
		unitAssert( FileSystem::getDirectoryContents("/foo", files) );
		// These will come out of the directory listing in sorted order.
		unitAssertEquals( files, String("bar|baz").tokenize("|") );

		unitAssert( FileSystem::removeFile("/foo/bar") );
		unitAssert( FileSystem::getDirectoryContents("/foo", files) );
		unitAssertEquals( files, String("baz").tokenize("|") );

		unitAssert( FileSystem::removeFile("/foo/baz") );
		unitAssert( FileSystem::getDirectoryContents("/foo", files) );
		unitAssertEquals( files, StringArray() );

	}
}

AUTO_UNIT_TEST(test_zerofs)
{
	//	LogAppenderScope lgr(new CerrAppender);

	blocxx::Reference<blocxx::FileSystemMockObject> zerofs = blocxx::Test::createZeroFSObject();

	{
		blocxx::Test::FileSystemMockObjectScope mos(zerofs);

		unitAssert(FileSystem::exists("/dev/mem"));
		unitAssert(FileSystem::canRead("/dev/mem"));
		File foo = FileSystem::openFile("/dev/mem");

		unitAssert(foo);

		Array<char> buffer(1024, 1);

		unitAssert(foo.read(&buffer[0], buffer.size()) == buffer.size());
		// It should be zeroed now...
		unitAssertEquals(buffer, Array<char>(1024,0));

		unitAssert(FileSystem::getFileContents("/random/file").empty());
		unitAssert(FileSystem::getFileLines("/random/file").empty());

		// Should fail for any writes...
		unitAssert(foo.write(&buffer[0], buffer.size()) == size_t(-1));
	}
}

AUTO_UNIT_TEST(test_reroot)
{
	// FIXME! This test isn't anywhere near complete.  It is mostly intended to
	// test some basic capabilities of the remapped files.

	//	LogAppenderScope lgr(new CerrAppender);

	String currentDir = FileSystem::Path::getCurrentWorkingDirectory();
	blocxx::Reference<blocxx::FileSystemMockObject> reroot = blocxx::Test::createRerootedFSObject(currentDir);

	{
		unitAssert(!FileSystem::exists("/Makefile"));

		blocxx::Test::FileSystemMockObjectScope mos(reroot);

		unitAssert(FileSystem::exists("/Makefile"));

		String barf_source = "/some_barf_source.cpp";
		String myfile = "/MockFileSystemTestCases.cpp";

		// Check a remapped file...
		unitAssert(!FileSystem::exists(barf_source));
		unitAssert(blocxx::Test::remapFileName(reroot, barf_source,  currentDir + myfile));
		unitAssert(FileSystem::exists(barf_source));

		String contents = FileSystem::getFileContents(barf_source);
		// Check for a string in a remapped path to the original source file
		// containing the string...  Tricky. :)
		unitAssert(contents.indexOf("!!!!!!!Some very freaky string that should not appear in most random files~~~~~~~") != String::npos);

		// Check that the contents of the remapped file are identical to the original file.
		unitAssert(contents == FileSystem::getFileContents(myfile));
	}
}
