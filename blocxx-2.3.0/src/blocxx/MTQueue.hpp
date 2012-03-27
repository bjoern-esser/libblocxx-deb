/*******************************************************************************
* Copyright (C) 2008 Quest Software, Inc. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of source code must retain the above copyright notice,
*    this list of conditions and the following disclaimer.
*
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
*
*  - Neither the name of Quest Software, Inc. nor the names of its
*    contributors may be used to endorse or promote products derived from this
*    software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL Vintela, Inc. OR THE CONTRIBUTORS
* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/
#ifndef BLOCXX_MTQUEUE_HPP_INCLUDE_GUARD
#define BLOCXX_MTQUEUE_HPP_INCLUDE_GUARD

/**
 * @author Kevin S. Van Horn
 * @author Dan Nuffer
 */

#include "blocxx/BLOCXX_config.h"
#include "blocxx/Condition.hpp"
#include "blocxx/NonRecursiveMutex.hpp"
#include "blocxx/NonRecursiveMutexLock.hpp"
#include "blocxx/Timeout.hpp"
#include "blocxx/Types.hpp"
#include <deque>

namespace BLOCXX_NAMESPACE
{

namespace MTQueueEnum
{
	enum EPopResult
	{
		E_TIMED_OUT, E_SHUT_DOWN, E_VALUE
	};
}

class MTQueueBase
{
public:
	typedef MTQueueEnum::EPopResult EPopResult;

	MTQueueBase();
	MTQueueBase(UInt32 maxQueueSize);
	virtual ~MTQueueBase();

	/**
	 * Set the maximum number of items the queue can hold before pushBack() blocks.
	 * This function is not thread-safe and must be called before pushBack() or popFront() are called.
	 *
	 * @param maxQueueSize
	 */
	void setMaxQueueSize(UInt32 maxQueueSize);

	/**
	 * Shut down the queue. Any threads blocked in popFront() will be woken, and
	 * E_SHUT_DOWN will be returned from popFront(). Subsequent calls to pushBack() will
	 * not insert items in the queue.
	 */
	void shutdown();

protected:
	virtual std::size_t count() = 0;

	void enterPushBack(NonRecursiveMutexLock & lock);

	EPopResult enterPopFront(
		NonRecursiveMutexLock & lock, Timeout const & timeout);

	NonRecursiveMutex m_mutex;
	Condition m_cond;
	UInt32 m_maxQueueSize;
	bool m_shutdown;
	unsigned m_pushesBlocked;
	unsigned m_popsBlocked;
};

template <typename T>
struct MTSourceIfc
{
	typedef MTQueueEnum::EPopResult EPopResult;
	virtual ~MTSourceIfc() { }

	virtual EPopResult popFront(Timeout const & timeout, T & value) = 0;
};

/**
 * MTQueue is a generic class that wraps a queue in a
 * thread-safe wrapper.
 * @param T type of item held in the queue.
 */
template <typename T>
class MTQueue : private MTQueueBase, public MTSourceIfc<T>
{
public:
	typedef MTQueueEnum::EPopResult EPopResult;
	// See MTQueueBase for documentation of these 2 functions.
	using MTQueueBase::setMaxQueueSize;
	using MTQueueBase::shutdown;

	/**
	 * A MTQueue instance created with this constructor will have the max
	 * queue size of std::numeric_limits<UInt32>::max()
	 */
	MTQueue()
	{
	}

	/**
	 * Constructor
	 *
	 * @param maxQueueSize When this many items are in the queue, calls to pushBack() will block.
	 */
	MTQueue(UInt32 maxQueueSize)
		: MTQueueBase(maxQueueSize)
	{
	}

	/**
	 * Add val to the back of the queue. If another thread is waiting
	 * in pop(), it will be woken up.
	 * No-op if shutdown() has been called.
	 * @param val Item to add to the back of the queue.
	 */
	void pushBack(T const & val)
	{
		NonRecursiveMutexLock lock(m_mutex);
		enterPushBack(lock);
		if (!m_shutdown)
		{
			m_deque.push_back(val);
		}
	}

	/**
	 * If the queue is not empty, remove one element from the front
	 * of the queue, otherwise wait until timeout for pushBack() to be
	 * called. If timeout has passed, then return E_TIMED_OUT. If an
	 * item is available in the queue it will be assigned to value,
	 * and E_VALUE will be returned. If shutdown() is called while
	 * waiting, E_SHUT_DOWN will be returned.
	 */
	virtual EPopResult popFront(Timeout const & timeout, T & value)
	{
		NonRecursiveMutexLock lock(m_mutex);
		EPopResult rv = enterPopFront(lock, timeout);
		if (rv == MTQueueEnum::E_VALUE)
		{
			value = m_deque.front();
			m_deque.pop_front();
		}
		return rv;
	}

private:
	// non-copyable
	MTQueue(const MTQueue&) BLOCXX_FUNCTION_NOT_IMPLEMENTED;
	MTQueue& operator=(const MTQueue&) BLOCXX_FUNCTION_NOT_IMPLEMENTED;

	virtual std::size_t count()
	{
		return m_deque.size();
	}

	std::deque<T> m_deque;
};

}

#endif
