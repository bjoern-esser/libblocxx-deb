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
#define PROVIDE_AUTO_TEST_MAIN
#include "AutoTest.hpp"


#include "blocxx/AtomicOps.hpp"
#include "blocxx/NonRecursiveMutex.hpp"
#include "blocxx/NonRecursiveMutexLock.hpp"
#include "blocxx/Runnable.hpp"
#include "blocxx/ThreadPool.hpp"
#include "blocxx/ThreadBarrier.hpp"
#include "blocxx/List.hpp"

using namespace blocxx;

AUTO_UNIT_TEST(AtomicOpsTestCases_testInc)
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

AUTO_UNIT_TEST(AtomicOpsTestCases_testDec)
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

AUTO_UNIT_TEST(AtomicOpsTestCases_testDecAndTest)
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

AUTO_UNIT_TEST(AtomicOpsTestCases_testIncAndGet)
{
	// A very simple (too simple) test of the AtomicIncAndGet function
	int count = 1000;
	Atomic_t val(0);
	unitAssertEquals( AtomicGet(val), 0 );
	for(int i = 0; i < count; ++i )
	{
		unitAssertEquals(AtomicIncAndGet(val), i + 1);
	}
	unitAssertEquals( AtomicGet(val), count );
}

AUTO_UNIT_TEST(AtomicOpsTestCases_testDecAndGet)
{
	// A very simple (too simple) test of the AtomicDecAndGet function
	int count = 1000;
	Atomic_t val(count);
	unitAssertEquals( AtomicGet(val), count );
	for(int i = 0; i < count; ++i )
	{
		unitAssertEquals(AtomicDecAndGet(val), count - i - 1);
	}
	unitAssertEquals( AtomicGet(val), 0 );
}

namespace // anonymous
{
	typedef Array<bool> ResultTracker;
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

	class IncAndGetThread : public Runnable
	{
		int my_amount;
		ThreadBarrier& barrier;
		ResultTracker & returned;
	public:
		IncAndGetThread(int amount, ThreadBarrier& tb, ResultTracker & returned) : my_amount(amount), barrier(tb), returned(returned) { }

		void run()
		{
			barrier.wait();
			for(int i = 0; i < my_amount; ++i )
			{
				int val = AtomicIncAndGet(VALUE) - 1;
				if (val >= 0 && (unsigned int)val < returned.size())
				{
					returned[val] = true;
				}
			}
			barrier.wait();
		}
	};

	class DecAndGetThread : public Runnable
	{
		int my_amount;
		ThreadBarrier& barrier;
		ResultTracker & returned;
	public:
		DecAndGetThread(int amount, ThreadBarrier& tb, ResultTracker & returned) : my_amount(amount), barrier(tb), returned(returned) { }

		void run()
		{
			barrier.wait();
			for(int i = 0; i < my_amount; ++i )
			{
				int val = AtomicDecAndGet(VALUE);
				if (val >= 0 && (unsigned int)val < returned.size())
				{
					returned[val] = true;
				}
			}
			barrier.wait();
		}
	};

} // end anonymous namespace

AUTO_UNIT_TEST(AtomicOpsTestCases_testAtomicnessInc)
{
	int thread_count = 6;
	int iterations = 10000;
	if (getenv("BLOCXXLONGTEST"))
	{
		iterations = 10000000;
	}

	// Test atomicIncrement
	ThreadPool incrementerPool(ThreadPool::FIXED_SIZE, thread_count, thread_count, NullLogger(), "IncrementerPool");
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

AUTO_UNIT_TEST(AtomicOpsTestCases_testAtomicnessDec)
{
	int thread_count = 6;
	int iterations = 10000;
	if (getenv("BLOCXXLONGTEST"))
	{
		iterations = 10000000;
	}

	// Test atomicDecrement
	ThreadPool decrementerPool(ThreadPool::FIXED_SIZE, thread_count, thread_count, NullLogger(), "DecrementerPool");
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

AUTO_UNIT_TEST(AtomicOpsTestCases_testAtomicnessIncAndGet)
{
	int thread_count = 6;
	int iterations = 10000;
	List<ResultTracker> results;
	// Long test would need to be able to allocate 6 chunks each of ~7.5 MB for each ResultTracker
	if (getenv("BLOCXXLONGTEST"))
	{
		iterations = 10000000;
	}

	// Test atomic Increment and get
	ThreadPool pool(ThreadPool::FIXED_SIZE, thread_count, thread_count, NullLogger(), "IncAndGetPool");
	ThreadBarrier barrier(thread_count + 1);

	for( int i = 0; i < 6; ++i )
	{
		results.push_back(ResultTracker(iterations * thread_count, false));
		pool.addWork(new IncAndGetThread(iterations, barrier, results.back()));
	}

	VALUE = Atomic_t(0);
	barrier.wait(); // Kick off the threads.
	barrier.wait(); // Wait for threads to finish
	pool.shutdown();

	// Make sure none of the threads stomped on any other.
	unitAssertEquals(AtomicGet(VALUE), thread_count * iterations);
	// Only one thread should have received a result for each of the possible values
	for (int i = 0; i < thread_count * iterations; ++i)
	{
		int count = 0;
		for(List<ResultTracker>::iterator iter = results.begin(); iter != results.end(); ++iter)
		{
			count += (*iter)[i] ? 1 : 0;
		}
		unitAssertEquals(count, 1);
	}
}

AUTO_UNIT_TEST(AtomicOpsTestCases_testAtomicnessDecAndGet)
{
	int thread_count = 6;
	int iterations = 10000;
	List<ResultTracker> results;
	// Long test would need to be able to allocate 6 chunks each of ~7.5 MB for each ResultTracker
	if (getenv("BLOCXXLONGTEST"))
	{
		iterations = 10000000;
	}

	// Test atomic Decrement and get
	ThreadPool pool(ThreadPool::FIXED_SIZE, thread_count, thread_count, NullLogger(), "DecAndGetPool");
	ThreadBarrier barrier(thread_count + 1);

	for( int i = 0; i < 6; ++i )
	{
		results.push_back(ResultTracker(iterations * thread_count, false));
		pool.addWork(new DecAndGetThread(iterations, barrier, results.back()));
	}

	VALUE = Atomic_t(thread_count * iterations);
	barrier.wait(); // Kick off the threads.
	barrier.wait(); // Wait for threads to finish
	pool.shutdown();

	// Make sure none of the threads stomped on any other.
	unitAssertEquals(AtomicGet(VALUE), 0);
	// Only one thread should have received a result for each of the possible values
	for (int i = 0; i < thread_count * iterations; ++i)
	{
		int count = 0;
		for(List<ResultTracker>::iterator iter = results.begin(); iter != results.end(); ++iter)
		{
			count += (*iter)[i] ? 1 : 0;
		}
		unitAssertEquals(count, 1);
	}
}
