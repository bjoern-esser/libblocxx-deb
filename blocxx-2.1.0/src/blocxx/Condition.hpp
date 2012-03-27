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

#ifndef BLOCXX_CONDITION_HPP_INCLUDE_GUARD_
#define BLOCXX_CONDITION_HPP_INCLUDE_GUARD_
#include "blocxx/BLOCXX_config.h"
#include "blocxx/ThreadTypes.hpp"
#include "blocxx/Exception.hpp"
#include "blocxx/Types.hpp"
#include "blocxx/CommonFwd.hpp"

namespace BLOCXX_NAMESPACE
{

BLOCXX_DECLARE_APIEXCEPTION(ConditionLock, BLOCXX_COMMON_API);
BLOCXX_DECLARE_APIEXCEPTION(ConditionResource, BLOCXX_COMMON_API);

/**
 * The Condition class represents a synchronization device that allows threads
 * to suspend execution and relinquish the processors until another thread
 * signals the condition. The thread that signals the Condition object has
 * the option of only letting one waiting thread receive the notification,
 * or all threads.
 *
 * Note that descriptions of what exceptions may be thrown assumes that object
 * is used correctly, i.e. method preconditions are satisfied.
 */
class BLOCXX_COMMON_API Condition
{
public:
	/**
	 * Construct a new Condition object.
	 * @throw Exception if needed system resources exhausted.
	 */
	Condition();

	/**
	 * Destroy  this Condition object.
	 */
	~Condition();
	/**
	 * Signal one thread that is currently waiting on the Condition object
	 * through the wait or timedWait methods. All other threads waiting on
	 * this object through wait or timedWait will continue to block.
	 * @throw no exceptions
	 */
	void notifyOne();
	/**
	 * Signal all threads that are currently waiting on the Condition object.
	 * This will cause all threads waiting on a call to 'wait' or timedWait
	 * to stop blocking and continue execution.
	 * @throw no exceptions
	 */
	void notifyAll();
	/**
	 * Atomically unlock a given mutex and wait for the this Condition object
	 * to get signalled. The thread execution is suspended and does not
	 * consume any CPU time until the Condition object is notified (signalled).
	 * The mutex lock must  be locked  by the calling thread on entrance
	 * to wait. Before returning to the calling thread, wait re-acquires
	 * the mutex lock. This function should always be called within a while
	 * loop that checks the condition.
	 * @param lock The mutex lock object that must be acquired before calling
	 *		this method.
	 * @throw ThreadCancelledException
	 */
	void wait(NonRecursiveMutexLock& lock);

	/**
	 * Atomically unlock a given mutex and wait for a given amount of time
	 * for this Condition object to get signalled. The thread execution is
	 * suspended and does not consume any CPU time until the Condition 
	 * object is notified (signalled). The mutex lock must  be locked  
	 * by the calling thread on entrance to wait. Before returning to the 
	 * calling thread, wait re-acquires the mutex lock. This function should
	 * always be called within a while loop that checks the condition.
	 * @param lock The mutex lock object that must be acquired before calling
	 *		this method.
	 * @param timeout The time to wait for the condition to be signalled. 
	 *  Because of spurious wakeups, it is recommended that an absolute 
	 *  time timeout is used.
	 * @returns true if condition was signaled within the duration, 
	 *  false if timeout occurred.
	 * @throw ThreadCancelledException
	 */
	bool timedWait(NonRecursiveMutexLock& lock, const Timeout& timeout);

	/**
	 * Atomically unlock a given mutex and wait for a given amount of time
	 * for this Condition object to get signalled. The thread execution is
	 * suspended and does not consume any CPU time until the Condition 
	 * object is notified (signalled). The mutex lock must  be locked  
	 * by the calling thread on entrance to wait. Before returning to the 
	 * calling thread, wait re-acquires the mutex lock. This function should
	 * always be called within a while loop that checks the condition.
	 * @param lock The mutex lock object that must be acquired before calling
	 *		this method.
	 * @param sTimeout The number of seconds to wait for this Condition to
	 *		get signalled.
	 * @param usTimeout The number of micro seconds (1/1000000th) to wait for
	 *			this Condition to get signalled.
	 * The total wait time is sTimeout * 1000000 + usTimeout micro seconds.
	 * This function should always be called within a while loop that
	 * checks the condition.
	 *
	 * @returns true if the lock was acquired, false if timeout occurred.
	 * @throw ThreadCancelledException
	 */
	bool timedWait(NonRecursiveMutexLock& lock, UInt32 sTimeout, UInt32 usTimeout=0) BLOCXX_DEPRECATED;
private:
	// unimplemented
	Condition(const Condition&);
	Condition& operator=(const Condition&);
	void doWait(NonRecursiveMutex& mutex);
	bool doTimedWait(NonRecursiveMutex& mutex, const Timeout& timeout);
	ConditionVar_t m_condition;
};

} // end namespace BLOCXX_NAMESPACE

#endif
