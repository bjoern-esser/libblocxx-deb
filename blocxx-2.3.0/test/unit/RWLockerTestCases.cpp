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

#include "blocxx/BLOCXX_config.h"
#define PROVIDE_AUTO_TEST_MAIN
#include "AutoTest.hpp"

#include "blocxx/RWLocker.hpp"
#include "blocxx/Thread.hpp"
#include "blocxx/ThreadBarrier.hpp"
#include "blocxx/TimeoutException.hpp"
#include "blocxx/Runnable.hpp"
#include "blocxx/ThreadPool.hpp"
#include "blocxx/Format.hpp"
#include "blocxx/Timeout.hpp"
#include "blocxx/TimeoutTimer.hpp"
#include "blocxx/RandomNumber.hpp"

extern "C"
{
#ifdef BLOCXX_HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif
#ifdef BLOCXX_HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef BLOCXX_HAVE_UNISTD_H
#include <unistd.h>
#endif
}

using namespace blocxx;
using namespace std;

class testDeadlockThread : public Thread
{
public:
	testDeadlockThread(RWLocker* locker, const ThreadBarrier& bar)
		: Thread()
		, m_locker(locker)
		, m_bar(bar)
	{
	}

protected:
	virtual Int32 run()
	{
		m_locker->getReadLock(Timeout::relative(0));
		m_bar.wait(); // 1
		// upgrade the lock. This should block waiting for the other thread to release the read lock.
		try
		{
			m_locker->getWriteLock(Timeout::relative(100));
		}
		catch (TimeoutException& e)
		{
			clog << "Got timeout waiting to upgrade read to write lock" << endl;
			abort();
		}
		m_locker->releaseWriteLock();

		m_locker->releaseReadLock();

		return 0;
	}

	RWLocker* m_locker;
	ThreadBarrier m_bar;
};

AUTO_UNIT_TEST(RWLockerTestCases_testDeadlock)
{
	RWLocker locker;
	ThreadBarrier bar(2);
	testDeadlockThread tdt(&locker, bar);
	tdt.start();
	Timeout to(Timeout::relative(100));
	locker.getReadLock(to);
	bar.wait(); // 1
	Thread::sleep(5); // give the other thread a chance to enter getWriteLock() before us.
	try
	{
		locker.getWriteLock(Timeout::relative(0));
		unitAssert(0);
	}
	catch (DeadlockException&)
	{
		// expecting this
	}
	locker.releaseReadLock();
}

class testThread : public Thread
{
public:
	testThread(RWLocker* locker, const ThreadBarrier& bar)
		: Thread()
		, m_locker(locker)
		, m_bar(bar)
	{
	}

protected:
	virtual Int32 run()
	{
		m_locker->getReadLock(Timeout::relative(0));
		m_bar.wait();
		m_bar.wait();
		m_locker->releaseReadLock();

		m_locker->getWriteLock(Timeout::relative(0));
		m_bar.wait();
		m_bar.wait();
		m_locker->releaseWriteLock();
		return 0;
	}

	RWLocker* m_locker;
	ThreadBarrier m_bar;
};

AUTO_UNIT_TEST(RWLockerTestCases_testTimeout)
{
	RWLocker locker;
	ThreadBarrier bar(2);
	testThread t1(&locker, bar);
	t1.start();
	bar.wait(); // wait for the thread to start. It's already got the read lock, and will keep it until the barrier is hit again.

	// now try to get a write lock.  Use a short timeout so the test doesn't have to wait long for an exception to be thrown.
	try
	{
		locker.getWriteLock(Timeout::relative(1e-6));
		unitAssert(0);
	}
	catch (const TimeoutException& e)
	{
	}

	bar.wait(); // signal the thread to release the read lock and get a write lock.
	bar.wait(); // wait for the thread to release the read lock and get a write lock.

	// now try to get a read lock.  Use a short timeout so the test doesn't have to wait long for an exception to be thrown.
	try
	{
		locker.getReadLock(Timeout::relative(1e-6));
		unitAssert(0);
	}
	catch (const TimeoutException& e)
	{
	}

	bar.wait();

	unitAssert(t1.join() == 0);
}

const int NUM_VALS = 1024; // 1K
volatile int vals[NUM_VALS];
volatile int curVal = 0;

