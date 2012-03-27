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

#ifndef BLOCXX_NON_RECURSIVE_MUTEX_LOCK_HPP_INCLUDE_GUARD_
#define BLOCXX_NON_RECURSIVE_MUTEX_LOCK_HPP_INCLUDE_GUARD_
#include "blocxx/BLOCXX_config.h"
#include "blocxx/NonRecursiveMutex.hpp"
#include <cassert>

namespace BLOCXX_NAMESPACE
{

//////////////////////////////////////////////////////////////////////////////
	/**
	* Note that descriptions of what exceptions may be thrown assumes that
	* object is used correctly, i.e., method preconditions are satisfied.
	*/
class BLOCXX_COMMON_API NonRecursiveMutexLock
{
public:
	/**
	* @throw no exception
	*/
	explicit NonRecursiveMutexLock(NonRecursiveMutex& mutex, bool initially_locked=true)
		: m_mutex(&mutex), m_locked(false)
	{
		if (initially_locked)
		{
			lock();
		}
	}
	~NonRecursiveMutexLock()
	{
		try
		{
			if (m_locked)
			{
				release();
			}
		}
		catch (...)
		{
			// don't let exceptions escape
		}
	}
	/**
	* @pre Mutex not already locked by this thread.
	* @throw no exception
	*/
	void lock()
	{
		assert(m_locked == false);
		m_mutex->acquire();
		m_locked = true;
	}
	/**
	* @pre Mutex currently locked by this thread and object.
	* @throw no exception
	*/
	void release()
	{
		assert(m_locked == true);
		m_mutex->release();
		m_locked = false;
	}
	/**
	* @throw no exception
	*/
	NonRecursiveMutexLock(const NonRecursiveMutexLock& arg)
		: m_mutex(arg.m_mutex), m_locked(arg.m_locked)
	{
		arg.m_locked = false;
	}
	/**
	* @throw no exception
	*/
	bool isLocked() const
	{
		return m_locked;
	}
	/*
	NonRecursiveMutexLock& operator= (const NonRecursiveMutexLock& arg)
	{
		release();
		m_locked = arg.m_locked;
		m_mutex = arg.m_mutex;
		arg.m_locked = false;
		return *this;
	}
	*/
private:
	NonRecursiveMutex* m_mutex;
	mutable bool m_locked;
	friend class Condition;
};

} // end namespace BLOCXX_NAMESPACE

#endif
