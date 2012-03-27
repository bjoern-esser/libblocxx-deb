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
 */

#include "blocxx/BLOCXX_config.h"
#include "blocxx/ThreadImpl.hpp"
#include "blocxx/Mutex.hpp"
#include "blocxx/Assertion.hpp"
#include "blocxx/Thread.hpp"
#include "blocxx/NonRecursiveMutexLock.hpp"
#include "blocxx/NonRecursiveMutex.hpp"
#include "blocxx/Condition.hpp"
#include "blocxx/Timeout.hpp"
#include "blocxx/Format.hpp"
#include "blocxx/TimeoutTimer.hpp"
#if defined(BLOCXX_WIN32)
#include "blocxx/Map.hpp"
#include "blocxx/MutexLock.hpp"
#endif
#include <cassert>
#include <cstring>
#include <cstddef>

extern "C"
{
#ifdef BLOCXX_HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include <sys/types.h>

#ifdef BLOCXX_HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <errno.h>
#include <signal.h>

#ifdef BLOCXX_USE_PTHREAD
#include <pthread.h>
#endif

#ifdef BLOCXX_WIN32
#include <process.h>
#endif
}

namespace BLOCXX_NAMESPACE
{

namespace ThreadImpl {

//////////////////////////////////////////////////////////////////////////////
// STATIC
void
sleep(UInt32 milliSeconds)
{
	sleep(Timeout::relative(milliSeconds / 1000.0));
}

void
sleep(const Timeout& timeout)
{
	NonRecursiveMutex mtx;
	NonRecursiveMutexLock lock(mtx);
	Condition cond;
	TimeoutTimer timer(timeout);
	while (!timer.expired())
	{
		// if it timed out, no reason to loop again
		if (!cond.timedWait(lock, timer.asAbsoluteTimeout()))
		{
			return;
		}
		timer.loop();
	}
}
//////////////////////////////////////////////////////////////////////////////
// STATIC
void
yield()
{
#if defined(BLOCXX_HAVE_SCHED_YIELD)
	sched_yield();
#elif defined(BLOCXX_WIN32)
	ThreadImpl::testCancel();
	::SwitchToThread();
#else
	ThreadImpl::sleep(1);
#endif
}

#if defined(BLOCXX_USE_PTHREAD)
namespace {
struct LocalThreadParm
{
	ThreadFunction m_func;
	void* m_funcParm;
};
extern "C" {
static void*
threadStarter(void* arg)
{
	// set our cancellation state
#ifdef BLOCXX_NCR
	pthread_setcancel(CANCEL_ON);
	pthread_setasynccancel(CANCEL_OFF);
#else
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
#endif

	// unblock SIGUSR1, which is used to implement thread cooperative cancellation
	sigset_t signalSet;
	int rv = sigemptyset(&signalSet);
	BLOCXX_ASSERT(rv == 0);
	rv = sigaddset(&signalSet, SIGUSR1);
	BLOCXX_ASSERT(rv == 0);
	rv = pthread_sigmask(SIG_UNBLOCK, &signalSet, 0);
	BLOCXX_ASSERT(rv == 0);

	LocalThreadParm* parg = static_cast<LocalThreadParm*>(arg);
	ThreadFunction func = parg->m_func;
	void* funcParm = parg->m_funcParm;
	delete parg;
	Int32 rval = (*func)(funcParm);
	void* prval = reinterpret_cast<void*>(static_cast<ptrdiff_t>(rval));
	pthread_exit(prval);
	return prval;
}
}
// The purpose of this class is to retrieve the default stack size only once
// at library load time and re-use it thereafter.
struct default_stack_size
{
#if !defined(BLOCXX_NCR)
	default_stack_size()
	{
		// if anything in this function fails, we'll just leave val == 0.
		val = 0;
		needsSetting = false;

// make sure we have a big enough stack.  BloCxx can use quite a bit, so we'll try to make sure we get at least 1 MB.
// 1 MB is just an arbitrary number.  The default on Linux is 2 MB which has never been a problem.  However, on UnixWare
// the default is really low (16K IIRC) and that isn't enough. It would be good to do some sort of measurement...
#ifdef _POSIX_THREAD_ATTR_STACKSIZE
		pthread_attr_t stack_size_attr;
		if (pthread_attr_init(&stack_size_attr) != 0)
		{
			return;
		}
		if (pthread_attr_getstacksize(&stack_size_attr, &val) != 0)
		{
			return;
		}

		if (val < 1048576) 
		{
			val = 1048576; // 1 MB
			needsSetting = true;
		}
#ifdef PTHREAD_STACK_MIN
		if (PTHREAD_STACK_MIN > val) 
		{
			val = PTHREAD_STACK_MIN;
			needsSetting = true;
		}
#endif

#endif //#ifdef _POSIX_THREAD_ATTR_STACKSIZE
	}

#else //#if !defined(BLOCXX_NCR)
	default_stack_size()
	{
		// if anything in this function fails, we'll just leave val == 0.
		val = 0;
		needsSetting = false;

// make sure we have a big enough stack.  BloCxx can use quite a bit, so we'll try to make sure we get at least 1 MB.
// 1 MB is just an arbitrary number.  The default on Linux is 2 MB which has never been a problem.  However, on UnixWare
// the default is really low (16K IIRC) and that isn't enough. It would be good to do some sort of measurement...
#ifdef _POSIX_THREAD_ATTR_STACKSIZE
		pthread_attr_t stack_size_attr;
		if (pthread_attr_create(&stack_size_attr) != 0)
		{
			return;
		}

		val = pthread_attr_getstacksize(stack_size_attr);
		if (static_cast<signed>(val) == -1)
		{
			return;
		}

		//we do not set the minimal stack size in 1 Mb because NCR returns 32K
		//and if we set 1M or even 256K we get 'Out of Memory'

#if defined(PTHREAD_STACK_MIN) && defined(_SC_THREAD_STACK_MIN)
		if (PTHREAD_STACK_MIN > val) 
		{
			val = PTHREAD_STACK_MIN;
			needsSetting = true;
		}
#endif

#endif //#ifdef _POSIX_THREAD_ATTR_STACKSIZE
	}
#endif //#if !defined(BLOCXX_NCR)