RandomNumber g_rn10(0, 10);
void doSomeYields()
{
	// 90% of the time don't yield
	Int32 reps = g_rn10.getNextNumber() == 0 ? g_rn10.getNextNumber() : 0;
	for (Int32 i = 0; i < reps; ++i)
	{
		Thread::yield();
	}
}

RandomNumber g_rn1000(0, 1000);
Timeout getRandomTimeout()
{
	return Timeout::relative(g_rn1000.getNextNumber() / 1000.0); // from 0 to 1 seconds.
}

class ReaderThread : public Runnable
{
public:
	ReaderThread(const ThreadBarrier& bar, RWLocker& locker)
		: m_bar(bar)
		, m_locker(locker)
	{
	}

	void run()
	{
		m_bar.wait();
		for (int i = 0; i < NUM_VALS; ++i)
		{
			try
			{
				m_locker.getReadLock(getRandomTimeout());
			}
			catch (TimeoutException& e)
			{
				doSomeYields();
				--i;
				continue; // timed out
			}

//			clog << "R gRL" << endl;
			if ((i <= curVal && vals[i] != i) || (i > curVal && vals[i] != 0))
			{
				clog << Format("R1: Unexpected value in vals[%1]: %2", i, String(vals[i])) << endl;
				abort();
			}
//			clog << "R rRL" << endl;
			m_locker.releaseReadLock();
			doSomeYields();
		}

//		cout << "reader finished" << endl;
	}

private:
	ThreadBarrier m_bar;
	RWLocker& m_locker;
};

void doWriteOp()
{

//	clog << "W gWL" << endl;
	++curVal;

	if (curVal >= NUM_VALS)
	{
		curVal = 0;
		std::fill(vals, vals + NUM_VALS, 0);
	}


	if (vals[curVal] != 0)
	{
		clog << "W1: Unexpected value in vals[" << curVal << "]: " << vals[curVal] << endl;
		abort();
	}

	vals[curVal] = -1;
	doSomeYields();

	if (vals[curVal] != -1)
	{
		clog << "W2: Unexpected value in vals[" << curVal << "]: " << vals[curVal] << endl;
		abort();
	}

	vals[curVal] = curVal;

//			clog << "W rWL" << endl;
}


class WriterThread : public Runnable
{
public:
	WriterThread(const ThreadBarrier& bar, RWLocker& locker)
		: m_bar(bar)
		, m_locker(locker)
	{
	}

	void run()
	{
		m_bar.wait();
		for (int i = 0; i < NUM_VALS; ++i)
		{
			try
			{
				m_locker.getWriteLock(getRandomTimeout());
			}
			catch (TimeoutException& e)
			{
				doSomeYields();
				--i;
				continue; // timed out
			}
			doWriteOp();
			m_locker.releaseWriteLock();
			doSomeYields();
		}

//		cout << "writer finished" << endl;
	}

private:
	ThreadBarrier m_bar;
	RWLocker& m_locker;
};

class UpgraderThread : public Runnable
{
public:
	UpgraderThread(const ThreadBarrier& bar, RWLocker& locker)
		: m_bar(bar)
		, m_locker(locker)
	{
	}

	void run()
	{
		m_bar.wait();
		for (int i = 0; i < NUM_VALS; ++i)
		{
			try
			{
				m_locker.getReadLock(getRandomTimeout());
			}
			catch (TimeoutException& e)
			{
				doSomeYields();
				--i;
				continue; // timed out
			}
//			clog << "U gRL" << endl;

			if ((i <= curVal && vals[i] != i) || (i > curVal && vals[i] != 0))
			{
				// aCC doesn't accept passing volatile vars to Format... :-(
				clog << Format("U1: Unexpected value in vals[%1]: %2", String(curVal), String(vals[curVal])) << endl;
				abort();
			}

			doSomeYields();

			try
			{
				m_locker.getWriteLock(getRandomTimeout());
//				clog << "U gWL" << endl;
			}
			catch (DeadlockException&)
			{
//				clog << "Upgrader got DeadlockException" << endl;
//				clog << "U rRL" << endl;

				// another thread already is waiting for a write lock.  Release and try again.
				m_locker.releaseReadLock();
				doSomeYields();
				--i;
				continue;
			}
			catch (TimeoutException& e)
			{
				m_locker.releaseReadLock();
				doSomeYields();
				--i;
				continue; // timed out
			}

			doWriteOp();
			m_locker.releaseWriteLock();


			doSomeYields();

			if (vals[curVal] != curVal)
			{
				clog << Format("U4: Unexpected value in vals[%1]: %2", String(curVal), String(vals[curVal])) << endl;
				abort();
			}

//			clog << "U rRL" << endl;
			m_locker.releaseReadLock();
			doSomeYields();
		}

		//cout << "upgrader finished" << endl;
	}

private:
	ThreadBarrier m_bar;
	RWLocker& m_locker;
};

