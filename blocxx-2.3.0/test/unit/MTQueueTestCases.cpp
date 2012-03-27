/*******************************************************************************
* Copyright (C) 2008 Quest Software, Inc. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of source code must retain the above copyright notice,
*    this list of conditions and the following disclaimer.
*
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
*
*  - Neither the name of Quest Software, Inc. nor the names of its
*    contributors may be used to endorse or promote products derived from this
*    software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL Vintela, Inc. OR THE CONTRIBUTORS
* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

#include "blocxx/BLOCXX_config.h"
#define PROVIDE_AUTO_TEST_MAIN
#include "AutoTest.hpp"

#include "blocxx/MTQueue.hpp"
#include "blocxx/ThreadBarrier.hpp"
#include "blocxx/Thread.hpp"
#include "blocxx/Timeout.hpp"

using namespace blocxx;

AUTO_UNIT_TEST(MTQueueTestCases_testPushAndPopOneItem)
{
	MTQueue<int> q(1);
	int someNumber = 3297;
	int n = 0;

	q.pushBack(someNumber);
	MTQueueEnum::EPopResult res = q.popFront(Timeout::relative(1.0), n);

	unitAssertEquals(MTQueueEnum::E_VALUE, res);
	unitAssertEquals(someNumber, n);

	someNumber = 29347;
	n = someNumber;
	res = q.popFront(Timeout::relative(0.01), n);

	unitAssertEquals(MTQueueEnum::E_TIMED_OUT, res);
	unitAssertEquals(someNumber, n);
}

AUTO_UNIT_TEST(MTQueueTestCases_testItemsPoppedInOrderPushedNotInterleaved)
{
	MTQueue<int> q(2);
	int num1 = 32947;
	int num2 = -23948;
	int n1 = 0;
	int n2 = 0;

	q.pushBack(num1);
	q.pushBack(num2);
	MTQueueEnum::EPopResult res1 = q.popFront(Timeout::relative(100.0), n1);
	MTQueueEnum::EPopResult res2 = q.popFront(Timeout::relative(111.1), n2);

	unitAssertEquals(MTQueueEnum::E_VALUE, res1);
	unitAssertEquals(num1, n1);
	unitAssertEquals(MTQueueEnum::E_VALUE, res2);
	unitAssertEquals(num2, n2);

	int const someNumber = 29347;
	int n = someNumber;
	MTQueueEnum::EPopResult res = q.popFront(Timeout::relative(0.01), n);

	unitAssertEquals(MTQueueEnum::E_TIMED_OUT, res);
	unitAssertEquals(someNumber, n);
}

AUTO_UNIT_TEST(MTQueueTestCases_testItemsPoppedInOrderPushedYesInterleaved)
{
	MTQueue<int> q(1);
	int num1 = 32947;
	int num2 = -23948;
	int n1 = 0;
	int n2 = 0;

	q.pushBack(num1);
	MTQueueEnum::EPopResult res1 = q.popFront(Timeout::relative(100.0), n1);
	q.pushBack(num2);
	MTQueueEnum::EPopResult res2 = q.popFront(Timeout::relative(111.1), n2);

	unitAssertEquals(MTQueueEnum::E_VALUE, res1);
	unitAssertEquals(num1, n1);
	unitAssertEquals(MTQueueEnum::E_VALUE, res2);
	unitAssertEquals(num2, n2);

	int const someNumber = 29347;
	int n = someNumber;
	MTQueueEnum::EPopResult res = q.popFront(Timeout::relative(0.01), n);

	unitAssertEquals(MTQueueEnum::E_TIMED_OUT, res);
	unitAssertEquals(someNumber, n);
}

AUTO_UNIT_TEST(MTQueueTestCases_testShutdownClearsQueue)
{
	MTQueue<int> q(10);
	int num1 = 10923;
	int n1 = 0;

	q.pushBack(num1);
	q.shutdown();
	MTQueueEnum::EPopResult res1 = q.popFront(Timeout::infinite, n1);

	unitAssertEquals(MTQueueEnum::E_SHUT_DOWN, res1);
}

AUTO_UNIT_TEST(MTQueueTestCases_testShutdownWhenEmptyQueue)
{
	MTQueue<int> q(10);
	int n1 = 0;

	q.shutdown();
	MTQueueEnum::EPopResult res1 = q.popFront(Timeout::infinite, n1);

	unitAssertEquals(MTQueueEnum::E_SHUT_DOWN, res1);
}

AUTO_UNIT_TEST(MTQueueTestCases_testPushAfterShutdownIsIgnored)
{
	MTQueue<int> q(1);
	int n1 = 0;

	q.shutdown();
	q.pushBack(293);
	q.pushBack(324972);
	q.pushBack(34827);
	MTQueueEnum::EPopResult res1 = q.popFront(Timeout::infinite, n1);

	unitAssertEquals(MTQueueEnum::E_SHUT_DOWN, res1);
}

AUTO_UNIT_TEST(MTQueueTestCases_testPopTimeout)
{
	MTQueue<int> q(1);
	int n = 0;

	DateTime wakeup = DateTime::getCurrent();
	wakeup.addMicroseconds(50000);

	MTQueueEnum::EPopResult res = q.popFront(Timeout::absolute(wakeup), n);

	// No reliable comparisons cam be made with the times because the current
	// time may be a few microseconds before the timeout or some indeterminate
	// number of seconds later, depending on the process scheduler.
	unitAssert(res == MTQueueEnum::E_TIMED_OUT);
}

namespace
{
	struct PushThread : public Thread
	{
		PushThread(MTQueue<int> & q, unsigned count)
		: m_barrier(2),
		  m_q(q),
		  m_count(count),
		  m_index(0)
		{
		}

		void sync()
		{
			m_barrier.wait();
			Thread::sleep(10);
		}

		unsigned index()
		{
			NonRecursiveMutexLock lock(m_mutex);
			return m_index;
		}

	private:
		void inc_index()
		{
			NonRecursiveMutexLock lock(m_mutex);
			++m_index;
		}

		virtual Int32 run()
		{
			m_barrier.wait();
			for ( ; index() < m_count; inc_index())
			{
				m_q.pushBack(0);
			}

			return 0;
		}

		NonRecursiveMutex m_mutex;
		ThreadBarrier m_barrier;
		MTQueue<int> & m_q;
		unsigned m_count;
		int m_index;
	};
}

AUTO_UNIT_TEST(MTQueueTestCases_testPushBlocksWhenQueueFull)
{
	int n;
	MTQueue<int> q;
	q.setMaxQueueSize(1);

	PushThread th1(q, 2);
	th1.start();

	PushThread th2(q, 1);
	th2.start();

	// Put initial value on queue so that it is full
	q.pushBack(0);

	// Verify that thread 1 blocks trying to push to queue
	th1.sync();
	unitAssertEquals(0U, th1.index());

	// Pop value and wait for thread 1 to unblock
	q.popFront(Timeout::infinite, n);
	while (th1.index() == 0)
	{
		Thread::sleep(10);
	}

	// Verify that both thread 1 and thread 2 are blocked trying to push
	th2.sync();
	unitAssertEquals(0U, th2.index()); // th2 blocked
	unitAssertEquals(1U, th1.index()); // th1 blocked

	// Pop value and wait for one of the threads to unblock
	q.popFront(Timeout::infinite, n);
	while (th1.index() == 1 && th2.index() == 0)
	{
		Thread::sleep(10);
	}

	// Pop value and wait for the other thread to unblock
	q.popFront(Timeout::infinite, n);
	while (th1.index() == 1 || th2.index() == 0)
	{
		Thread::sleep(10);
	}
}

AUTO_UNIT_TEST(MTQueueTestCases_testShutdownUnblocksAllPushes)
{
	int n;
	MTQueue<int> q;
	q.setMaxQueueSize(1);

	PushThread th1(q, 2);
	th1.start();

	PushThread th2(q, 1);
	th2.start();

	// Put initial value on queue so that it is full
	q.pushBack(0);

	// Verify that threads block trying to push to queue
	th1.sync();
	th2.sync();
	unitAssertEquals(0U, th1.index());
	unitAssertEquals(0U, th2.index());

	// Call shutdown() and wait for threads to unblock
	q.shutdown();
	while (th1.index() < 2 && th2.index() < 1)
	{
		Thread::sleep(10);
	}
}

namespace
{
	struct PopThread : public Thread
	{
		PopThread(MTQueue<int> & q, unsigned numPops)
		: m_barrier(2),
		  m_q(q),
		  m_cnt(0),
		  m_numPops(numPops)
		{
		}

		void sync()
		{
			m_barrier.wait();
			Thread::sleep(10);
		}

		unsigned count()
		{
			NonRecursiveMutexLock lock(m_mutex);
			return m_cnt;
		}

	private:
		void inc_count()
		{
			NonRecursiveMutexLock lock(m_mutex);
			++m_cnt;
		}

		virtual Int32 run()
		{
			m_barrier.wait();
			MTQueueEnum::EPopResult res = MTQueueEnum::E_VALUE;
			MTQueueEnum::EPopResult shutDown = MTQueueEnum::E_SHUT_DOWN;
			int n;
			for ( ; count() < m_numPops && res != shutDown; inc_count())
			{
				res = m_q.popFront(Timeout::infinite, n);
			}

			return 0;
		}

		NonRecursiveMutex m_mutex;
		ThreadBarrier m_barrier;
		MTQueue<int> & m_q;
		unsigned m_cnt;
		bool m_numPops;
	};
}

AUTO_UNIT_TEST(MTQueueTestCases_testPopBlocksWhenQueueEmpty)
{
	MTQueue<int> q;
	q.setMaxQueueSize(1);

	PopThread th(q, 1);
	th.start();
	th.sync();

	// Verify thread blocked
	unitAssertEquals(0U, th.count());

	q.pushBack(0);

	// Wait for thread to unblock
	th.join();
	unitAssertEquals(1U, th.count());
}

AUTO_UNIT_TEST(MTQueueTestCases_testShutdownUnblocksAllPops)
{
	MTQueue<int> q;
	q.setMaxQueueSize(1);

	PopThread th1(q, 1);
	th1.start();
	th1.sync();

	PopThread th2(q, 1);
	th2.start();
	th2.sync();

	// Verify both threads blocked
	unitAssertEquals(0U, th1.count());
	unitAssertEquals(0U, th2.count());

	q.shutdown();

	// Wait for threads to unblock and see shutdown
	th1.join();
	th2.join();
	unitAssertEquals(1U, th1.count());
	unitAssertEquals(1U, th2.count());
}
