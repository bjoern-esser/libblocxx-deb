/*******************************************************************************
* Copyright (C) 2005, 2010, Quest Software, Inc. All rights reserved.
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
#include "blocxx/ThreadPool.hpp"
#include "blocxx/Array.hpp"
#include "blocxx/Thread.hpp"
#include "blocxx/NonRecursiveMutex.hpp"
#include "blocxx/NonRecursiveMutexLock.hpp"
#include "blocxx/Condition.hpp"
#include "blocxx/Format.hpp"
#include "blocxx/Mutex.hpp"
#include "blocxx/MutexLock.hpp"
#include "blocxx/NullLogger.hpp"
#include "blocxx/Timeout.hpp"
#include "blocxx/TimeoutTimer.hpp"
#include "blocxx/GlobalString.hpp"

#include <deque>

#ifdef BLOCXX_DEBUG
#include <iostream> // for cerr
#endif

namespace BLOCXX_NAMESPACE
{

BLOCXX_DEFINE_EXCEPTION(ThreadPool);

// logger can be null
#define BLOCXX_POOL_LOG_DEBUG(logger, arg) do { BLOCXX_LOG_DEBUG(logger, m_poolName + ": " + arg); } while (0)
#define BLOCXX_POOL_LOG_DEBUG2(logger, arg) do { BLOCXX_LOG_DEBUG2(logger, m_poolName + ": " + arg); } while (0)
#define BLOCXX_POOL_LOG_DEBUG3(logger, arg) do { BLOCXX_LOG_DEBUG3(logger, m_poolName + ": " + arg); } while (0)
#define BLOCXX_POOL_LOG_ERROR(logger, arg) do { BLOCXX_LOG_ERROR(logger, m_poolName + ": " + arg); } while (0)
#define BLOCXX_POOL_LOG_FATAL_ERROR(logger, arg) do { BLOCXX_LOG_FATAL_ERROR(logger, m_poolName + ": " + arg); } while (0)

/////////////////////////////////////////////////////////////////////////////
class ThreadPoolImpl : public IntrusiveCountableBase
{
public:
	// returns true if work is placed in the queue to be run and false if not.
	virtual bool addWork(const RunnableRef& work, const Timeout& timeout) = 0;
	virtual void shutdown(ThreadPool::EShutdownQueueFlag finishWorkInQueue, const Timeout& shutdownTimeout, const Timeout& definitiveCancelTimeout) = 0;
	virtual void waitForEmptyQueue() = 0;
	virtual ~ThreadPoolImpl()
	{
	}
};
namespace {

GlobalString COMPONENT_NAME = BLOCXX_GLOBAL_STRING_INIT("blocxx.ThreadPool");

class FixedSizePoolImpl;
/////////////////////////////////////////////////////////////////////////////
class FixedSizePoolWorkerThread : public Thread
{
public:
	FixedSizePoolWorkerThread(FixedSizePoolImpl* thePool)
		: Thread()
		, m_thePool(thePool)
	{
	}
	virtual Int32 run();
private:
	virtual void doShutdown()
	{
		MutexLock lock(m_guard);
		if (m_currentRunnable)
		{
			m_currentRunnable->doShutdown();
		}
	}
	virtual void doCooperativeCancel()
	{
		MutexLock lock(m_guard);
		if (m_currentRunnable)
		{
			m_currentRunnable->doCooperativeCancel();
		}
	}
	virtual void doDefinitiveCancel()
	{
		MutexLock lock(m_guard);
		if (m_currentRunnable)
		{
			m_currentRunnable->doDefinitiveCancel();
		}
	}

	FixedSizePoolImpl* m_thePool;

	Mutex m_guard;
	RunnableRef m_currentRunnable;

	// non-copyable
	FixedSizePoolWorkerThread(const FixedSizePoolWorkerThread&);
	FixedSizePoolWorkerThread& operator=(const FixedSizePoolWorkerThread&);
};
/////////////////////////////////////////////////////////////////////////////
class CommonPoolImpl : public ThreadPoolImpl
{
protected:
	CommonPoolImpl(UInt32 maxQueueSize, const Logger& logger, const String& poolName)
		: m_maxQueueSize(maxQueueSize)
		, m_queueClosed(false)
		, m_shutdown(false)
		, m_logger(logger)
		, m_poolName(poolName)
	{
	}

	virtual ~CommonPoolImpl()
	{
	}

	// assumes that m_queueLock is locked. DynamicSizeNoQueuePoolImpl overrides this.
	virtual bool queueIsFull() const
	{
		return ((m_maxQueueSize > 0) && (m_queue.size() == m_maxQueueSize));
	}

	// assumes that m_queueLock is locked
	bool queueClosed() const
	{
		return m_shutdown || m_queueClosed;
	}

	bool finishOffWorkInQueue(ThreadPool::EShutdownQueueFlag finishWorkInQueue, const Timeout& timeout)
	{
		NonRecursiveMutexLock l(m_queueLock);
		// the pool is in the process of being destroyed
		if (queueClosed())
		{
			BLOCXX_POOL_LOG_DEBUG2(m_logger, "Queue is already closed.  Why are you trying to shutdown again?");
			return false;
		}
		m_queueClosed = true;
		BLOCXX_POOL_LOG_DEBUG2(m_logger, "Queue closed");

		if (finishWorkInQueue)
		{
			TimeoutTimer timer(timeout);
			while (m_queue.size() != 0)
			{
				if (timer.infinite())
				{
					BLOCXX_POOL_LOG_DEBUG2(m_logger, "Waiting forever for queue to empty");
					m_queueEmpty.wait(l);
				}
				else
				{
					BLOCXX_POOL_LOG_DEBUG2(m_logger, "Waiting w/timout for queue to empty");
					if (!m_queueEmpty.timedWait(l, timer.asAbsoluteTimeout()))
					{
						BLOCXX_POOL_LOG_DEBUG2(m_logger, "Wait timed out. Work in queue will be discarded.");
						break; // timed out
					}
				}
			}
		}
		m_shutdown = true;
		return true;
	}

	virtual void waitForEmptyQueue()
	{
		NonRecursiveMutexLock l(m_queueLock);
		while (m_queue.size() != 0)
		{
			BLOCXX_POOL_LOG_DEBUG2(m_logger, "Waiting for empty queue");
			m_queueEmpty.wait(l);
		}
		BLOCXX_POOL_LOG_DEBUG2(m_logger, "Queue empty: the wait is over");
	}

	void shutdownThreads(ThreadPool::EShutdownQueueFlag finishWorkInQueue, const Timeout& shutdownTimeout, const Timeout& definitiveCancelTimeout)
	{
		TimeoutTimer shutdownTimer(shutdownTimeout);
		TimeoutTimer dTimer(definitiveCancelTimeout);
		if (!finishOffWorkInQueue(finishWorkInQueue, shutdownTimer.asAbsoluteTimeout()))
		{
			return;
		}

		// Wake up any workers so they recheck shutdown flag
		m_queueNotEmpty.notifyAll();
		m_queueNotFull.notifyAll();

		if (!shutdownTimer.infinite())
		{
			// Tell all the threads to shutdown
			for (UInt32 i = 0; i < m_threads.size(); ++i)
			{
				BLOCXX_POOL_LOG_DEBUG(m_logger, Format("Calling shutdown on thread %1", i));
				m_threads[i]->shutdown();
			}

			// Wait until shutdownTimeout for the threads to finish
			Timeout absoluteShutdownTimeout(shutdownTimer.asAbsoluteTimeout());
			for (UInt32 i = 0; i < m_threads.size(); ++i)
			{
				BLOCXX_POOL_LOG_DEBUG2(m_logger, Format("Waiting for thread %1 to exit.", i));
				m_threads[i]->timedWait(absoluteShutdownTimeout);
			}

			// Tell all the threads to cooperative cancel
			for (UInt32 i = 0; i < m_threads.size(); ++i)
			{
				BLOCXX_POOL_LOG_DEBUG2(m_logger, Format("Calling cooperativeCancel on thread %1", i));
				m_threads[i]->cooperativeCancel();
			}

			if (!dTimer.infinite())
			{
				// If any still haven't shut down, definitiveCancel will kill them.
				Timeout absoluteDefinitiveTimeout(dTimer.asAbsoluteTimeout());
				for (UInt32 i = 0; i < m_threads.size(); ++i)
				{
					BLOCXX_POOL_LOG_DEBUG2(m_logger, Format("Calling definitiveCancel on thread %1", i));
					try
					{
						if (!m_threads[i]->definitiveCancel(absoluteDefinitiveTimeout))
						{
							BLOCXX_POOL_LOG_FATAL_ERROR(m_logger, Format("Thread %1 was forcibly cancelled.", i));
						}
					}
					catch (CancellationDeniedException& e)
					{
						BLOCXX_POOL_LOG_ERROR(m_logger, Format("Caught CanacellationDeniedException: %1 for thread %2. Pool shutdown may hang.", e, i));
					}
				}
			}

		}

		// Clean up after the threads and/or wait for them to exit.
		for (UInt32 i = 0; i < m_threads.size(); ++i)
		{
			BLOCXX_POOL_LOG_DEBUG2(m_logger, Format("calling join() on thread %1", i));
			m_threads[i]->join();
			BLOCXX_POOL_LOG_DEBUG2(m_logger, Format("join() finished for thread %1", i));
		}
	}

	RunnableRef getWorkFromQueue(bool waitForWork)
	{
		NonRecursiveMutexLock l(m_queueLock);
		while ((m_queue.size() == 0) && (!m_shutdown))
		{
			if (waitForWork)
			{
				BLOCXX_POOL_LOG_DEBUG3(m_logger, "Waiting for work");
				m_queueNotEmpty.wait(l);
			}
			else
			{
				// wait 1 sec for work, to more efficiently handle a stream
				// of single requests.
				if (!m_queueNotEmpty.timedWait(l,Timeout::relative(1)))
				{
					BLOCXX_POOL_LOG_DEBUG3(m_logger, "No work after 1 sec. I'm not waiting any longer");
					return RunnableRef();
				}
			}
		}
		// check to see if a shutdown started while the thread was sleeping
		if (m_shutdown)
		{
			BLOCXX_POOL_LOG_DEBUG(m_logger, "The pool is shutdown, not getting any more work");
			return RunnableRef();
		}

		RunnableRef work = m_queue.front();
		m_queue.pop_front();

		// This needs to happen before the call to queueIsFull() because the worker count can affect the result of queueIsFull()
		incrementWorkerCount();
		// handle threads waiting in addWork().
		if (!queueIsFull())
		{
			m_queueNotFull.notifyAll();
		}

		// handle waiting shutdown thread or callers of waitForEmptyQueue()
		if (m_queue.size() == 0)
		{
			m_queueEmpty.notifyAll();
		}
		BLOCXX_POOL_LOG_DEBUG3(m_logger, "A thread got some work to do");
		return work;
	}

	// hooks for DynamicSizeNoQueuePoolImpl subclass. Yes this is a horrible design, it just saves code duplication.
	virtual void incrementWorkerCount()
	{
	}

	virtual void decrementWorkerCount()
	{
	}

	// pool characteristics
	UInt32 m_maxQueueSize;
	// pool state
	Array<ThreadRef> m_threads;
	std::deque<RunnableRef> m_queue;
	bool m_queueClosed;
	bool m_shutdown;
	// pool synchronization
	NonRecursiveMutex m_queueLock;
	Condition m_queueNotFull;
	Condition m_queueEmpty;
	Condition m_queueNotEmpty;
	Logger m_logger;
	String m_poolName;
};
class FixedSizePoolImpl : public CommonPoolImpl
{
public:
	FixedSizePoolImpl(UInt32 numThreads, UInt32 maxQueueSize, const Logger& logger, const String& poolName)
		: CommonPoolImpl(maxQueueSize, logger, poolName)
	{
		// create the threads and start them up.
		m_threads.reserve(numThreads);
		for (UInt32 i = 0; i < numThreads; ++i)
		{
			m_threads.push_back(ThreadRef(new FixedSizePoolWorkerThread(this)));
		}
		for (UInt32 i = 0; i < numThreads; ++i)
		{
			try
			{
				m_threads[i]->start();
			}
			catch (ThreadException& e)
			{
				BLOCXX_POOL_LOG_ERROR(m_logger, Format("Failed to start thread #%1: %2", i, e));
				m_threads.resize(i); // remove non-started threads
				// shutdown the rest
				this->FixedSizePoolImpl::shutdown(ThreadPool::E_DISCARD_WORK_IN_QUEUE, Timeout::relative(0.5), Timeout::relative(0.5));
				throw;
			}
		}
		BLOCXX_POOL_LOG_DEBUG(m_logger, "Threads are started and ready to go");
	}
	// returns true if work is placed in the queue to be run and false if not.
	virtual bool addWork(const RunnableRef& work, const Timeout& timeout)
	{
		// check precondition: work != NULL
		if (!work)
		{
			BLOCXX_POOL_LOG_DEBUG(m_logger, "Trying to add NULL work! Shame on you.");
			return false;
		}

		NonRecursiveMutexLock l(m_queueLock);
		TimeoutTimer timer(timeout);
		while ( queueIsFull() && !queueClosed() )
		{
			BLOCXX_POOL_LOG_DEBUG3(m_logger, "Queue is full. Waiting until a spot opens up so we can add some work");
			if (!m_queueNotFull.timedWait(l, timer.asAbsoluteTimeout()))
			{
				// timed out
				BLOCXX_POOL_LOG_DEBUG3(m_logger, "Queue is full and timeout expired. Not adding work and returning false");
				return false;
			}
		}

		// the pool is in the process of being destroyed
		if (queueClosed())
		{
			BLOCXX_POOL_LOG_DEBUG3(m_logger, "Queue was closed out from underneath us. Not adding work and returning false");
			return false;
		}

		m_queue.push_back(work);

		// if the queue was empty, there may be workers just sitting around, so we need to wake them up!
		if (m_queue.size() == 1)
		{
			BLOCXX_POOL_LOG_DEBUG3(m_logger, "Waking up sleepy workers");
			m_queueNotEmpty.notifyAll();
		}

		BLOCXX_POOL_LOG_DEBUG(m_logger, "Work has been added to the queue");
		return true;
	}

	// we keep this around so it can be called in the destructor
	virtual void shutdown(ThreadPool::EShutdownQueueFlag finishWorkInQueue, const Timeout& shutdownTimeout, const Timeout& definitiveCancelTimeout)
	{
		shutdownThreads(finishWorkInQueue, shutdownTimeout, definitiveCancelTimeout);
	}
	virtual ~FixedSizePoolImpl()
	{
		// can't let exception escape the destructor
		try
		{
			// don't need a lock here, because we're the only thread left.
			if (!queueClosed())
			{
				// Make sure the pool is shutdown.
				// Specify which shutdown() we want so we don't get undefined behavior calling a virtual function from the destructor.
				this->FixedSizePoolImpl::shutdown(ThreadPool::E_DISCARD_WORK_IN_QUEUE, Timeout::relative(0.5), Timeout::relative(0.5));
			}
		}
		catch (...)
		{
		}
	}
private:
	friend class FixedSizePoolWorkerThread;
};
void runRunnable(const RunnableRef& work)
{
	// don't let exceptions escape, we need to keep going, except for ThreadCancelledException, in which case we need to stop.
	try
	{
		work->run();
	}
	catch (ThreadCancelledException&)
	{
		throw;
	}
	catch (Exception& ex)
	{
#ifdef BLOCXX_DEBUG
		std::clog << "!!! Exception: " << ex.type() << " caught in ThreadPool worker: " << ex << std::endl;
#endif
		Logger logger(COMPONENT_NAME);
		BLOCXX_LOG_ERROR(logger, Format("!!! Exception caught in ThreadPool worker: %1", ex));
	}
	catch(std::exception& ex)
	{
#ifdef BLOCXX_DEBUG
		std::clog << "!!! std::exception what = \"" << ex.what() << "\" caught in ThreadPool worker" << std::endl;
#endif
		Logger logger(COMPONENT_NAME);
		BLOCXX_LOG_ERROR(logger, Format("!!! std::exception caught in ThreadPool worker: %1", ex.what()));
	}
	catch (...)
	{
#ifdef BLOCXX_DEBUG
		std::clog << "!!! Unknown Exception caught in ThreadPool worker" << std::endl;
#endif
		Logger logger(COMPONENT_NAME);
		BLOCXX_LOG_ERROR(logger, "!!! Unknown Exception caught in ThreadPool worker.");
	}
}
Int32 FixedSizePoolWorkerThread::run()
{
	while (true)
	{
		// check queue for work
		RunnableRef work = m_thePool->getWorkFromQueue(true);
		if (!work)
		{
			return 0;
		}
		// save this off so it can be cancelled by another thread.
		{
			MutexLock lock(m_guard);
			m_currentRunnable = work;
		}
		runRunnable(work);
		{
			MutexLock lock(m_guard);
			m_currentRunnable = 0;
		}
	}
	return 0;
}
class DynamicSizePoolImpl;
/////////////////////////////////////////////////////////////////////////////
class DynamicSizePoolWorkerThread : public Thread
{
public:
	DynamicSizePoolWorkerThread(DynamicSizePoolImpl* thePool)
		: Thread()
		, m_thePool(thePool)
	{
	}
	virtual Int32 run();
private:
	virtual void doShutdown()
	{
		MutexLock lock(m_guard);
		if (m_currentRunnable)
		{
			m_currentRunnable->doShutdown();
		}
	}
	virtual void doCooperativeCancel()
	{
		MutexLock lock(m_guard);
		if (m_currentRunnable)
		{
			m_currentRunnable->doCooperativeCancel();
		}
	}
	virtual void doDefinitiveCancel()
	{
		MutexLock lock(m_guard);
		if (m_currentRunnable)
		{
			m_currentRunnable->doDefinitiveCancel();
		}
	}

	DynamicSizePoolImpl* m_thePool;

	Mutex m_guard;
	RunnableRef m_currentRunnable;

	// non-copyable
	DynamicSizePoolWorkerThread(const DynamicSizePoolWorkerThread&);
	DynamicSizePoolWorkerThread& operator=(const DynamicSizePoolWorkerThread&);
};
/////////////////////////////////////////////////////////////////////////////
class DynamicSizePoolImpl : public CommonPoolImpl
{
public:
	DynamicSizePoolImpl(UInt32 maxThreads, UInt32 maxQueueSize, const Logger& logger, const String& poolName)
		: CommonPoolImpl(maxQueueSize, logger, poolName)
		, m_maxThreads(maxThreads)
	{
	}
	// returns true if work is placed in the queue to be run and false if not.
	virtual bool addWork(const RunnableRef& work, const Timeout& timeout)
	{
		// check precondition: work != NULL
		if (!work)
		{
			BLOCXX_POOL_LOG_DEBUG(m_logger, "Trying to add NULL work! Shame on you.");
			return false;
		}
		NonRecursiveMutexLock l(m_queueLock);

		// the pool is in the process of being destroyed
		if (queueClosed())
		{
			BLOCXX_POOL_LOG_DEBUG3(m_logger, "Queue was closed out from underneath us. Not adding work and returning false");
			return false;
		}

		// Can't touch m_threads until *after* we check for the queue being closed, shutdown
		// requires that m_threads not change after the queue is closed.
		// Now clean up dead threads (before we add the new one, so we don't need to check it)
		size_t i = 0;
		while (i < m_threads.size())
		{
			if (!m_threads[i]->isRunning())
			{
				BLOCXX_POOL_LOG_DEBUG3(m_logger, Format("Thread %1 is finished. Cleaning up it's remains.", i));
				m_threads[i]->join();
				m_threads.remove(i);
			}
			else
			{
				++i;
			}
		}

		TimeoutTimer timer(timeout);
		while ( queueIsFull() && !queueClosed() )
		{
			BLOCXX_POOL_LOG_DEBUG3(m_logger, "Queue is full. Waiting until a spot opens up so we can add some work");
			if (!m_queueNotFull.timedWait(l, timer.asAbsoluteTimeout()))
			{
				// timed out
				BLOCXX_POOL_LOG_DEBUG3(m_logger, "Queue is full and timeout expired. Not adding work and returning false");
				return false;
			}
		}

		// The previous loop could have ended because a spot opened in the queue or it was closed.  Check for the close.
		if (queueClosed())
		{
			BLOCXX_POOL_LOG_DEBUG3(m_logger, "Queue was closed out from underneath us. Not adding work and returning false");
			return false;
		}

		m_queue.push_back(work);

		BLOCXX_POOL_LOG_DEBUG(m_logger, "Work has been added to the queue");

		// Release the lock and wake up a thread waiting for work in the queue
		// This bit of code is a race condition with the thread,
		// but if we acquire the lock again before it does, then we
		// properly handle that case.  The only disadvantage if we win
		// the "race" is that we'll unnecessarily start a new thread.
		// In practice it works all the time.
		l.release();
		m_queueNotEmpty.notifyOne();
		Thread::yield(); // give the thread a chance to run
		l.lock();

		// Start up a new thread to handle the work in the queue.
		if (!m_queue.empty() && m_threads.size() < m_maxThreads)
		{
			ThreadRef theThread(new DynamicSizePoolWorkerThread(this));
			m_threads.push_back(theThread);
			BLOCXX_POOL_LOG_DEBUG3(m_logger, "About to start a new thread");
			try
			{
				theThread->start();
			}
			catch (ThreadException& e)
			{
				BLOCXX_POOL_LOG_ERROR(m_logger, Format("Failed to start thread: %1", e));
				m_threads.pop_back();
				throw;
			}
			BLOCXX_POOL_LOG_DEBUG2(m_logger, "New thread started");
		}
		return true;
	}

	// we keep this around so it can be called in the destructor
	virtual void shutdown(ThreadPool::EShutdownQueueFlag finishWorkInQueue, const Timeout& shutdownTimeout, const Timeout& definitiveCancelTimeout)
	{
		shutdownThreads(finishWorkInQueue, shutdownTimeout, definitiveCancelTimeout);
	}
	virtual ~DynamicSizePoolImpl()
	{
		// can't let exception escape the destructor
		try
		{
			// don't need a lock here, because we're the only thread left.
			if (!queueClosed())
			{
				// Make sure the pool is shutdown.
				// Specify which shutdown() we want so we don't get undefined behavior calling a virtual function from the destructor.
				this->DynamicSizePoolImpl::shutdown(ThreadPool::E_DISCARD_WORK_IN_QUEUE, Timeout::relative(0.5), Timeout::relative(0.5));
			}
		}
		catch (...)
		{
		}
	}

protected:
	UInt32 getMaxThreads() const
	{
		return m_maxThreads;
	}

private:
	// pool characteristics
	UInt32 m_maxThreads;
	friend class DynamicSizePoolWorkerThread;
};
Int32 DynamicSizePoolWorkerThread::run()
{
	while (true)
	{
		// check queue for work
		RunnableRef work = m_thePool->getWorkFromQueue(false);
		if (!work)
		{
			return 0;
		}
		// save this off so it can be cancelled by another thread.
		{
			MutexLock lock(m_guard);
			m_currentRunnable = work;
		}
		runRunnable(work);
		m_thePool->decrementWorkerCount();
		{
			MutexLock lock(m_guard);
			m_currentRunnable = 0;
		}
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
class DynamicSizeNoQueuePoolImpl : public DynamicSizePoolImpl
{
public:
	DynamicSizeNoQueuePoolImpl(UInt32 maxThreads, const Logger& logger, const String& poolName)
		: DynamicSizePoolImpl(maxThreads, maxThreads, logger, poolName) // allow queue in superclass, but prevent it from having any backlog
		, m_workingThreads(0)
	{
	}

	virtual ~DynamicSizeNoQueuePoolImpl()
	{
	}

	virtual void incrementWorkerCount()
	{
		++m_workingThreads;
	}

	virtual void decrementWorkerCount()
	{
		NonRecursiveMutexLock lock(m_queueLock);
		--m_workingThreads;
		// wake up any threads waiting to start some work
		m_queueNotFull.notifyAll();
	}

	// One difference between this class and DynamicSizePoolImpl is that we change the definition of queueIsFull()
	virtual bool queueIsFull() const
	{
		// don't let the queue get bigger than the number of free threads. This effectively prevents work from being
		// queued up which can't be immediately serviced.
		size_t freeThreads = getMaxThreads() -  AtomicGet(m_workingThreads);
		return (freeThreads <= m_queue.size());
	}

private:
	// Keep track of the number of threads doing work. Protected by m_guard
	size_t m_workingThreads;

};

} // end anonymous namespace
/////////////////////////////////////////////////////////////////////////////
ThreadPool::ThreadPool(PoolType poolType, UInt32 numThreads, UInt32 maxQueueSize, const Logger& logger, const String& poolName)
{
	switch (poolType)
	{
		case FIXED_SIZE:
			m_impl = new FixedSizePoolImpl(numThreads, maxQueueSize, logger, poolName);
			break;
		case DYNAMIC_SIZE:
			m_impl = new DynamicSizePoolImpl(numThreads, maxQueueSize, logger, poolName);
			break;
		case DYNAMIC_SIZE_NO_QUEUE:
			m_impl = new DynamicSizeNoQueuePoolImpl(numThreads, logger, poolName);
			break;
	}
}
/////////////////////////////////////////////////////////////////////////////
bool ThreadPool::addWork(const RunnableRef& work)
{
	return m_impl->addWork(work, Timeout::infinite);
}
/////////////////////////////////////////////////////////////////////////////
bool ThreadPool::tryAddWork(const RunnableRef& work)
{
	return m_impl->addWork(work, Timeout::relative(0));
}
/////////////////////////////////////////////////////////////////////////////
bool ThreadPool::tryAddWork(const RunnableRef& work, const Timeout& timeout)
{
	return m_impl->addWork(work, timeout);
}
/////////////////////////////////////////////////////////////////////////////
void ThreadPool::shutdown(EShutdownQueueFlag finishWorkInQueue, const Timeout& timeout)
{
	m_impl->shutdown(finishWorkInQueue, timeout, timeout);
}
/////////////////////////////////////////////////////////////////////////////
void ThreadPool::shutdown(EShutdownQueueFlag finishWorkInQueue, const Timeout& shutdownTimeout, const Timeout& definitiveCancelTimeout)
{
	m_impl->shutdown(finishWorkInQueue, shutdownTimeout, definitiveCancelTimeout);
}
/////////////////////////////////////////////////////////////////////////////
void ThreadPool::waitForEmptyQueue()
{
	m_impl->waitForEmptyQueue();
}
/////////////////////////////////////////////////////////////////////////////
ThreadPool::~ThreadPool()
{
}
/////////////////////////////////////////////////////////////////////////////
ThreadPool::ThreadPool(const ThreadPool& x)
	: IntrusiveCountableBase(x)
	, m_impl(x.m_impl)
{
}
/////////////////////////////////////////////////////////////////////////////
ThreadPool& ThreadPool::operator=(const ThreadPool& x)
{
	m_impl = x.m_impl;
	return *this;
}

} // end namespace BLOCXX_NAMESPACE

