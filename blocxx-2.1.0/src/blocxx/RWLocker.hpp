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
 * @author Jon Carey
 * @author Dan Nuffer
 */

#ifndef BLOCXX_RWLOCKER_HPP_INCLUDE_GUARD_
#define BLOCXX_RWLOCKER_HPP_INCLUDE_GUARD_
#include "blocxx/BLOCXX_config.h"
#include "blocxx/GenericRWLockImpl.hpp"

namespace BLOCXX_NAMESPACE
{

BLOCXX_DECLARE_APIEXCEPTION(RWLocker, BLOCXX_COMMON_API);
//////////////////////////////////////////////////////////////////////////////
// The locker is recursive and also supports upgrading a read-lock to a write lock
class BLOCXX_COMMON_API RWLocker
{
public:
	RWLocker();
	~RWLocker();
	
	/**
	 * @throws TimeoutException if the lock isn't acquired within the timeout.
	 */
	void getReadLock(const Timeout& timeout);
	void getReadLock(UInt32 sTimeout, UInt32 usTimeout=0) BLOCXX_DEPRECATED;
	
	/**
	 * @throws TimeoutException if the lock isn't acquired within the timeout.
	 * @throws DeadlockException if this call would upgrade a read lock to a write lock
	 *   and another thread is already waiting to upgrade. If this happens, the calling
	 *   thread must release it's read lock in order for forward progress to be made.
	 */
	void getWriteLock(const Timeout& timeout);
	void getWriteLock(UInt32 sTimeout, UInt32 usTimeout=0) BLOCXX_DEPRECATED;
	
	/**
	 * @throws RWLockerException if a read lock hasn't been acquired.
	 */
	void releaseReadLock();
	
	/**
	 * @throws RWLockerException if a write lock hasn't been acquired.
	 */
	void releaseWriteLock();

private:
	// Have to do this because on some platforms one thread may have different values for
	// a Thread_t, and ThreadImpl::sameThreads() has to be called to know if they refer
	// to the same one.
	struct ThreadComparer
	{
		bool operator()(Thread_t x, Thread_t y) const;
	};

	GenericRWLockImpl<Thread_t, ThreadComparer> m_impl;

	// unimplemented
	RWLocker(const RWLocker&);
	RWLocker& operator=(const RWLocker&);
};
//////////////////////////////////////////////////////////////////////////////
class BLOCXX_COMMON_API ReadLock
{
public:
	ReadLock(RWLocker& locker, const Timeout& timeout)
		: m_locker(&locker)
		, m_released(false)
	{
		m_locker->getReadLock(timeout);
	}
	ReadLock(RWLocker& locker, UInt32 sTimeout, UInt32 usTimeout=0) BLOCXX_DEPRECATED; // in 4.0.0
	~ReadLock()
	{
		release();
	}
	void lock(const Timeout& timeout)
	{
		if (m_released)
		{
			m_locker->getReadLock(timeout);
			m_released = false;
		}
	}
	BLOCXX_DEPRECATED void lock(UInt32 sTimeout, UInt32 usTimeout=0)
	{
		if (m_released)
		{
			m_locker->getReadLock(Timeout::relative(sTimeout + static_cast<float>(usTimeout) * 1000000.0));
			m_released = false;
		}
	}
	void release()
	{
		if (!m_released)
		{
			m_locker->releaseReadLock();
			m_released = true;
		}
	}
private:
	RWLocker* m_locker;
	bool m_released;
	// noncopyable
	ReadLock(const ReadLock&);
	ReadLock& operator=(const ReadLock&);
};

inline
ReadLock::ReadLock(RWLocker& locker, UInt32 sTimeout, UInt32 usTimeout)
	: m_locker(&locker)
	, m_released(false)
{
	m_locker->getReadLock(Timeout::relative(sTimeout + static_cast<float>(usTimeout) * 1000000.0));
} 
//////////////////////////////////////////////////////////////////////////////
class BLOCXX_COMMON_API WriteLock
{
public:
	WriteLock(RWLocker& locker, const Timeout& timeout)
		: m_locker(&locker)
		, m_released(false)
	{
		m_locker->getWriteLock(timeout);
	}
	WriteLock(RWLocker& locker, UInt32 sTimeout, UInt32 usTimeout=0) BLOCXX_DEPRECATED; // in 4.0.0
	~WriteLock()
	{
		release();
	}
	void lock(const Timeout& timeout)
	{
		if (m_released)
		{
			m_locker->getWriteLock(timeout);
			m_released = false;
		}
	}
	BLOCXX_DEPRECATED void lock(UInt32 sTimeout, UInt32 usTimeout=0) // in 4.0.0
	{
		if (m_released)
		{
			m_locker->getWriteLock(Timeout::relative(sTimeout + static_cast<float>(usTimeout) * 1000000.0));
			m_released = false;
		}
	}
	void release()
	{
		if (!m_released)
		{
			m_locker->releaseWriteLock();
			m_released = true;
		}
	}
private:
	RWLocker* m_locker;
	bool m_released;
	
	// noncopyable
	WriteLock(const WriteLock&);
	WriteLock& operator=(const WriteLock&);
};

inline
WriteLock::WriteLock(RWLocker& locker, UInt32 sTimeout, UInt32 usTimeout)
	: m_locker(&locker)
	, m_released(false)
{
	m_locker->getWriteLock(Timeout::relative(sTimeout + static_cast<float>(usTimeout) * 1000000.0));
}

} // end namespace BLOCXX_NAMESPACE

#endif
