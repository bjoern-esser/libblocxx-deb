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
*************************/

/**
 * @author Kevin S. Van Horn
 */

#include "blocxx/BLOCXX_config.h"
#include "MTQueue.hpp"
#include <limits>

using namespace blocxx;

MTQueueBase::MTQueueBase()
	: m_maxQueueSize(std::numeric_limits<UInt32>::max())
	, m_shutdown(false)
	, m_pushesBlocked(0)
	, m_popsBlocked(0)
{
}

MTQueueBase::MTQueueBase(UInt32 maxQueueSize)
	: m_maxQueueSize(maxQueueSize)
	, m_shutdown(false)
	, m_pushesBlocked(0)
	, m_popsBlocked(0)
{
}

MTQueueBase::~MTQueueBase()
{
}

void MTQueueBase::setMaxQueueSize(UInt32 maxQueueSize)
{
	m_maxQueueSize = maxQueueSize;
}

// Nothrow
void MTQueueBase::shutdown()
{
	NonRecursiveMutexLock lock(m_mutex);
	m_shutdown = true;
	m_cond.notifyAll();
}

void MTQueueBase::enterPushBack(NonRecursiveMutexLock & lock)
{
	while (!m_shutdown && count() >= m_maxQueueSize)
	{
		++m_pushesBlocked;
		m_cond.timedWait(lock, Timeout::infinite);
		--m_pushesBlocked;
	}
	if (m_popsBlocked > 0)
	{
		m_cond.notifyOne();
	}
}

MTQueueBase::EPopResult MTQueueBase::enterPopFront(
	NonRecursiveMutexLock & lock, Timeout const & timeout)
{
	while (!m_shutdown && count() == 0)
	{
		++m_popsBlocked;
		bool signalled = m_cond.timedWait(lock, timeout);
		--m_popsBlocked;
		if (!signalled)
		{
			return MTQueueEnum::E_TIMED_OUT;
		}
	}
	if (m_pushesBlocked > 0)
	{
		m_cond.notifyOne();
	}
	return m_shutdown ? MTQueueEnum::E_SHUT_DOWN : MTQueueEnum::E_VALUE;
}
