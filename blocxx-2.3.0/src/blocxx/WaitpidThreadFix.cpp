/*******************************************************************************
* Copyright (C) 2005, Quest Software, Inc. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*     * Redistributions of source code must retain the above copyright notice,
*       this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of the Network Associates, nor Quest Software, Inc., nor the
*       names of its contributors or employees may be used to endorse or promote
*       products derived from this software without specific prior written
*       permission.
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
* @author Mat Bess
*/
#include "blocxx/BLOCXX_config.h"
#include "blocxx/WaitpidThreadFix.hpp"
#include "blocxx/Thread.hpp"
#include "blocxx/Exec.hpp"
#include "blocxx/WaitpidThreadFixFwd.hpp"
#include "blocxx/ThreadOnce.hpp"
#include "blocxx/NonRecursiveMutex.hpp"
#include "blocxx/NonRecursiveMutexLock.hpp"
#include "blocxx/Condition.hpp"
#include "blocxx/Reference.hpp"
#include "blocxx/IntrusiveReference.hpp"
#include <queue>
#include <sys/types.h>
#ifndef BLOCXX_WIN32
#include <sys/wait.h>
#endif

using namespace blocxx;

namespace BLOCXX_NAMESPACE
{

namespace
{
	bool g_useWaitpidThreadFix =
#ifdef BLOCXX_WAITPID_THREADING_PROBLEM
		true;
#else
		false;
#endif

	class ProcessThread;

	OnceFlag g_initThreadGuard = BLOCXX_ONCE_INIT;
	ProcessThread* g_processThread = 0;

	void initThread();

	Thread_t getWorkerThreadId();

}

bool WaitpidThreadFix::setWaitpidThreadFixEnabled(bool enabled)
{
	bool rv = g_useWaitpidThreadFix;
	g_useWaitpidThreadFix = enabled;
	return rv;
}

bool WaitpidThreadFix::shouldUseWaitpidThreadFix()
{
	if (!g_useWaitpidThreadFix)
	{
		return false;
	}
	Thread_t currThread = ThreadImpl::currentThread();
	Thread_t workerThread = getWorkerThreadId();

	// If we are already in the WaitpidThreadFix worker thread
	// then we dont want to cause an infinite loop
	if (ThreadImpl::sameThreads(currThread, workerThread))
	{
		return false;
	}
	return true;
}

namespace
{
	typedef Reference<Exception> ExceptionPtr;


	class WorkSignal
	{
	public:
		WorkSignal()
			: m_signal(false)
		{
		}

		~WorkSignal()
		{
		}

		void signal()
		{
			NonRecursiveMutexLock lock(m_mutex);
			m_signal = true;
			m_cond.notifyAll();
		}

		void waitForSignal()
		{
			NonRecursiveMutexLock lock(m_mutex);

			while(!m_signal)
			{
				m_cond.wait(lock);
			}
		}

	private:
		bool m_signal;
		Condition m_cond;
		NonRecursiveMutex m_mutex;
	};

	//***************************************************************************
	// - This base class represents the work to be performed by ControlledAccessThread
	// - This class and all derived classes must be thread safe
	class WorkItem : public IntrusiveCountableBase
	{
	public:
		virtual ~WorkItem()
		{
		}

		virtual void doWork() = 0;

		void signalDone()
		{
			m_doneSig.signal();
		}

		void saveException(Exception* err)
		{
			NonRecursiveMutexLock lock(m_errMutex);
			m_err = err;
		}

		Exception* getException()
		{
			NonRecursiveMutexLock lock(m_errMutex);
			return m_err.getPtr();
		}

	protected:
		ExceptionPtr m_err;
		NonRecursiveMutex m_errMutex;
		WorkSignal m_doneSig;
	};


	//***************************************************************************
	class SpawnWorkItem : public WorkItem
	{
	public:
		SpawnWorkItem(char const * execPath, char const * const argv[],
					  char const * const envp[], Exec::PreExec & preExec)
			: m_execPath(execPath)
			, m_argv(argv)
			, m_envp(envp)
			, m_preExec(preExec)
		{
		}

		virtual ~SpawnWorkItem()
		{
		}

		virtual void doWork()
		{
			NonRecursiveMutexLock lock(m_resultMutex);
			m_result = Exec::spawnImpl(m_execPath, m_argv, m_envp, m_preExec);
		}

		ProcessRef waitTillDone()
		{
			m_doneSig.waitForSignal();

			NonRecursiveMutexLock lock(m_resultMutex);
			return m_result;
		}

	protected:
		ProcessRef m_result;
		NonRecursiveMutex m_resultMutex;

		const char * m_execPath;
		const char * const * m_argv;
		const char * const * m_envp;
		Exec::PreExec& m_preExec;
	};


