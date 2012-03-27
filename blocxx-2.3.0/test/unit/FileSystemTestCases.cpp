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
#define PROVIDE_AUTO_TEST_MAIN
#include "AutoTest.hpp"

#include "blocxx/FileSystem.hpp"
#include "blocxx/File.hpp"
#include "blocxx/String.hpp"
#include "blocxx/Array.hpp"
#include "blocxx/Exec.hpp"
#include <errno.h>
#ifndef BLOCXX_WIN32
#include <unistd.h>
#include <sys/wait.h>
#endif
#include <sys/types.h>
#include <string.h>  // for strlen

using namespace blocxx;
using namespace std;

AUTO_UNIT_TEST(FileSystemTestCases_testgetLock)
{
	File f = FileSystem::openFile("Makefile");
	unitAssert(f);
	unitAssert( f.getLock() == 0 );
}

AUTO_UNIT_TEST(FileSystemTestCases_testtryLock)
{
	File f = FileSystem::openFile("Makefile");
	unitAssert(f);
	unitAssert( f.getLock() == 0 );
#ifndef BLOCXX_WIN32	// no fork :(
	// the lock is recursive, meaning to get a block, we've got to try to
	// acquire it from another process.  So fork()
	int rval = 0;
	pid_t p = fork();
	switch (p)
	{
		case -1:
			unitAssert(0);
			break;
		case 0: // child
			if ( f.tryLock() == -1 && (errno == EAGAIN || errno == EACCES) )
				_exit(0);
			else
				_exit(1);
			break;
		default: // parent
			{
				int status;
				pid_t p2 = waitpid(p, &status, 0);
				unitAssert(p2 == p);
				unitAssert(WIFEXITED(status));
				rval = WEXITSTATUS(status);
			}

			break;
	}
	// child should have returned 0 if the test worked.
	unitAssert( rval == 0 );
#else //WIN32
	char const * run_file_name   = "./TestTryLockChildProcess.exe";
	char const * argv[] = {
		run_file_name, "./Makefile", "0", 0
	}; // "0" for lock test
	ProcessRef proc = Exec::spawn(argv);
	proc->waitCloseTerm(Timeout::relative(5.0), Timeout::relative(0.0), Timeout::relative(8.0));
	unitAssert(proc->processStatus().terminatedSuccessfully());
#endif
}

AUTO_UNIT_TEST(FileSystemTestCases_testunlock)
{
	File f = FileSystem::openFile("Makefile");
	unitAssert(f);
	unitAssert( f.getLock() == 0 );
	unitAssert( f.unlock() == 0 );
#ifndef BLOCXX_WIN32	// no fork :(
	// the lock is recursive, meaning to get a block, we've got to try to
	// acquire it from another process.  So fork()
	int rval = 0;
	pid_t p = fork();
	switch (p)
	{
		case -1:
			unitAssert(0);
			break;
		case 0: // child
			if ( f.tryLock() == 0 )
				_exit(0);
			else
				_exit(1);
			break;
		default: // parent
			{
				int status;
				pid_t p2 = waitpid(p, &status, 0);
				unitAssert(p2 == p);
				unitAssert(WIFEXITED(status));
				rval = WEXITSTATUS(status);
			}

			break;
	}
	// child should have returned 0 if the test worked.
	unitAssert( rval == 0 );
#else //WIN32
	char const * run_file_name   = "./TestTryLockChildProcess.exe";
	char const * argv[] = {
		run_file_name, "./Makefile", "1", 0
	}; // "1" for unlock test
	ProcessRef proc = Exec::spawn(argv);
	proc->waitCloseTerm(Timeout::relative(5.0), Timeout::relative(0.0), Timeout::relative(8.0));
	unitAssert(proc->processStatus().terminatedSuccessfully());
#endif
}

AUTO_UNIT_TEST(FileSystemTestCases_testopenOrCreateFile)
{
	// make sure it's gone.
	FileSystem::removeFile("testfile");
	// first it should be created
	File f = FileSystem::openOrCreateFile("testfile");
	unitAssert(f);
	unitAssert(f.close() == 0);
	// now it will be opened
	f = FileSystem::openOrCreateFile("testfile");
	unitAssert(f);
	unitAssert(f.close() == 0);
	unitAssert(FileSystem::removeFile("testfile"));
}

