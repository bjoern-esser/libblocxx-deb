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
 * @author Jon Carey
 * @author Dan Nuffer
 */

#include "blocxx/BLOCXX_config.h"
#include "blocxx/Thread.hpp"
#include "blocxx/Assertion.hpp"
#include "blocxx/Format.hpp"
#include "blocxx/ThreadBarrier.hpp"
#include "blocxx/NonRecursiveMutexLock.hpp"
#include "blocxx/ExceptionIds.hpp"
#include "blocxx/Timeout.hpp"
#include "blocxx/TimeoutTimer.hpp"
#include "blocxx/DateTime.hpp"
#include "blocxx/Logger.hpp"

#include <cstring>
#include <cstdio>
#include <cerrno>
#include <iostream> // for cerr
#include <csignal>
#include <cassert>

#ifdef BLOCXX_HAVE_OPENSSL
#include <openssl/err.h>
#endif


namespace BLOCXX_NAMESPACE
{

//////////////////////////////////////////////////////////////////////////////
BLOCXX_DEFINE_EXCEPTION_WITH_ID(Thread);
BLOCXX_DEFINE_EXCEPTION_WITH_ID(CancellationDenied);

namespace
{
const GlobalString COMPONENT_NAME = BLOCXX_GLOBAL_STRING_INIT("blocxx");

//////////////////////////////////////////////////////////////////////
// this is what's really passed to threadRunner
struct ThreadParam
{
	ThreadParam(Thread* t, const ThreadDoneCallbackRef& c, const ThreadBarrier& b)
		: thread(t)
		, cb(c)
		, thread_barrier(b)
	{}
	Thread* thread;
	ThreadDoneCallbackRef cb;
	ThreadBarrier thread_barrier;
};

//////////////////////////////////////////////////////////////////////
static Thread_t
zeroThread()
{
	Thread_t zthr;
	::memset(&zthr, 0, sizeof(zthr));
	return zthr;
}

static Thread_t NULLTHREAD = zeroThread();
//////////////////////////////////////////////////////////////////////
static inline bool
sameId(const Thread_t& t1, const Thread_t& t2)
{
	return ThreadImpl::sameThreads(t1, t2);
}
} // end unnamed namespace

//////////////////////////////////////////////////////////////////////////////
// Constructor
Thread::Thread() 
	: m_id(NULLTHREAD)
	, m_isRunning(false)
	, m_joined(false)
	, m_cancelRequested(0)
	, m_cancelled(false)
{
}
//////////////////////////////////////////////////////////////////////////////
// Destructor
Thread::~Thread()
{
	try
	{
		if (!sameId(m_id, NULLTHREAD))
		{
			if (!m_joined)
			{
				join();
			}
			assert(m_isRunning == false);
			ThreadImpl::destroyThread(m_id);
		}
	}
	catch (...)
	{
		// don't let exceptions escape
	}
}
//////////////////////////////////////////////////////////////////////////////
// Start the thread
void
Thread::start(const ThreadDoneCallbackRef& cb)
{
	if (isRunning())
	{
		BLOCXX_THROW(ThreadException,
			"Thread::start - thread is already running");
	}
	if (!sameId(m_id, NULLTHREAD))
	{
		BLOCXX_THROW(ThreadException,
			"Thread::start - cannot start previously run thread");
	}
	UInt32 flgs = BLOCXX_THREAD_FLG_JOINABLE;
	ThreadBarrier thread_barrier(2);
	// p will be delted by threadRunner
	ThreadParam* p = new ThreadParam(this, cb, thread_barrier);
	int result = ThreadImpl::createThread(m_id, threadRunner, p, flgs);
	if (result != 0)
	{
		BLOCXX_THROW_ERRNO_MSG1(ThreadException, "ThreadImpl::createThread() failed", result);
	}
	thread_barrier.wait();
}
//////////////////////////////////////////////////////////////////////////////
// Wait for this object's thread execution (if any) to complete.
Int32
Thread::join()
{
	BLOCXX_ASSERT(!sameId(m_id, NULLTHREAD));
	Int32 rval;
	if (ThreadImpl::joinThread(m_id, rval) != 0)
	{
		BLOCXX_THROW(ThreadException,
			Format("Thread::join - ThreadImpl::joinThread: %1(%2)", 
				   errno, strerror(errno)).c_str());
	}
	m_joined = true;
	return rval;
}
//////////////////////////////////////////////////////////////////////////////
// STATIC
// Method used for starting threads
Int32
Thread::threadRunner(void* paramPtr)
{
	Thread_t theThreadID;
	Int32 rval = -1;
	try
	{
		// scope is important so destructors will run before the thread is clobbered by exitThread
		BLOCXX_ASSERT(paramPtr != NULL);
		ThreadParam* pParam = static_cast<ThreadParam*>(paramPtr);
		Thread* pTheThread = pParam->thread;
		ThreadImpl::saveThreadInTLS(pTheThread);
		theThreadID = pTheThread->m_id;
		ThreadDoneCallbackRef cb = pParam->cb;
		ThreadBarrier thread_barrier = pParam->thread_barrier;
		delete pParam;
		pTheThread->m_isRunning = true;
		thread_barrier.wait();

		try
		{
			rval = pTheThread->run();
		}
		// make sure we get all exceptions so the thread is cleaned up properly
		catch (ThreadCancelledException&)
		{
		}
		catch (Exception& ex)
		{
#ifdef BLOCXX_DEBUG		
			std::clog << "!!! Exception: " << ex.type() << " caught in Thread class\n";
			std::clog << ex << std::endl;
#endif
			Logger logger(COMPONENT_NAME);
			BLOCXX_LOG_ERROR(logger, Format("!!! Exception caught in Thread class: %1", ex));
			pTheThread->doneRunning(cb);
			// we need to re-throw here, otherwise we'll segfault
			// if pthread_cancel() does forced stack unwinding.
			throw;
		}
		catch (std::exception& ex)
		{
#ifdef BLOCXX_DEBUG		
			std::clog << "!!! std::exception: " << ex.what() << " caught in Thread class" << std::endl;
#endif
			Logger logger(COMPONENT_NAME);
			BLOCXX_LOG_ERROR(logger, Format("!!! std::exception caught in Thread class: %1", ex.what()));
			pTheThread->doneRunning(cb);
			// we need to re-throw here, otherwise we'll segfault
			// if pthread_cancel() does forced stack unwinding.
			throw;
		}
		catch (...)
		{
#ifdef BLOCXX_DEBUG		
			std::clog << "!!! Unknown Exception caught in Thread class" << std::endl;
#endif
			Logger logger(COMPONENT_NAME);
			BLOCXX_LOG_ERROR(logger, "!!! Unknown Exception caught in Thread class.");

			pTheThread->doneRunning(cb);
			// we need to re-throw here, otherwise we'll segfault
			// if pthread_cancel() does forced stack unwinding.
			throw;
		}

		pTheThread->doneRunning(cb);
		
	}
	catch (Exception& ex)
	{
#ifdef BLOCXX_DEBUG		
		std::clog << "!!! Exception: " << ex.type() << " caught in Thread class\n";
		std::clog << ex << std::endl;
#endif
		Logger logger(COMPONENT_NAME);
		BLOCXX_LOG_ERROR(logger, Format("!!! Exception caught in Thread class: %1", ex));
		// end the thread.  exitThread never returns.
		ThreadImpl::exitThread(theThreadID, rval);
	}
	catch (std::exception& ex)
	{
#ifdef BLOCXX_DEBUG		
		std::clog << "!!! std::exception: " << ex.what() << " caught in Thread class" << std::endl;
#endif
		Logger logger(COMPONENT_NAME);
		BLOCXX_LOG_ERROR(logger, Format("!!! std::exception caught in Thread class: %1", ex.what()));
		// end the thread.  exitThread never returns.
		ThreadImpl::exitThread(theThreadID, rval);
	}
	catch (...)
	{
#ifdef BLOCXX_DEBUG		
		std::clog << "!!! Unknown Exception caught in Thread class" << std::endl;
#endif
		Logger logger(COMPONENT_NAME);
		BLOCXX_LOG_ERROR(logger, "!!! Unknown Exception caught in Thread class.");
		// end the thread.  exitThread never returns.
		ThreadImpl::exitThread(theThreadID, rval);
	}
	// end the thread.  exitThread never returns.
	ThreadImpl::exitThread(theThreadID, rval);
	return rval;
}

//////////////////////////////////////////////////////////////////////
void
Thread::doneRunning(const ThreadDoneCallbackRef& cb)
{
	{
		NonRecursiveMutexLock lock(m_stateGuard);
		m_isRunning = false;
		m_stateCond.notifyAll();
	}

	if (cb)
	{
		cb->notifyThreadDone(this);
	}
#ifdef BLOCXX_HAVE_OPENSSL
	// this is necessary to free memory associated with the OpenSSL error queue for this thread.
	ERR_remove_state(0);
#endif
}

//////////////////////////////////////////////////////////////////////
void
Thread::shutdown()
{
	doShutdown();
}
//////////////////////////////////////////////////////////////////////
bool
Thread::shutdown(const Timeout& timeout)
{
	doShutdown();
	return timedWait(timeout);
}
//////////////////////////////////////////////////////////////////////
void
Thread::cooperativeCancel()
{
	if (!isRunning())
	{
		return;
	}

	// give the thread a chance to clean up a bit or abort the cancellation.
	doCooperativeCancel();
	m_cancelRequested = Atomic_t(1);

#if !defined(BLOCXX_WIN32)
	// send a SIGUSR1 to the thread to break it out of any blocking syscall.
	// SIGUSR1 is ignored.  It's set to SIG_IGN in ThreadImpl.cpp
	// If the thread has already exited, an ThreadException will be thrown
	// we just want to ignore that.
	try
	{
		ThreadImpl::sendSignalToThread(m_id, SIGUSR1);
	}
	catch (ThreadException&) 
	{
	}
#endif
}
//////////////////////////////////////////////////////////////////////
bool
Thread::definitiveCancel(UInt32 waitForCooperativeSecs)
{
	return definitiveCancel(Timeout::relative(waitForCooperativeSecs));
}
//////////////////////////////////////////////////////////////////////
bool
Thread::definitiveCancel(const Timeout& timeout)
{
	if (!isRunning())
	{
		return true;
	}

	// give the thread a chance to clean up a bit or abort the cancellation.
	doCooperativeCancel();
	m_cancelRequested = Atomic_t(1);

#if !defined(BLOCXX_WIN32)
	// send a SIGUSR1 to the thread to break it out of any blocking syscall.
	// SIGUSR1 is ignored.  It's set to SIG_IGN in ThreadImpl.cpp
	// If the thread has already exited, an ThreadException will be thrown
	// we just want to ignore that.
	try
	{
		ThreadImpl::sendSignalToThread(m_id, SIGUSR1);
	}
	catch (ThreadException&) 
	{
	}
#endif

	Logger logger(COMPONENT_NAME);
	TimeoutTimer timer(timeout);
	NonRecursiveMutexLock l(m_stateGuard);
	while (!m_cancelled && isRunning())
	{
		BLOCXX_LOG_DEBUG3(logger, "Thread::definitiveCancel waiting for thread to exit.");
		if (!m_stateCond.timedWait(l, timer.asAbsoluteTimeout()))
		{
			// give the thread a chance to clean up a bit or abort the cancellation.
			doDefinitiveCancel();
			// thread didn't (or won't) exit by itself, we'll have to really kill it.
			if (!m_cancelled && isRunning())
			{
				BLOCXX_LOG_ERROR(logger, "Thread::definitiveCancel cancelling thread because it did not exit!");
				this->cancel_internal(true);
			}
			return false;
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////
void
Thread::cancel()
{
	this->cancel_internal(false);
}

//////////////////////////////////////////////////////////////////////
void
Thread::cancel_internal(bool is_locked)
{
	// Ignore errors from ThreadImpl (usually caused by the fact that the thread
	// has already exited)
	try
	{
		ThreadImpl::cancel(m_id);
	}
	catch (ThreadException&)
	{
	}
	{
		NonRecursiveMutex mtx;
		NonRecursiveMutexLock l(is_locked ? mtx : m_stateGuard);
		m_cancelled = true;
		m_isRunning = false;
		m_stateCond.notifyAll();
	}
}
//////////////////////////////////////////////////////////////////////
void
Thread::testCancel()
{
	ThreadImpl::testCancel();
}

//////////////////////////////////////////////////////////////////////
void
Thread::doShutdown()
{
}
//////////////////////////////////////////////////////////////////////
void 
Thread::doCooperativeCancel()
{
}
//////////////////////////////////////////////////////////////////////
void 
Thread::doDefinitiveCancel()
{
}

//////////////////////////////////////////////////////////////////////
bool
Thread::timedWait(const Timeout& timeout)
{
	TimeoutTimer tt(timeout);
	NonRecursiveMutexLock lock(m_stateGuard);
	while (m_isRunning == true)
	{
		if (!m_stateCond.timedWait(lock, tt.asAbsoluteTimeout()))
		{
			return false; // timeout
		}
	}
	return true; // exited
}

} // end namespace BLOCXX_NAMESPACE

