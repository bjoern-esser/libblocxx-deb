/*******************************************************************************
* Copyright (C) 2008, Quest Software, Inc. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*     * Redistributions of source code must retain the above copyright notice,
*       this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of Quest Software, Inc.,
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
#include "blocxx/DivRem.hpp"
#include "blocxx/Types.hpp"

using namespace blocxx;

// This is here only to prevent nasty compiler warnings about implicit virtual constructors.
AUTO_UNIT_TEST(DivRemTestCases_testPosWithNonzeroRemainder)
{
	unitAssertEquals(2, divTrunc(13, 5));
	unitAssertEquals(3, remTrunc(13, 5));
	unitAssertEquals(2, divFloor(13, 5));
	unitAssertEquals(3, remFloor(13, 5));

	unitAssertEquals(Int64(2), divTrunc(Int64(13), Int64(5)));
	unitAssertEquals(Int64(3), remTrunc(Int64(13), Int64(5)));
	unitAssertEquals(Int64(2), divFloor(Int64(13), Int64(5)));
	unitAssertEquals(Int64(3), remFloor(Int64(13), Int64(5)));
}

AUTO_UNIT_TEST(DivRemTestCases_testPosWithZeroRemainder)
{
	unitAssertEquals(2, divTrunc(12, 6));
	unitAssertEquals(0, remTrunc(12, 6));
	unitAssertEquals(2, divFloor(12, 6));
	unitAssertEquals(0, remFloor(12, 6));

	unitAssertEquals(Int64(2), divTrunc(Int64(12), Int64(6)));
	unitAssertEquals(Int64(0), remTrunc(Int64(12), Int64(6)));
	unitAssertEquals(Int64(2), divFloor(Int64(12), Int64(6)));
	unitAssertEquals(Int64(0), remFloor(Int64(12), Int64(6)));
}

AUTO_UNIT_TEST(DivRemTestCases_testZero)
{
	unitAssertEquals(0, divTrunc(0, 7));
	unitAssertEquals(0, remTrunc(0, 7));
	unitAssertEquals(0, divFloor(0, 7));
	unitAssertEquals(0, remFloor(0, 7));

	unitAssertEquals(Int64(0), divTrunc(Int64(0), Int64(7)));
	unitAssertEquals(Int64(0), remTrunc(Int64(0), Int64(7)));
	unitAssertEquals(Int64(0), divFloor(Int64(0), Int64(7)));
	unitAssertEquals(Int64(0), remFloor(Int64(0), Int64(7)));
}

AUTO_UNIT_TEST(DivRemTestCases_testNegWithNonzeroRemainder)
{
	unitAssertEquals(-2, divTrunc(-13, 5));
	unitAssertEquals(-3, remTrunc(-13, 5));
	unitAssertEquals(-3, divFloor(-13, 5));
	unitAssertEquals(+2, remFloor(-13, 5));

	unitAssertEquals(Int64(-2), divTrunc(Int64(-13), Int64(5)));
	unitAssertEquals(Int64(-3), remTrunc(Int64(-13), Int64(5)));
	unitAssertEquals(Int64(-3), divFloor(Int64(-13), Int64(5)));
	unitAssertEquals(Int64(+2), remFloor(Int64(-13), Int64(5)));
}

AUTO_UNIT_TEST(DivRemTestCases_testNegWithZeroRemainder)
{
	unitAssertEquals(-2, divTrunc(-12, 6));
	unitAssertEquals( 0, remTrunc(-12, 6));
	unitAssertEquals(-2, divFloor(-12, 6));
	unitAssertEquals( 0, remFloor(-12, 6));

	unitAssertEquals(Int64(-2), divTrunc(Int64(-12), Int64(6)));
	unitAssertEquals(Int64( 0), remTrunc(Int64(-12), Int64(6)));
	unitAssertEquals(Int64(-2), divFloor(Int64(-12), Int64(6)));
	unitAssertEquals(Int64( 0), remFloor(Int64(-12), Int64(6)));
}