AUTO_UNIT_TEST(FileSystemTestCases_testgetFileContents)
{
	FileSystem::removeFile("testfile");
	File f = FileSystem::openOrCreateFile("testfile");
	const char* contents = "line1\nline2";
	f.write(contents, ::strlen(contents));
	f.close();
	unitAssert(FileSystem::getFileContents("testfile") == contents);
	unitAssert(FileSystem::removeFile("testfile"));
}

AUTO_UNIT_TEST(FileSystemTestCases_testgetFileLines)
{
	FileSystem::removeFile("testfile");
	File f = FileSystem::openOrCreateFile("testfile");
	const char* contents = "line1\nline2";
	f.write(contents, ::strlen(contents));
	f.close();
	unitAssert(FileSystem::getFileLines("testfile").size() == 2);
	unitAssert(FileSystem::getFileLines("testfile")[0] == "line1");
	unitAssert(FileSystem::getFileLines("testfile")[1] == "line2");
	unitAssert(FileSystem::removeFile("testfile"));
}

AUTO_UNIT_TEST(FileSystemTestCases_testdirname)
{
#ifndef BLOCXX_WIN32 //for Windows is different dir separator
	unitAssert(FileSystem::Path::dirname("/x/y") == "/x");
	unitAssert(FileSystem::Path::dirname("/x/y/") == "/x");
	unitAssert(FileSystem::Path::dirname("/x/") == "/");
	unitAssert(FileSystem::Path::dirname("/x") == "/");
	unitAssert(FileSystem::Path::dirname("x") == ".");
	unitAssert(FileSystem::Path::dirname("//x") == "/");
	unitAssert(FileSystem::Path::dirname("///x////") == "/");
	unitAssert(FileSystem::Path::dirname("/") == "/");
	unitAssert(FileSystem::Path::dirname("////") == "/");
	unitAssert(FileSystem::Path::dirname(".") == ".");
	unitAssert(FileSystem::Path::dirname("..") == ".");
	unitAssert(FileSystem::Path::dirname("x/") == ".");
	unitAssert(FileSystem::Path::dirname("x//") == ".");
	unitAssert(FileSystem::Path::dirname("x///") == ".");
	unitAssert(FileSystem::Path::dirname("x/y") == "x");
	unitAssert(FileSystem::Path::dirname("x///y") == "x");
	unitAssert(FileSystem::Path::dirname("") == ".");
#else
	unitAssert(FileSystem::Path::dirname("\\x\\y") == "\\x");
	unitAssert(FileSystem::Path::dirname("\\x\\y\\") == "\\x");
	unitAssert(FileSystem::Path::dirname("\\x\\") == "\\");
	unitAssert(FileSystem::Path::dirname("\\x") == "\\");
	unitAssert(FileSystem::Path::dirname("x") == ".");
	unitAssert(FileSystem::Path::dirname("\\\\x") == "\\");
	unitAssert(FileSystem::Path::dirname("\\\\\\x\\\\\\\\") == "\\");
	unitAssert(FileSystem::Path::dirname("\\") == "\\");
	unitAssert(FileSystem::Path::dirname("\\\\\\\\") == "\\");
	unitAssert(FileSystem::Path::dirname(".") == ".");
	unitAssert(FileSystem::Path::dirname("..") == ".");
	unitAssert(FileSystem::Path::dirname("x\\") == ".");
	unitAssert(FileSystem::Path::dirname("x\\\\") == ".");
	unitAssert(FileSystem::Path::dirname("x\\\\\\") == ".");
	unitAssert(FileSystem::Path::dirname("x\\y") == "x");
	unitAssert(FileSystem::Path::dirname("x\\\\\\y") == "x");
#endif
}

