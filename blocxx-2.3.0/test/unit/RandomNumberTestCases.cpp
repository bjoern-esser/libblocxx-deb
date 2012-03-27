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
#include "RandomNumberTestCases.hpp"

#define PROVIDE_AUTO_TEST_MAIN
#include "AutoTest.hpp"
#include "TestSuite.hpp"
#include "TestCaller.hpp"
AUTO_UNIT_TEST_SUITE_NAMED(RandomNumberTestCases, "RandomNumber");
#include "TestSuite.hpp"
#include "TestCaller.hpp"
#include "blocxx/RandomNumber.hpp"
#include "blocxx/Array.hpp"
#include "blocxx/Format.hpp"

using namespace blocxx;

void RandomNumberTestCases::setUp()
{
	RandomNumber::initRandomness();
}

void RandomNumberTestCases::tearDown()
{
	RandomNumber::saveRandomState();
}

// Need a fairly high test count to get a good test of randomness.
// But don't make it too high, or else the unit tests will be too slow.
const int MIN_TEST_COUNT = 1000;
const int LONG_TEST_COUNT = 100000;
const int MAX_TEST_COUNT = 10000000;

void RandomNumberTestCases::doTestRange(int low, int high)
{
	RandomNumber gen(low, high);
	bool saw_low = false;
	bool saw_high = false;
	int i = 0;
	while ((i < MIN_TEST_COUNT) || ((i < MAX_TEST_COUNT) && (!saw_low || !saw_high)))
	{
		Int32 rn = gen.getNextNumber();
		unitAssert(rn >= low && rn <= high);
		if (rn == low)
			saw_low = true;
		if (rn == high)
			saw_high = true;
		++i;
	}
	unitAssert(saw_low);
	unitAssert(saw_high);
}

void RandomNumberTestCases::testRandomNumbers()
{
	// test default.  range: 0-RAND_MAX
	RandomNumber g1;
	int count = MIN_TEST_COUNT;
	if (getenv("BLOCXXLONGTEST"))
	{
		count = LONG_TEST_COUNT;
	}
	for (int i = 0; i < count; ++i)
	{
		Int32 rn = g1.getNextNumber();
		unitAssert(rn >= 0 && rn <= RAND_MAX);
	}

	// test 0-10
	doTestRange(0, 10);

	// test 1000001-1000010
	doTestRange(1000001, 1000010);

	// test 0-1
	doTestRange(0, 1);

}


void RandomNumberTestCases::testMersenneTwister()
{
	// NOTE: This test is not specific to the mersenne twister.  It could be
	// modified to work with any random number generator.
	const unsigned iterations = 500000;
	Array<unsigned> buckets(1000, 0u);

	MersenneTwisterRandomNumber generator;

	UInt64 sumTotal = 0;
	for( unsigned i = 0; i < iterations; ++i )
	{
		UInt32 value = generator.getNextNumber(buckets.size() - 1);

		sumTotal += value;

		unitAssert(value < buckets.size());

		++buckets[value];
	}

	// The average number per bucket (if evenly distributed) should be iterations / buckets.size().
	double expected = double(iterations) / buckets.size();


	unsigned exceeded = 0;
	double chiSquared = 0;
	for(Array<unsigned>::const_iterator iter = buckets.begin(); iter != buckets.end(); ++iter )
	{
		double difference = (*iter - expected);
		chiSquared += difference * difference / expected;

		if( difference > 0 )
		{
			++exceeded;
		}
	}

	// If it is uniformly distributed, it should exceed the expected amount 50%
	// of the time.  A good distribution is between 20% and 80%.  The mersenne
	// twister (a good distribution) shoud actually be between 40 and 60%
	double exceedPercent = double(exceeded) / buckets.size();
	double mean = double(sumTotal) / iterations;

	std::cout << Format("\nMersenne Twister: Samples=%1, Values=%2, ChiSquared=%3, Mean=%4 (ExpectedMean=%5) ExceedPercent=%6",
		iterations, buckets.size(), chiSquared, mean, expected, exceedPercent * 100) << std::endl;

	unitAssertLess(exceedPercent, 0.80);
	unitAssertGreater(exceedPercent, 0.20);

	unitAssertLess(mean, expected * 1.1);
	unitAssertGreater(mean, expected * 0.9);
}

Test* RandomNumberTestCases::suite()
{
	TestSuite *testSuite = new TestSuite ("RandomNumber");

	ADD_TEST_TO_SUITE(RandomNumberTestCases, testRandomNumbers);
	ADD_TEST_TO_SUITE(RandomNumberTestCases, testMersenneTwister);

	return testSuite;
}

