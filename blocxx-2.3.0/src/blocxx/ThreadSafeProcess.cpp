/*******************************************************************************
* Copyright (C) 2005, 2008, 2009, Quest Software, Inc. All rights reserved.
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


#include "blocxx/BLOCXX_config.h"
#include "blocxx/ThreadSafeProcess.hpp"
#include "blocxx/NonRecursiveMutexLock.hpp"
#include "blocxx/Exception.hpp"
#include "blocxx/Logger.hpp"
#include "blocxx/Format.hpp"

namespace BLOCXX_NAMESPACE
{
	namespace
	{
		const char* const COMPONENT_NAME = "blocxx.ThreadSafeProcess";
	}

	ThreadSafeProcess::ThreadSafeProcess(const ProcessRef& procToWrap)
		: m_proc(procToWrap)
	{
	}

	ThreadSafeProcess::~ThreadSafeProcess()
	{
		try
		{
			release();
		}
		catch(const Exception& e)
		{
			Logger lgr(COMPONENT_NAME);
			BLOCXX_LOG_DEBUG(lgr, Format("Exception while releasing process: %1", e));
		}
		catch(...)
		{
			// nothing should escape.
			Logger lgr(COMPONENT_NAME);
			BLOCXX_LOG_DEBUG(lgr, "Unknown exception while releasing process.");
		}
	}

	void ThreadSafeProcess::release()
	{
		NonRecursiveMutexLock lock(m_guard);
		m_proc->release();
	}

	UnnamedPipeRef ThreadSafeProcess::in() const
	{
		NonRecursiveMutexLock lock(m_guard);
		return m_proc->in();
	}

	UnnamedPipeRef ThreadSafeProcess::out() const
	{
		NonRecursiveMutexLock lock(m_guard);
		return m_proc->out();
	}


	UnnamedPipeRef ThreadSafeProcess::err() const
	{
		NonRecursiveMutexLock lock(m_guard);
		return m_proc->err();
	}

	ProcId ThreadSafeProcess::pid() const
	{
		NonRecursiveMutexLock lock(m_guard);
		return m_proc->pid();
	}


	Process::Status ThreadSafeProcess::processStatus()
	{
		NonRecursiveMutexLock lock(m_guard);
		return m_proc->processStatus();
	}

	void ThreadSafeProcess::waitCloseTerm(
		const Timeout& wait_initial,
		const Timeout& wait_close,
		const Timeout& wait_term,
		Process::ETerminationSelectionFlag terminationSelectionFlag)
	{
		NonRecursiveMutexLock lock(m_guard);
		return m_proc->waitCloseTerm(wait_initial, wait_close, wait_term, terminationSelectionFlag);
	}

	void ThreadSafeProcess::waitCloseTerm(float wait_initial, float wait_close, float wait_term)
	{
		NonRecursiveMutexLock lock(m_guard);
		return m_proc->waitCloseTerm(wait_initial, wait_close, wait_term);
	}

} // end namespace BLOCXX_NAMESPACE

