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

#if       !defined(BLOCXX_TEST_CANNED_FILE_SYSTEM_HPP_INCLUDE_GUARD_)
#define            BLOCXX_TEST_CANNED_FILE_SYSTEM_HPP_INCLUDE_GUARD_

#include "blocxx/BLOCXX_config.h"
#define BLOCXX_ENABLE_TEST_HOOKS

#include "blocxx/FileSystemMockObject.hpp"
#include "blocxx/FileSystem.hpp"
#include "blocxx/String.hpp"
#include "blocxx/Reference.hpp"

#include "blocxx_test/CannedFileSystemEntryIFC.hpp"
#include "blocxx_test/CannedFileSystemNormalFile.hpp"

namespace BLOCXX_NAMESPACE
{
	namespace Test
	{

		/**
		 * An example of how to easily write tests which use the mock
		 * filesystem object:
		 *
		 * BOOST_AUTO_UNIT_TEST(test_junk)
		 * {
		 * 	BOOST_CHECKPOINT("test_junk()");
		 *
		 * 	// Set up a logger.  This is only needed if you want to see what is going on.
		 * 	LogAppenderScope lgr(new CerrAppender);
		 *
		 * 	Test::FSMockObjectRef mockfiles = Test::createCannedFSObject();
		 *
		 * 	// Add the needed files.
		 * 	Test::addNormalFile(mockfiles, "/foo/bar", "foobar");
		 *
		 * 	// Set up the mock object (usually done in some narrow scope).
		 * 	Test::FileSystemMockObjectScope mos(mockfiles);
		 *
		 * 	// Run your tests...
		 * 	BOOST_CHECK_EQUAL( FileSystem::getFileContents("/foo/bar"), "foobar" );
		 * }
		 *
		 * For larger examples which include different file types, see
		 * test/unit/MockFileSystemTestCases.cpp
		 */

		typedef blocxx::Reference<blocxx::FileSystemMockObject> FSMockObjectRef;

		/**
		 * Create the canned mock object which can be shoved in a
		 * FileSystemMockObjectScope for easy mock object manipulation.
		 *
		 * NOTE: This mock object is NOT currently thread safe, so adding files,
		 * writing files, renaming, etc (when implemented) should not be done
		 * from multiple threads.
		 */
		FSMockObjectRef createCannedFSObject(const blocxx::String& name = "unnamed");

		/**
		 * Create mock object which prefixes all pathes in FileSystem interface
		 * by newRoot. It allows to place all interesting files in test specific
		 * directory and work with them as with normal files. Since code dealing
		 * with files uses AutoDescriptor, File and these are value types they
		 * are hard to replace by mockable objects.
		 *
		 * In general, the use of this function should be avoided unless
		 * there is a relatively large number of files located in a
		 * shallow directory structure.  Using this function would
		 * otherwise encourage sloppy unit tests and the creation of
		 * many directories within CVS.  The more directories we get the
		 * slower checkouts and builds will become, so avoid random
		 * directory creation when possible.  Don't take that to mean
		 * that you should use random tarballs instead because those are
		 * just as sloppy and impossible to compare when doing a diff or
		 * merging.
		 *
		 * The normal canned filesystem object should be preferred in
		 * almost all cases.  If needed, a FSEntryRef can be added for a
		 * non-default file type which can handle any unusual behavior
		 * that would normally be hard to mock.
		 */
		FSMockObjectRef createRerootedFSObject(blocxx::String const & newRoot);

		/**
		 * Add a remapping for a file.  Only works with a rerooted filesystem.
		 * Any file accessed as <path> will be translated to the real path
		 * <realpath> instead of following the normal reroooted behavior.  Yes,
		 * this is kind of a stretch for the original intention of this mock
		 * object, but it adds great flexibility.  It still retains the ability
		 * to do all real file actions on the files.
		 *
		 * Note: Remapping a file does NOT mean that the parent directories of
		 * the virtual path will be virtualized.  Only the path itself and
		 * subdirectories thereof will be valid.
		 *
		 * Be VERY careful about what you use for a real path here.
		 * File/directory deletion and modification is possible.
		 */
		bool remapFileName(const FSMockObjectRef& obj, const blocxx::String& path, const blocxx::String& realpath);

		/**
		 * Add a file to the filesystem.  This can be anything based on the
		 * FileSystemEntry interface.  You won't normally call this function --
		 * use addNormalFile instead.
		 *
		 * @param object An object returned from createCannedFSObject().
		 * @param file A file entry.  The path() function of this object will be
		 * 	called and parent directories automatically created.
		 * @return true if the file was successfully added.
		 */
		bool addFSFile(FSMockObjectRef & object, FSEntryRef file);

		/**
		 * Add a "normal file" to the filesystem.  This will be visible through
		 * the mock object.
		 *
		 * @param object An object returned from createCannedFSObject().
		 * @param path The full path to the file.  Parent directories will be
		 * 	automatically created
		 * @param contents The initial contents of a file.  These may change
		 * 	through calls to "write" providing that the file is writeable.
		 * @param mode The access mode to the file.  Mode is all implemented as
		 * 	XYZ, where X == Y == Z.  For testing, there is no easy way to set
		 * 	the current user or handle group permissons, so a file basically
		 * 	has one octal access digit.
		 * @return true if the file could be added.
		 */
		bool addNormalFile(FSMockObjectRef & object, const blocxx::String & path
			, const blocxx::String & contents, PermissionFlags mode = E_FILE_READABLE
			, blocxx::UserId owner = 0);

		/**
		 * Add a "normal file" to the filesystem.  This will be visible through
		 * the mock object.
		 *
		 * @param object An object returned from createCannedFSObject().
		 * @param path The full path to the file.  Parent directories will be
		 * 	automatically created
		 * @param contents The initial contents of a file.  These may change
		 * 	through calls to "write" providing that the file is writeable.
		 * @param info The initial lstat() information to store in the file
		 *    entry.  Members such as mtime and atime will be altered in the file
		 *    entry as needed.
		 * @return true if the file could be added.
		 */
		bool addNormalFile(FSMockObjectRef & object, const blocxx::String & path
			, const blocxx::String & contents, const FileInformation& info);

		/**
		 * Add a symlink to the filesystem.  This will be visible through
		 * the mock object.
		 *
		 * @param object An object returned from createCannedFSObject().
		 * @param path The full path to the symlink.  Parent directories will be
		 * 	automatically created
		 * @param value The file to which this symlink will link.  This does not
		 * 	need to exist, because invalid links are allowed.
		 * @return true if the symlink could be added.
		 *
		 * NOTE: Symlinks with relative paths are not yet implemented.
		 */
		bool addSymlink(FSMockObjectRef & object, const blocxx::String & path
			, const blocxx::String & value, blocxx::UserId owner = 0);

		/**
		 * Add a directory (empty) to the filesystem.  This will be visible through
		 * the mock object.
		 *
		 * @param object An object returned from createCannedFSObject().
		 * @param path The full path to the directory.  Parent directories will
		 * 	be automatically created
		 * @return true if the directory could be added.
		 */
		bool addDirectory(FSMockObjectRef & object, const blocxx::String & path
			, blocxx::UserId owner = 0);

	} // end namespace Test
} // end namespace BLOCXX_NAMESPACE
#endif /* !defined(BLOCXX_TEST_CANNED_FILE_SYSTEM_HPP_INCLUDE_GUARD_) */
