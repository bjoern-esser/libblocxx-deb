/*******************************************************************************
* Copyright (C) 2009, Quest Software, Inc. All rights reserved.
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

#include "blocxx/Format.hpp"
#include "blocxx/String.hpp"


using namespace blocxx;

AUTO_UNIT_TEST(test_equal)
{
	unitAssertEquals(1, 1);
	unitAssertEquals(String("abcd"), "abcd");
	unitAssertEquals(Format("%1%2%3", 'a', 'b', 'c'), "abc");
}

AUTO_UNIT_TEST(test_not_equal)
{
	unitAssertNotEquals(1, 2);
	unitAssertNotEquals(String("abcd"), "dcba");
	unitAssertNotEquals(Format("%1%2%3", 'a', 'b', 'c'), "xyz");
}

AUTO_UNIT_TEST(test_one_of)
{
	const int value = 1;
	unitAssertEqualsOneOf(1)(value, 1);

	unitAssertEqualsOneOf(2)(value, 1, 2);
	unitAssertEqualsOneOf(2)(value, 2, 1);

	unitAssertEqualsOneOf(3)(value, 1, 2, 3);
	unitAssertEqualsOneOf(3)(value, 1, 3, 2);
	unitAssertEqualsOneOf(3)(value, 2, 1, 3);
	unitAssertEqualsOneOf(3)(value, 2, 3, 1);
	unitAssertEqualsOneOf(3)(value, 3, 1, 2);
	unitAssertEqualsOneOf(3)(value, 3, 2, 1);

	unitAssertEqualsOneOf(4)(value, 2, 3.0, String("hello"), 100 / (10 * 10));
}