	static size_t val;
	static bool needsSetting;
};

size_t default_stack_size::val = 0;
bool default_stack_size::needsSetting(false);
default_stack_size g_theDefaultStackSize;
//////////////////////////////////////////////////////////////////////
pthread_once_t once_control = BLOCXX_THREAD_ONCE_INIT;
pthread_key_t theKey;
extern "C" {

#ifdef BLOCXX_NCR
static void
SIGUSR1Handler()	
{
	// do nothing
}
#else
static void
SIGUSR1Handler(int sig)
{
	// do nothing
}
#endif

//////////////////////////////////////////////////////////////////////
static void doOneTimeThreadInitialization()
{
#ifdef BLOCXX_NCR
	pthread_keycreate(&theKey, NULL);
#else
	pthread_key_create(&theKey, NULL);
#endif
	// Handle SIGUSR1 so we can safely send it to threads when we want to cancel them.
	struct sigaction temp;
	memset(&temp, '\0', sizeof(temp));
	sigaction(SIGUSR1, 0, &temp);
	temp.sa_handler = SIGUSR1Handler;
	sigemptyset(&temp.sa_mask);
	temp.sa_flags = 0;
	sigaction(SIGUSR1, &temp, NULL);
}

} // end extern "C"
} // end unnamed namespace
//////////////////////////////////////////////////////////////////////////////

#if !defined(BLOCXX_NCR)
// STATIC
int
createThread(Thread_t& handle, ThreadFunction func,
	void* funcParm, UInt32 threadFlags)
{
	int cc = 0;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	if (!(threadFlags & BLOCXX_THREAD_FLG_JOINABLE))
	{
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	}

#if !defined(BLOCXX_VALGRIND_SUPPORT) // valgrind doesn't like us to set the stack size
	// Won't be set to true unless _POSIX_THREAD_ATTR_STACKSIZE is defined
	if (default_stack_size::needsSetting)
	{
		pthread_attr_setstacksize(&attr, default_stack_size::val);
	}
#endif

	LocalThreadParm* parg = new LocalThreadParm;
	parg->m_func = func;
	parg->m_funcParm = funcParm;
	cc = pthread_create(&handle, &attr, threadStarter, parg);
	pthread_attr_destroy(&attr);
	return cc;
}

#else //#if !defined(BLOCXX_NCR)
// STATIC
int
createThread(Thread_t& handle, ThreadFunction func,
	void* funcParm, UInt32 threadFlags)
{
	int cc = 0;
	pthread_attr_t attr;
	pthread_attr_create(&attr);

#if !defined(BLOCXX_VALGRIND_SUPPORT) // valgrind doesn't like us to set the stack size
	// Won't be set to true unless _POSIX_THREAD_ATTR_STACKSIZE is defined
	if (default_stack_size::needsSetting)
	{
		pthread_attr_setstacksize(&attr, default_stack_size::val);
	}
#endif

	LocalThreadParm* parg = new LocalThreadParm;
	parg->m_func = func;
	parg->m_funcParm = funcParm;
	if (pthread_create(&handle, attr, threadStarter, parg) != 0)
	{
		cc = -1;
	}

	if (cc != -1 && !(threadFlags & BLOCXX_THREAD_FLG_JOINABLE))
	{
		pthread_detach(&handle);
	}

	pthread_attr_delete(&attr);
	return cc;
}
#endif //#if !defined(BLOCXX_NCR)
//////////////////////////////////////////////////////////////////////////////
// STATIC
void
exitThread(Thread_t&, Int32 rval)
{
	void* prval = reinterpret_cast<void*>(static_cast<ptrdiff_t>(rval));
	pthread_exit(prval);
}


#if defined(BLOCXX_SIZEOF_PTHREAD_T)
#if BLOCXX_SIZEOF_PTHREAD_T == 2
#define BLOCXX_THREAD_CONVERTER UInt16
#elif BLOCXX_SIZEOF_PTHREAD_T == 4
#define BLOCXX_THREAD_CONVERTER UInt32
#elif BLOCXX_SIZEOF_PTHREAD_T == 8
#define BLOCXX_THREAD_CONVERTER UInt64
#else
#ifdef BLOCXX_NCR //BLOCXX_SIZEOF_PTHREAD_T=0 for this OS
#define BLOCXX_THREAD_CONVERTER UInt16
#else /* BLOCXX_SIZEOF_PTHREAD_T */
#error Unexpected size for pthread_t
#endif /* BLOCXX_NCR */
#endif /* BLOCXX_SIZEOF_PTHREAD_T */
#else
#error No pthread_t size was found!
#endif /* defined(BLOCXX_SIZEOF_PTHREAD_T) */

UInt64 thread_t_ToUInt64(Thread_t thr)
{
#ifdef BLOCXX_NCR
	return UInt64(BLOCXX_THREAD_CONVERTER(cma_thread_get_unique(&thr)));
#else
	return UInt64(BLOCXX_THREAD_CONVERTER(thr));
#endif
}
#undef BLOCXX_THREAD_CONVERTER

//////////////////////////////////////////////////////////////////////////////
// STATIC
void
destroyThread(Thread_t& )
{
}
//////////////////////////////////////////////////////////////////////////////
// STATIC
int
setThreadDetached(Thread_t& handle)
{
#ifdef BLOCXX_NCR
	int cc = pthread_detach(&handle);
#else
	int cc = pthread_detach(handle);
#endif
	if (cc != 0)
	{
		if (cc != EINVAL)
		{
			cc = -1;
		}
	}
	return cc;
}
//////////////////////////////////////////////////////////////////////////////
// STATIC
int
joinThread(Thread_t& handle, Int32& rval)
{
	void* prval(0);
	if ((errno = pthread_join(handle, &prval)) == 0)
	{
		rval = static_cast<Int32>(reinterpret_cast<ptrdiff_t>(prval));
		return 0;
	}
	else
	{
		return 1;
	}
}
//////////////////////////////////////////////////////////////////////
void
testCancel()
{
	// set up our TLS which will be used to store the Thread* in.
	pthread_once(&once_control, &doOneTimeThreadInitialization);
	Thread* theThread = NULL;
#ifdef BLOCXX_NCR
	pthread_addr_t addr_ptr = NULL;
	int ret = pthread_getspecific(theKey, &addr_ptr);
	if (ret == 0)
	{
		theThread = reinterpret_cast<Thread*>(addr_ptr);
	}
#else
	theThread = reinterpret_cast<Thread*>(pthread_getspecific(theKey));
#endif
	if (theThread == 0)
	{
		return;
	}
	if (AtomicGet(theThread->m_cancelRequested) == 1)
	{
		// We don't use BLOCXX_THROW here because 
		// ThreadCancelledException is special.  It's not derived
		// from Exception on purpose so it can be propagated up
		// the stack easier. This exception shouldn't be caught and not
		// re-thrown anywhere except in Thread::threadRunner()
		throw ThreadCancelledException();
	}
}
//////////////////////////////////////////////////////////////////////
void saveThreadInTLS(void* pTheThread)
{
	// set up our TLS which will be used to store the Thread* in.
	pthread_once(&once_control, &doOneTimeThreadInitialization);
	int rc;
	if ((rc = pthread_setspecific(theKey, pTheThread)) != 0)
	{
		BLOCXX_THROW(ThreadException, Format("pthread_setspecific failed.  error = %1(%2)", rc, strerror(rc)).c_str());
	}
}
//////////////////////////////////////////////////////////////////////
void sendSignalToThread(Thread_t threadID, int signo)
{
	int rc;
	if ((rc = pthread_kill(threadID, signo)) != 0)
	{
		BLOCXX_THROW(ThreadException, Format("pthread_kill failed.  error = %1(%2)", rc, strerror(rc)).c_str());
	}
}
//////////////////////////////////////////////////////////////////////
void cancel(Thread_t threadID)
{
	int rc;
	if ((rc = pthread_cancel(threadID)) != 0)
	{
		BLOCXX_THROW(ThreadException, Format("pthread_cancel failed.  error = %1(%2)", rc, strerror(rc)).c_str());
	}
}
#endif // #ifdef BLOCXX_USE_PTHREAD

#if defined(BLOCXX_WIN32)

namespace {

struct WThreadInfo
{
	HANDLE	handle;
	BLOCXX_NAMESPACE::Thread* pTheThread;
};

typedef Map<DWORD, WThreadInfo> Win32ThreadMap;
Win32ThreadMap g_threads;
Mutex g_threadsGuard;

struct LocalThreadParm
{
	ThreadFunction m_func;
	void* m_funcParm;
};

//////////////////////////////////////////////////////////////////////////////
extern "C" {
unsigned __stdcall threadStarter(void* arg)
{
	LocalThreadParm* parg = reinterpret_cast<LocalThreadParm*>(arg);
	ThreadFunction func = parg->m_func;
	void* funcParm = parg->m_funcParm;
	delete parg;
	Int32 rval = (*func)(funcParm);
	::_endthreadex(static_cast<unsigned>(rval));
	return rval;
}
}	// End extern "C"

//////////////////////////////////////////////////////////////////////////////
void
addThreadToMap(DWORD threadId, HANDLE threadHandle)
{
	MutexLock ml(g_threadsGuard);
	WThreadInfo wi;
	wi.handle = threadHandle;
	wi.pTheThread = 0;
	g_threads[threadId] = wi;
}

//////////////////////////////////////////////////////////////////////////////
HANDLE
getThreadHandle(DWORD threadId)
{
	MutexLock ml(g_threadsGuard);
	HANDLE chdl = 0;
	Win32ThreadMap::iterator it = g_threads.find(threadId);
	if (it != g_threads.end())
	{
		chdl = it->second.handle;
	}
	return chdl;
}

//////////////////////////////////////////////////////////////////////////////
void
setThreadPointer(DWORD threadId, Thread* pTheThread)
{
	MutexLock ml(g_threadsGuard);
	Win32ThreadMap::iterator it = g_threads.find(threadId);
	if (it != g_threads.end())
	{
		it->second.pTheThread = pTheThread;
	}
}

//////////////////////////////////////////////////////////////////////////////
HANDLE
removeThreadFromMap(DWORD threadId)
{
	MutexLock ml(g_threadsGuard);
	HANDLE chdl = 0;
	Win32ThreadMap::iterator it = g_threads.find(threadId);
	if (it != g_threads.end())
	{
		chdl = it->second.handle;
		g_threads.erase(it);
	}
	return chdl;
}

//////////////////////////////////////////////////////////////////////////////
Thread*
getThreadObject(DWORD threadId)
{
	Thread* pTheThread = 0;
	MutexLock ml(g_threadsGuard);
	Win32ThreadMap::iterator it = g_threads.find(threadId);
	if (it != g_threads.end())
	{
		pTheThread = it->second.pTheThread;
	}
	return pTheThread;
}

}	// End unnamed namespace

//////////////////////////////////////////////////////////////////////////////
// STATIC
int
createThread(Thread_t& handle, ThreadFunction func,
	void* funcParm, UInt32 threadFlags)
{
	int cc = -1;
	HANDLE hThread;
	unsigned threadId;

	LocalThreadParm* parg = new LocalThreadParm;
	parg->m_func = func;
	parg->m_funcParm = funcParm;
	hThread = reinterpret_cast<HANDLE>(::_beginthreadex(NULL, 0, threadStarter,
		parg, 0, &threadId));
	if (hThread != 0)
	{
		addThreadToMap(threadId, hThread);
		handle = threadId;
		cc = 0;
	}
	else
	{
		cc = errno;
	}

	return cc;
}
//////////////////////////////////////////////////////////////////////////////
// STATIC
void
exitThread(Thread_t&, Int32 rval)
{
	::_endthreadex(static_cast<unsigned>(rval));
}

//////////////////////////////////////////////////////////////////////////////
// STATIC
UInt64 thread_t_ToUInt64(Thread_t thr)
{
	//  This should really be a compile time assert.
	BLOCXX_ASSERTMSG(sizeof(unsigned long) >= sizeof(Thread_t),"  Thread_t truncated!");
	return static_cast<UInt64>(thr);
}

//////////////////////////////////////////////////////////////////////////////
// STATIC
void
destroyThread(Thread_t& threadId)
{
	HANDLE thdl = removeThreadFromMap(threadId);
	if (thdl != 0)
	{
		::CloseHandle(thdl);
	}
}
//////////////////////////////////////////////////////////////////////////////
// STATIC
int
setThreadDetached(Thread_t& handle)
{
	// No need for this on Win32
	return 0;
}
//////////////////////////////////////////////////////////////////////////////
// STATIC
int
joinThread(Thread_t& threadId, Int32& rvalArg)
{
	int cc = -1;
	DWORD rval;
	HANDLE thdl = getThreadHandle(threadId);
	if (thdl != 0)
	{
		if (::WaitForSingleObject(thdl, INFINITE) != WAIT_FAILED)
		{
			if (::GetExitCodeThread(thdl, &rval) != 0)
			{
				rvalArg = static_cast<Int32>(rval);
				cc = 0;
			}
		}
	}
	return cc;
}

//////////////////////////////////////////////////////////////////////
void
testCancel()
{
	DWORD threadId = ThreadImpl::currentThread();
	Thread* pTheThread = getThreadObject(threadId);
	if (pTheThread)
	{
		if (AtomicGet(pTheThread->m_cancelRequested) == 1)
		{
			// We don't use BLOCXX_THROW here because 
			// ThreadCancelledException is special.  It's not derived
			// from Exception on purpose so it can be propagated up
			// the stack easier. This exception shouldn't be caught and not
			// re-thrown anywhere except in Thread::threadRunner()
			throw ThreadCancelledException();
		}
	}
}
//////////////////////////////////////////////////////////////////////
void saveThreadInTLS(void* pThreadArg)
{
	Thread* pThread = static_cast<Thread*>(pThreadArg);
	DWORD threadId = pThread->getId();
    setThreadPointer(threadId, pThread);
}
//////////////////////////////////////////////////////////////////////
void sendSignalToThread(Thread_t threadID, int signo)
{
}
//////////////////////////////////////////////////////////////////////
void cancel(Thread_t threadId)
{
	HANDLE thdl = getThreadHandle(threadId);
	if (thdl != 0)
	{
		::TerminateThread(thdl, -1);
	}
}

#endif // #ifdef BLOCXX_WIN32
} // end namespace BLOCXX_ThreadImpl

} // end namespace BLOCXX_NAMESPACE

