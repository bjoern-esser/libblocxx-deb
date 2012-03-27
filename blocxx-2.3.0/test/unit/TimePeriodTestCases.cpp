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
#define PROVIDE_AUTO_TEST_MAIN
#include "AutoTest.hpp"

#include "blocxx/TimePeriod.hpp"
#include "blocxx/TimeUtils.hpp"
#include "blocxx/String.hpp"

using namespace blocxx;

namespace BLOCXX_NAMESPACE
{
	namespace Time
	{
		static std::ostream& operator<<(std::ostream& o, const Time::TimePeriod& tp)
		{
			return o << tp.toString();
		}

		static std::ostream& operator<<(std::ostream& o, const Time::TimeDuration& td)
		{
			return o << td.toString();
		}
	}
}

//
// For these comparisons, there are 7 comparison conditions
// (ignoring invalid comparisons):
//
// 1.  a |--------|
//     b          |--------|
//
// 2.  a          |--------|
//     b |--------|
//
// 3.  a |--------|
//     b      |--------|
//
// 4.  a      |--------|
//     b |--------|
//
// 5.  a |--------|
//     b |--------|
//
// 6.  a |----------------|
//     b      |--------|
//
// 7.  a      |--------|
//     b |----------------|
//


AUTO_UNIT_TEST(TimePeriodTestCases_testNonOverlapping)
{
	Time::TimePeriod invalid = Time::invalidTimePeriod();

	{
		// Case 1...
		Time::TimePeriod p1(DateTime(1990,1,1), DateTime(1999,12,31));
		Time::TimePeriod p2(DateTime(2000,1,1), DateTime(2009,12,31));

		unitAssert(p1 != invalid);
		unitAssert(p2 != invalid);

		unitAssert(!intersect(p1,p2));
		unitAssert(intersection(p1,p2) == invalid);
		unitAssert(merge(p1,p2) == invalid);
		unitAssert(span(p1,p2) == Time::TimePeriod(p1.begin(), p2.end()));

		// 10 years minus a day, plus 2 leap years (1994, 1998)
		unitAssertEquals(365 * 10 - 1 + 2, p1.length().relativeDays());

		// 10 years minus a day, plus 3 leap years (2000, 2004, 2008)
		unitAssertEquals(365 * 10 - 1 + 3, p2.length().relativeDays());


		// Reversed.
		// Case 2...
		std::swap(p1, p2);

		unitAssert(p1 != invalid);
		unitAssert(p2 != invalid);

		unitAssert(p2 != p1);
		unitAssert(!(p2 == p1));

		unitAssert(!intersect(p2,p1));
		unitAssert(intersection(p2,p1) == invalid);
		unitAssert(merge(p2,p1) == invalid);
		unitAssert(span(p2,p1) == Time::TimePeriod(p2.begin(), p1.end()));

		// 10 years minus a day, plus 2 leap years (1994, 1998)
		unitAssertEquals(365 * 10 - 1 + 2, p2.length().relativeDays());

		// 10 years minus a day, plus 3 leap years (2000, 2004, 2008)
		unitAssertEquals(365 * 10 - 1 + 3, p1.length().relativeDays());
	}

	{
		// Case 1...
		Time::TimePeriod p1(DateTime(2008,1,1), Time::days(30));
		Time::TimePeriod p2(DateTime(2008,2,1), Time::days(29));

		unitAssert(p1 != invalid);
		unitAssert(p2 != invalid);
		unitAssert(p1 != p2);
		unitAssert(!(p1 == p2));

		unitAssert(!intersect(p1,p2));
		unitAssert(intersection(p1,p2) == invalid);
		unitAssert(merge(p1,p2) == invalid);
		unitAssert(span(p1,p2) == Time::TimePeriod(p1.begin(), p2.end()));

		unitAssertEquals(30, p1.length().relativeDays());
		unitAssertEquals(29, p2.length().relativeDays());
	}

	{
		// A very short period.
		// NOTE: The end of P1 is the same as the start of P2, but the end is non-inclusive.
		// Case 1...
		Time::TimePeriod p1(DateTime(2008,1,1,0,0,0,0), Time::microseconds(2));
		Time::TimePeriod p2(DateTime(2008,1,1,0,0,0,2), Time::microseconds(2));

		unitAssert(p1 != invalid);
		unitAssert(p2 != invalid);
		unitAssert(p1 != p2);
		unitAssert(!(p1 == p2));

		unitAssert(!intersect(p1,p2));
		unitAssertEquals(invalid, intersection(p1,p2));

		// Merge will include the endpoints, so merge and span should be identical.
		unitAssertEquals(span(p1,p2), merge(p1,p2));

		unitAssertEquals(Time::TimePeriod(p1.begin(), p2.end()), span(p1,p2));

		unitAssertEquals(Time::microseconds(4), span(p1,p2).length());
	}
}