namespace // anonymous
{
	// Get the maximum number of processes for the current user.
	long getMaxProcs()
	{
#ifdef BLOCXX_WIN32
#pragma message(Reminder "TODO: implement it for Win!")

		long maxProcs = 0;
#else
		long sysconfValue = sysconf(_SC_CHILD_MAX);
		long maxProcs = sysconfValue;
		rlimit rl;
		rl.rlim_cur = rlim_t(0);
#ifdef RLIM_NPROC
		if( getrlimit(RLIMIT_NPROC, &rl) != -1 )
		{
			if( sysconfValue < 0 )
			{
				maxProcs = rl.rlim_cur;
			}
			else
			{
				maxProcs = std::min<rlim_t>(rl.rlim_cur, sysconfValue);
			}
		}
#endif
#endif
		return maxProcs;
	}
} // end anonymous namespace

AUTO_UNIT_TEST(RWLockerTestCases_testReadAndWrite)
{
	Timeout timeout = Timeout::relative(0.01); // short test is 1/100 of a second
	if (getenv("BLOCXXLONGTEST"))
	{
		timeout = Timeout::relative(300); // 5 minutes
	}

	TimeoutTimer timer(timeout);
	while (!timer.expired())
	{
		cout << '.' << flush;
		int NUM_READERS = 10;
		int NUM_UPGRADERS = 4;
		int NUM_WRITERS = 4;
		if (getenv("BLOCXXLONGTEST"))
		{
			// We don't want to exceed 75% (3/4) of the maximum user processes.
			// Going beyond this will cause thread creation to fail with EAGAIN on
			// some platforms because "make" and any other processes will easily
			// push it above the limit.
			int maxthreads = std::min<int>(30, getMaxProcs() / 4);

			NUM_READERS = RandomNumber(0, maxthreads).getNextNumber();
			NUM_UPGRADERS = RandomNumber(0, maxthreads).getNextNumber();
			NUM_WRITERS = RandomNumber(1, maxthreads).getNextNumber(); // have to have at least 1.
		}

		int TOTAL_THREADS = NUM_READERS + NUM_UPGRADERS + NUM_WRITERS;
		ThreadPool pool(ThreadPool::FIXED_SIZE, TOTAL_THREADS, TOTAL_THREADS);

		curVal = 0;
		std::fill(vals, vals + NUM_VALS, 0);
		RWLocker locker;
		ThreadBarrier startingLine(TOTAL_THREADS + 1); // +1 for the test thread
		for (int i = 0; i < NUM_WRITERS; ++i)
		{
			pool.addWork(new WriterThread(startingLine, locker));
		}
		for (int i = 0; i < NUM_UPGRADERS; ++i)
		{
			pool.addWork(new UpgraderThread(startingLine, locker));
		}
		for (int i = 0; i < NUM_READERS; ++i)
		{
			pool.addWork(new ReaderThread(startingLine, locker));
		}
		startingLine.wait(); // This should set them all running
		pool.shutdown();

		for (int i = 0; i <= curVal; ++i)
		{
			unitAssert(vals[i] == i);
		}
		for (int i = curVal + 1; i < NUM_VALS; ++i)
		{
			unitAssert(vals[i] == 0);
		}
		timer.loop();
	}
}

AUTO_UNIT_TEST(RWLockerTestCases_testRWLockerException)
{
	RWLocker rwl;
	unitAssertThrows(rwl.releaseReadLock());
	unitAssertThrows(rwl.releaseWriteLock());

	// try out crossed locks.
	rwl.getReadLock(Timeout::relative(0));
	rwl.getWriteLock(Timeout::relative(0));
	rwl.releaseReadLock();
	rwl.releaseWriteLock();

	// make sure it still throws.
	unitAssertThrows(rwl.releaseReadLock());
	unitAssertThrows(rwl.releaseWriteLock());
}
