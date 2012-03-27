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
 * @author Anton Afanasiev - for Win
 * @author Joel Smith - posix group support
 */

#include "blocxx/BLOCXX_config.h"
#include "blocxx/UserUtils.hpp"
#include "blocxx/Mutex.hpp"
#include "blocxx/MutexLock.hpp"
#include "blocxx/GlobalMutex.hpp"
#include "blocxx/String.hpp"

#ifdef BLOCXX_HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef BLOCXX_HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef BLOCXX_HAVE_PWD_H
#include <pwd.h>
#endif

#ifdef BLOCXX_HAVE_GRP_H
#include <grp.h>
#endif

#include <cerrno>
#include <vector>

#ifdef BLOCXX_WIN32
/////////////////////////////////////////////////////////////////////////////
BLOCXX_NAMESPACE::UserId geteuid(void )
{
	// SID/uid Win32/NIX wrapper
	BLOCXX_NAMESPACE::UserId sid = (BLOCXX_NAMESPACE::UserId)NULL;
	HANDLE pToken = (HANDLE)0L;
	DWORD bufLength = 256;
	static int* tkUser[256]; // only a static buffer we need with no SID copy privileges :(

	if ( ::OpenProcessToken(::GetCurrentProcess(), TOKEN_QUERY, &pToken) )
	{
		::GetTokenInformation(pToken, TokenUser, tkUser, bufLength, &bufLength);
		sid = ((TOKEN_USER*)tkUser)->User.Sid;
		TCHAR sName[MAX_PATH], sDName[MAX_PATH];
		DWORD sNameLen, sDNameLen = sNameLen = MAX_PATH;
		SID_NAME_USE eUse;
		::LookupAccountSid( NULL, sid, sName, &sNameLen, sDName, &sDNameLen, &eUse);
		CloseHandle(pToken);
	}
	return sid;
}
#endif //BLOCXX_WIN32

namespace BLOCXX_NAMESPACE
{

namespace UserUtils
{

/////////////////////////////////////////////////////////////////////////////
String getEffectiveUserId()
{
#ifdef BLOCXX_WIN32
	/// @todo
	// The user ID is represented by a SID on Win32. Going to return 0 for
	// admin user on win32 for now. Eventually blocxx will
	// deal with userid on Win32 the proper way.

	// 20070625 Anton Afanasiev - maybe the better idea is to use ConvertSidToStringSid routing
	// the code below implements the same
	PSID_IDENTIFIER_AUTHORITY psia;
	DWORD dwSubAuthorities;
	DWORD dwSidRev=SID_REVISION;
	DWORD dwCounter;
	DWORD dwSidSize;
	UserId uid = ::geteuid();
	String strResult, strSubResult;

	if (!uid || !IsValidSid(uid))
		return String();
	psia = GetSidIdentifierAuthority(uid);
	dwSubAuthorities = *GetSidSubAuthorityCount(uid);
	dwSidSize = (15 + 12 + (12 * dwSubAuthorities) + 1) * sizeof(TCHAR);
	if ( (psia->Value[0] != 0) || (psia->Value[1] != 0) )
	{
		strSubResult.format(
			TEXT("0x%02hx%02hx%02hx%02hx%02hx%02hx"),
			(USHORT)psia->Value[0],
			(USHORT)psia->Value[1],
			(USHORT)psia->Value[2],
			(USHORT)psia->Value[3],
			(USHORT)psia->Value[4],
			(USHORT)psia->Value[5]);
	}
	else
	{
		strSubResult.format(
			TEXT("%lu"),
			(ULONG)(psia->Value[5]      )   +
			(ULONG)(psia->Value[4] <<  8)   +
			(ULONG)(psia->Value[3] << 16)   +
			(ULONG)(psia->Value[2] << 24) );
	}

	for (dwCounter=0 ; dwCounter < dwSubAuthorities ; dwCounter++)
	{
		strSubResult.format(
			TEXT("%s-%lu"), strSubResult.c_str(),
			*GetSidSubAuthority(uid, dwCounter) );
	}

	strResult.format(TEXT("S-%lu-%s"), dwSidRev, strSubResult);
	return strResult;
#else
	return String(Int64(::geteuid()));
#endif
}

/////////////////////////////////////////////////////////////////////////////
String getEffectiveGroupId()
{
#ifdef BLOCXX_WIN32
#pragma message ("Not implemented")
	return String();
#else
	return String(Int64(::getegid()));
#endif
}

//////////////////////////////////////////////////////////////////////////////
String getCurrentUserName()
{
	bool ok;
#ifdef BLOCXX_WIN32
	return getUserName(geteuid(), ok);
#else
	return getUserName(getuid(),ok);
#endif
}

//////////////////////////////////////////////////////////////////////////////
String getCurrentGroupName()
{
	bool ok;
#ifdef BLOCXX_WIN32
#pragma message ("Not implemented")
	return String();
#else
	return getGroupName(getgid(),ok);
#endif
}

namespace
{
GlobalMutex g_getpwMutex = BLOCXX_GLOBAL_MUTEX_INIT();
GlobalMutex g_getgrMutex = BLOCXX_GLOBAL_MUTEX_INIT();
}

namespace // anonymous
{
	// Get a sysconf value.  If no value is set (or another error occurs), return the default value.
	long getSysconfValue(int name, long default_value, int& error)
	{
#ifdef BLOCXX_WIN32
#pragma message(Reminder "TODO: Implement for Win if you use getSysconfValue not only for _SC_GETPW_R_SIZE_MAX")
		error = 0;
		return default_value;
#else
		errno = 0;

		long l = sysconf(name);

		if( l == -1 )
		{
			if( errno == 0 )
			{
				// The POSIX standard says this means the limit is indefinite (not infinite).
				error = 0;
				return default_value;
			}
			else
			{
				error = errno;
				return default_value;
			}
		}
		else
		{
			error = 0;
			return l;
		}
#endif
	}

