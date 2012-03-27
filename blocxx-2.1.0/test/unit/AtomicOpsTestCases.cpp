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
 * @author Kevin Harris
 */

#include "blocxx/BLOCXX_config.h"
#include "TestSuite.hpp"
#include "TestCaller.hpp"
#include "AtomicOpsTestCases.hpp"
#include "blocxx/AtomicOps.hpp"

#include "blocxx/NonRecursiveMutex.hpp"
#include "blocxx/NonRecursiveMutexLock.hpp"

#include "blocxx/Runnable.hpp"
#include "blocxx/ThreadPool.hpp"
#include "blocxx/ThreadBarrier.hpp"

using namespace blocxx;

void AtomicOpsTestCases::setUp()
{
}

void AtomicOpsTestCases::tearDown()
{
}

void AtomicOpsTestCases::testInc()
{
	// A very simple (too simple) test of the AtomicInc function
	int count = 1000;
	Atomic_t val(0);
	unitAssertEquals( AtomicGet(val), 0 );
	for(int i = 0; i < count; ++i )
	{
		AtomicInc(val);
	}
	unitAssertEquals( AtomicGet(val), count );
}

void AtomicOpsTestCases::testDec()
{
	// A very simple (too simple) test of the AtomicDec function
	int count = 1000;
	Atomic_t val(count);
	unitAssertEquals( AtomicGet(val), count );
	for(int i = 0; i < count; ++i )
	{
		AtomicDec(val);
	}
	unitAssertEquals( AtomicGet(val), 0 );
}

void AtomicOpsTestCases::testDecAndTest()
{
	// A very simple (too simple) test of the AtomicDecAndTest function
	int count = 1000;
	Atomic_t val(count);
	unitAssertEquals( AtomicGet(val), count );
	int iterations = 0;
	do
	{
		++iterations;
	}
	while( !AtomicDecAndTest(val) );
	unitAssertEquals( AtomicGet(val), 0 );
	unitAssertEquals( iterations, count );
}

namespace // anonymous
{
	Atomic_t VALUE;
	NonRecursiveMutex MUTEX;
	int DEC_AND_TEST_COUNT;

	class IncrementerThread : public Runnable
	{
		int my_amount;
		ThreadBarrier& barrier;
	public:
		IncrementerThread(int amount, ThreadBarrier& tb) : my_amount(amount), barrier(tb) { }

		void run()
		{
			barrier.wait();
			for(int i = 0; i < my_amount; ++i )
			{
				AtomicInc(VALUE);
			}
		}
	};

	class DecrementerThread : public Runnable
	{
		int my_amount;
		ThreadBarrier& barrier;
	public:
		DecrementerThread(int amount, ThreadBarrier& tb) : my_amount(amount), barrier(tb) { }

		void run()
		{
			barrier.wait();
			for(int i = 0; i < my_amount; ++i )
			{
				if( AtomicDecAndTest(VALUE) )
				{
					NonRecursiveMutexLock ml(MUTEX);
					++DEC_AND_TEST_COUNT;
				}
			}
		}
	};

} // end anonymous namespace

void AtomicOpsTestCases::testAtomicnessInc()
{
	int thread_count = 6;
	int iterations = 10000;
	if (getenv("OWLONGTEST"))
	{
		iterations = 10000000;
	}

	// Test atomicIncrement
	ThreadPool incrementerPool(ThreadPool::FIXED_SIZE, thread_count, thread_count, "IncrementerPool");
	ThreadBarrier incrementerBarrier(thread_count + 1);
	for( int i = 0; i < 6; ++i )
	{
		incrementerPool.addWork(new IncrementerThread(iterations, incrementerBarrier));
	}

	VALUE = Atomic_t(0);

	incrementerBarrier.wait(); // Kick off the threads.
	incrementerPool.shutdown();

	// Make sure none of the threads stomped on any other.
	unitAssertEquals(AtomicGet(VALUE), thread_count * iterations);
}

void AtomicOpsTestCases::testAtomicnessDec()
{
	int thread_count = 6;
	int iterations = 10000;
	if (getenv("OWLONGTEST"))
	{
		iterations = 10000000;
	}

	// Test atomicDecrement
	ThreadPool decrementerPool(ThreadPool::FIXED_SIZE, thread_count, thread_count, "DecrementerPool");
	ThreadBarrier decrementerBarrier(thread_count + 1);

	for( int i = 0; i < 6; ++i )
	{
		decrementerPool.addWork(new DecrementerThread(iterations, decrementerBarrier));
	}

	DEC_AND_TEST_COUNT = 0;
	VALUE = Atomic_t(thread_count * (iterations - 1)); // this does -1 to ensure that every thread has a chance (probably very slim) of crossing the zero point.
	decrementerBarrier.wait(); // Kick off the threads.
	decrementerPool.shutdown();

	// Make sure none of the threads stomped on any other.
	unitAssertEquals(AtomicGet(VALUE), -thread_count);
	// Only one thread should have had AtomicDecAndTest return true.
	unitAssertEquals(DEC_AND_TEST_COUNT, 1);
}


Test* AtomicOpsTestCases::suite()
{
	TestSuite *testSuite = new TestSuite ("AtomicOps");

	ADD_TEST_TO_SUITE(AtomicOpsTestCases, testInc);
	ADD_TEST_TO_SUITE(AtomicOpsTestCases, testDec);
	ADD_TEST_TO_SUITE(AtomicOpsTestCases, testDecAndTest);
	ADD_TEST_TO_SUITE(AtomicOpsTestCases, testAtomicnessInc);
	ADD_TEST_TO_SUITE(AtomicOpsTestCases, testAtomicnessDec);

	return testSuite;
}

