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
 * @author Jon Carey
 * @author Dan Nuffer
 */

#ifndef BLOCXX_THREAD_IMPL_HPP_INCLUDE_GUARD_
#define BLOCXX_THREAD_IMPL_HPP_INCLUDE_GUARD_
#include "blocxx/BLOCXX_config.h"
#include "blocxx/Types.hpp"
#include "blocxx/ThreadTypes.hpp"
#include "blocxx/Timeout.hpp"

#ifdef BLOCXX_NCR
#ifndef PTHREAD_ONCE_INIT
#define PTHREAD_ONCE_INIT pthread_once_init
#endif
#endif


// The classes and functions defined in this file are not meant for general
// use, they are internal implementation details.  They may change at any time.

namespace BLOCXX_NAMESPACE
{

/*----------------------------------
 * Definitions to support threading.
 -----------------------------------*/
#define BLOCXX_THREAD_FLG_JOINABLE 0x000000001
typedef Int32 (*ThreadFunction)(void*);
/**
 * The ThreadImpl namespace represents the functionality needed by the
 * BloCxx Thread class (Thread). It also contains other misellaneous
 * functions which are useful for synchronization and threads.
 * It is essentially an abstraction layer over a thread implementation.
 */
namespace ThreadImpl
{
	/**
	 * Starts a thread running the given function.
	 * @param handle	A platform specific thread handle
	 * @param func		The function that will run within the new thread.
	 * @param funcParm The parameter to func
	 * @param flags	The flags to use when creating the thread.
	 *	Currently flags can contain the following:
	 *		THREAD_JOINABLE - Thread will be created in the joinable state.
	 *
	 * @return 0 on success. Otherwise -1 or a system error number (e.g. errno)
	 */
	BLOCXX_COMMON_API int createThread(Thread_t& handle, ThreadFunction func,
		void* funcParm, UInt32 threadFlags);
	/**
	 * Destroy any resources associated with a thread that was created with
	 * the createThread method.
	 * @param handle	A platform specific thread handle
	 */
	BLOCXX_COMMON_API void destroyThread(Thread_t& handle);
	/**
	 * Check two platform dependant thread types for equality.
	 * @param handle1	The 1st thread type for the comparison.
	 * @param handle2	The 2nd thread type for the comparison.
	 * @return true if the thread types are equal. Otherwise false
	 */
	inline bool sameThreads(const volatile Thread_t& handle1,
		const volatile Thread_t& handle2)
	{
	#if defined(BLOCXX_WIN32)
		return handle1 == handle2;
	#elif BLOCXX_USE_PTHREAD
		return pthread_equal(handle1, handle2);
	#endif
	}
	/**
	 * Exit thread method. This method is called everytime a thread exits.
	 * When the POSIX threads are being used, pthread_exit is called.
	 * This function does not return.
	 * @param handle The thread handle of the calling thread.
	 * @param rval The thread's return value. This can get picked up by joinThread.
	 */
	BLOCXX_COMMON_API void exitThread(Thread_t& handle, Int32 rval);
	/**
	 * @return The thread handle for the current running thread.
	 */
	inline Thread_t currentThread()
	{
#if defined(BLOCXX_WIN32)
		return GetCurrentThreadId();
#else
		return pthread_self();
#endif
	}


	/**
	 * Convert a Thread_t to an UInt64. This hides platform
	 * dependencies like that fact that on some platforms (e.g linux)
	 * Thread_t is integral type, and can therefor be static_cast<>ed to
	 * unsigned long, but on other platforms, like FreeBSD, Thread_t is
	 * void*, or something else, and reinterpret_cast<> (hopefully that
	 * works ...) must be used.
	 */
	BLOCXX_COMMON_API UInt64 thread_t_ToUInt64(Thread_t thr);

	/**
	 * Set a thread that was previously in the joinable state to a detached
	 * state. This will allow the threads resources to be released upon
	 * termination without being joined. A thread that is in the detached
	 * state can no longer be joined.
	 * @param handle		The thread to set to the detached state.
	 * @return 0 on success. Otherwise -1
	 */
	BLOCXX_COMMON_API int setThreadDetached(Thread_t& handle);
	/**
	 * Join a thread that has been previously set to joinable. It is
	 * Assumed that if the thread has already terminated, this method
	 * will still succeed and return immediately.
	 * @param handle	A handle to the thread to join with.
	 * @param rval An out parameter of the thread's return code.
	 * @return 0 on success. Otherwise -1
	 */
	BLOCXX_COMMON_API int joinThread(Thread_t& handle, Int32& rval);
	/**
	 * Voluntarily yield to the processor giving the next thread in the chain
	 * the opportunity to run.
	 */
	BLOCXX_COMMON_API void yield();
	/**
	 * Suspend execution of the current thread until the given number
	 * of milliSeconds have elapsed.
	 * @param milliSeconds	The number of milliseconds to suspend execution for.
	 */
	BLOCXX_COMMON_API void sleep(UInt32 milliSeconds);
	/**
	 * Suspend execution of the current thread until the given number
	 * of seconds have elapsed.
	 * @param seconds	The number of seconds to suspend execution for.
	 */
	BLOCXX_COMMON_API void sleep(const Timeout& timeout);
	/**
	 * Test if this thread has been cancelled.  If so, a
	 * ThreadCancelledException will be thrown.  DO NOT catch this exception.
	 * ThreadCancelledException is not derived from anything.
	 * Do not write code like this:
	 * try {
	 *  //...
	 * } catch (...) {
	 *  // swallow all exceptions
	 * }
	 *
	 * Instead do this:
	 * try {
	 *  //...
	 * } catch (ThreadCancelledException&) {
	 *  throw;
	 * } catch (std::exception& e) {
	 *  // handle the exception
	 * }
	 * The only place ThreadCancelledException should be caught is in
	 * Thread::threadRunner(). main() shouldn't need to catch it, as the main
	 * thread of an application should never be cancelled.  The main thread
	 * shouldn't need to ever call testCancel.
	 * Note that this method is staic, and it will check the the current running
	 * thread has been cacelled.  Thus, you can't call it on an object that doesn't
	 * represent the current running thread and expect it to work.
	 */
	BLOCXX_COMMON_API void testCancel();
	BLOCXX_COMMON_API void saveThreadInTLS(void* pTheThread);
	BLOCXX_COMMON_API void sendSignalToThread(Thread_t threadID, int signo);
	BLOCXX_COMMON_API void cancel(Thread_t threadID);
};

} // end namespace BLOCXX_NAMESPACE

#endif