AUTO_UNIT_TEST(TimePeriodTestCases_testOverlapping)
{
	Time::TimePeriod invalid = Time::invalidTimePeriod();

	{
		Time::TimePeriod p1(DateTime(1990,1,1), DateTime(1995,1,1));
		Time::TimePeriod p2(DateTime(1995,1,1), DateTime(2000,1,1));
		Time::TimePeriod p3(DateTime(1993,1,1), DateTime(1998,1,1));
		Time::TimePeriod p4(DateTime(1990,1,1), DateTime(2000,1,1));

		Time::TimePeriod p5(DateTime(1990,1,1), DateTime(1994,1,1));
		Time::TimePeriod p6(DateTime(1996,1,1), DateTime(2000,1,1));

		unitAssert(!Time::intersect(p1, p2)); // case 1 (shared)
		unitAssert(!Time::intersect(p5, p6)); // case 1 (disjoint)
		unitAssert(!Time::intersect(p2, p1)); // case 2 (shared)
		unitAssert(!Time::intersect(p6, p5)); // case 2 (disjoint)

		unitAssert(Time::intersect(p1, p3)); // case 3
		unitAssert(Time::intersect(p3, p1)); // case 4
		unitAssert(Time::intersect(p1, p1)); // case 5

		unitAssert(Time::intersect(p4, p1)); // case 6 (boundary)
		unitAssert(Time::intersect(p4, p2)); // case 6 (boundary)
		unitAssert(Time::intersect(p4, p3)); // case 6
		unitAssert(Time::intersect(p4, p5)); // case 6 (boundary)
		unitAssert(Time::intersect(p4, p6)); // case 6 (boundary)

		unitAssert(Time::intersect(p1, p4)); // case 7 (boundary)
		unitAssert(Time::intersect(p2, p4)); // case 7 (boundary)
		unitAssert(Time::intersect(p3, p4)); // case 7
		unitAssert(Time::intersect(p5, p4)); // case 7 (boundary)
		unitAssert(Time::intersect(p6, p4)); // case 7 (boundary)


		unitAssertEquals(Time::intersection(p4, p1), p1);
		unitAssertEquals(Time::intersection(p4, p2), p2);
		unitAssertEquals(Time::intersection(p4, p3), p3);
		unitAssertEquals(Time::intersection(p4, p4), p4);
		unitAssertEquals(Time::intersection(p4, p5), p5);
		unitAssertEquals(Time::intersection(p4, p6), p6);

		unitAssertEquals(Time::intersection(p1, p4), p1);
		unitAssertEquals(Time::intersection(p2, p4), p2);
		unitAssertEquals(Time::intersection(p3, p4), p3);
		unitAssertEquals(Time::intersection(p4, p4), p4);
		unitAssertEquals(Time::intersection(p5, p4), p5);
		unitAssertEquals(Time::intersection(p6, p4), p6);
	}

	{
		// Overlap by 1 microsecond at the end of p1.
		// Case 5...
		Time::TimePeriod p1(DateTime(1990,1,1), DateTime(2000,1,1,0,0,0,1));
		Time::TimePeriod p2(p1);

		unitAssert(p1 != invalid);
		unitAssert(p2 != invalid);

		unitAssert(p1 == p2);
		unitAssert(!(p1 != p2));

		unitAssert(intersect(p1,p2));
		unitAssert(intersection(p1,p2) == p1);
		unitAssert(merge(p1,p2) == p1);
		unitAssert(span(p1,p2) == p1);
	}

	{
		// Overlap by 1 microsecond at the end of p1.
		// Case 3...
		Time::TimePeriod p1(DateTime(1990,1,1), DateTime(2000,1,1,0,0,0,1));
		Time::TimePeriod p2(DateTime(2000,1,1), DateTime(2009,12,31));

		unitAssert(p1 != invalid);
		unitAssert(p2 != invalid);

		unitAssert(p1 != p2);
		unitAssert(!(p1 == p2));

		unitAssert(intersect(p1,p2));
		unitAssert(intersection(p1,p2) == Time::TimePeriod(DateTime(2000,1,1), Time::microseconds(1)));
		unitAssert(merge(p1,p2) == Time::TimePeriod(p1.begin(), p2.end()));
		unitAssert(span(p1,p2) == Time::TimePeriod(p1.begin(), p2.end()));

		unitAssert(intersection(p1,p2).length() == Time::microseconds(1));
	}

	{
		// Overlap by 1 microsecond at the end of p2.
		// Case 4...
		Time::TimePeriod p1(DateTime(2000,1,1), DateTime(2009,12,31));
		Time::TimePeriod p2(DateTime(1990,1,1), DateTime(2000,1,1,0,0,0,1));

		unitAssert(p1 != invalid);
		unitAssert(p2 != invalid);

		unitAssert(p1 != p2);
		unitAssert(!(p1 == p2));

		unitAssert(intersect(p1,p2));
		unitAssert(intersection(p1,p2) == Time::TimePeriod(DateTime(2000,1,1), Time::microseconds(1)));
		unitAssert(merge(p1,p2) == Time::TimePeriod(p2.begin(), p1.end()));
		unitAssert(span(p1,p2) == Time::TimePeriod(p2.begin(), p1.end()));

		unitAssert(intersection(p1,p2).length() == Time::microseconds(1));
	}

	{
		// Completely contained in...
		// Case 6...
		Time::TimePeriod p1 = Time::TimePeriod(DateTime(), Time::days(31));
		Time::TimePeriod p2 = Time::TimePeriod(DateTime() + Time::days(4), Time::days(23));

		unitAssert(p1 != invalid);
		unitAssert(p2 != invalid);

		unitAssert(p1 != p2);
		unitAssert(!(p1 == p2));

		unitAssert(intersect(p1,p2));
		unitAssertEquals(intersection(p1,p2), p2);
		unitAssertEquals(merge(p1,p2), p1);
		unitAssertEquals(span(p1,p2), p1);

		unitAssert(intersection(p1,p2).length() == Time::days(23));

		// Reversed
		// Case 7...
		std::swap(p1,p2);
		unitAssert(p1 != p2);
		unitAssert(!(p1 == p2));

		unitAssert(intersect(p1,p2));
		unitAssertEquals(intersection(p1,p2), p1);
		unitAssertEquals(merge(p1,p2), p2);
		unitAssertEquals(span(p1,p2), p2);

		unitAssert(intersection(p1,p2).length() == Time::days(23));
	}
}

