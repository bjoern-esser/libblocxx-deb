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
 * @author Dan Nuffer
 */
#include "blocxx/BLOCXX_config.h"
#include "blocxx/Condition.hpp"
#include "blocxx/NonRecursiveMutexLock.hpp"
#include "blocxx/ExceptionIds.hpp"
#include "blocxx/Timeout.hpp"
#include "blocxx/TimeoutTimer.hpp"
#include "blocxx/ThreadImpl.hpp"

#include <cassert>
#include <cerrno>
#include <limits>
#ifdef BLOCXX_HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

namespace BLOCXX_NAMESPACE
{

BLOCXX_DEFINE_EXCEPTION_WITH_ID(ConditionLock);
BLOCXX_DEFINE_EXCEPTION_WITH_ID(ConditionResource);
#if defined(BLOCXX_USE_PTHREAD)
/////////////////////////////////////////////////////////////////////////////
Condition::Condition()
{
	int res = pthread_cond_init(&m_condition, PTHREAD_COND_ATTR_DEFAULT);
	if (res != 0)
	{
		BLOCXX_THROW(ConditionResourceException, "Failed initializing condition variable");
	}
}
/////////////////////////////////////////////////////////////////////////////
Condition::~Condition()
{
	int res = pthread_cond_destroy(&m_condition);
	assert(res == 0);
}
/////////////////////////////////////////////////////////////////////////////
void
Condition::notifyOne()
{
	int res = pthread_cond_signal(&m_condition);
	assert(res == 0);
}
/////////////////////////////////////////////////////////////////////////////
void
Condition::notifyAll()
{
	int res = pthread_cond_broadcast(&m_condition);
	assert(res == 0);
}
/////////////////////////////////////////////////////////////////////////////
void
Condition::doWait(NonRecursiveMutex& mutex)
{
	ThreadImpl::testCancel();
	int res;
	NonRecursiveMutexLockState state;
	mutex.conditionPreWait(state);
	res = pthread_cond_wait(&m_condition, state.pmutex);
	mutex.conditionPostWait(state);
	assert(res == 0 || res == EINTR);
	if (res == EINTR)
	{
		ThreadImpl::testCancel();
	}
}
/////////////////////////////////////////////////////////////////////////////
namespace
{
	inline 
	bool timespec_less(struct timespec const & x, struct timespec const & y)
	{
		return x.tv_sec < y.tv_sec ||
			x.tv_sec == y.tv_sec && x.tv_nsec < y.tv_nsec;
	}