AUTO_UNIT_TEST(FileSystemTestCases_testbasename)
{
#ifndef BLOCXX_WIN32 //for Windows is different dir separator
	unitAssert(FileSystem::Path::basename("/x/y") == "y");
	unitAssert(FileSystem::Path::basename("/x/y/") == "y");
	unitAssert(FileSystem::Path::basename("/x/") == "x");
	unitAssert(FileSystem::Path::basename("/x") == "x");
	unitAssert(FileSystem::Path::basename("x") == "x");
	unitAssert(FileSystem::Path::basename("//x") == "x");
	unitAssert(FileSystem::Path::basename("///x////") == "x");
	unitAssert(FileSystem::Path::basename("/") == "/");
	unitAssert(FileSystem::Path::basename("////") == "/");
	unitAssert(FileSystem::Path::basename(".") == ".");
	unitAssert(FileSystem::Path::basename("..") == "..");
	unitAssert(FileSystem::Path::basename("x/") == "x");
	unitAssert(FileSystem::Path::basename("x//") == "x");
	unitAssert(FileSystem::Path::basename("x///") == "x");
	unitAssert(FileSystem::Path::basename("x/y") == "y");
	unitAssert(FileSystem::Path::basename("x///y") == "y");
	unitAssert(FileSystem::Path::basename("") == "");
	String fn("/usr/lib/foo");
	unitAssert(FileSystem::Path::dirname(fn) + BLOCXX_FILENAME_SEPARATOR +
		FileSystem::Path::basename(fn) == fn);
#else
	unitAssert(FileSystem::Path::basename("\\x\\y") == "y");
	unitAssert(FileSystem::Path::basename("\\x\\y\\") == "y");
	unitAssert(FileSystem::Path::basename("\\x\\") == "x");
	unitAssert(FileSystem::Path::basename("\\x") == "x");
	unitAssert(FileSystem::Path::basename("x") == "x");
	unitAssert(FileSystem::Path::basename("\\\\x") == "x");
	unitAssert(FileSystem::Path::basename("\\\\\\x\\\\\\\\") == "x");
	unitAssert(FileSystem::Path::basename("\\") == "\\");
	unitAssert(FileSystem::Path::basename("\\\\\\\\") == "\\");
	unitAssert(FileSystem::Path::basename(".") == ".");
	unitAssert(FileSystem::Path::basename("..") == "..");
	unitAssert(FileSystem::Path::basename("x\\") == "x");
	unitAssert(FileSystem::Path::basename("x\\\\") == "x");
	unitAssert(FileSystem::Path::basename("x\\\\\\") == "x");
	unitAssert(FileSystem::Path::basename("x\\y") == "y");
	unitAssert(FileSystem::Path::basename("x\\\\\\y") == "y");
	unitAssert(FileSystem::Path::basename("") == "");
	String fn("\\usr\\lib\\foo");
	unitAssert(FileSystem::Path::dirname(fn) + BLOCXX_FILENAME_SEPARATOR +
		FileSystem::Path::basename(fn) == fn);
#endif
}



