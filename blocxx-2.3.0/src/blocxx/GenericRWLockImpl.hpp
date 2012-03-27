/*******************************************************************************
* Copyright (C) 2007, Quest Software All rights reserved.
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
*       Quest Software,
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

#ifndef BLOCXX_GENERIC_RWLOCK_IMPL_HPP_INCLUDE_GUARD_
#define BLOCXX_GENERIC_RWLOCK_IMPL_HPP_INCLUDE_GUARD_
#include "blocxx/BLOCXX_config.h"
#include "blocxx/CommonFwd.hpp"
#include "blocxx/NonRecursiveMutexLock.hpp"
#include "blocxx/Condition.hpp"
#include "blocxx/Exception.hpp"
#include "blocxx/TimeoutException.hpp"
#include "blocxx/Timeout.hpp"
#include "blocxx/TimeoutTimer.hpp"
#include "blocxx/Assertion.hpp"

#include <map>

namespace BLOCXX_NAMESPACE
{

BLOCXX_DECLARE_APIEXCEPTION(GenericRWLockImpl, BLOCXX_COMMON_API);
/**
 * This class is the implementation of the read/write lock. It isn't thread specific and thus may be used for other
 * types of locking, such as for transactions which may span threads, in which case the transaction would be the id. The
 * read/write lock is recursive and also supports upgrading a read-lock to a write lock.
 */
template <typename IdT, typename CompareT>
class GenericRWLockImpl
{
public:
	GenericRWLockImpl();
	~GenericRWLockImpl();

	/**
	 * @throws TimeoutException if the lock isn't acquired within the timeout.
	 */
	void acquireReadLock(const IdT id, const Timeout& timeout);

	/**
	 * @throws TimeoutException if the lock isn't acquired within the timeout.
	 * @throws DeadlockException if this call would upgrade a read lock to a write lock
     *   and another id is already waiting to upgrade. If this happens, the calling
	 *   thread must release it's read lock in order for forward progress to be made.
	 */
	void acquireWriteLock(const IdT id, const Timeout& timeout);

	/**
	 * @throws GenericRWLockImplException if a read lock hasn't been acquired.
	 */
	void releaseReadLock(const IdT id);

	/**
	 * @throws GenericRWLockImplException if a write lock hasn't been acquired.
	 */
	void releaseWriteLock(const IdT id);

private:

	Condition   m_waiting_writers;

	bool m_canRead;
	Condition   m_waiting_readers;

	NonRecursiveMutex	m_guard;
	unsigned m_numReaders;
	unsigned m_numWriters; // current writer + upgrading writer

	struct LockerInfo
	{
		unsigned int readCount;
		unsigned int writeCount;

		bool isReader() const
		{
			return readCount > 0;
		}

		bool isWriter() const
		{
			return writeCount > 0;
		}
	};

	typedef std::map<IdT, LockerInfo, CompareT> IdMap;
	IdMap m_lockerInfo;

	// unimplemented
	GenericRWLockImpl(const GenericRWLockImpl&);
	GenericRWLockImpl& operator=(const GenericRWLockImpl&);
};

//////////////////////////////////////////////////////////////////////////////
template <typename IdT, typename CompareT>
GenericRWLockImpl<IdT, CompareT>::GenericRWLockImpl()
	: m_canRead(true)
	, m_numReaders(0)
	, m_numWriters(0)
{
}
//////////////////////////////////////////////////////////////////////////////
template <typename IdT, typename CompareT>
GenericRWLockImpl<IdT, CompareT>::~GenericRWLockImpl()
{
}
//////////////////////////////////////////////////////////////////////////////
template <typename IdT, typename CompareT>
void
GenericRWLockImpl<IdT, CompareT>::acquireReadLock(const IdT id, const Timeout& timeout)
{
	TimeoutTimer timer(timeout);

	NonRecursiveMutexLock l(m_guard);
	typename IdMap::iterator info = m_lockerInfo.find(id);

	if (info != m_lockerInfo.end())
	{
		LockerInfo& ti(info->second);
		// id already have a read or write lock, so just increment.
		BLOCXX_ASSERT(ti.isReader() || ti.isWriter());
		++ti.readCount;
		return;
	}

	// id is a new reader
	while (!m_canRead || m_numWriters > 0)
	{
		if (!m_waiting_readers.timedWait(l, timer.asAbsoluteTimeout()))
		{
			BLOCXX_THROW(TimeoutException, "Timeout while waiting for read lock.");
		}
	}

	// Increase the reader count
	LockerInfo lockerInfo;
	lockerInfo.readCount = 1;
	lockerInfo.writeCount = 0;
	m_lockerInfo.insert(typename IdMap::value_type(id, lockerInfo));

	++m_numReaders;
}

