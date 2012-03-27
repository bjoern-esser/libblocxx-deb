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

#ifndef BLOCXX_SEMAPHORE_HPP_INCLUDE_GUARD_
#define BLOCXX_SEMAPHORE_HPP_INCLUDE_GUARD_
#include "blocxx/BLOCXX_config.h"
#include "blocxx/Types.hpp"
#include "blocxx/NonRecursiveMutex.hpp"
#include "blocxx/Condition.hpp"
#include "blocxx/NonRecursiveMutexLock.hpp"
#include "blocxx/Timeout.hpp"
#include "blocxx/TimeoutTimer.hpp"

namespace BLOCXX_NAMESPACE
{
// TODO: uninline this class.
class Semaphore
{
public:
	Semaphore()
		: m_curCount(0)
	{}
	Semaphore(Int32 initCount)
		: m_curCount(initCount)
	{}
	void wait()
	{
		NonRecursiveMutexLock l(m_mutex);
		while (m_curCount <= 0)
		{
			m_cond.wait(l);
		}
		--m_curCount;
	}
	BLOCXX_DEPRECATED bool timedWait(UInt32 sTimeout, UInt32 usTimeout=0) // in 4.0.0
	{
		return timedWait(Timeout::relative(sTimeout + static_cast<float>(usTimeout) * 1000000.0));
	}
	bool timedWait(const Timeout& timeout)
	{
		bool ret = true;
		NonRecursiveMutexLock l(m_mutex);
		TimeoutTimer timer(timeout);
		while (m_curCount <= 0 && ret == true)
		{
			ret = m_cond.timedWait(l, timer.asAbsoluteTimeout());
		}
		if (ret == true)
		{
			--m_curCount;
		}
		return ret;
	}
	void signal()
	{
		NonRecursiveMutexLock l(m_mutex);
		++m_curCount;
		m_cond.notifyAll();
	}
	Int32 getCount()
	{
		NonRecursiveMutexLock l(m_mutex);
		return m_curCount;
	}
private:
	Int32 m_curCount;
	Condition m_cond;
	NonRecursiveMutex m_mutex;
	// noncopyable
	Semaphore(const Semaphore&);
	Semaphore& operator=(const Semaphore&);
};

} // end namespace BLOCXX_NAMESPACE

#endif
