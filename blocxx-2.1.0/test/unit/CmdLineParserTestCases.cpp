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
#include "TestSuite.hpp"
#include "TestCaller.hpp"
#include "CmdLineParserTestCases.hpp"
#include "blocxx/CmdLineParser.hpp"

using namespace blocxx;

void CmdLineParserTestCases::setUp()
{
}

void CmdLineParserTestCases::tearDown()
{
}

void CmdLineParserTestCases::testSomething()
{
	enum
	{
		opt1,
		opt2,
		opt3,
		invalidOpt
	};

	CmdLineParser::Option options[] =
	{
		{opt1, '1', "one", CmdLineParser::E_NO_ARG, 0, "first description"},
		{opt2, '2', "two", CmdLineParser::E_OPTIONAL_ARG, "optional", "second description"},
		{opt3, '3', "three", CmdLineParser::E_REQUIRED_ARG, 0, "third description"},
		{0, 0, 0, CmdLineParser::E_NO_ARG, 0, 0}
	};
	
	{
		int argc = 5;
		const char* argv[] = {
			"progname",
			"-1",
			"--two=abc",
			"-",
			"1"
		};
		CmdLineParser parser(argc, argv, options, CmdLineParser::E_NON_OPTION_ARGS_ALLOWED);
		unitAssert(parser.isSet(opt1));
		unitAssert(parser.isSet(opt2));
		unitAssert(!parser.isSet(opt3));
		unitAssert(!parser.isSet(invalidOpt));
		unitAssert(parser.getOptionValue(opt2) == "abc");
		unitAssert(parser.mustGetOptionValue(opt2) == "abc");
		unitAssert(parser.getOptionValue(opt3) == "");
		unitAssert(parser.getOptionValue(opt3, "opt3default") == "opt3default");
		unitAssert(parser.getNonOptionCount() == 2);
		unitAssert(parser.getNonOptionArg(0) == "-");
		unitAssert(parser.getNonOptionArg(1) == "1");
		try
		{
			parser.mustGetOptionValue(opt3, "opt3 desc");
			unitAssert(0);
		}
		catch (CmdLineParserException& e)
		{
			unitAssert(e.getErrorCode() == CmdLineParser::E_MISSING_OPTION);
			unitAssert(e.getMessage() == String("opt3 desc"));
		}
	}
	{
		int argc = 3;
		const char* argv[] = {
			"progname",
			"--two",
			"abc"
		};
		CmdLineParser parser(argc, argv, options, CmdLineParser::E_NON_OPTION_ARGS_INVALID);
		unitAssert(parser.isSet(opt2));
		unitAssert(parser.getOptionValue(opt2) == "abc");
		unitAssert(parser.getNonOptionCount() == 0);
	}
	{
		int argc = 2;
		const char* argv[] = {
			"progname",
			"--two"
		};
		CmdLineParser parser(argc, argv, options, CmdLineParser::E_NON_OPTION_ARGS_INVALID);
		unitAssert(parser.isSet(opt2));
		unitAssert(parser.getOptionValue(opt2) == "optional");
		unitAssert(parser.getNonOptionCount() == 0);
	}
	{
		int argc = 5;
		const char* argv[] = {
			"progname",
			"--one",
			"one",
			"two",
			"three"
		};
		CmdLineParser parser(argc, argv, options, CmdLineParser::E_NON_OPTION_ARGS_ALLOWED);
		unitAssert(parser.getNonOptionCount() == 3);
		unitAssert(parser.getNonOptionArg(0) == "one");
		unitAssert(parser.getNonOptionArg(1) == "two");
		unitAssert(parser.getNonOptionArg(2) == "three");
	}
	{
		int argc = 8;
		const char* argv[] = {
			"progname",
			"--two=first",
			"--two",
			"second",
			"-2third",
			"-2=fourth",
			"-2",
			"fifth"
		};
		CmdLineParser parser(argc, argv, options, CmdLineParser::E_NON_OPTION_ARGS_ALLOWED);
		unitAssert(parser.getOptionValue(opt2) == "fifth");
		StringArray opts = parser.getOptionValueList(opt2);
		unitAssert(opts.size() == 5);
		unitAssert(opts[0] == "first");
		unitAssert(opts[1] == "second");
		unitAssert(opts[2] == "third");
		unitAssert(opts[3] == "fourth");
		unitAssert(opts[4] == "fifth");
	}
	{
		int argc = 2;
		const char* argv[] = {
			"progname",
			"--invalid"
		};
		try
		{
			CmdLineParser parser(argc, argv, options, CmdLineParser::E_NON_OPTION_ARGS_INVALID);
			unitAssert(0);
		}
		catch (CmdLineParserException& e)
		{
			unitAssert(e.getErrorCode() == CmdLineParser::E_INVALID_OPTION);
		}
	}
	{
		int argc = 2;
		const char* argv[] = {
			"progname",
			"invalid"
		};
		try
		{
			CmdLineParser parser(argc, argv, options, CmdLineParser::E_NON_OPTION_ARGS_INVALID);
			unitAssert(0);
		}
		catch (CmdLineParserException& e)
		{
			unitAssert(e.getErrorCode() == CmdLineParser::E_INVALID_NON_OPTION_ARG);
		}
	}
	{
		int argc = 2;
		const char* argv[] = {
			"progname",
			"--three"
		};
		try
		{
			CmdLineParser parser(argc, argv, options, CmdLineParser::E_NON_OPTION_ARGS_INVALID);
			unitAssert(0);
		}
		catch (CmdLineParserException& e)
		{
			unitAssert(e.getErrorCode() == CmdLineParser::E_MISSING_ARGUMENT);
		}
	}
	{
		int argc = 6;
		const char* argv[] = {
			"progname",
			"-2",
			"-",
			"-3",
			"-",
			"-1"
		};
		try
		{
			CmdLineParser parser(argc, argv, options, CmdLineParser::E_NON_OPTION_ARGS_INVALID);
			unitAssert(parser.isSet(opt1));
			unitAssert(parser.isSet(opt2));
			unitAssert(parser.isSet(opt3));
			unitAssert(!parser.isSet(invalidOpt));
			unitAssert(parser.getOptionValue(opt2) == "-");
			unitAssert(parser.getOptionValue(opt3) == "-");
		}
		catch (CmdLineParserException& e)
		{
			unitAssert(0);
		}
	}
	{
		unitAssert(CmdLineParser::getUsage(options) ==
			"Options:\n"
			"  -1, --one                 first description\n"
			"  -2, --two [arg]           second description (default is optional)\n"
			"  -3, --three <arg>         third description\n"
			);
	}

	CmdLineParser::Option options2[] =
	{
		{opt1, '1', 0, CmdLineParser::E_NO_ARG, 0, "first description"},
		{opt2, '\0', "two", CmdLineParser::E_OPTIONAL_ARG, "optional", "second description"},
		{opt3, '3', "three", CmdLineParser::E_REQUIRED_ARG, 0, "third description"},
		{0, 0, 0, CmdLineParser::E_NO_ARG, 0, 0}
	};
	
	{
		int argc = 5;
		const char* argv[] = {
			"progname",
			"-1",
			"--two=abc",
			"-3",
			"1"
		};
		CmdLineParser parser(argc, argv, options2, CmdLineParser::E_NON_OPTION_ARGS_INVALID);
		unitAssert(parser.isSet(opt1));
		unitAssert(parser.isSet(opt2));
		unitAssert(parser.isSet(opt3));
		unitAssert(!parser.isSet(invalidOpt));
		unitAssert(parser.getOptionValue(opt2) == "abc");
		unitAssert(parser.getOptionValue(opt3) == "1");
	}
	{
		unitAssert(CmdLineParser::getUsage(options2) ==
			"Options:\n"
			"  -1                        first description\n"
			"  --two [arg]               second description (default is optional)\n"
			"  -3, --three <arg>         third description\n"
			);
	}


	{
		CmdLineParser::Option optionsOverlapping[] =
		{
			{opt1, '1', "XX", CmdLineParser::E_NO_ARG, 0, "first description"},
			{opt2, '2', "XXX", CmdLineParser::E_NO_ARG, 0, "second description"},
			{opt3, '3', "XXXX", CmdLineParser::E_NO_ARG, 0, "third description"},
			{0, 0, 0, CmdLineParser::E_NO_ARG, 0, 0}
		};

		int argc = 3;
		const char* argv[] = {
			"progname",
			"--XX",
			"--XXXX"
		};
		CmdLineParser parser(argc, argv, optionsOverlapping, CmdLineParser::E_NON_OPTION_ARGS_INVALID);
		unitAssert(parser.isSet(opt1));
		unitAssert(!parser.isSet(opt2));
		unitAssert(parser.isSet(opt3));
	}
}

Test* CmdLineParserTestCases::suite()
{
	TestSuite *testSuite = new TestSuite ("CmdLineParser");

	ADD_TEST_TO_SUITE(CmdLineParserTestCases, testSomething);

	return testSuite;
}

