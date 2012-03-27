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

#include "blocxx/BLOCXX_config.h"
#include "blocxx/RWLocker.hpp"
#include "blocxx/ThreadImpl.hpp"

namespace BLOCXX_NAMESPACE
{

BLOCXX_DEFINE_EXCEPTION_WITH_ID(RWLocker);

inline bool RWLocker::ThreadComparer::operator()(Thread_t x, Thread_t y) const
{
#ifdef BLOCXX_NCR
	return !ThreadImpl::sameThreads(x, y) && (cma_thread_get_unique(&x) < cma_thread_get_unique(&y));
#else
	return !ThreadImpl::sameThreads(x, y) && x < y;
#endif
}

//////////////////////////////////////////////////////////////////////////////
RWLocker::RWLocker()
{
}
//////////////////////////////////////////////////////////////////////////////
RWLocker::~RWLocker()
{
}
//////////////////////////////////////////////////////////////////////////////
void
RWLocker::getReadLock(UInt32 sTimeout, UInt32 usTimeout)
{
	getReadLock(Timeout::relative(sTimeout + static_cast<float>(usTimeout) * 1000000.0));
}

//////////////////////////////////////////////////////////////////////////////
void
RWLocker::getReadLock(const Timeout& timeout)
{
	Thread_t tid = ThreadImpl::currentThread();
	m_impl.acquireReadLock(tid, timeout);
}

//////////////////////////////////////////////////////////////////////////////
void
RWLocker::releaseReadLock()
{
	Thread_t tid = ThreadImpl::currentThread();
	try
	{
		m_impl.releaseReadLock(tid);
	}
	catch (GenericRWLockImplException& e)
	{
		BLOCXX_THROW_SUBEX(RWLockerException, "Cannot release a read lock when no read lock is held", e);
	}
}

//////////////////////////////////////////////////////////////////////////////
void
RWLocker::getWriteLock(UInt32 sTimeout, UInt32 usTimeout)
{
	getWriteLock(Timeout::relative(sTimeout + static_cast<float>(usTimeout) * 1000000.0));
}
//////////////////////////////////////////////////////////////////////////////
void
RWLocker::getWriteLock(const Timeout& timeout)
{
	Thread_t tid = ThreadImpl::currentThread();
	m_impl.acquireWriteLock(tid, timeout);
}

//////////////////////////////////////////////////////////////////////////////
void
RWLocker::releaseWriteLock()
{
	Thread_t tid = ThreadImpl::currentThread();
	try
	{
		m_impl.releaseWriteLock(tid);
	}
	catch (GenericRWLockImplException& e)
	{
		BLOCXX_THROW_SUBEX(RWLockerException, "Cannot release a write lock when no write lock is held", e);
	}
}

} // end namespace BLOCXX_NAMESPACE

