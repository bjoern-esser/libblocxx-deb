/*******************************************************************************
* Copyright (C) 2005, 2009-2010, Quest Software, Inc. All rights reserved.
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

#ifndef BLOCXX_THREAD_HPP_INCLUDE_GUARD_
#define BLOCXX_THREAD_HPP_INCLUDE_GUARD_
#include "blocxx/BLOCXX_config.h"
#include "blocxx/Exception.hpp"
#include "blocxx/ThreadImpl.hpp"
#include "blocxx/IntrusiveReference.hpp"
#include "blocxx/IntrusiveCountableBase.hpp"
#include "blocxx/Condition.hpp"
#include "blocxx/NonRecursiveMutex.hpp"
#include "blocxx/ThreadDoneCallback.hpp"
#include "blocxx/ThreadCancelledException.hpp"
#include "blocxx/Timeout.hpp"
#include "blocxx/AtomicOps.hpp"

namespace BLOCXX_NAMESPACE
{

//////////////////////////////////////////////////////////////////////////////
BLOCXX_DECLARE_APIEXCEPTION(CancellationDenied, BLOCXX_COMMON_API);
BLOCXX_DECLARE_APIEXCEPTION(Thread, BLOCXX_COMMON_API);
//////////////////////////////////////////////////////////////////////////////
	/**
	* Descriptions of exceptions thrown assume that the object is used
	* correctly, i.e., method preconditions are satisfied.
	*/
class BLOCXX_COMMON_API Thread : public IntrusiveCountableBase
{
public:
	/**
	 * Create a new Thread object.
	 * @throw Exception if needed system resources exhausted
	 */
	Thread();
	/**
	 * Destroy this Thread object.  The destructor will call join() if it
	 * hasn't been previously called.
	 * This function won't return until the thread has exited.
	 */
	virtual ~Thread();
	/**
	 * Start this Thread's execution.
	 * @pre start() not previously called on this object.
	 * @throw std::bad_alloc if memory exhausted.
	 * @throw Exception if needed system resources exhausted.
	 * @throw ThreadCancelledException
	 * @exception ThreadException (programmer error only)
	 */
	virtual void start(const ThreadDoneCallbackRef& cb = ThreadDoneCallbackRef(0));

	/**
	 * Call the thread's doShutdown(), which may be used by the thread to safely
	 * stop.
	 * @throw whatever doShutdown() throws.
	 */
	void shutdown();

	/**
	 * Call the thread's doShutdown(), which may be used by the thread to safely stop,
	 * and then wait until the thread has exited or timeout has expired.
	 * @return true if the thread is finished, false if the timeout expired.
	 * @throw whatever doShutdown() throws.
	 * @throw ThreadCancelledException.
	 */
	bool shutdown(const Timeout& timeout);

	/**
	 * Attempt to cooperatively cancel this Threads execution.
	 * You should still call join() in order to clean up resources allocated
	 * for this thread.
	 * This function will set a flag that the thread has been
	 * cancelled, which can be checked by testCancel().  If the thread does
	 * not call testCancel(), it may keep running.
	 * The thread may (probably) still be running after this function returns,
	 * and it will exit as soon as it calls testCancel().
	 *
	 * Note that a thread may not be able to utilize doCooperativeCancel()
	 * in order to cleanly stop if it uses any blocking BloCxx APIs.
	 * Many of them call Thread::testCancel(), which will cause a
	 * ThreadCancelledException to be thrown, thus interrupting the thread's
	 * cleanup process.
	 * The supported mechanism for cleanly stopping a thread is for the thread to
	 * implement doShutdown().
	 *
	 * It is also possible for an individual thread to override the cancellation
	 * request, if it knows that cancellation at this time may crash the system
	 * or cause a deadlock.  If this happens, a CancellationDeniedException
	 * will be thrown.
	 *
	 * @exception CancellationDeniedException if doCooperativeCancel throws
	 * it to indicate that the thread cannot be safely cancelled at this time.
	 * @throw whatever doCooperativeCancel() throws.
	 */
	void cooperativeCancel();