	long getSysconfValue(int name, long default_value)
	{
		int unused;
		return getSysconfValue(name, default_value, unused);
	}
} // end annymous namespace

//////////////////////////////////////////////////////////////////////////////
String getUserName(uid_t uid,bool& ok)
{
#ifdef BLOCXX_WIN32
	/// @todo
	// Ignore uid for right now. Just return the current User (WRONG!)
	// Need to come back to this later when the uid_t stuff is worked out.

	// 20070625 Anton Afanasiev
	TCHAR cchName[256], cchDomainName[256];
	SID_NAME_USE snuOutVar;
	DWORD cchNameBufLen = sizeof(cchName), cchDomainNameBufLen = sizeof(cchDomainName);

	ok = ::LookupAccountSid(NULL,
				uid,
				cchName,
				&cchNameBufLen,
				cchDomainName,
				&cchDomainNameBufLen,
				&snuOutVar);
	return String(cchName);
#else

	if( uid == INVALID_USERID )
	{
		ok = false;
		return "";
	}


#ifdef BLOCXX_HAVE_GETPWUID_R
	passwd pw;
	size_t const additionalSize =
#ifdef _SC_GETPW_R_SIZE_MAX
		getSysconfValue(_SC_GETPW_R_SIZE_MAX, 10240);
#else
		10240;
#endif
	std::vector<char> additional(additionalSize);
	passwd* result;
	int rv = 0;
	do
	{
		rv = ::getpwuid_r(uid, &pw, &additional[0], additional.size(), &result);
		if (rv == ERANGE)
		{
			additional.resize(additional.size() * 2);
		}
	} while (rv == ERANGE);
#else
	MutexLock lock(g_getpwMutex);
	passwd* result = ::getpwuid(uid);
#endif
	if (result)
	{
		ok = true;
		return result->pw_name;
	}
	ok = false;
	return "";
#endif
}

//////////////////////////////////////////////////////////////////////////////
String getGroupName(gid_t gid,bool& ok)
{
#ifdef BLOCXX_WIN32
#pragma message ("Not implemented")
	return String();
#else
	if( gid == INVALID_GROUPID )
	{
		ok = false;
		return "";
	}

#ifdef BLOCXX_HAVE_GETGRGID_R
	group gr;
	size_t const additionalSize =
#ifdef _SC_GETGR_R_SIZE_MAX
		getSysconfValue(_SC_GETGR_R_SIZE_MAX, 10240);
#else
		10240;
#endif
	std::vector<char> additional(additionalSize);
	group* result;
	int rv = 0;
	do
	{
		rv = ::getgrgid_r(gid, &gr, &additional[0], additional.size(), &result);
		if (rv == ERANGE)
		{
			additional.resize(additional.size() * 2);
		}
	} while (rv == ERANGE);
#else
	MutexLock lock(g_getgrMutex);
	group* result = ::getgrgid(gid);
#endif
	if (result)
	{
		ok = true;
		return result->gr_name;
	}
	ok = false;
	return "";
#endif
}

//////////////////////////////////////////////////////////////////////////////
UserId
getUserId(const String& userName, bool& validUserName)
{
	validUserName = false;

#ifdef BLOCXX_WIN32
	// 20070625 Anton Afanasiev
	static DWORD uid[64]; // AA: do we really need for 'static' here?
	DWORD cbUid = sizeof(uid) * sizeof(DWORD);
	SID_NAME_USE snuOutVar;
	DWORD cbDomainBufSize = MAX_PATH;
	TCHAR strDomainBuf[MAX_PATH] = {0};

	return (validUserName=::LookupAccountName(
			NULL,
			userName.c_str(),
			&uid,
			&cbUid,
			strDomainBuf,
			&cbDomainBufSize,
			&snuOutVar))? &uid : NULL;

#else


#ifdef BLOCXX_HAVE_GETPWNAM_R
	size_t bufsize =
#ifdef _SC_GETPW_R_SIZE_MAX
		getSysconfValue(_SC_GETPW_R_SIZE_MAX, 10240);
#else
		1024;
#endif
	std::vector<char> buf(bufsize);
	struct passwd pwd;
	passwd* result = 0;
	int rv = 0;
	do
	{
		rv = ::getpwnam_r(userName.c_str(), &pwd, &buf[0], buf.size(), &result);
		if (rv == ERANGE)
		{
			buf.resize(buf.size() * 2);
		}
	} while (rv == ERANGE);

	if (rv != 0)
	{
		return INVALID_USERID;
	}

#else
	MutexLock ml(g_getpwMutex);
	struct passwd* result;
	result = ::getpwnam(userName.c_str());
#endif
	if (result)
	{
		validUserName = true;
		return result->pw_uid;
	}
	return INVALID_USERID;
#endif
}

//////////////////////////////////////////////////////////////////////////////
GroupId
getGroupId(const String& groupName, bool& validGroupName)
{
	validGroupName = false;

#ifdef BLOCXX_WIN32
#pragma message ("Not implemented")
	return 0;
#else

	group* result = NULL;

#ifdef BLOCXX_HAVE_GETGRNAM_R
	size_t bufsize =
#ifdef _SC_GETGR_R_SIZE_MAX
		getSysconfValue(_SC_GETGR_R_SIZE_MAX, 10240);
#else
		1024;
#endif
	std::vector<char> buf(bufsize);
	struct group grp;
	int rv = 0;
	do
	{
		rv = ::getgrnam_r(groupName.c_str(), &grp, &buf[0], buf.size(), &result);
		if (rv == ERANGE)
		{
			buf.resize(buf.size() * 2);
		}
	} while (rv == ERANGE);

	if (rv != 0)
	{
		return INVALID_USERID;
	}

#else
	MutexLock ml(g_getgrMutex);
	result = ::getgrnam(groupName.c_str());
#endif
	if (result)
	{
		validGroupName = true;
		return result->gr_gid;
	}
	return INVALID_GROUPID;
#endif
}
} // end namespace UserUtils
} // end namespace BLOCXX_NAMESPACE


