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
#include <vector>
#include <algorithm>
#include <iostream>
#include "EnumerationTestCases.hpp"
#include "blocxx/Enumeration.hpp"
#include "blocxx/String.hpp"

using std::cout;
using std::endl;
using std::cerr;

using namespace blocxx;

void EnumerationTestCases::setUp()
{
}

void EnumerationTestCases::tearDown()
{
}

const unsigned int loopVal = 100;
void EnumerationTestCases::testAll()
{
	try
	{
		Enumeration<String> e;
		for (unsigned int i = 1; i <= loopVal; i++)
		{
			e.addElement(String(i));
		}
		for (unsigned int i = 0; i < loopVal; ++i)
		{
			unitAssert(e.hasMoreElements());
			unitAssert(e.numberOfElements() == loopVal - i);
			unitAssert(e.nextElement() == String(i + 1));
		}
		unitAssert(!e.hasMoreElements());
		unitAssert(e.numberOfElements() == 0);
		unitAssertThrows(e.nextElement());
	}
	catch(Exception& e)
	{
		cerr << e << endl;
		throw;
	}
}

const unsigned int bigLoopVal = 1000;
void EnumerationTestCases::testAllBig()
{
	try
	{
		Enumeration<String> e;
		for (unsigned int i = 1; i <= bigLoopVal; i++)
		{
			e.addElement(String(i));
		}
		for (unsigned int i = 0; i < bigLoopVal; ++i)
		{
			unitAssert(e.hasMoreElements());
			unitAssert(e.numberOfElements() == bigLoopVal - i);
			String foo = e.nextElement();
			unitAssert(foo == String(i + 1));
		}
		unitAssert(!e.hasMoreElements());
		unitAssert(e.numberOfElements() == 0);
		unitAssertThrows(e.nextElement());
	}
	catch(Exception& e)
	{
		cerr << e << endl;
		throw;
	}
}
		
void EnumerationTestCases::testQueue()
{
	try
	{
		Enumeration<String> e;
		// now test it's queue like capabilities
		e.addElement("9");
		for (unsigned int i = 10; i <= loopVal; ++i)
		{
			e.addElement(String(i));
			unitAssert(e.hasMoreElements());
			unitAssert(e.numberOfElements() == 2);
			unitAssert(e.nextElement() == String(i - 1));
		}
		unitAssert(e.hasMoreElements());
		unitAssert(e.numberOfElements() == 1);
		unitAssert(e.nextElement() == String(loopVal));
		unitAssert(!e.hasMoreElements());
		unitAssert(e.numberOfElements() == 0);
		unitAssertThrows(e.nextElement());
	}
	catch (Exception& e)
	{
		cout << e;
		throw;
	}
}

void EnumerationTestCases::testQueueBig()
{
	try
	{
		Enumeration<String> e;
		// now test it's queue like capabilities
		e.addElement("9");
		for (unsigned int i = 10; i <= bigLoopVal; ++i)
		{
			e.addElement(String(i));
			unitAssert(e.hasMoreElements());
			unitAssert(e.numberOfElements() == 2);
			String foo = e.nextElement();
			unitAssert(foo == String(i - 1));
		}
		unitAssert(e.hasMoreElements());
		unitAssert(e.numberOfElements() == 1);
		unitAssert(e.nextElement() == String(bigLoopVal));
		unitAssert(!e.hasMoreElements());
		unitAssert(e.numberOfElements() == 0);
		unitAssertThrows(e.nextElement());
	}
	catch (Exception& e)
	{
		cout << e;
		throw;
	}
}

void EnumerationTestCases::testCreation()
{
	Enumeration<String> e;
	e.addElement("5");
	unitAssert(e.numberOfElements() == 1);
}

void EnumerationTestCases::testInputIterator()
{
	try
	{
		Enumeration<String> e;
		for (unsigned int i = 1; i <= loopVal; i++)
		{
			e.addElement(String(i));
		}

		std::vector<String> vec;
		std::copy(Enumeration_input_iterator<String>(e), // start of source
			Enumeration_input_iterator<String>(), // end of source
			std::back_inserter(vec)); // destination

		for (unsigned int i = 0; i < loopVal; ++i)
		{
			unitAssert(vec[i] == String(i + 1));
		}
		unitAssert(!e.hasMoreElements());
		unitAssert(e.numberOfElements() == 0);
		unitAssertThrows(e.nextElement());
	}
	catch (Exception& e)
	{
		cout << e << endl;
		throw;
	}
}

void EnumerationTestCases::testInsertIterator()
{
	try
	{
		std::vector<String> as;
		for (unsigned int i = 1; i <= loopVal; i++)
		{
			as.push_back(String(i));
		}

		Enumeration<String> e;
		std::copy(as.begin(), as.end(), Enumeration_insert_iterator<String>(e));

		for (unsigned int i = 0; i < loopVal; ++i)
		{
			unitAssert(e.hasMoreElements());
			unitAssert(e.numberOfElements() == loopVal - i);
			unitAssert(e.nextElement() == String(i + 1));
		}
		unitAssert(!e.hasMoreElements());
		unitAssert(e.numberOfElements() == 0);
		unitAssertThrows(e.nextElement());
	}
	catch (Exception& e)
	{
		cout << e << endl;
		throw;
	}
}

Test* EnumerationTestCases::suite()
{
	TestSuite *testSuite = new TestSuite ("Enumeration");
	testSuite->addTest (new TestCaller <EnumerationTestCases>
			("testInputIterator",
			&EnumerationTestCases::testInputIterator));
	testSuite->addTest (new TestCaller <EnumerationTestCases>
			("testInsertIterator",
			&EnumerationTestCases::testInsertIterator));
	testSuite->addTest (new TestCaller <EnumerationTestCases>
			("testCreation",
			&EnumerationTestCases::testCreation));
	testSuite->addTest (new TestCaller <EnumerationTestCases>
			("testAll",
			&EnumerationTestCases::testAll));
	testSuite->addTest (new TestCaller <EnumerationTestCases>
			("testQueue",
			&EnumerationTestCases::testQueue));
	testSuite->addTest (new TestCaller <EnumerationTestCases>
			("testAllBig",
			&EnumerationTestCases::testAllBig));
	testSuite->addTest (new TestCaller <EnumerationTestCases>
			("testQueueBig",
			&EnumerationTestCases::testQueueBig));

	return testSuite;
}

