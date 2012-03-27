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
 * @author Dan Nuffer
 */

#ifndef BLOCXX_THREAD_COUNTER_HPP_INCLUDE_GUARD_
#define BLOCXX_THREAD_COUNTER_HPP_INCLUDE_GUARD_
#include "blocxx/BLOCXX_config.h"
#include "blocxx/Types.hpp"
#include "blocxx/NonRecursiveMutex.hpp"
#include "blocxx/Condition.hpp"
#include "blocxx/IntrusiveReference.hpp"
#include "blocxx/IntrusiveCountableBase.hpp"
#include "blocxx/ThreadDoneCallback.hpp"

namespace BLOCXX_NAMESPACE
{

// Note: Do not inline any functions in these classes, the code must
// be contained in the main library, if a loadable library contains any,
// it will cause a race-condition that may segfault the application.
class BLOCXX_COMMON_API ThreadCounter : public IntrusiveCountableBase
{
public:
	ThreadCounter(Int32 maxThreads);
	~ThreadCounter();
	// Throws TimeoutException in case of timeout
	void incThreadCount(const Timeout& timeout);
	void decThreadCount();
	Int32 getThreadCount();
	// Throws TimeoutException in case of timeout
	void waitForAll(const Timeout& timeout);
	void setMax(Int32 maxThreads);
private:
	Int32 m_maxThreads;
	Int32 m_runCount;
	NonRecursiveMutex m_runCountGuard;
	Condition m_runCountCondition;
	// noncopyable
	ThreadCounter(ThreadCounter const&) BLOCXX_FUNCTION_NOT_IMPLEMENTED;
	ThreadCounter& operator=(ThreadCounter const&) BLOCXX_FUNCTION_NOT_IMPLEMENTED;
};

class BLOCXX_COMMON_API ThreadCountDecrementer : public ThreadDoneCallback
{
public:
	ThreadCountDecrementer(ThreadCounterRef const& x);
	virtual ~ThreadCountDecrementer();
private:
	virtual void doNotifyThreadDone(Thread *);

#ifdef BLOCXX_WIN32
#pragma warning (push)
#pragma warning (disable: 4251)
#endif

	ThreadCounterRef m_counter;

#ifdef BLOCXX_WIN32
#pragma warning (pop)
#endif

	// noncopyable
	ThreadCountDecrementer(ThreadCountDecrementer const&) BLOCXX_FUNCTION_NOT_IMPLEMENTED;
	ThreadCountDecrementer& operator=(ThreadCountDecrementer const&) BLOCXX_FUNCTION_NOT_IMPLEMENTED;
};
BLOCXX_EXPORT_TEMPLATE(BLOCXX_COMMON_API, IntrusiveReference, ThreadCounter);

} // end namespace BLOCXX_NAMESPACE

#endif
