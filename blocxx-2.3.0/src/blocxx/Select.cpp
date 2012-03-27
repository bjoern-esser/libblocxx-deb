/*******************************************************************************
* Copyright (C) 2005, 2010, Quest Software, Inc. All rights reserved.
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
 * @author Jon Carey
 * @author Dan Nuffer
 */

#include "blocxx/BLOCXX_config.h"
#include "blocxx/Select.hpp"
#include "blocxx/AutoPtr.hpp"
#include "blocxx/Assertion.hpp"
#include "blocxx/Thread.hpp" // for testCancel()
#include "blocxx/TimeoutTimer.hpp"
#include "blocxx/AutoDescriptor.hpp"

#if defined(BLOCXX_WIN32)
#include <cassert>
#endif

extern "C"
{

#ifndef BLOCXX_WIN32
 #ifdef BLOCXX_HAVE_SYS_EPOLL_H
  #include <sys/epoll.h>
 #endif
 #if defined (BLOCXX_HAVE_SYS_POLL_H)
  #include <sys/poll.h>
 #endif
 #if defined (BLOCXX_HAVE_SYS_SELECT_H)
  #include <sys/select.h>
 #endif
#endif

#ifdef BLOCXX_HAVE_SYS_TIME_H
 #include <sys/time.h>
#endif

#include <sys/types.h>

#ifdef BLOCXX_HAVE_UNISTD_H
 #include <unistd.h>
#endif

#include <errno.h>
}