AUTO_UNIT_TEST(FileSystemTestCases_testrealPath)
{
#ifdef BLOCXX_WIN32
#pragma message(Reminder "TODO: implement testrealPath() for Win IF IT'S NECESSARY!")
#else
	using namespace FileSystem::Path;

#if defined(BLOCXX_DARWIN)
	// On MacOS, /etc is really a symlink.
	const String nonSymlinkDir = "/dev";
#else
	const String nonSymlinkDir = "/etc";
#endif

	/// @todo: These should be unlinked before symlinking.
	unitAssert(symlink("SYMLINK_LOOP", "SYMLINK_LOOP") == 0);
	unitAssert(symlink(".", "SYMLINK_1") == 0);
	unitAssert(symlink(String("//////./../.." + nonSymlinkDir).c_str(), "SYMLINK_2") == 0);
	unitAssert(symlink("SYMLINK_1", "SYMLINK_3") == 0);
	unitAssert(symlink("SYMLINK_2", "SYMLINK_4") == 0);
	unitAssert(symlink("doesNotExist", "SYMLINK_5") == 0);
	File f(FileSystem::createFile("doesExist"));
	FileSystem::makeDirectory("doesExistDir");

	unitAssert(realPath("/") == "/");
	unitAssert(realPath("/////////////////////////////////") == "/");
	unitAssert(realPath("/.././.././.././..///") ==  "/");
	unitAssert(realPath(nonSymlinkDir) == nonSymlinkDir);
	unitAssert(realPath(nonSymlinkDir + "/.." + nonSymlinkDir) == nonSymlinkDir);
	try
	{
		realPath("/doesNotExist/../etc");
		unitAssert(0);
	}
	catch (FileSystemException& e)
	{
		unitAssert(e.getErrorCode() == ENOENT);
	}
	unitAssert(realPath("././././.") == getCurrentWorkingDirectory());
	unitAssert(realPath("/././././.") == "/");
	unitAssert(realPath(nonSymlinkDir + "/./././.") == nonSymlinkDir);
	try
	{
		realPath("/etc/.//doesNotExist");
		unitAssert(0);
	}
	catch (FileSystemException& e)
	{
		unitAssert(e.getErrorCode() == ENOENT);
	}
	unitAssert(realPath("./doesExist") == getCurrentWorkingDirectory() + "/doesExist");
	unitAssert(realPath("./doesExistDir/") == getCurrentWorkingDirectory() + "/doesExistDir");
	try
	{
		realPath("./doesNotExist/");
		unitAssert(0);
	}
	catch (FileSystemException & e)
	{
		unitAssert(e.getErrorCode() == ENOENT);
	}
	unitAssert(realPath("./doesExistDir/../doesExist") == getCurrentWorkingDirectory() + "/doesExist");
	try
	{
		realPath("./doesNotExist/../doesExist");
		unitAssert(0);
	}
	catch (FileSystemException & e)
	{
		unitAssert(e.getErrorCode() == ENOENT);
	}
	try
	{
		realPath("doesNotExist");
		unitAssert(0);
	}
	catch (FileSystemException& e)
	{
		unitAssert(e.getErrorCode() == ENOENT);
	}
	unitAssert(realPath(".") == getCurrentWorkingDirectory());
	try
	{
		realPath("./doesNotExist");
		unitAssert(0);
	}
	catch (FileSystemException& e)
	{
		unitAssert(e.getErrorCode() == ENOENT);
	}
	try
	{
		realPath("SYMLINK_LOOP");
		unitAssert(0);
	}
	catch (FileSystemException& e)
	{
		unitAssert(e.getErrorCode() == ELOOP);
	}
	try
	{
		realPath("./SYMLINK_LOOP");
		unitAssert(0);
	}
	catch (FileSystemException& e)
	{
		unitAssert(e.getErrorCode() == ELOOP);
	}
	unitAssert(realPath("SYMLINK_1") == getCurrentWorkingDirectory());
	try
	{
		realPath("SYMLINK_1/doesNotExist");
		unitAssert(0);
	}
	catch (FileSystemException& e)
	{
		unitAssert(e.getErrorCode() == ENOENT);
	}
	unitAssert(realPath("SYMLINK_2") == nonSymlinkDir);
	unitAssert(realPath("SYMLINK_3") == getCurrentWorkingDirectory());
	unitAssert(realPath("SYMLINK_4") == nonSymlinkDir);
	unitAssert(realPath("../unit/SYMLINK_1") == getCurrentWorkingDirectory());
	unitAssert(realPath("../unit/SYMLINK_2") == nonSymlinkDir);
	unitAssert(realPath("../unit/SYMLINK_3") == getCurrentWorkingDirectory());
	unitAssert(realPath("../unit/SYMLINK_4") == nonSymlinkDir);
	try
	{
		realPath("./SYMLINK_5");
		unitAssert(0);
	}
	catch (FileSystemException& e)
	{
		unitAssert(e.getErrorCode() == ENOENT);
	}
	try
	{
		realPath("SYMLINK_5");
		unitAssert(0);
	}
	catch (FileSystemException& e)
	{
		unitAssert(e.getErrorCode() == ENOENT);
	}
	try
	{
		realPath("SYMLINK_5/doesNotExist");
		unitAssert(0);
	}
	catch (FileSystemException& e)
	{
		unitAssert(e.getErrorCode() == ENOENT);
	}
	unitAssert(realPath("doesExistDir/../../unit/doesExist") == getCurrentWorkingDirectory() + "/doesExist");
	unitAssert(realPath("doesExistDir/../../unit/.") == getCurrentWorkingDirectory());

	FileSystem::removeDirectory("doesExistDir");
	FileSystem::removeFile("doesExist");
	FileSystem::removeFile("SYMLINK_LOOP");
	FileSystem::removeFile("SYMLINK_1");
	FileSystem::removeFile("SYMLINK_2");
	FileSystem::removeFile("SYMLINK_3");
	FileSystem::removeFile("SYMLINK_4");
	FileSystem::removeFile("SYMLINK_5");
#endif
}