	/**
	 * Attempt to cooperatively and then definitively cancel this Thread's
	 * execution.  You should still call join() in order to clean up resources
	 * allocated for this thread.
	 * This function will set a flag that the thread has been
	 * cancelled, which can be checked by testCancel().
	 * definitiveCancel() wil first try to stop the thread in a cooperative
	 * manner to avoid leaks or corruption. If the thread has not
	 * exited after waitForCoopeartiveSecs seconds, it will be cancelled.
	 * Note that when using this function, any objects on the thread's stack will
	 * not be cleaned up unless the operating system and compiler support stack
	 * unwinding on thread cancellation.  As such, it may cause memory leaks or
	 * inconsistent state or even memory corruption.
	 * Also note that this still may not stop the thread, since a thread can
	 * make itself non-cancellable, or it may not ever call any cancellation
	 * points.
	 * By default all Thread objects are asynchronously cancellable, and
	 * so may be immediately cancelled.
	 * The thread may (unlikely) still be running after this function returns.
	 *
	 * It is also possible for an individual thread to override the cancellation
	 * request, if it knows that cancellation at this time may crash the system
	 * or cause a deadlock.  If this happens, an CancellationDeniedException
	 * will be thrown.
	 *
	 * @param timeout The time to wait for cooperative cancellation to succeed
	 * before attempting to forcibly cancel the thread.
	 *
	 * @return true if the thread exited cleanly.  false if the thread was
	 * forcibly cancelled.
	 *
	 * @exception CancellationDeniedException if doCooperativeCancel throws
	 * this to indicate that the thread cannot be safely cancelled at this time.
	 * @throw whatever doCooperativeCancel throws
	 * @throw std::bad_alloc if memory exhausted
	 * @throw Exception if needed system resources exhausted
	 * @throw ThreadCancelledException
	 */
	bool definitiveCancel(const Timeout& timeout = Timeout::relative(60));
	/**
	 * Definitively cancel this Threads execution. The thread is *NOT*
	 * given a chance to clean up or override the cancellation.
	 * DO NOT call this function without first trying definitiveCancel().
	 *
	 * You should still call join() in order to clean up resources
	 * allocated for this thread.
	 *
	 * Note that when using this function, any objects on the thread's stack will
	 * not be cleaned up unless the operating system and compiler support stack
	 * unwinding on thread cancellation.  As such, it may cause memory leaks or
	 * inconsistent state or even memory corruption.
	 * Also note that this still may not stop the thread, since a thread can
	 * make itself non-cancellable, or it may not ever call any cancellation
	 * points.
	 * By default all Thread objects are asynchronously cancellable, and
	 * so may be immediately cancelled.
	 * The thread may (unlikely) still be running after this function returns.
	 *
	 * @throw Exception if needed system resources exhausted.
	 */
	void cancel();
	/**
	 * Test if this thread has been cancelled.  If so, a
	 * ThreadCancelledException will be thrown.  DO NOT catch this exception.
	 * ThreadCancelledException is not derived from anything.
	 * Except for in destructors, do not write code like this:
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
	 * Thread::threadRunner() or a destructor. main() shouldn't need to catch it, as the main
	 * thread of an application should never be cancelled.  The main thread
	 * shouldn't need to ever call testCancel.
	 * Note that this method is staic, and it will check if the current running
	 * thread has been cancelled.  Thus, you can't call it on an object that doesn't
	 * represent the current running thread and expect it to work.
	 *
	 * @throw ThreadCancelledException
	 */
	static void testCancel();
private:

	void cancel_internal(bool is_locked);

	/**
	 * This function is available for subclasses of Thread to override if they
	 * wish to be notified when shutdown() is invoked on the instance.
	 * This function will be invoked in a separate thread.
	 * For instance, a thread may use this function to:
	 * 1. Set a flag and then signal a condition variable to wake up the thread.
	 * 2. Write to a pipe or socket, if Thread::run() is blocked in select(),
	 * it can be unblocked and then exit.
	 */
	virtual void doShutdown();

