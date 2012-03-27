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

#ifndef BLOCXX_NON_RECURSIVE_MUTEX_INCLUDE_GUARD_HPP_
#define BLOCXX_NON_RECURSIVE_MUTEX_INCLUDE_GUARD_HPP_
#include "blocxx/BLOCXX_config.h"
#include "blocxx/ThreadTypes.hpp"
#include "blocxx/Exception.hpp"

namespace BLOCXX_NAMESPACE
{

BLOCXX_DECLARE_APIEXCEPTION(Deadlock, BLOCXX_COMMON_API)
	/**
	* Note that descriptions of what exceptions may be thrown assumes that
	* object is used correctly, i.e., method preconditions are satisfied.
	*/
class BLOCXX_COMMON_API NonRecursiveMutex
{
public:
	/**
	 * Create a new NonRecursiveMutex object.
	 * @throw Exception if needed system resources exhausted
	 */
	NonRecursiveMutex();
	/**
	 * Destroy this NonRecursiveMutex object.
	 */
	~NonRecursiveMutex();
	/**
	 * Acquire ownership of this NonRecursiveMutex object.
	 * This call will block if another thread has
	 * ownership of this NonRecursiveMutex. When it returns, the current thread will be
	 * the owner of this NonRecursiveMutex object.
	 * If this thread is the owner of the mutex, then an Deadlock
	 * exception will be thrown.
	 * @pre Mutex not currently locked by this thread.
	 * @throw no exception
	 */
	void acquire();
	/**
	 * Release ownership of this NonRecursiveMutex object. If another thread is waiting
	 * to acquire the ownership of this mutex it will stop blocking and acquire
	 * ownership when this call returns.
	 * @pre Mutex currently locked by this thread.
	 * @throw no exception
	 */
	bool release();
private:
	NonRecursiveMutex_t m_mutex;
	// noncopyable
	NonRecursiveMutex(const NonRecursiveMutex&) BLOCXX_FUNCTION_NOT_IMPLEMENTED;
	NonRecursiveMutex& operator = (const NonRecursiveMutex&) BLOCXX_FUNCTION_NOT_IMPLEMENTED;
	friend class Condition;
	/** @throw no exception */
	void conditionPreWait(NonRecursiveMutexLockState& state);
	/** @throw no exception */
	void conditionPostWait(NonRecursiveMutexLockState& state);
};

} // end namespace BLOCXX_NAMESPACE

#endif
