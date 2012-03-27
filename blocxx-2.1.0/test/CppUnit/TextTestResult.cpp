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



#include "TextTestResult.hpp"
#include "CppUnitException.hpp"
#include "TestFailure.hpp"
#include "Test.hpp"

#include <iostream>
#include <cstdlib>  // for exit

using namespace std;

void TextTestResult::addError (Test *test, CppUnitException *e)
{
	TestResult::addError (test, e);
	cerr << "E\n";

}

void TextTestResult::addFailure (Test *test, CppUnitException *e)
{
	TestResult::addFailure (test, e);
	cerr << "F\n";

}

void TextTestResult::startTest (Test *test)
{
	TestResult::startTest (test);
	cerr << "." << std::flush;

}


void TextTestResult::printErrors (ostream& stream)
{
	if (testErrors () != 0) {

		if (testErrors () == 1)
			stream << "There was " << testErrors () << " error: " << endl;
		else
			stream << "There were " << testErrors () << " errors: " << endl;

		int i = 1;

		for (vector<TestFailure *>::iterator it = errors ().begin (); it != errors ().end (); ++it) {
			TestFailure             *failure    = *it;
			CppUnitException        *e          = failure->thrownException ();
				Test							*t				= failure->failedTest();

			stream << i
				   << ") "
						 << t->toString()
						 << " line: ";
				if (e)
					stream << e->lineNumber();
				stream << " "
				   << (e ? e->fileName () : "") << " "
				   << "\"" << failure->thrownException ()->what () << "\""
				   << endl;
			i++;
		}
		exit(1); // stop once we've reported the failed test.
	}

}

void TextTestResult::printFailures (ostream& stream)
{
	if (testFailures () != 0) {
		if (testFailures () == 1)
			stream << "There was " << testFailures () << " failure: " << endl;
		else
			stream << "There were " << testFailures () << " failures: " << endl;

		int i = 1;

		for (vector<TestFailure *>::iterator it = failures ().begin (); it != failures ().end (); ++it) {
			TestFailure             *failure    = *it;
			CppUnitException        *e          = failure->thrownException ();
				Test							*t				= failure->failedTest();

			stream << i
				   << ") "
						 << t->toString()
				   << " line: ";
				if (e)
					stream << e->lineNumber ();
				stream << " "
				   << (e ? e->fileName () : "") << " "
				   << "\"" << failure->thrownException ()->what () << "\""
				   << endl;
			i++;
		}

		exit(1); // stop once we've reported the failed test.
	}

}


void TextTestResult::print (ostream& stream)
{
	printHeader (stream);
	printErrors (stream);
	printFailures (stream);

}


void TextTestResult::printHeader (ostream& stream)
{
	if (wasSuccessful ())
		stream << endl << "OK (" << runTests () << " tests)" << endl;
	else
	{
		stream << endl
			 << "!!!FAILURES!!!" << endl
			 << "Test Results:" << endl
			 << "Run:  "
			 << runTests ()
			 << "   Failures: "
			 << testFailures ()
			 << "   Errors: "
			 << testErrors ()
			 << endl;
	}

}
