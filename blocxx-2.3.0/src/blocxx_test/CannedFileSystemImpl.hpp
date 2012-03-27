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


#if      !defined (BLOCXX_TEST_CANNED_FILE_SYSTEM_IMPL_HPP_INCLUDE_GUARD_)
#define            BLOCXX_TEST_CANNED_FILE_SYSTEM_IMPL_HPP_INCLUDE_GUARD_

// This file is a non-public implementation detaul.
#include "blocxx_test/CannedFileSystem.hpp"

namespace BLOCXX_NAMESPACE
{
	namespace Test
	{
		namespace CannedFSImpl
		{
			/**
			 * Create the canned mock object which can be shoved in a
			 * FileSystemMockObjectScope for easy mock object manipulation.
			 *
			 * NOTE: This mock object is NOT currently thread safe, so adding files,
			 * writing files, renaming, etc (when implemented) should not be done
			 * from multiple threads.
			 */
			FSMockObjectRef createCannedFSObject(const blocxx::String& name);

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
			bool addFSFile(FSMockObjectRef& object, FSEntryRef file);

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
			bool addNormalFile(FSMockObjectRef& object, const blocxx::String& path
				, const blocxx::String& contents, PermissionFlags mode = E_FILE_READABLE
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
			bool addSymlink(FSMockObjectRef& object, const blocxx::String& path
				, const blocxx::String& value, blocxx::UserId owner);

			/**
			 * Add a directory (empty) to the filesystem.  This will be visible through
			 * the mock object.
			 *
			 * @param object An object returned from createCannedFSObject().
			 * @param path The full path to the directory.  Parent directories will
			 * 	be automatically created
			 * @return true if the directory could be added.
			 */
			bool addDirectory(FSMockObjectRef& object, const blocxx::String& path, blocxx::UserId owner);

		} // end namespace Impl
	} // end namespace Test
} // end namespace BLOCXX_NAMESPACE

#endif /* !defined(BLOCXX_TEST_CANNED_FILE_SYSTEM_IMPL_HPP_INCLUDE_GUARD_) */
