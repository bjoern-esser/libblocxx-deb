/*******************************************************************************
* Copyright (C) 2005, Vintela, Inc. All rights reserved.
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
*       Vintela, Inc., 
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
 * @author Anton Afanasiev
 */

#include "blocxx/BLOCXX_config.h"
#include "blocxx/PathSecurity.hpp"

namespace BLOCXX_NAMESPACE
{

	inline bool group_ok(::gid_t gid)
	{
#ifdef BLOCXX_SOLARIS
		return gid == 0 /* root */ || gid == 3 /* sys */;
#elif defined(BLOCXX_DARWIN)
		return gid == 0 /* root */ || gid == 80 /* admin */;
#else
		return gid == 0 /* root */;
#endif
	}

	inline bool check_grp_oth(struct stat const & x)
	{
		::mode_t badmsk = group_ok(x.st_gid) ? S_IWOTH : (S_IWGRP | S_IWOTH);
		return !(x.st_mode & badmsk);
	}

	EFileStatusReturn file_ok(struct stat const & x, ::uid_t uid, bool full_path)
	{
		// Note: originally this disallowed multiple hard links to a file,
		// but that restriction is not necessary, as the permissions for a
		// file are associated with its inode, and not with its directory
		// entries.  Note also that it's not a problem if someone does an
		// unlink of an alternate path to the file, as this just removes
		// the alternate directory entry -- the file itself is not actually
		// deleted until there are no hard links at all to it.
 		EFileStatusReturn retval(E_FILE_OK);
 		if (x.st_uid == 0 || 
#if defined(BLOCXX_HPUX) || defined(BLOCXX_AIX)
			 // on HP-UX & AIX, many system dirs & files are owned by the bin user, which has a uid of 2.
			 x.st_uid == 2 ||
#endif
 			 x.st_uid == uid)
 		{
 			if (!(S_ISLNK(x.st_mode) || check_grp_oth(x) || S_ISDIR(x.st_mode) && !full_path && (x.st_mode & S_ISVTX)))
 			{
 				retval = E_FILE_BAD_OTHER;
 			}
 		}
 		else
 		{
 			retval = E_FILE_BAD_OWNER;
 		}
 		return retval;
	}

	EFileStatusReturn getFileStatus(struct stat const & x, uid_t uid, bool is_full_path, const String& path)
	{
		return file_ok(x, uid, is_full_path);
	}

	bool isPathAbsolute(String const & path)
	{
		return path.startsWith("/") ? true : false;
	}

}  // end namespace BLOCXX_NAMESPACE
