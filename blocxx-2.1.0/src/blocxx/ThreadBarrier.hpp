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

#ifndef BLOCXX_THREAD_BARRIER_HPP_INCLUDE_GUARD_
#define BLOCXX_THREAD_BARRIER_HPP_INCLUDE_GUARD_
#include "blocxx/BLOCXX_config.h"
#include "blocxx/Types.hpp"
#include "blocxx/IntrusiveReference.hpp"
#include "blocxx/Exception.hpp"

namespace BLOCXX_NAMESPACE
{

class ThreadBarrierImpl;
BLOCXX_DECLARE_APIEXCEPTION(ThreadBarrier, BLOCXX_COMMON_API);
/**
 * The ThreadBarrier class is used to synchronize threads.  Each thread that calls
 * wait() will block until <i>threshold</i> number of threads has called wait()
 * This class is freely copyable.  All copies reference the same underlying
 * implementation.
 *
 * Note that descriptions of what exceptions may be thrown assumes that the
 * object is used correctly, i.e., method preconditions are satsified.
 */
class BLOCXX_COMMON_API ThreadBarrier
{
public:
	/**
	 * Constructor
	 * @param threshold The number of threads that must call wait() before any of them successfully 
	 * return from the call. The value specified by threshold must be greater than zero.
	 * @throw ThreadBarrierException if needed system resources exhausted.
	 * @throw std::bad_alloc if memory exhausted.
	 */
	ThreadBarrier(UInt32 threshold);
	/**
	 * Synchronize participating threads at the barrier. The calling thread shall block until the 
	 * required number of threads have called wait().
	 * @throw ThreadBarrierException (programmer error only)
	 * @throw ThreadCancelledException
	 */
	void wait();
	~ThreadBarrier();
	ThreadBarrier(const ThreadBarrier& x);
	ThreadBarrier& operator=(const ThreadBarrier& x);
private:

#ifdef BLOCXX_WIN32
#pragma warning (push)
#pragma warning (disable: 4251)
#endif

	IntrusiveReference<ThreadBarrierImpl> m_impl;

#ifdef BLOCXX_WIN32
#pragma warning (pop)
#endif

};

} // end namespace BLOCXX_NAMESPACE

#endif