AUTO_UNIT_TEST(TimePeriodTestCases_testInvalid)
{
	Time::TimePeriod invalid = Time::invalidTimePeriod();

	unitAssertEquals(Time::TimeDuration(Time::E_TIME_NADT), invalid.length());

	{
		Time::TimePeriod p1 = Time::TimePeriod(DateTime::getNADT(), DateTime());
		Time::TimePeriod p2 = Time::TimePeriod(DateTime(), DateTime::getNADT());

		unitAssert(p1 == invalid);
		unitAssert(p2 == invalid);

		unitAssert(p1 == p2);
		unitAssert(!(p1 != p2));

		unitAssert(!intersect(p1,p2));
		unitAssert(intersection(p1,p2) == invalid);
		unitAssert(merge(p1,p2) == invalid);
		unitAssert(span(p1,p2) == invalid);

		unitAssertEquals(Time::TimeDuration(Time::E_TIME_NADT), intersection(p1,p2).length());
	}

	{
		Time::TimePeriod p1 = Time::TimePeriod(DateTime(), Time::hours(12));
		Time::TimePeriod p2 = Time::TimePeriod(DateTime(), DateTime::getNADT());

		unitAssert(p1 != invalid);
		unitAssert(p2 == invalid);

		unitAssert(p1 != p2);
		unitAssert(!(p1 == p2));

		unitAssert(!intersect(p1,p2));
		unitAssertEquals(invalid, intersection(p1,p2));
		unitAssertEquals(invalid, merge(p1,p2));
		unitAssertEquals(invalid, span(p1,p2));

		unitAssert(intersection(p1,p2).length().isInvalid());
	}
}
