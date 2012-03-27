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


#include "blocxx/BLOCXX_config.h"
#include "blocxx/Array.hpp"
#include "blocxx/Secure.hpp"
#include "blocxx/FileSystem.hpp"
#include "blocxx/String.hpp"
#include "blocxx/Paths.hpp"
#include "blocxx/Format.hpp"
#include "blocxx/LazyGlobal.hpp"
#ifdef BLOCXX_HAVE_DIRENT_H
#include <dirent.h>
#endif
#include <fcntl.h>
#ifndef BLOCXX_WIN32
#include <grp.h>
#endif
#include <limits.h>
#ifdef BLOCXX_HAVE_PWD_H
#include <pwd.h>
#endif
#ifdef BLOCXX_HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#ifdef BLOCXX_HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <cstdlib>
#include <cstdio>
#include <cerrno>
#include <vector>
#include <algorithm>

#ifdef AIX
#include "blocxx/StringBuffer.hpp"
#include "blocxx/NonRecursiveMutex.hpp"
#include "blocxx/NonRecursiveMutexLock.hpp"
#include <fstream>
#include <cctype>
#endif

#if defined(BLOCXX_NO_SETRESGID_PROTO) && defined(BLOCXX_HAVE_SETRESGID)
extern "C" { int setresgid(gid_t rgid, gid_t egid, gid_t sgid); }
#endif

#if defined(BLOCXX_NO_SETRESUID_PROTO) && defined(BLOCXX_HAVE_SETRESUID)
extern "C" { int setresuid(uid_t ruid, uid_t euid, uid_t suid); }
#endif

using namespace blocxx;

#define THRBLOCXX_IF(tst, ExceptionClass, msg) \
	do \
	{ \
		if (tst) \
		{ \
			BLOCXX_THROW(ExceptionClass, (msg)); \
		} \
	} while (false)

#define THRBLOCXX_ERRNO_IF(tst, ExceptionClass, msg) \
	do \
	{ \
		if (tst) \
		{ \
			BLOCXX_THROW_ERRNO_MSG(ExceptionClass, (msg)); \
		} \
	} while (false)

#define ABORT_IF(tst, msg) THRBLOCXX_IF((tst), Secure::ProcessAbortException, (msg))

#define ABORT_ERRNO_IF(tst, msg) \
  THRBLOCXX_ERRNO_IF((tst), Secure::ProcessAbortException, (msg))

namespace
{
#if !defined(BLOCXX_HAVE_SETEUID) && defined(BLOCXX_HAVE_SETREUID)
int seteuid(uid_t euid)
{
	return (setreuid(-1, euid));
}

#endif

#if !defined(BLOCXX_HAVE_SETEGID) && defined(BLOCXX_HAVE_SETRESGID)
int setegid(uid_t egid)
{
	return(setresgid(-1, egid, -1));
}
#endif

} // end anonymous namespace

namespace BLOCXX_NAMESPACE
{
namespace Secure
{
	BLOCXX_DEFINE_EXCEPTION(ProcessAbort);