	/**
	 * This function is available for subclasses of Thread to override if they
	 * wish to be notified when a cooperative cancel is being invoked on the
	 * instance.  Note that this function will be invoked in a separate thread.
	 *
	 * It is also possible for an individual thread to override the cancellation
	 * request, if it knows that cancellation at this time may crash the system
	 * or cause a deadlock.  To do this, the thread should throw an
	 * CancellationDeniedException.  Note that threads are usually only
	 * cancelled in the event of a system shutdown or restart, so a thread
	 * should make a best effort to actually shutdown.
	 *
	 * @throws CancellationDeniedException
	 */
	virtual void doCooperativeCancel();

	/**
	 * See the documentation for doCooperativeCancel().  When definitiveCancel()
	 * is called on a thread, first doCooperativeCancel() will be called, and
	 * then doDefinitiveCancel() will be called.
	 *
	 * @throws CancellationDeniedException
	 */
	virtual void doDefinitiveCancel();
public:
	/**
	 * @return true if this thread is currently running. Otherwise false.
	 */
	bool isRunning()
	{
		return m_isRunning == true;
	}
	/**
	 * Wait for the thread to finish. If the return value is true, join()
	 * can be called without blocking indefinitely.
	 *
	 * @param timeout How long to wait
	 * @return true if the thread is finished, false if the timeout expired.
	 * @throw ThreadCancelledException
	 */
	bool timedWait(const Timeout& timeout);
	/**
	 * Join with this Thread's execution. This method should be called
	 * on all joinable threads. The destructor will call it as well.
	 * If this Thread object is executing, this method
	 * will block until this Thread's run method returns.
	 * join() should not be called until after start() has returned.  It may
	 * be called by a different thread.
	 *
	 * @pre start() called on this object.
	 * @exception ThreadException (programmer error only)
	 * @return The return value from the thread's run()
	 */
	Int32 join();
	/**
	 * Get this Thread object's id.
	 *
	 * This function cannot be called on a non-joinable self-deleting thread
	 * after it has started.
	 *
	 * @return The id of this Thread if it is currently running. Otherwise
	 * return a NULL thread id.
	 * @throw no exception
	 */
	Thread_t getId()
	{
		return m_id;
	}
	/**
	 * Suspend execution of the current thread until the given number
	 * of milliSeconds have elapsed.
	 * @param milliSeconds	The number of milliseconds to suspend execution for.
	 * @throw ThreadCancelledException
	 * @throw Exception if needed system resources exhausted
	 */
	static void sleep(UInt32 milliSeconds)
	{
		ThreadImpl::sleep(milliSeconds);
	}
	/**
	 * Suspend execution of the current thread until the given number
	 * of seconds have elapsed.
	 * @param seconds The number of seconds to suspend execution for.
	 * @throw ThreadCancelledException
	 * @throw Exception if needed system resources exhausted
	 */
	static void sleep(const Timeout& timeout)
	{
		ThreadImpl::sleep(timeout);
	}
	/**
	 * Voluntarily yield to the processor giving the next thread in the chain
	 * the opportunity to run.
	 * @throw ThreadCancelledException
	 * @throw Exception if needed system resources exhausted
	 */
	static void yield()
	{
		ThreadImpl::yield();
	}
private:
	/**
	 * The method that will be run when the start method is called.
	 */
	virtual Int32 run() = 0;
	// thread state
	Thread_t m_id;
	bool m_isRunning;
	bool m_joined;
	// used to implement cancellation.
	friend void ThreadImpl::testCancel();
	Atomic_t m_cancelRequested;
	bool m_cancelled;

	NonRecursiveMutex m_stateGuard;
	Condition m_stateCond;

	static Int32 threadRunner(void* paramPtr);
	void doneRunning(const ThreadDoneCallbackRef& cb);

	// non-copyable
	Thread(const Thread&) BLOCXX_FUNCTION_NOT_IMPLEMENTED;
	Thread& operator=(const Thread&) BLOCXX_FUNCTION_NOT_IMPLEMENTED;

};
BLOCXX_EXPORT_TEMPLATE(BLOCXX_COMMON_API, IntrusiveReference, Thread);

} // end namespace BLOCXX_NAMESPACE

#endif

