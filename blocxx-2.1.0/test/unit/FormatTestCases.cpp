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


#include "TestSuite.hpp"
#include "TestCaller.hpp"
#include "FormatTestCases.hpp"
#include "blocxx/Format.hpp"

using namespace blocxx;

void FormatTestCases::setUp()
{
}

void FormatTestCases::tearDown()
{
}

void FormatTestCases::testFormat()
{
	unitAssert( Format("%1 != %2", 1, 2) == "1 != 2" );
	unitAssert( Format("%1 != %2", 1, 2).toString() == "1 != 2" );
	unitAssert( Format("%1,%2,%3,%4,%5,%6,%7,%8,%9", 
				1,2,3,4,5,6,7,8,9).toString() == "1,2,3,4,5,6,7,8,9" );
	unitAssert( String(Format("%1 != %2", 1, 2).c_str()) == "1 != 2" );
}

void FormatTestCases::testPercent()
{
	unitAssert( Format("%1%%%2", "abc", "def").toString() == "abc%def" );
	unitAssert( Format("%2%%%1%3%2", "one", "two", '%').toString() == 
			"two%one%two" );
}

void FormatTestCases::testError()
{
	unitAssert( Format("%1,%2xx", 1).toString() == "1,\n*** Parameter specifier too large.\n*** Error in format string at \"2xx\".\n" );
	unitAssert( Format("%xABC", 'e').toString() == "\n*** Error in format string at \"xABC\".\n" );
	unitAssert( Format("%0ABC", 'e').toString() == "\n*** Error in format string at \"0ABC\".\n" );
}

Test* FormatTestCases::suite()
{
	TestSuite *testSuite = new TestSuite ("Format");

	testSuite->addTest (new TestCaller <FormatTestCases> 
			("testFormat", 
			&FormatTestCases::testFormat));
	testSuite->addTest (new TestCaller <FormatTestCases> 
			("testPercent", 
			&FormatTestCases::testPercent));
	testSuite->addTest (new TestCaller <FormatTestCases> 
			("testError", 
			&FormatTestCases::testError));

	return testSuite;
}