namespace BLOCXX_NAMESPACE
{

namespace Select
{

namespace
{
	const float LOOP_TIMEOUT = 10.0;
}

#if defined(BLOCXX_WIN32)
//////////////////////////////////////////////////////////////////////////////
int
selectRW(SelectObjectArray& selarray, const Timeout& timeout)
{
	int rc;
	size_t hcount = static_cast<DWORD>(selarray.size());
	AutoPtrVec<HANDLE> hdls(new HANDLE[hcount]);

	size_t handleidx = 0;
	for (size_t i = 0; i < selarray.size(); i++, handleidx++)
	{
		if(selarray[i].s.isSocket && selarray[i].s.networkevents)
		{
			::WSAEventSelect(selarray[i].s.sockfd,
				selarray[i].s.event, selarray[i].s.networkevents);
		}

		hdls[handleidx] = selarray[i].s.event;
	}

	TimeoutTimer timer(timeout);
	timer.start();
	DWORD cc = ::WaitForMultipleObjects(hcount, hdls.get(), FALSE, timer.asDWORDMs());

	assert(cc != WAIT_ABANDONED);

	switch (cc)
	{
		case WAIT_FAILED:
			rc = Select::SELECT_ERROR;
			break;
		case WAIT_TIMEOUT:
			rc = Select::SELECT_TIMEOUT;
			break;
		default:
			rc = cc - WAIT_OBJECT_0;

			// If this is a socket, set it back to
			// blocking mode
			if(selarray[rc].s.isSocket)
			{
				if(selarray[rc].s.networkevents
					&& selarray[rc].s.doreset == false)
				{
					::WSAEventSelect(selarray[rc].s.sockfd,
						selarray[rc].s.event, selarray[rc].s.networkevents);
				}
				else
				{
					// Set socket back to blocking
					::WSAEventSelect(selarray[rc].s.sockfd,
						selarray[rc].s.event, 0);
					u_long ioctlarg = 0;
					::ioctlsocket(selarray[rc].s.sockfd, FIONBIO, &ioctlarg);
				}
			}
			break;
	}

	if( rc < 0 )
		return rc;

	int availableCount = 0;
	for (size_t i = 0; i < selarray.size(); i++)
	{
		if( WaitForSingleObject(selarray[i].s.event, 0) == WAIT_OBJECT_0 )
		{
			if( selarray[i].waitForRead )
				selarray[i].readAvailable = true;
			if( selarray[i].waitForWrite )
				selarray[i].writeAvailable = true;
			++availableCount;
		}
		else
		{
			selarray[i].readAvailable = false;
			selarray[i].writeAvailable = false;
		}
	}
	return availableCount;
}


#else

//////////////////////////////////////////////////////////////////////////////
// epoll version
int
selectRWEpoll(SelectObjectArray& selarray, const Timeout& timeout)
{
#ifdef BLOCXX_HAVE_SYS_EPOLL_H
	int ecc = 0;
	AutoPtrVec<epoll_event> events(new epoll_event[selarray.size()]);
	AutoDescriptor epfd(epoll_create(selarray.size()));
	if(epfd.get() == -1)
	{
		if (errno == ENOSYS) // kernel doesn't support it
		{
			return SELECT_NOT_IMPLEMENTED;
		}
		// Need to return something else?
		return Select::SELECT_ERROR;
	}

	UInt32 const read_events = EPOLLIN | EPOLLPRI | EPOLLERR | EPOLLHUP;
	UInt32 const write_events = EPOLLOUT | EPOLLERR | EPOLLHUP;
	for (size_t i = 0; i < selarray.size(); i++)
	{
		BLOCXX_ASSERT(selarray[i].s >= 0);
		selarray[i].readAvailable = false;
		selarray[i].writeAvailable = false;
		selarray[i].wasError = false;
		events[i].data = epoll_data_t(); // zero-init to make valgrind happy
		events[i].data.u32 = i;
		events[i].events = 0;
		if(selarray[i].waitForRead)
		{
			events[i].events |= read_events;
		}
		if(selarray[i].waitForWrite)
		{
			events[i].events |= write_events;
		}

		if(epoll_ctl(epfd.get(), EPOLL_CTL_ADD, selarray[i].s, &events[i]) != 0)
		{
			return errno == EPERM ? SELECT_NOT_IMPLEMENTED : SELECT_ERROR;
		}
	}

	// here we spin checking for thread cancellation every so often.

	TimeoutTimer timer(timeout);
	timer.start();
	int savedErrno;
	do
	{
		Thread::testCancel();
		const float maxWaitSec = LOOP_TIMEOUT;
		ecc = epoll_wait(epfd.get(), events.get(), selarray.size(), timer.asIntMs(maxWaitSec));
		savedErrno = errno;
		if (ecc < 0 && errno == EINTR)
		{
			ecc = 0;
			errno = 0;
			Thread::testCancel();
		}
		timer.loop();
	} while ((ecc == 0) && !timer.expired());

	if (ecc < 0)
	{
		errno = savedErrno;
		return Select::SELECT_ERROR;
	}
	if (ecc == 0)
	{
		return Select::SELECT_TIMEOUT;
	}

	for(int i = 0; i < ecc; i++)
	{
		SelectObject & so = selarray[events[i].data.u32];
		so.readAvailable = so.waitForRead && (events[i].events & read_events);
		so.writeAvailable = so.waitForWrite && (events[i].events & write_events);
	}

	return ecc;
#else
	return SELECT_NOT_IMPLEMENTED;
#endif
}

//////////////////////////////////////////////////////////////////////////////
// poll() version
int
selectRWPoll(SelectObjectArray& selarray, const Timeout& timeout)
{
#if defined (BLOCXX_HAVE_SYS_POLL_H)
	int rc = 0;

	AutoPtrVec<pollfd> pfds(new pollfd[selarray.size()]);

	// here we spin checking for thread cancellation every so often.
	TimeoutTimer timer(timeout);
	timer.start();

	int savedErrno;
	do
	{
		for (size_t i = 0; i < selarray.size(); i++)
		{
			BLOCXX_ASSERT(selarray[i].s >= 0);
			selarray[i].readAvailable = false;
			selarray[i].writeAvailable = false;
			selarray[i].wasError = false;
			pfds[i].revents = 0;
			pfds[i].fd = selarray[i].s;
			pfds[i].events = selarray[i].waitForRead ? (POLLIN | POLLPRI) : 0;
			if(selarray[i].waitForWrite)
				pfds[i].events |= POLLOUT;
		}

		Thread::testCancel();
		const float maxWaitSec = LOOP_TIMEOUT;
		rc = ::poll(pfds.get(), selarray.size(), timer.asIntMs(maxWaitSec));
		savedErrno = errno;
		if (rc < 0 && errno == EINTR)
		{
			rc = 0;
			errno = 0;
			Thread::testCancel();
#ifdef  BLOCXX_NETWARE
			//  When the NetWare server is shutting down, select will
			//  set errno to EINTR on return. If this thread does not
			//  yield control (cooperative multitasking) then we end
			//  up in a very tight loop and get a CPUHog server abbend.
			pthread_yield();
#endif
		}

		timer.loop();
	} while ((rc == 0) && !timer.expired());

	if (rc < 0)
	{
		errno = savedErrno;
		return Select::SELECT_ERROR;
	}
	if (rc == 0)
	{
		return Select::SELECT_TIMEOUT;
	}
	for (size_t i = 0; i < selarray.size(); i++)
	{
		if (pfds[i].revents & (POLLERR | POLLNVAL))
		{
			selarray[i].wasError = true;
		}

		if(selarray[i].waitForRead)
		{
			selarray[i].readAvailable = (pfds[i].revents &
				(POLLIN | POLLPRI | POLLHUP));
		}

		if(selarray[i].waitForWrite)
		{
			selarray[i].writeAvailable = (pfds[i].revents &
				(POLLOUT | POLLHUP));
		}
	}

	return rc;
#else
	return SELECT_NOT_IMPLEMENTED;
#endif
}
//////////////////////////////////////////////////////////////////////////////
// ::select() version
int
selectRWSelect(SelectObjectArray& selarray, const Timeout& timeout)
{
#if defined (BLOCXX_HAVE_SYS_SELECT_H)
	int rc = 0;
	fd_set ifds;
	fd_set ofds;

	// here we spin checking for thread cancellation every so often.
	TimeoutTimer timer(timeout);
	timer.start();

	int savedErrno;
	do
	{
		int maxfd = 0;
		FD_ZERO(&ifds);
		FD_ZERO(&ofds);
		for (size_t i = 0; i < selarray.size(); ++i)
		{
			int fd = selarray[i].s;
			BLOCXX_ASSERT(fd >= 0);
			if (maxfd < fd)
			{
				maxfd = fd;
			}
			if (fd < 0 || fd >= FD_SETSIZE)
			{
				errno = EINVAL;
				return Select::SELECT_ERROR;
			}
			if (selarray[i].waitForRead)
			{
				FD_SET(fd, &ifds);
			}
			if (selarray[i].waitForWrite)
			{
				FD_SET(fd, &ofds);
			}
		}

		Thread::testCancel();
		struct timeval tv;
		const float maxWaitSec = LOOP_TIMEOUT;
		rc = ::select(maxfd+1, &ifds, &ofds, NULL, timer.asTimeval(tv, maxWaitSec));
		savedErrno = errno;
		if (rc < 0 && errno == EINTR)
		{
			rc = 0;
			errno = 0;
			Thread::testCancel();
#ifdef  BLOCXX_NETWARE
			//  When the NetWare server is shutting down, select will
			//  set errno to EINTR on return. If this thread does not
			//  yield control (cooperative multitasking) then we end
			//  up in a very tight loop and get a CPUHog server abbend.
			pthread_yield();
#endif
		}

		timer.loop();
	} while ((rc == 0) && !timer.expired());

	if (rc < 0)
	{
		errno = savedErrno;
		return Select::SELECT_ERROR;
	}
	if (rc == 0)
	{
		return Select::SELECT_TIMEOUT;
	}
	int availableCount = 0;
	int cval;
	for (size_t i = 0; i < selarray.size(); i++)
	{
		selarray[i].wasError = false;
		cval = 0;
		if (FD_ISSET(selarray[i].s, &ifds))
		{
			selarray[i].readAvailable = true;
			cval = 1;
		}
		else
		{
			selarray[i].readAvailable = false;
		}

		if (FD_ISSET(selarray[i].s, &ofds))
		{
			selarray[i].writeAvailable = true;
			cval = 1;
		}
		else
		{
			selarray[i].writeAvailable = false;
		}

		availableCount += cval;

	}

	return availableCount;
#else
	return SELECT_NOT_IMPLEMENTED;
#endif
}

int
selectRW(SelectObjectArray& selarray, const Timeout& timeout)
{
	int rv = selectRWEpoll(selarray, timeout);
	if (rv != SELECT_NOT_IMPLEMENTED)
	{
		return rv;
	}

	rv = selectRWPoll(selarray, timeout);
	if (rv != SELECT_NOT_IMPLEMENTED)
	{
		return rv;
	}

	rv = selectRWSelect(selarray, timeout);
	BLOCXX_ASSERT(rv != SELECT_NOT_IMPLEMENTED);
	return rv;
}

//////////////////////////////////////////////////////////////////////////////
#endif	// #else BLOCXX_WIN32


//////////////////////////////////////////////////////////////////////////////
int
select(const SelectTypeArray& selarray, const Timeout& timeout)
{
	SelectObjectArray soa;
	soa.reserve(selarray.size());
	for (size_t i = 0; i < selarray.size(); ++i)
	{
		SelectObject curObj(selarray[i]);
		curObj.waitForRead = true;
		soa.push_back(curObj);
	}
	int rv = selectRW(soa, timeout);
	if (rv < 0)
	{
		return rv;
	}

	// find the first selected object
	for (size_t i = 0; i < soa.size(); ++i)
	{
		if (soa[i].readAvailable)
		{
			return i;
		}
	}
	errno = 0;
	return SELECT_ERROR;
}

} // end namespace Select

} // end namespace BLOCXX_NAMESPACE