	// Original source: Item 1.3, _Secure Programming Cookbook for C and C++_, by
	// John Viega and Matt Messier. 
	// Original C code reformatted and modified for C++.
	// Some inspiration provided by uidswap.c from openssh-portable
	void dropPrivilegesPermanently(::uid_t newuid, ::gid_t newgid, EChildGroupAction extendedGroupAction)
	{
#ifdef BLOCXX_WIN32
#pragma message(Reminder "TODO: implement it for Win!")
#else
		// Note: If any manipulation of privileges cannot be completed
		// successfully, it is safest to assume that the process is in an
		// unknown state and not allow it to continue (abort).

		if (newgid == ::gid_t(-1))
		{
			newgid = ::getgid();
		}
		::gid_t oldegid = ::getegid();
		::gid_t oldgid = ::getgid();
		if (newuid == ::uid_t(-1))
		{
			newuid = ::getuid();
		}
		::uid_t oldeuid = ::geteuid();
		::uid_t olduid = ::getuid();

		// If root privileges are to be dropped, be sure to pare down the
		// ancillary groups for the process before doing anything else because
		// the setgroups() system call requires root privileges.  Drop ancillary
		// groups regardless of whether privileges are being dropped temporarily
		// or permanently.
		if (oldeuid == 0)
		{
			struct passwd *newuser(NULL);
			if (extendedGroupAction == E_SOURCE_EXTENDED_GROUPS)
			{
				newuser = ::getpwuid(newuid);
			}
			if (newuser)
			{
				::initgroups(newuser->pw_name, newgid);
			}
			else
			{
			::setgroups(1, &newgid);
		}
		}

		if (newgid != oldegid)
		{
#if defined(BLOCXX_HAVE_SETRESGID) && !defined(BLOCXX_BROKEN_SETRESGID)
			ABORT_ERRNO_IF(::setresgid(newgid, newgid, newgid) == -1, "drop_privileges [1]");
#elif defined(BLOCXX_HAVE_SETREGID) && !defined(BLOCXX_BROKEN_SETREGID)
			ABORT_ERRNO_IF(::setregid(newgid, newgid) == -1, "drop_privileges [1]");
#else
			ABORT_ERRNO_IF(::setegid(newgid) == -1, "drop_privileges [1]");
			ABORT_ERRNO_IF(::setgid(newgid) == -1, "drop_privileges [1.1]");
#endif
		}

		if (newuid != oldeuid)
		{
#if defined(BLOCXX_HAVE_SETRESUID) && !defined(BLOCXX_BROKEN_SETRESUID)
			ABORT_ERRNO_IF(::setresuid(newuid, newuid, newuid) == -1, "drop_privileges [2]");
#elif defined(BLOCXX_HAVE_SETREUID) && !defined(BLOCXX_BROKEN_SETREUID)
			ABORT_ERRNO_IF(::setreuid(newuid, newuid) == -1, "drop_privileges [2]");
#else
#if !defined(BLOCXX_SETEUID_BREAKS_SETUID)
			ABORT_ERRNO_IF(::seteuid(newuid) == -1, "drop_privileges [2]");
#endif
			ABORT_ERRNO_IF(::setuid(newuid) == -1, "drop_privileges [2.1]");
#endif
		}

		// verify that the changes were successful
		// make sure gid drop was successful
		ABORT_IF(::getgid() != newgid || ::getegid() != newgid, "drop_privileges [3]");

		// make sure gid restoration fails
		ABORT_IF(
			newuid != 0 && newgid != oldegid &&
#if defined(BLOCXX_HAVE_SETRESGID) && !defined(BLOCXX_BROKEN_SETRESGID)
			(::setresgid(oldegid, oldegid, oldegid) != -1 || ::setgid(oldgid) != -1),
#elif defined(BLOCXX_HAVE_SETREGID) && !defined(BLOCXX_BROKEN_SETREGID)
			(::setregid(oldegid, oldegid) != -1 || ::setgid(oldgid) != -1),
#else
			(::setegid(oldegid) != -1 || ::setgid(oldgid) != -1),
#endif
			"drop_privileges [4]"
		);

		// make sure uid drop was successful
		ABORT_IF(::getuid() != newuid || ::geteuid() != newuid, "drop_privileges [5]");

		// make sure uid restoration fails
		ABORT_IF(
			newuid != 0 && newuid != oldeuid &&
#if defined(BLOCXX_HAVE_SETRESUID) && !defined(BLOCXX_BROKEN_SETRESUID)
			(::setresuid(oldeuid, oldeuid, oldeuid) != -1 || ::setuid(olduid) != -1),
#elif defined(BLOCXX_HAVE_SETREUID) && !defined(BLOCXX_BROKEN_SETREUID)
			(::setreuid(oldeuid, oldeuid) != -1 || ::setuid(olduid) != -1),
#else
			(::seteuid(oldeuid) != -1 || ::setuid(olduid) != -1),
#endif
			"drop_privileges [6]"
		);
#endif
	}

namespace
{
#ifdef AIX
	NonRecursiveMutex envMutex;
	String odmdir;

