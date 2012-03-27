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
 * @author Bart Whiteley
 * @author Dan Nuffer
 */

#include "blocxx/BLOCXX_config.h"
#include "blocxx/SocketException.hpp"
#include "blocxx/SocketUtils.hpp"
#include "blocxx/Assertion.hpp"
#include "blocxx/Socket.hpp"
#include "blocxx/Format.hpp"
#include "blocxx/Thread.hpp"
#include "blocxx/System.hpp"
#include "blocxx/Select.hpp"
#include "blocxx/TimeoutTimer.hpp"

#if defined(BLOCXX_WIN32)
#include "blocxx/SocketAddress.hpp"
#endif

#ifndef BLOCXX_HAVE_GETHOSTBYNAME_R
#include "blocxx/Mutex.hpp"
#include "blocxx/MutexLock.hpp"
#endif

extern "C"
{
#if !defined(BLOCXX_WIN32)
#include "blocxx/PosixUnnamedPipe.hpp"

#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/socket.h>
#ifdef BLOCXX_HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif
}

#include <cstring>
#include <cstdio>
#include <cerrno>

namespace BLOCXX_NAMESPACE
{

namespace SocketUtils 
{

//////////////////////////////////////////////////////////////////////////////
String
inetAddrToString(UInt64 addr)
{
	sockaddr_in iaddr;
	iaddr.sin_family = AF_INET;
	iaddr.sin_addr.s_addr = addr;
	iaddr.sin_port = 0;
#ifdef BLOCXX_HAVE_IPV6
	char buf[INET6_ADDRSTRLEN];
	String s(inet_ntop(iaddr.sin_family, &(iaddr.sin_addr), buf, sizeof(buf)));
#else
	String s(inet_ntoa(iaddr.sin_addr));
#endif

	return s;
}

#if defined(BLOCXX_WIN32)
int
waitForIO(SocketHandle_t fd, HANDLE eventArg, int timeOutSecs,
	long networkEvents)
{
	return waitForIO(fd, eventArg, Timeout::relative(timeOutSecs), networkEvents);
}

int
waitForIO(SocketHandle_t fd, HANDLE eventArg, const Timeout& classTimeout,
	  long networkEvents)
{
	TimeoutTimer timer(classTimeout);

	DWORD timeout= timer.asDWORDMs();

	if (networkEvents != -1L)
	{
		if(::WSAEventSelect(fd, eventArg, networkEvents) != 0)
		{
			BLOCXX_THROW(SocketException, 
				Format("WSAEventSelect failed in waitForIO: %1",
				System::lastErrorMsg(true)).c_str());
		}
	}

	int cc;
	if(Socket::getShutDownMechanism() != NULL)
	{
		HANDLE events[2];
		events[0] = Socket::getShutDownMechanism();
		events[1] = eventArg;

		DWORD index = ::WaitForMultipleObjects(
			2,
			events,
			FALSE,
			timeout);

		switch (index)
		{
			case WAIT_FAILED:
				cc = -1;
				break;
			case WAIT_TIMEOUT:
				cc = ETIMEDOUT;
				break;
			default:
				index -= WAIT_OBJECT_0;
				// If not shutdown event, then reset
				if (index != 0)
				{
					::ResetEvent(eventArg);
					cc = 0;
				}
				else
				{
					// Shutdown handle was signaled
					cc = -2;
				}
				break;
		}
	}
	else
	{
		switch(::WaitForSingleObject(eventArg, timeout))
		{
			case WAIT_OBJECT_0:
				::ResetEvent(eventArg);
				cc = 0;
				break;
			case WAIT_TIMEOUT:
				cc = ETIMEDOUT;
				break;
			default:
				cc = -1;
				break;
		}
	}

	// Set socket back to blocking
	if(::WSAEventSelect(fd, eventArg, 0) != 0)
	{
		BLOCXX_THROW(SocketException, 
			Format("Resetting socket with WSAEventSelect failed: %1",
			System::lastErrorMsg(true)).c_str());
	}
	u_long ioctlarg = 0;
	::ioctlsocket(fd, FIONBIO, &ioctlarg);
	return cc;
}

#else
//////////////////////////////////////////////////////////////////////////////
int
waitForIO(SocketHandle_t fd, int timeOutSecs, SocketFlags::EWaitDirectionFlag waitFlag)
{
	return waitForIO(fd, Timeout::relative(timeOutSecs), waitFlag);
}

//////////////////////////////////////////////////////////////////////////////
int
waitForIO(SocketHandle_t fd, const Timeout& timeout, SocketFlags::EWaitDirectionFlag waitFlag)
{
	if (fd == -1)
	{
		errno = EBADF;
		return -1;
	}

	Select::SelectObject so(fd); 
	if (waitFlag == SocketFlags::E_WAIT_FOR_INPUT)
	{
		so.waitForRead = true; 
	}
	else if (waitFlag == SocketFlags::E_WAIT_FOR_OUTPUT)
	{
		so.waitForWrite = true; 
	}
	else
	{
		so.waitForRead = true; 
		so.waitForWrite = true; 
	}
	Select::SelectObjectArray selarray; 
	selarray.push_back(so); 

	PosixUnnamedPipeRef lUPipe;
	int pipefd = -1;
	if (Socket::getShutDownMechanism())
	{
  		UnnamedPipeRef foo = Socket::getShutDownMechanism();
  		lUPipe = foo.cast_to<PosixUnnamedPipe>();
		BLOCXX_ASSERT(lUPipe);
		pipefd = lUPipe->getInputHandle();
	}
	if (pipefd != -1)
	{
		so = Select::SelectObject(pipefd); 
		so.waitForRead = true; 
		selarray.push_back(so); 
	}

	int rc = Select::selectRW(selarray, timeout); 
	switch (rc)
	{
	case Select::SELECT_TIMEOUT:
		rc = ETIMEDOUT; 
		break; 
	case 2:
		rc = -1; // pipe was signalled
		errno = ECANCELED;
		break; 
	case 1: 
		if (pipefd != -1)
		{
			if (selarray[1].readAvailable)
			{
				rc = -1; 
			}
		}
		if (selarray[0].writeAvailable || selarray[0].readAvailable)
		{
			rc = 0; 
		}
		break; 
	default: 
		rc = -1; 
	}
	return rc; 

}
#endif	// 

#ifndef BLOCXX_HAVE_GETHOSTBYNAME_R
} // end namespace SocketUtils
extern Mutex gethostbynameMutex;  // defined in SocketAddress.cpp
namespace SocketUtils {
#endif

#ifndef BLOCXX_WIN32
String getFullyQualifiedHostName()
{
	char hostName [2048];
	if (gethostname (hostName, sizeof(hostName)) == 0)
	{
#ifndef BLOCXX_HAVE_GETHOSTBYNAME_R
		MutexLock lock(gethostbynameMutex);
		struct hostent *he;
		if ((he = gethostbyname (hostName)) != 0)
		{
		   return he->h_name;
		}
		else
		{
			BLOCXX_THROW(SocketException, Format("SocketUtils::getFullyQualifiedHostName: gethostbyname failed: %1", h_errno).c_str());
		}
#else
		hostent hostbuf;
		hostent* host = &hostbuf;
#if (BLOCXX_GETHOSTBYNAME_R_ARGUMENTS == 6 || BLOCXX_GETHOSTBYNAME_R_ARGUMENTS == 5)
		char buf[2048];
		int h_err = 0;
#elif (BLOCXX_GETHOSTBYNAME_R_ARGUMENTS == 3)
		hostent_data hostdata;
		int h_err = 0;		
#else
#error Not yet supported: gethostbyname_r() with other argument counts.
#endif /* BLOCXX_GETHOSTBYNAME_R_ARGUMENTS */
		// gethostbyname_r will randomly fail on some platforms/networks
		// maybe the DNS server is overloaded or something.  So we'll
		// give it a few tries to see if it can get it right.
		bool worked = false;
		for (int i = 0; i < 10 && (!worked || host == 0); ++i)
		{
#if (BLOCXX_GETHOSTBYNAME_R_ARGUMENTS == 6)		  
			if (gethostbyname_r(hostName, &hostbuf, buf, sizeof(buf),
						&host, &h_err) != -1)
			{
				worked = true;
				break;
			}
#elif (BLOCXX_GETHOSTBYNAME_R_ARGUMENTS == 5)
			// returns NULL if not successful
			if ((host = gethostbyname_r(hostName, &hostbuf, buf, sizeof(buf), &h_err))) {
				worked = true;
				break;
			}
#elif (BLOCXX_GETHOSTBYNAME_R_ARGUMENTS == 3)
			if (gethostbyname_r(hostName, &hostbuf, &hostdata) == 0)
			{
				worked = true;
				break;
			}
			else
			{
			  h_err = h_errno;
			}
#else
#error Not yet supported: gethostbyname_r() with other argument counts.
#endif /* BLOCXX_GETHOSTBYNAME_R_ARGUMENTS */
		}
		if (worked && host != 0)
		{
			return host->h_name;
		}
		else
		{
			BLOCXX_THROW(SocketException, Format("SocketUtils::getFullyQualifiedHostName: gethostbyname_r(%1) failed: %2", hostName, h_err).c_str());
		}
#endif
	}
	else
	{
		BLOCXX_THROW(SocketException, Format("SocketUtils::getFullyQualifiedHostName: gethostname failed: %1(%2)", errno, strerror(errno)).c_str());
	}
	return "";
}
#else
// WIN32 defined
String getFullyQualifiedHostName()
{
	String rv;
	struct hostent *hostentp;
	char bfr[1024], ipaddrstr[128];
	struct in_addr iaHost;

	if(gethostname(bfr, sizeof(bfr)-1) == SOCKET_ERROR)
	{
		BLOCXX_THROW(SocketException, 
			Format("SocketUtils::getFullyQualifiedHostName: gethostname failed: %1(%2)", 
				WSAGetLastError(), System::lastErrorMsg(true)).c_str());
	}

	if(strchr(bfr, '.'))
	{
		// Guess we already have the DNS name
		return String(bfr);
	}

	if((hostentp = gethostbyname(bfr)) == NULL)
	{
		BLOCXX_THROW(SocketException, 
			Format("SocketUtils::getFullyQualifiedHostName: gethostbyname"
				" failed: %1(%2)", WSAGetLastError(),
				System::lastErrorMsg(true)).c_str());
	}

	if(strchr(hostentp->h_name, '.'))
	{
		rv = hostentp->h_name;
	}
	else
	{
		sockaddr_in     addr;
		addr.sin_family = AF_INET;
		addr.sin_port   = 0;
		memcpy(&addr.sin_addr, hostentp->h_addr_list[0], sizeof(addr.sin_addr));
#ifdef BLOCXX_HAVE_IPV6
		char buf[INET6_ADDRSTRLEN];
		rv = inet_ntop(addr.sin_family, &(addr.sin_addr), buf, sizeof(buf));
#else
		rv = inet_ntoa(addr.sin_addr);
#endif

		iaHost.s_addr = inet_addr(rv.c_str());
		if(iaHost.s_addr != INADDR_NONE)
		{
			hostentp = gethostbyaddr((const char*)&iaHost,
				sizeof(struct in_addr), AF_INET);
			if(hostentp)
			{
				if(strchr(hostentp->h_name, '.'))
				{
					// GOT IT
					rv = hostentp->h_name;
				}
			}
		}
	}

	return rv;
}
#endif


} // end namespace SocketUtils

} // end namespace BLOCXX_NAMESPACE