	//***************************************************************************
	class WaitpidWorkItem : public WorkItem
	{
	public:
		WaitpidWorkItem(const ::pid_t& pid)
			: m_pid(pid)
		{
		}

		virtual ~WaitpidWorkItem()
		{
		}

		virtual void doWork()
		{
			NonRecursiveMutexLock lock(m_resultMutex);
			m_result = pollStatusImpl(m_pid);
		}

		Process::Status waitTillDone()
		{
			m_doneSig.waitForSignal();

			NonRecursiveMutexLock lock(m_resultMutex);
			return m_result;
		}


	protected:
		Process::Status m_result;
		NonRecursiveMutex m_resultMutex;

		const ::pid_t& m_pid;
	};

	typedef IntrusiveReference<SpawnWorkItem> SpawnWorkItemPtr;
	typedef IntrusiveReference<WaitpidWorkItem> WaitpidWorkItemPtr;

	class WorkQueue
	{
	public:
		WorkQueue() {}
		virtual ~WorkQueue() {}

		WorkItem* getWork()
		{
			NonRecursiveMutexLock lock(m_workMutex);

			// Wait for some work to show up
			// by checking the predicate in a loop
			while(m_work.empty())
			{
				m_workNotEmpty.wait(lock);
			}

			WorkItem* newWork = m_work.front();
			m_work.pop();

			return newWork;
		}

		void addWork(WorkItem* newWork)
		{
			NonRecursiveMutexLock lock(m_workMutex);
			m_work.push(newWork);
			m_workNotEmpty.notifyAll();
		}

	private:
		std::queue<WorkItem*> m_work;
		Condition m_workNotEmpty;
		NonRecursiveMutex m_workMutex;
	};

	//***************************************************************************
	// This is the worker thread that launches processes and/or calls
	// waitpid on them when BLOCXX_WAITPID_THREADING_PROBLEM is defined
	//***************************************************************************
	class ProcessThread : public Thread
	{
	public:
		ProcessThread();
		virtual ~ProcessThread();

		virtual Int32 run();

		ProcessRef spawn(
			char const * exec_path,
			char const * const argv[],
			char const * const envp[],
			Exec::PreExec & pre_exec
			);

		Process::Status waitPid(const ProcId& pid);

	protected:
		WorkQueue m_workQueue;

		NonRecursiveMutex m_idMutex;
	};

	ProcessThread::ProcessThread()
	{
	}

	ProcessThread::~ProcessThread()
	{
	}

	// This function will never exit until the process terminates itself.
	Int32 ProcessThread::run()
	{
		// Infinite loop.
		while(true)
		{
			WorkItem* newWork;
			newWork = m_workQueue.getWork();

			try
			{
				newWork->doWork();
			}
			catch(Exception& e)
			{
				newWork->saveException(e.clone());
			}
			newWork->signalDone();
		}

		// A return (never reached) to make various compilers happy.
		return 0;
	}

	ProcessRef ProcessThread::spawn(char const * exec_path, char const * const argv[],
			char const * const envp[], Exec::PreExec & pre_exec)
	{
		SpawnWorkItemPtr newWork(new SpawnWorkItem(exec_path, argv, envp, pre_exec));
		m_workQueue.addWork(newWork.getPtr());

		ProcessRef result = newWork->waitTillDone();

		Exception* err = newWork->getException();
		if(err != 0)
		{
			err->rethrow();
		}

		return result;
	}

	Process::Status ProcessThread::waitPid(const ProcId& pid)
	{
		WaitpidWorkItemPtr newWork(new WaitpidWorkItem(pid));
		m_workQueue.addWork(newWork.getPtr());

		Process::Status result = newWork->waitTillDone();

		Exception* err = newWork->getException();
		if(err != 0)
		{
			err->rethrow();
		}

		return result;
	}


	void initThread()
	{
		// create the worker thread
		g_processThread = new ProcessThread();
		g_processThread->start();
	}

	Thread_t getWorkerThreadId()
	{
		callOnce(g_initThreadGuard, initThread);
		return g_processThread->getId();
	}

} // namespace (anon)


ProcessRef WaitpidThreadFix::spawnProcess(char const * exec_path,
	char const * const argv[], char const * const envp[], Exec::PreExec & pre_exec)
{
	callOnce(g_initThreadGuard, initThread);
	return g_processThread->spawn(exec_path, argv, envp, pre_exec);
}

Process::Status WaitpidThreadFix::waitPid(const ProcId& pid)
{
	callOnce(g_initThreadGuard, initThread);
	return g_processThread->waitPid(pid);
}

} //namespace BLOCXX_NAMESPACE

