/*******************************************************************************
* Copyright (C) 2008, Quest Software, Inc. All rights reserved.
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
#include "TmUtils_timeGmTestCases.hpp"

#define PROVIDE_AUTO_TEST_MAIN
#include "AutoTest.hpp"
#include "TestSuite.hpp"
#include "TestCaller.hpp"
AUTO_UNIT_TEST_SUITE_NAMED(TmUtils_timeGmTestCases, "TmUtils_timeGm");
#include "blocxx/TmUtils.hpp"
#include <ctime>
#include <iostream>
#include <cstring>
#include <iostream>
#include <iomanip>
#include "blocxx/String.hpp"
#include "blocxx/DateTime.hpp"

using namespace blocxx;
using namespace std;

// This is here only to prevent nasty compiler warnings about implicit virtual constructors.
TmUtils_timeGmTestCases::~TmUtils_timeGmTestCases()
{
}

void TmUtils_timeGmTestCases::setUp()
{
}

void TmUtils_timeGmTestCases::tearDown()
{
}

namespace
{
	void convertStrToTm(char const * tmstr, struct tm & tm)
	{
		// .toInt(); .substring(pos, length);
		String s(tmstr);
		// year-month-day hour:minute:second
		size_t year_beg = 0;
		size_t year_end = s.indexOf('-');
		size_t mon_beg = year_end + 1;
		size_t mon_end = s.indexOf('-', mon_beg);
		size_t day_beg = mon_end + 1;
		size_t day_end = s.indexOf(' ', day_beg);
		size_t hour_beg = day_end + 1;
		size_t hour_end = s.indexOf(':', hour_beg);
		size_t min_beg = hour_end + 1;
		size_t min_end = s.indexOf(':', min_beg);
		size_t sec_beg = min_end + 1;
		tm.tm_year = s.substring(year_beg, year_end - year_beg).toInt() - 1900;
		tm.tm_mon = s.substring(mon_beg, mon_end - mon_beg).toInt() - 1;
		tm.tm_mday = s.substring(day_beg, day_end - day_beg).toInt();
		tm.tm_hour = s.substring(hour_beg, hour_end - hour_beg).toInt();
		tm.tm_min = s.substring(min_beg, min_end - min_beg).toInt();
		tm.tm_sec = s.substring(sec_beg).toInt();
	}

}

void TmUtils_timeGmTestCases::timeGmTestCase(char const * inp, time_t expected)
{
	struct tm tminp;
	convertStrToTm(inp, tminp);
	struct tm tmexpected = tminp;
	normTm(tmexpected);

	time_t t = timeGm(tminp);

	unitAssertEquals(expected, t);
	unitAssertEquals(0, memcmp(&tmexpected, &tminp, sizeof(tmexpected)));
}

void TmUtils_timeGmTestCases::testAll()
{
	// start of epoch; January
	timeGmTestCase("1970-01-01 00:00:00", 0);
	// every month
	timeGmTestCase("1984-02-15 12:06:37", 445694797);
	timeGmTestCase("1993-03-24 07:43:58", 732959038);
	timeGmTestCase("2005-04-05 18:31:11", 1112725871);
	timeGmTestCase("2011-05-12 22:26:43", 1305239203);
	timeGmTestCase("2015-06-30 13:17:35", 1435670255);
	timeGmTestCase("2016-07-03 01:00:28", 1467507628);
	timeGmTestCase("2018-08-13 23:54:15", 1534204455);
	timeGmTestCase("2021-09-17 20:36:00", 1631910960);
	timeGmTestCase("2022-10-31 08:21:33", 1667204493);
	timeGmTestCase("2025-11-10 10:51:53", 1762771913);
	timeGmTestCase("2027-12-02 09:13:49", 1827738829);
	// end of last complete year representable in 32-bit signed time_t
	timeGmTestCase("2037-12-31 23:59:59", 2145916799);
#if BLOCXX_DATETIME_MINIMUM_YEAR <= 1902
	// start of first complete year representable in 32-bit signed time_t
	timeGmTestCase("1902-01-01 00:00:00", -2145916800);
#endif
	// leap year
	timeGmTestCase("1996-02-28 12:36:24", 825510984);
	timeGmTestCase("1996-02-29 05:49:51", 825572991);
	timeGmTestCase("1996-03-01 18:06:35", 825703595);
	// special leap year
	timeGmTestCase("2000-02-28 12:36:24", 951741384);
	timeGmTestCase("2000-02-29 05:49:51", 951803391);
	timeGmTestCase("2000-03-01 18:06:35", 951933995);
	// not a leap year
	timeGmTestCase("2003-02-28 12:36:24", 1046435784);
	timeGmTestCase("2003-02-29 05:49:51", 1046497791);
	timeGmTestCase("2003-03-01 18:06:35", 1046541995);
}

void TmUtils_timeGmTestCases::errorCase(
	int year, int month, int day, int hour, int minute, int sec)
{
	struct tm tm;
	tm.tm_year = year;
	tm.tm_mon = month;
	tm.tm_mday = day;
	tm.tm_hour = hour;
	tm.tm_min = minute;
	tm.tm_sec = sec;
	unitAssertThrowsEx(timeGm(tm), DateTimeException);
}

void TmUtils_timeGmTestCases::testErrors()
{
	errorCase(BLOCXX_DATETIME_MINIMUM_YEAR - 1 - 1900, 11, 31, 23, 59, 59);
	errorCase(BLOCXX_DATETIME_MAXIMUM_YEAR + 1 - 1900,  0,  1,  0,  0,  0);
}

Test* TmUtils_timeGmTestCases::suite()
{
	TestSuite *testSuite = new TestSuite ("TmUtils_timeGm");

	ADD_TEST_TO_SUITE(TmUtils_timeGmTestCases, testAll);
	ADD_TEST_TO_SUITE(TmUtils_timeGmTestCases, testErrors);

	return testSuite;
}

