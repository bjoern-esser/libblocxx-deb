/*******************************************************************************
* Copyright (C) 2001-2004 Vintela, Inc. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of source code must retain the above copyright notice,
*    this list of conditions and the following disclaimer.
*
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
*
*  - Neither the name of Vintela, Inc. nor the names of its
*    contributors may be used to endorse or promote products derived from this
*    software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL Vintela, Inc. OR THE CONTRIBUTORS
* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/


#ifndef CPPUNIT_TESTCASE_H
#define CPPUNIT_TESTCASE_H

#include "Guards.hpp"
#include "Test.hpp"
#include "CppUnitException.hpp"
#include <iostream>
#include <sstream>

class TestResult;



/*
 * A test case defines the fixture to run multiple tests. To define a test case
 * 1) implement a subclass of TestCase
 * 2) define instance variables that store the state of the fixture
 * 3) initialize the fixture state by overriding setUp
 * 4) clean-up after a test by overriding tearDown.
 *
 * Each test runs in its own fixture so there
 * can be no side effects among test runs.
 * Here is an example:
 *
 * class MathTest : public TestCase {
 *     protected: int m_value1;
 *     protected: int m_value2;
 *
 *     public: MathTest (string name)
 *                 : TestCase (name) {
 *     }
 *
 *     protected: void setUp () {
 *         m_value1 = 2;
 *         m_value2 = 3;
 *     }
 * }
 *
 *
 * For each test implement a method which interacts
 * with the fixture. Verify the expected results with assertions specified
 * by calling assert on the expression you want to test:
 *
 *    protected: void testAdd () {
 *        int result = value1 + value2;
 *        assert (result == 5);
 *    }
 *
 * Once the methods are defined you can run them. To do this, use
 * a TestCaller.
 *
 * Test *test = new TestCaller<MathTest>("testAdd", MathTest::testAdd);
 * test->run ();
 *
 *
 * The tests to be run can be collected into a TestSuite. CppUnit provides
 * different test runners which can run a test suite and collect the results.
 * The test runners expect a static method suite as the entry
 * point to get a test to run.
 *
 * public: static MathTest::suite () {
 *      TestSuite *suiteOfTests = new TestSuite;
 *      suiteOfTests->addTest(new TestCaller<MathTest>("testAdd", testAdd));
 *      suiteOfTests->addTest(new TestCaller<MathTest>("testDivideByZero", testDivideByZero));
 *      return suiteOfTests;
 *  }
 *
 * Note that the caller of suite assumes lifetime control
 * for the returned suite.
 *
 * see TestResult, TestSuite and TestCaller
 *
 */


class TestCase : public Test
{
	REFERENCEOBJECT (TestCase)

public:
						TestCase         (const char* Name);
						~TestCase        ();

	virtual void        run              (TestResult *result);
	virtual TestResult  *run             ();

	template <class T>
	bool runFuncAndCatchErrors( T func, const char* msg, TestResult* result );

	virtual int         countTestCases   ();
	const char*         name             ();
	virtual const char* toString         ();

	virtual void        setUp            ();
	virtual void        tearDown         ();

protected:
	virtual void        runTest          ();

	TestResult          *defaultResult   ();
	void                assertImplementation
										 (bool         condition,
										  const char*  conditionExpression,
										  long         lineNumber,
										  const char*  fileName);

	void                assertEquals     (long         expected,
										  long         actual,
										  long         lineNumber,
										  const char*  fileName);

	void                assertEquals     (double       expected,
										  double       actual,
										  double       delta,
										  long         lineNumber,
										  const char*  fileName);
	template <typename T1, typename T2>
	void                assertEquals     (const T1& expected,
										  const T2& actual,
										  long         lineNumber,
										  const char*  fileName)
	{
		if (expected != actual)
		{
			std::ostringstream oss;
			oss << "expected: \"" << expected << "\" but was: \"" << actual << "\"";
			assertImplementation (false, oss.str().c_str(), lineNumber, fileName);
		}
	}