	int check_timedwait(
		int rc, pthread_cond_t * cond, pthread_mutex_t * mtx,
		struct timespec const * abstime
	)
	{
#ifdef BLOCXX_NCR
		if (rc == -1 && errno == EAGAIN)
		{
			return ETIMEDOUT;
		}
#endif
		if (rc != EINVAL)
		{
			return rc;
		}
		// Solaris won't let you wait more than 10 ** 8 seconds.
		time_t const max_future = 99999999;
		time_t const max_time = std::numeric_limits<time_t>::max();
		time_t now_sec = DateTime::getCurrent().get();
		struct timespec new_abstime;
		new_abstime.tv_sec = (
			now_sec <= max_time - max_future
			? now_sec + max_future
			: max_time
		);
		new_abstime.tv_nsec = 0;
		bool early = timespec_less(new_abstime, *abstime);
		if (!early)
		{
			new_abstime = *abstime;
		}
		int newrc = pthread_cond_timedwait(cond, mtx, &new_abstime);
		return (newrc == ETIMEDOUT && early ? EINTR : newrc);
	}
}

bool
Condition::doTimedWait(NonRecursiveMutex& mutex, const Timeout& timeout)
{
	ThreadImpl::testCancel();
	int res;
	NonRecursiveMutexLockState state;
	mutex.conditionPreWait(state);
	bool ret = false;

	timespec ts;
	TimeoutTimer timer(timeout);

	res = pthread_cond_timedwait(&m_condition, state.pmutex, timer.asTimespec(ts));
	res = check_timedwait(res, &m_condition, state.pmutex, &ts);
	mutex.conditionPostWait(state);
	assert(res == 0 || res == ETIMEDOUT || res == EINTR);
	if (res == EINTR)
	{
		ThreadImpl::testCancel();
	}
	ret = res != ETIMEDOUT;
	return ret;
}
#elif defined (BLOCXX_WIN32)
/////////////////////////////////////////////////////////////////////////////
Condition::Condition()
	: m_condition(new ConditionInfo_t)
{
	m_condition->waitersCount = 0;
	m_condition->wasBroadcast = false;
	m_condition->queue = ::CreateSemaphore(
		NULL,		// No security
		0,			// initially 0
		0x7fffffff,	// max count
		NULL);		// Unnamed
	::InitializeCriticalSection(&m_condition->waitersCountLock);
	m_condition->waitersDone = ::CreateEvent(
		NULL,		// No security
		false,		// auto-reset
		false,		// non-signaled initially
		NULL);		// Unnamed
}
/////////////////////////////////////////////////////////////////////////////
Condition::~Condition()
{
	::CloseHandle(m_condition->queue);
	::DeleteCriticalSection(&m_condition->waitersCountLock);
	::CloseHandle(m_condition->waitersDone);
	delete m_condition;
}
/////////////////////////////////////////////////////////////////////////////
void
Condition::notifyOne()
{
	::EnterCriticalSection(&m_condition->waitersCountLock);
	bool haveWaiters = m_condition->waitersCount > 0;
	::LeaveCriticalSection(&m_condition->waitersCountLock);

	// If no threads waiting, then this is a no-op
	if (haveWaiters)
	{
		::ReleaseSemaphore(m_condition->queue, 1, 0);
	}
}
/////////////////////////////////////////////////////////////////////////////
void
Condition::notifyAll()
{
	::EnterCriticalSection(&m_condition->waitersCountLock);
	bool haveWaiters = false;
	if (m_condition->waitersCount > 0)
	{
		// It's gonna be a broadcast, even if there's only one waiting thread.
		haveWaiters = m_condition->wasBroadcast = true;
	}

	if (haveWaiters)
	{
		// Wake up all the waiting threads atomically
		::ReleaseSemaphore(m_condition->queue, m_condition->waitersCount, 0);
		::LeaveCriticalSection(&m_condition->waitersCountLock);

		// Wait for all the threads to acquire the counting semaphore
		::WaitForSingleObject(m_condition->waitersDone, INFINITE);
		m_condition->wasBroadcast = false;
	}
	else
	{
		::LeaveCriticalSection(&m_condition->waitersCountLock);
	}
}
/////////////////////////////////////////////////////////////////////////////
void
Condition::doWait(NonRecursiveMutex& mutex)
{
  doTimedWait(mutex, Timeout::infinite);
}
/////////////////////////////////////////////////////////////////////////////
bool
//Condition::doTimedWait(NonRecursiveMutex& mutex, UInt32 sTimeout, UInt32 usTimeout)
Condition::doTimedWait(NonRecursiveMutex& mutex, const Timeout& timeout)
{
	ThreadImpl::testCancel();
	bool cc = true;
	NonRecursiveMutexLockState state;
	mutex.conditionPreWait(state);

	::EnterCriticalSection(&m_condition->waitersCountLock);
	m_condition->waitersCount++;
	::LeaveCriticalSection(&m_condition->waitersCountLock);

	TimeoutTimer timer(timeout);
	// Atomically release the mutex and wait on the
	// queue until signal/broadcast.
	if (::SignalObjectAndWait(mutex.m_mutex, m_condition->queue, timer.asDWORDMs(),
		false) == WAIT_TIMEOUT)
	{
		cc = false;
	}

	::EnterCriticalSection(&m_condition->waitersCountLock);
	m_condition->waitersCount--;

	// Check to see if we're the last waiter after the broadcast
	bool isLastWaiter = (m_condition->wasBroadcast && m_condition->waitersCount == 0
		&& cc == true);

	::LeaveCriticalSection(&m_condition->waitersCountLock);

	// If this is the last thread waiting for this broadcast, then let all the
	// other threads proceed.
	if (isLastWaiter)
	{
		// Atomically signal the waitersDone event and wait to acquire
		// the external mutex. Enusres fairness
		::SignalObjectAndWait(m_condition->waitersDone, mutex.m_mutex,
			INFINITE, false);
	}
	else
	{
		// Re-gain ownership of the external mutex
		::WaitForSingleObject(mutex.m_mutex, INFINITE);
	}
	mutex.conditionPostWait(state);
	return cc;
}
#else
#error "port me!"
#endif
/////////////////////////////////////////////////////////////////////////////
void
Condition::wait(NonRecursiveMutexLock& lock)
{
	if (!lock.isLocked())
	{
		BLOCXX_THROW(ConditionLockException, "Lock must be locked");
	}
	doWait(*(lock.m_mutex));
}
/////////////////////////////////////////////////////////////////////////////
bool
Condition::timedWait(NonRecursiveMutexLock& lock, UInt32 sTimeout, UInt32 usTimeout)
{
	return timedWait(lock, Timeout::relative(sTimeout + static_cast<float>(usTimeout) / 1000000.0));
}

/////////////////////////////////////////////////////////////////////////////
bool
Condition::timedWait(NonRecursiveMutexLock& lock, const Timeout& timeout)
{
	if (!lock.isLocked())
	{
		BLOCXX_THROW(ConditionLockException, "Lock must be locked");
	}
	return doTimedWait(*(lock.m_mutex), timeout);
}

} // end namespace BLOCXX_NAMESPACE

