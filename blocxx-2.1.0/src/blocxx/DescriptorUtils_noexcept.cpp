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
 * @author Kevin Van Horn
 * @author Dan Nuffer
 * @author Andrey Mukha // Windows part
 */

// THIS CODE MUST NOT THROW EXCEPTIONS.  IT IS ALSO HIGHLY PREFERRED THAT
// IT NOT RELY ON ANY LIBRARY OTHER THAN STANDARD SYSTEM LIBRARIES AND THE
// STANDARD C++ LIBRARY, AS IT IS USED IN libowcprivman, AND WE WANT TO AVOID
// LINKING OTHER LIBRARIES IN WITH libowcprivman.

#include "blocxx/BLOCXX_config.h"
#include "blocxx/DescriptorUtils_noexcept.hpp"
#include "blocxx/AutoDescriptor.hpp"

#include <cstring>
#include <sys/types.h>
#ifdef BLOCXX_HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifndef BLOCXX_WIN32
#include <sys/uio.h>
#else
#include "blocxx/WinProcessUtils.hpp"
#endif

namespace BLOCXX_NAMESPACE
{

namespace
{
	char const MAGIC_CHAR = '\xa5';

	AutoDescriptor copy_error(char * dst, size_t dstsz, char const * src)
	{
		std::strncpy(dst, src, dstsz);
		dst[dstsz - 1] = '\0';
		return AutoDescriptor();
	}
}


#ifdef BLOCXX_WIN32

int passDescriptor(Descriptor streamPipe, Descriptor descriptor, ProcId targetProcessHd)
{
	if (streamPipe == BLOCXX_INVALID_HANDLE)
	{
		return -1;
	}

	DWORD targetProcessId = WinUtils::getProcessIdNT(targetProcessHd);

	DWORD rc = -1;
	HANDLE dupDescriptor = INVALID_HANDLE_VALUE;
	HANDLE hProcess = targetProcessId == 0 ? GetCurrentProcess() : OpenProcess(PROCESS_ALL_ACCESS, FALSE, targetProcessId);

	BOOL fSuccess = DuplicateHandle(GetCurrentProcess(), descriptor,
		hProcess, &dupDescriptor, 0,
		FALSE, DUPLICATE_SAME_ACCESS);

	CloseHandle(hProcess);

	if (!fSuccess)
	{
		return -1;
	}

	OVERLAPPED ovl;
	ZeroMemory(&ovl, sizeof(OVERLAPPED));
	
	fSuccess = WriteFile(streamPipe, &dupDescriptor, sizeof(long), &rc, &ovl);

	if (!fSuccess)
	{
		DWORD lastError = GetLastError();

		if (lastError != ERROR_IO_INCOMPLETE && lastError != ERROR_IO_PENDING)
		{
			SetLastError(lastError);
			return -1;
		}
		else 
		{
			DWORD waitFlag = WaitForSingleObject(streamPipe, INFINITE);

			if (waitFlag == WAIT_OBJECT_0)
			{
				GetOverlappedResult(streamPipe, &ovl, &rc, FALSE);
			}
			else
			{
				return -1;
			}
		}
	}

	return rc;
}

AutoDescriptor receiveDescriptor(Descriptor streamPipe, char * errbuf, size_t bufsz)
{
	long desc;
	DWORD rc = -1;
	BOOL bSuccess = FALSE;

	if (streamPipe != BLOCXX_INVALID_HANDLE)
	{
		OVERLAPPED ovl;
		ZeroMemory(&ovl, sizeof(OVERLAPPED));

		bSuccess = ReadFile(streamPipe, &desc, sizeof(long), &rc, &ovl);

		if (!bSuccess)
		{
			DWORD lastError = GetLastError();

			if (lastError != ERROR_IO_INCOMPLETE && lastError != ERROR_IO_PENDING)
			{
				SetLastError(lastError);
				return copy_error(errbuf, bufsz, "ReadFile() failed");
			}
			else
			{
				DWORD waitFlag = WaitForSingleObject(streamPipe, INFINITE);

				if (waitFlag == WAIT_OBJECT_0)
				{
					GetOverlappedResult(streamPipe, &ovl, &rc, FALSE);
				}
				else
				{
					return copy_error(errbuf, bufsz, "WaitForSingleObject() failed");
				}
			}
		}

		return AutoDescriptor(reinterpret_cast<HANDLE>(desc));
	}

	return copy_error(errbuf, bufsz, "receiveDescriptor() error");
}

#else

int passDescriptor(Descriptor streamPipe, Descriptor descriptor, ProcId targetProcessId)
{
	struct msghdr msg;
	::memset(&msg, 0, sizeof(msg));
	struct iovec iov[1];
	::memset(iov, 0, sizeof(iov[0]));

#ifdef BLOCXX_HAVE_MSGHDR_MSG_CONTROL

// We need the newer CMSG_LEN() and CMSG_SPACE() macros, but few
// implementations support them today.  These two macros really need
// an ALIGN() macro, but each implementation does this differently.
#ifndef CMSG_LEN
#define CMSG_LEN(size)      (sizeof(struct cmsghdr) + (size))
#endif

#ifndef CMSG_SPACE
#define CMSG_SPACE(size)    (sizeof(struct cmsghdr) + (size))
#endif

	union {
	  struct cmsghdr cm;
	  char control[CMSG_SPACE(sizeof(int))];
	} control_un;
	::memset(&control_un, 0, sizeof(control_un));
	struct cmsghdr * cmptr;

	msg.msg_control = control_un.control;
	msg.msg_controllen = sizeof(control_un.control);

	cmptr = CMSG_FIRSTHDR(&msg);
	cmptr->cmsg_len = CMSG_LEN(sizeof(int));
	cmptr->cmsg_level = SOL_SOCKET;
	cmptr->cmsg_type = SCM_RIGHTS;
	*(reinterpret_cast<int *>(CMSG_DATA(cmptr))) = descriptor;
#else

#ifdef BLOCXX_NCR
	void *temp_cast = &descriptor;
	msg.msg_accrights = static_cast<caddr_t>(temp_cast);
#else
	msg.msg_accrights = static_cast<caddr_t>(&descriptor);
#endif

	msg.msg_accrightslen = sizeof(int);
#endif

	msg.msg_name = 0;
	msg.msg_namelen = 0;

	char dummy[1] = { MAGIC_CHAR };
	iov[0].iov_base = dummy;
	iov[0].iov_len = 1;
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;

	return ::sendmsg(streamPipe, &msg, 0);
}

AutoDescriptor receiveDescriptor(Descriptor streamPipe, char * errbuf, size_t bufsz)
{
	struct msghdr msg;
	struct iovec iov[1];

	msg = msghdr(); // zero-init to make valgrind happy
#ifdef BLOCXX_HAVE_MSGHDR_MSG_CONTROL
	union {
	  struct cmsghdr cm;
	  char control[CMSG_SPACE(sizeof(int))];
	} control_un;

	msg.msg_control = control_un.control;
	msg.msg_controllen = sizeof(control_un.control);
#else
	int	newfd = -1;

#ifdef BLOCXX_NCR
	void *temp_cast = &newfd;
	msg.msg_accrights = static_cast<caddr_t>(temp_cast);
#else
	msg.msg_accrights = static_cast<caddr_t>(&newfd);
#endif

	msg.msg_accrightslen = sizeof(int);
#endif

	msg.msg_name = 0;
	msg.msg_namelen = 0;

	char dummy[1] = { '\x7F' };
	iov[0].iov_base = dummy;
	iov[0].iov_len = 1;
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;

	ssize_t	n = ::recvmsg(streamPipe, &msg, 0);
	if (n == 0)
	{
		return copy_error(errbuf, bufsz,
			"unexpected end of input when receiving handle");
	}
	if (n < 0)
	{
		return copy_error(errbuf, bufsz, "recvmsg() failed");
	}
	if (n != 1)
	{
		return copy_error(errbuf, bufsz, "received more than 1 byte.");
	}
	if (dummy[0] != MAGIC_CHAR)
	{
		return copy_error(errbuf, bufsz, "bad magic char when receiving handle");
	}


#ifdef BLOCXX_HAVE_MSGHDR_MSG_CONTROL
	struct cmsghdr * cmptr = CMSG_FIRSTHDR(&msg);
	if (!cmptr)
	{
		return copy_error(errbuf, bufsz,
			"missing control message when receiving handle");
	}
	// as far as I can tell, HP-UX is just broken and sets cmptr->cmsg_len to 12. Things work anyway.
#if !defined (BLOCXX_HPUX)
	if (cmptr->cmsg_len != CMSG_LEN(sizeof(int)))
	{
		return copy_error(errbuf, bufsz,
			"cmptr->cmsg_len != CMSG_LEN(sizeof(int)) when receiving handle");
	}
#endif
	if (cmptr->cmsg_level != SOL_SOCKET)
	{
		return copy_error(errbuf, bufsz,
			"control level != SOL_SOCKET when receiving handle");
	}
	if (cmptr->cmsg_type != SCM_RIGHTS)
	{
		return copy_error(errbuf, bufsz,
			"control type != SCM_RIGHTS when receiving handle");
	}
	return AutoDescriptor(*(reinterpret_cast<int *>(CMSG_DATA(cmptr))));
#else
	if (msg.msg_accrightslen != sizeof(int))
	{
		return copy_error(errbuf, bufsz,
			"bad control message when receiving handle");
	}
	return AutoDescriptor(newfd);
#endif
}

#endif


} // end namespace BLOCXX_NAMESPACE