	template <typename T1, typename T2>
	void                assertNotEquals     (const T1& expected,
										  const T2& actual,
										  long         lineNumber,
										  const char*  fileName)
	{
		if (expected == actual)
		{
			std::ostringstream oss;
			oss << "expected not equal to: \"" << expected << "\" but it was.";
			assertImplementation (false, oss.str().c_str(), lineNumber, fileName);
		}
	}

	const char*         notEqualsMessage (long         expected,
										  long         actual);

	const char*         notEqualsMessage (double       expected,
										  double       actual);

private:
	const char*   m_name;



};




// A set of macros which allow us to get the line number
// and file name at the point of an error.
// Just goes to show that preprocessors do have some
// redeeming qualities.

#define CPPUNIT_SOURCEANNOTATION

#ifdef CPPUNIT_SOURCEANNOTATION

	#undef unitAssert
	#define unitAssert(condition)\
	try{this->assertImplementation ((condition),(#condition),\
		__LINE__, __FILE__);\
		 } catch (const CppUnitException& e){throw e;} \
			catch( const std::exception& e ){ throw CppUnitException( e.what(), __LINE__, __FILE__);} \
		  catch (...) {throw CppUnitException((#condition), __LINE__, __FILE__);}

	#undef unitAssertFail
	#define unitAssertFail(condition)\
	try{this->assertImplementation ((!(condition)),("!("#condition")"),\
		__LINE__, __FILE__);\
		 } catch (const CppUnitException& e){throw e;} \
			catch( const std::exception& e ){ throw CppUnitException( e.what(), __LINE__, __FILE__);} \
		  catch (...) {throw CppUnitException((#condition), __LINE__, __FILE__);}

	#undef unitAssertThrows
	#define unitAssertThrows(condition)\
	try{condition;\
		 this->assertImplementation (false, #condition,\
				 __LINE__, __FILE__);\
		 } catch (const CppUnitException& e){throw e;} \
		  catch (...) {}

	#undef unitAssertThrowsSpecific
	#define unitAssertThrowsSpecific(exceptionType, condition) \
	try { condition;\
		this->assertImplementation (false, #condition, __LINE__, __FILE__); \
	} catch (const CppUnitException& e){throw e;} \
	catch(const exceptionType& e) {} \
	catch (...) { \
		this->assertImplementation (false, #condition, __LINE__, __FILE__); \
	}

	#undef unitAssertNoThrow
	#define unitAssertNoThrow(condition)\
	try{condition;\
	} catch (const CppUnitException& e){throw e;} \
	catch( const std::exception& e ){ std::cout << "Caught exception " << e.what() << std::endl; this->assertImplementation( false, #condition, __LINE__, __FILE__);} \
	catch (...) { this->assertImplementation( false, #condition,\
			__LINE__, __FILE__);}

	#undef unitAssertNoThrowSpecific
	#define unitAssertNoThrowSpecific(exceptionType, condition)	\
	try {condition;} \
	catch (const CppUnitException& e){throw e;} \
	catch (const exceptionType& e) { this->assertImplementation (false, #condition, __LINE__, __FILE__); } \
	catch (...) { }

#else

	#undef unitAssert
	#define unitAssert(condition)\
	(this->assertImplementation ((condition),"",\
		__LINE__, __FILE__))

#endif


// Macros for primitive value comparisons
#define unitAssertDoublesEqual(expected,actual,delta)\
(this->assertEquals ((expected),\
		(actual),(delta),__LINE__,__FILE__))

#define unitAssertLongsEqual(expected,actual)\
(this->assertEquals ((expected),\
		(actual),__LINE__,__FILE__))

#define unitAssertEquals(expected,actual)\
(this->assertEquals ((expected),\
		(actual),__LINE__,__FILE__))

#define unitAssertNotEquals(expected,actual)\
(this->assertNotEquals ((expected),\
		(actual),__LINE__,__FILE__))

#endif
