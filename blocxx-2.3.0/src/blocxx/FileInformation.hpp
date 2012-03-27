/*******************************************************************************
* Copyright (C) 2007, Quest Software, Inc. All rights reserved.
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

#ifndef BLOCXX_FILE_INFORMATION_HPP_INCLUDE_GUARD_
#define BLOCXX_FILE_INFORMATION_HPP_INCLUDE_GUARD_
#include "blocxx/BLOCXX_config.h"
#include "blocxx/Types.hpp"
#include "blocxx/DateTime.hpp"

namespace BLOCXX_NAMESPACE
{
	namespace FileSystem
	{
		/**
		 * A struct to hold the useful portions of a stat() call.  Not all
		 * members of "struct stat" are available.
		 *
		 * The following members of struct stat are not currently present:
		 * st_dev, st_ino, st_nlink, st_gid, st_rdev, st_blksize, and st_blocks
		 */
		struct FileInformation
		{
			/**
			 * File types.  A small subset of what a PosixFilesystem can hold.  Not
			 * all systems will support E_FILE_LINK or E_FILE_SPECIAL
			 */
			enum EFileType { E_FILE_TYPE_UNKNOWN, E_FILE_REGULAR, E_FILE_DIRECTORY,
				E_FILE_SYMLINK, E_FILE_SPECIAL };

			/**
			 * File permissions.  Not all platforms will support the execute bits,
			 * "group", or "everyone" permissions.
			 *
			 * These constants may not necessarily match the values for
			 * S_I[RWX](OTH|GRP|USR), but will have some valid translation for the
			 * properties that are valid for this platform.
			 */
			enum EFilePerms
				{
					E_FILE_PERM_UNKNOWN = 0,

					E_FILE_OTHER_EXECUTE = 1,
					E_FILE_OTHER_WRITE = 2,
					E_FILE_OTHER_READ = 4,
					E_FILE_OTHER_RWX = E_FILE_OTHER_EXECUTE | E_FILE_OTHER_WRITE | E_FILE_OTHER_READ,

					E_FILE_GROUP_EXECUTE = E_FILE_OTHER_EXECUTE << 3,
					E_FILE_GROUP_WRITE = E_FILE_OTHER_WRITE << 3,
					E_FILE_GROUP_READ = E_FILE_OTHER_READ << 3,
					E_FILE_GROUP_RWX = E_FILE_GROUP_EXECUTE | E_FILE_GROUP_WRITE | E_FILE_GROUP_READ,

					E_FILE_USER_EXECUTE = E_FILE_OTHER_EXECUTE << 6,
					E_FILE_USER_WRITE = E_FILE_OTHER_WRITE << 6,
					E_FILE_USER_READ = E_FILE_OTHER_READ << 6,
					E_FILE_USER_RWX = E_FILE_USER_EXECUTE | E_FILE_USER_WRITE | E_FILE_USER_READ,

					// These will never be reported on some platforms.
					E_FILE_STICKY = E_FILE_OTHER_EXECUTE << 9,
					E_FILE_SETUID = E_FILE_OTHER_WRITE << 9,
					E_FILE_SETGID = E_FILE_OTHER_READ << 9
				};


			/**
			 * A convienence constructor to zero the integral types (owner, size,
			 * permissions, etc).  There is no real invariant to maintain in this
			 * class as it is just informative and cannot (currently) be used to
			 * alter anything.
			 */
			FileInformation();

			/**
			 * Last modification time of the file.
			 */
			DateTime mtime;

			/**
			 * Last access time of the file.  May not be supported for all systems
			 * or filesystems.
			 */
			DateTime atime;

			/**
			 * Creation time of the file.  May not be supported for all systems or
			 * filesystems.
			 */
			DateTime ctime;

			/**
			 * ID of the owner
			 */
			UserId owner;

			/**
			 * ID of the group.  For systems that do not support this, it will be gid_t(0).
			 */
			gid_t group;

			/**
			 * Size of the file
			 */
			UInt64 size;

			/**
			 * Type of the file
			 */
			EFileType type;

			/**
			 * File permissions.  Not all permissions are supported on all systems
			 * or filesystems.
			 */
			EFilePerms permissions;
		};
	} // end namespace FileSystem

} // end namespace BLOCXX_NAMESPACE

#endif // BLOCXX_FILE_INFORMATION_HPP_INCLUDE_GUARD_
