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

#include "blocxx/UUID.hpp"
#include "blocxx/Array.hpp"
#include "blocxx/String.hpp"

#include <algorithm> // for std::sort

#include <iostream>

using namespace blocxx;

AUTO_UNIT_TEST(UUIDTestCases_testUnique)
{
	// Create 10000, and then make sure they're all different.
	// More would be better, but we need to keep the tests fast.
	const int NUM = 100000;
	Array<blocxx::UUID> uuids;
	uuids.reserve(NUM);
	for (int i = 0; i < NUM; ++i)
		uuids.push_back(blocxx::UUID());

	std::sort(uuids.begin(), uuids.end());
	for (int i = 0; i < NUM-1; ++i)
	{
		unitAssert(uuids[i] != uuids[i+1]);
	}
}

AUTO_UNIT_TEST(UUIDTestCases_testStringConversion)
{
	blocxx::UUID uuid1;
	String suuid1(uuid1.toString());
	blocxx::UUID uuid1_2(suuid1);
	unitAssert(uuid1 == uuid1_2);

	// test round-tripping
	String suuid1_2(uuid1_2.toString());
	unitAssert(suuid1 == suuid1_2);

	// construct from invalid make sure it throws.
	unitAssertThrows(blocxx::UUID("too short"));
	// wrong format
	unitAssertThrows(blocxx::UUID("6ba7b810-9dad-11d1-80b40-0c04fd430c8"));
	// non-hex value
	unitAssertThrows(blocxx::UUID("6ba7b810-9dad-11d1-80b4-X0c04fd430c8"));

	// construct 2 from the same string, make sure they're equal
	unitAssert(blocxx::UUID("6ba7b810-9dad-11d1-80b4-00c04fd430c8") ==
		blocxx::UUID("6ba7b810-9dad-11d1-80b4-00c04fd430c8"));
	unitAssert(blocxx::UUID("11111111-1111-1111-1111-111111111111") !=
		blocxx::UUID("22222222-2222-2222-2222-222222222222"));

	// copy 1 and make sure it outputs the same string
	blocxx::UUID uuid3;
	blocxx::UUID uuid3_2(uuid3);
	unitAssert(uuid3.toString() == uuid3_2.toString());
}