AUTO_UNIT_TEST(FileSystemTestCases_testopenFile)
{
	FileSystem::removeFile("testfile");
	File f = FileSystem::openOrCreateFile("testfile");
	f.close();

	char buffer[] = "line";
	const size_t BUFFER_SIZE = 4;
	const String LINE(buffer);

	File rw_file( FileSystem::openFile("testfile") );
	unitAssert( LINE.size() == rw_file.write( LINE.c_str(), LINE.size() ) );
	unitAssert( LINE.size() == rw_file.read( buffer, LINE.size(), 0 ) );
	unitAssert( LINE == String( buffer ) );
	rw_file.close();

	File r_file( FileSystem::openFile("testfile", FileSystem::E_READ) );
	unitAssert( size_t(-1) == r_file.write( LINE.c_str(), LINE.size() ) );
	unitAssert( LINE.size() == r_file.read( buffer, LINE.size(), 0 ) );
	unitAssert( LINE == String( buffer ) );
	r_file.close();

	File w_file( FileSystem::openFile("testfile", FileSystem::E_WRITE) );
	unitAssert( LINE.size() == w_file.write( LINE.c_str(), LINE.size() ) );
	unitAssert( size_t(-1) == w_file.read( buffer, LINE.size(), 0 ) );
	w_file.close();

	FileSystem::removeFile("testfile");
}

AUTO_UNIT_TEST(FileSystemTestCases_testfileInformation)
{
	// These tests are by no means complete.  Tests involving users, creation
	// time, etc. would be needed...
	FileSystem::removeFile("testfile");
	FileSystem::removeFile("testlink");

	// File removed, so getFileInformation should throw...
	unitAssertThrowsEx(FileSystem::getFileInformation("testfile"), FileSystemException);
	unitAssertThrowsEx(FileSystem::getFileInformation("testlink"), FileSystemException);

	File f = FileSystem::openOrCreateFile("testfile");
	f.close();
	unitAssert(symlink("testfile", "testlink") == 0);

	FileSystem::FileInformation info1, linfo1;
	unitAssertNoThrow(info1 = FileSystem::getFileInformation("testfile"));
	unitAssertNoThrow(linfo1 = FileSystem::getFileInformation("testlink"));
	unitAssertEquals(info1.size, UInt64(0)); // Freshly created file should be empty.

	unitAssertEquals(info1.type, FileSystem::FileInformation::E_FILE_REGULAR);
	unitAssertEquals(linfo1.type, FileSystem::FileInformation::E_FILE_SYMLINK);

	char buffer[] = "line";
	const size_t BUFFER_SIZE = 4;
	const String LINE(buffer);

	// Delay for a second to ensure times change.
	sleep(1);

	// Copied from testFileOpen()
	File rw_file( FileSystem::openFile("testfile") );
	unitAssert( LINE.size() == rw_file.write( LINE.c_str(), LINE.size() ) );
	unitAssert( LINE.size() == rw_file.read( buffer, LINE.size(), 0 ) );
	unitAssert( LINE == String( buffer ) );
	rw_file.close();

	FileSystem::FileInformation info2, linfo2;
	unitAssertNoThrow(info2 = FileSystem::getFileInformation("testfile"));
	unitAssertNoThrow(linfo2 = FileSystem::getFileInformation("testlink"));
	// At least LINE.size() bytes were added to the file, verify it.  This may
	// need to be changed to >= if some platform pads the file...  I would hope
	// that this isn't the case.
	unitAssert(info2.size == LINE.size());
	// Make sure the mtime changed.
	unitAssert(info2.mtime > info1.mtime);

	// Make sure the link mtime and size did not change.
	unitAssert(linfo1.mtime == linfo2.mtime);
	unitAssert(linfo1.size == linfo2.size);

	FileSystem::removeFile("testfile");
	FileSystem::removeFile("testlink");

	FileSystem::FileInformation dinfo;
	unitAssertNoThrow(dinfo = FileSystem::getFileInformation("."));
	unitAssertEquals(dinfo.type, FileSystem::FileInformation::E_FILE_DIRECTORY);
}