	char const default_odmdir[] = "ODMDIR=/etc/objrepos";

	String check_line(String const & line)
	{
		StringBuffer sb;
		char const * s;
		char c;
		for (s = line.c_str(); (c = *s) && !std::isspace(c); ++s)
		{
			switch (c)
			{
				case '\\':
					if (s[1] == '\0')
					{
						// Unexpected format
						return default_odmdir;
					}
					c = *++s;
					break;
				case '$':
				case '`':
				case '"':
				case '\'':
					// Unexpected format
					return default_odmdir;
				default:
					;
			}
			sb += c;
		}
		if (c == '\0')
		{
			return sb.releaseString();
		}
		while (std::isspace(*s))
		{
			++s;
		}
		if (*s == '#')
		{
			return sb.releaseString();
		}
		// Unexpected format
		return default_odmdir;
	}

	String setODMDIR()
	{
		String retval(default_odmdir);
		std::ifstream is("/etc/environment");
		while (is)
		{
			String s = String::getLine(is).trim();
			if (s.startsWith("ODMDIR="))
			{
				retval = check_line(s);
			}
		}
		return retval;
	}

	void addPlatformSpecificEnvVars(StringArray & environ)
	{
		NonRecursiveMutexLock lock(envMutex);
		if (odmdir.empty())
		{
			odmdir = setODMDIR();
		}
		environ.push_back(odmdir);
	}

#else

	void addPlatformSpecificEnvVars(StringArray &absEnvironment)
	{
#ifdef BLOCXX_WIN32
		char* const lpInheritedEnvironment = GetEnvironmentStrings();
		char* lpInheritedEnvIterator = lpInheritedEnvironment;
		if (lpInheritedEnvironment && *lpInheritedEnvironment && lpInheritedEnvironment[1])
		{
			for ( ; *lpInheritedEnvIterator; lpInheritedEnvIterator++)
			{
				absEnvironment.push_back( String( lpInheritedEnvIterator ) );
				lpInheritedEnvIterator += lstrlen(lpInheritedEnvIterator);
			}
			FreeEnvironmentStrings( (LPTCH)lpInheritedEnvironment );
		}
#endif
	}

#endif

	struct MinimalEnvironmentConstructor
	{
		static StringArray* create(int dummy)
		{
			AutoPtr<StringArray> retval(new StringArray);
			retval->push_back("IFS= \t\n");
			retval->push_back("PATH=" _PATH_STDPATH);
			char * tzstr = ::getenv("TZ");
			if (tzstr)
			{
				retval->push_back(String("TZ=") + tzstr);
			}
			addPlatformSpecificEnvVars(*retval);
			return retval.release();
		}
	};

	LazyGlobal<StringArray, int, MinimalEnvironmentConstructor> g_minimalEnvironment = BLOCXX_LAZY_GLOBAL_INIT(0);
} // end unnamed namespace

	StringArray minimalEnvironment()
	{
		return g_minimalEnvironment;
	}

	void runAs(char const * username, EChildGroupAction extendedGroupAction)
	{
#ifdef BLOCXX_WIN32
#pragma message(Reminder "TODO: implement it for Win!")
#else
		ABORT_IF(!username, "null user name");
		ABORT_IF(*username == '\0', "empty user name");
		ABORT_IF(::getuid() != 0 || ::geteuid() != 0, "non-root user calling runAs");
		errno = 0;
		struct passwd * pwent = ::getpwnam(username);
		// return value from getpwnam is a static, so don't free it.
		ABORT_ERRNO_IF(!pwent && errno != 0, Format("getpwnam(\"%1\") failed", username).c_str());
		ABORT_IF(!pwent, Format("user name (%1) not found", username).c_str());
		int rc = ::chdir("/");
		ABORT_ERRNO_IF(rc != 0, "chdir failed");
		Secure::dropPrivilegesPermanently(pwent->pw_uid, pwent->pw_gid, extendedGroupAction);
#endif
	}

} // namespace Secure
} // namespace BLOCXX_NAMESPACE
