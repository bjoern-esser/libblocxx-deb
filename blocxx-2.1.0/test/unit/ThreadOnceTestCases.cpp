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


#include "blocxx/BLOCXX_config.h"
#include "TestSuite.hpp"
#include "TestCaller.hpp"
#include "ThreadOnceTestCases.hpp"
#include "blocxx/ThreadOnce.hpp"
#include "blocxx/ThreadPool.hpp"
#include "blocxx/Runnable.hpp"
#include "blocxx/Thread.hpp"
#include "blocxx/Timeout.hpp"
#include "blocxx/TimeoutTimer.hpp"

using namespace blocxx;

void ThreadOnceTestCases::setUp()
{
}

void ThreadOnceTestCases::tearDown()
{
}

int g_value = 0;
OnceFlag once = BLOCXX_ONCE_INIT;


void initOnceValue()
{
    g_value++;
}

class OnceThread : public Runnable
{
	void run()
	{
		callOnce(once, initOnceValue);
	}
};

void ThreadOnceTestCases::testOnce()
{
    const int NUM_THREADS = 10;
    ThreadPool pool(ThreadPool::FIXED_SIZE, NUM_THREADS, ThreadPool::UNLIMITED_QUEUE_SIZE);
    for (int i = 0; i < NUM_THREADS; ++i)
	{
        pool.addWork(new OnceThread());
	}

    pool.shutdown();
	unitAssert(g_value == 1);

}

struct TestIncValue
{
	TestIncValue(Atomic_t& value)
	: m_value(value)
	{
	}

	void operator()()
	{
		AtomicInc(m_value);
	}
	Atomic_t& m_value;
};

class TestOnceThread2 : public Runnable
{
public:
	TestOnceThread2(Atomic_t& gate1, Atomic_t& gate2, Atomic_t& value, OnceFlag& once)
	: m_gate1(gate1)
	, m_gate2(gate2)
	, m_value(value)
	, m_once(once)
	{
	}
	void run()
	{
		AtomicInc(m_gate1);
		while (AtomicGet(m_gate2) == 0)
		{
			// do nothing -- busy wait.
		}

		callOnce(m_once, TestIncValue(m_value));
	}
private:
	Atomic_t& m_gate1;
	Atomic_t& m_gate2;
	Atomic_t& m_value;
	OnceFlag& m_once;
};

void ThreadOnceTestCases::testOnceRace()
{
	// This test can only fail on a dual proc box. We try and get two threads to run CallOnce at the same time.
	// I don't use any threading primitives because that would introduce too much serialization.
	Timeout timeout = Timeout::relative(0.01);
	if (getenv("OWLONGTEST"))
	{
		timeout = Timeout::relative(5 * 60);
	}
	TimeoutTimer timer(timeout);
	while (!timer.expired())
	{
		const int NUM_THREADS = 2;
		ThreadPool pool(ThreadPool::FIXED_SIZE, NUM_THREADS, ThreadPool::UNLIMITED_QUEUE_SIZE);
		Atomic_t gate1(0);
		Atomic_t gate2(0);
		Atomic_t value(0);
		OnceFlag once = BLOCXX_ONCE_INIT;
		for (int i = 0; i < NUM_THREADS; ++i)
		{
			pool.addWork(new TestOnceThread2(gate1, gate2, value, once));
		}
		while (AtomicGet(gate1) < NUM_THREADS)
		{
			Thread::yield();
		}
		AtomicInc(gate2);
		Thread::yield();

		pool.shutdown();
		unitAssert(AtomicGet(value) == 1);

		timer.loop();
	}

}

Test* ThreadOnceTestCases::suite()
{
	TestSuite *testSuite = new TestSuite ("ThreadOnce");

	ADD_TEST_TO_SUITE(ThreadOnceTestCases, testOnce);
	ADD_TEST_TO_SUITE(ThreadOnceTestCases, testOnceRace);

	return testSuite;
}