//////////////////////////////////////////////////////////////////////////////
template <typename IdT, typename CompareT>
void
GenericRWLockImpl<IdT, CompareT>::releaseReadLock(const IdT id)
{
	NonRecursiveMutexLock l(m_guard);

	typename IdMap::iterator pInfo = m_lockerInfo.find(id);

	if (pInfo == m_lockerInfo.end() || !pInfo->second.isReader())
	{
		BLOCXX_THROW(GenericRWLockImplException, "Cannot release a read lock when no read lock is held");
	}

	LockerInfo& info(pInfo->second);
	--info.readCount;

	if (!info.isWriter() && !info.isReader())
	{
		--m_numReaders;
		if (m_numReaders == 0)
		{
			// This needs to wake them all up. In the case where one thread is waiting to upgrade a read to a write lock
			// and others are waiting to get a write lock, we have to wake up the thread trying to upgrade.
			m_waiting_writers.notifyAll();
		}
		m_lockerInfo.erase(pInfo);
	}
}

//////////////////////////////////////////////////////////////////////////////
template <typename IdT, typename CompareT>
void
GenericRWLockImpl<IdT, CompareT>::acquireWriteLock(const IdT id, const Timeout& timeout)
{
	// 7 cases:
	// 1. No id has the lock
	//   Get the lock
	// 2. This id has the write lock
	//   Increment the lock count
	// 3. Another id has the write lock & other ids may be waiting for read and/or write locks.
	//   Block until the lock is acquired.
	// 4. Only this id has a read lock
	//   Increment the write lock count .
	// 5. >0 other ids have the read lock & other ids may be waiting for write locks.
	//   Block until the write lock is acquired.
	// 6. This id and other ids have the read lock
	//   Block new readers and writers and wait until existing readers finish.
	// 7. This id and other ids have the read lock and one of the other ids has requested a write lock.
	//   Throw an exception.

	TimeoutTimer timer(timeout);

	NonRecursiveMutexLock l(m_guard);

	typename IdMap::iterator pInfo = m_lockerInfo.find(id);
	if (pInfo != m_lockerInfo.end())
	{
		// This id already has some sort of lock
		LockerInfo& ti(pInfo->second);
		BLOCXX_ASSERT(ti.isReader() || ti.isWriter());

		if (!ti.isWriter())
		{
			// The id is upgrading

			BLOCXX_ASSERT(m_numWriters == 0 || m_numWriters == 1);
			if (m_numWriters == 1)
			{
				// another id beat us to upgrading the write lock.  Throw an exception.
				BLOCXX_THROW(DeadlockException, "Upgrading read lock to a write lock failed, another upgrade is already in progress.");
			}

			// switch from being a reader to a writer
			--m_numReaders;
			// mark us as a writer, this will prevent other ids from becoming a writer
			++m_numWriters;

			// This thread isn't the only reader. Wait for others to finish.
			while (m_numReaders != 0)
			{
				// stop new readers - inside while loop, because it may get reset by other ids releasing locks.
				m_canRead = false;

				if (!m_waiting_writers.timedWait(l, timer.asAbsoluteTimeout()))
				{
					// undo changes
					++m_numReaders;
					--m_numWriters;
					m_canRead = true;
					if (m_numWriters == 0)
					{
						m_waiting_readers.notifyAll();
					}
					BLOCXX_THROW(TimeoutException, "Timeout while waiting for write lock.");
				}
			}
		}
		++ti.writeCount;

	}
	else
	{
		// This id doesn't have any lock

		while (m_numReaders != 0 || m_numWriters != 0)
		{
			// stop new readers
			m_canRead = false;

			if (!m_waiting_writers.timedWait(l, timer.asAbsoluteTimeout()))
			{
				m_canRead = true;
				if (m_numWriters == 0)
				{
					m_waiting_readers.notifyAll();
				}
				BLOCXX_THROW(TimeoutException, "Timeout while waiting for write lock.");
			}
		}

		LockerInfo ti;
		ti.readCount = 0;
		ti.writeCount = 1;
		m_lockerInfo.insert(typename IdMap::value_type(id, ti));
		++m_numWriters;
		m_canRead = false;
	}

}

//////////////////////////////////////////////////////////////////////////////
template <typename IdT, typename CompareT>
void
GenericRWLockImpl<IdT, CompareT>::releaseWriteLock(const IdT id)
{
	NonRecursiveMutexLock l(m_guard);

	typename IdMap::iterator pInfo = m_lockerInfo.find(id);

	if (pInfo == m_lockerInfo.end() || !pInfo->second.isWriter())
	{
		BLOCXX_THROW(GenericRWLockImplException, "Cannot release a write lock when no write lock is held");
	}

	LockerInfo& ti(pInfo->second);

	BLOCXX_ASSERT(ti.isWriter());

	--ti.writeCount;

	if (!ti.isWriter())
	{
		--m_numWriters;

		BLOCXX_ASSERT(m_numWriters == 0);

		m_canRead = true;
		if (ti.isReader())
		{
			// restore reader status
			++m_numReaders;
		}
		else
		{
			// This id no longer holds locks.
			m_waiting_writers.notifyOne();
			m_lockerInfo.erase(pInfo);
		}
		m_waiting_readers.notifyAll();
	}
}


} // end namespace BLOCXX_NAMESPACE

#endif
