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

#include "blocxx/BLOCXX_config.h"
#include "blocxx/NonRecursiveMutex.hpp"
#include "blocxx/Assertion.hpp"
#include "blocxx/Exception.hpp"
#include "blocxx/Format.hpp"
#include "blocxx/NonRecursiveMutexImpl.hpp"
#include "blocxx/ExceptionIds.hpp"

#include <cstring>

namespace BLOCXX_NAMESPACE
{

BLOCXX_DEFINE_EXCEPTION_WITH_ID(Deadlock);
NonRecursiveMutex::NonRecursiveMutex()
{
	if (NonRecursiveMutexImpl::createMutex(m_mutex) != 0)
	{
		BLOCXX_THROW(AssertionException, "NonRecursiveMutexImpl::createMutex failed");
	}
}
NonRecursiveMutex::~NonRecursiveMutex()
{
	if (NonRecursiveMutexImpl::destroyMutex(m_mutex) == -1)
	{
		NonRecursiveMutexImpl::releaseMutex(m_mutex);
		NonRecursiveMutexImpl::destroyMutex(m_mutex);
	}
}
void 
NonRecursiveMutex::acquire()
{
	int rv = NonRecursiveMutexImpl::acquireMutex(m_mutex);
	if (rv != 0)
	{
		BLOCXX_THROW(AssertionException,
			"NonRecursiveMutexImpl::acquireMutex returned with error");
	}
}
bool
NonRecursiveMutex::release()
{
	int rc = NonRecursiveMutexImpl::releaseMutex(m_mutex);
	if (rc != 0)
	{
		BLOCXX_THROW(AssertionException, Format("NonRecursiveMutexImpl::releaseMutex returned with error %1", rc).c_str());
	}
	return true;
}
void 
NonRecursiveMutex::conditionPreWait(NonRecursiveMutexLockState& state)
{
	if (NonRecursiveMutexImpl::conditionPreWait(m_mutex, state) != 0)
	{
		BLOCXX_THROW(AssertionException, "NonRecursiveMutexImpl::releaseMutex returned with error");
	}
}
void
NonRecursiveMutex::conditionPostWait(NonRecursiveMutexLockState& state)
{
	if (NonRecursiveMutexImpl::conditionPostWait(m_mutex, state) != 0)
	{
		BLOCXX_THROW(AssertionException, "NonRecursiveMutexImpl::releaseMutex returned with error");
	}
}

} // end namespace BLOCXX_NAMESPACE

