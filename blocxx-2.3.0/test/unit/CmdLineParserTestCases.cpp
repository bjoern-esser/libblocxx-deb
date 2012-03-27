/*******************************************************************************
* Copyright (C) 2005,2009 Quest Software, Inc. All rights reserved.
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

#include "blocxx/CmdLineParser.hpp"
#include "blocxx/CerrAppender.hpp"
#include "blocxx/LogAppenderScope.hpp"


#include "blocxx/Logger.hpp"
#include "blocxx/Format.hpp"


using namespace blocxx;


AUTO_UNIT_TEST(CmdLineParser_testSomething)
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
		unitAssert(parser.isSet("-1"));
		unitAssert(parser.isSet("--one"));
		unitAssert(parser.isSet("-2"));
		unitAssert(parser.isSet("--two"));
		unitAssert(!parser.isSet("-3"));
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
	{
		CmdLineParser::Option options[] =
		{
			{opt1, 's', "something", CmdLineParser::E_NO_ARG, 0, "first description"},
			{opt2, 'o', "or-other", CmdLineParser::E_NO_ARG, 0, "second description"},
			{opt3, 'n', "never", CmdLineParser::E_NO_ARG, 0, "third description"},
			{0, 0, 0, CmdLineParser::E_NO_ARG, 0, 0}
		};
		StringArray arguments = String("name|--something|--or-other").tokenize("|");

		CmdLineParser parser(options);

		Array<int> optvalues = parser.getAllOptionIDs();

		unitAssertEquals(optvalues.size(), 3u);
		unitAssertEquals(optvalues[0], int(opt1));
		unitAssertEquals(optvalues[1], int(opt2));
		unitAssertEquals(optvalues[2], int(opt3));

		unitAssertEquals(parser.getShortOptionByID(opt1), "s");
		unitAssertEquals(parser.getShortOptionByID(opt2), "o");
		unitAssertEquals(parser.getShortOptionByID(opt3), "n");

		unitAssertEquals(parser.getLongOptionByID(opt1), "something");
		unitAssertEquals(parser.getLongOptionByID(opt2), "or-other");
		unitAssertEquals(parser.getLongOptionByID(opt3), "never");

		unitAssertEquals(parser.getDescriptionByID(opt1), "first description");
		unitAssertEquals(parser.getDescriptionByID(opt2), "second description");
		unitAssertEquals(parser.getDescriptionByID(opt3), "third description");


		parser.parse(arguments, CmdLineParser::E_NON_OPTION_ARGS_INVALID);

		unitAssert(parser.isSet(opt1));
		unitAssert(parser.isSet(opt2));
		unitAssert(!parser.isSet(opt3));
	}
}

AUTO_UNIT_TEST(CmdLineParser_testCaseSensitivity)
{
	CmdLineParser parser('-', "--", '=', CmdLineParser::E_CASE_SENSITIVE);

	parser.addOption('a', "option-a", CmdLineParser::E_NO_ARG, "Option \"a\"");

	unitAssertThrows(parser.parse(String("progname -A").tokenize(), CmdLineParser::E_NON_OPTION_ARGS_INVALID));
	unitAssertThrows(parser.parse(String("progname --Option-A").tokenize(), CmdLineParser::E_NON_OPTION_ARGS_INVALID));
	unitAssertNoThrow(parser.parse(String("progname -a").tokenize(), CmdLineParser::E_NON_OPTION_ARGS_INVALID));
	unitAssertNoThrow(parser.parse(String("progname --option-a").tokenize(), CmdLineParser::E_NON_OPTION_ARGS_INVALID));

	// No conflict.  All options here are case sensitive.
	unitAssertNoThrow(parser.addOption('A', "Option-A", CmdLineParser::E_NO_ARG, "Option \"A\""));
	unitAssertNoThrow(parser.addOption('B', "Option-B", CmdLineParser::E_NO_ARG, "Option \"B\""));
}

AUTO_UNIT_TEST(CmdLineParser_testCaseInsensitivity)
{
	CmdLineParser parser('-', "--", '=', CmdLineParser::E_CASE_INSENSITIVE);

	parser.addOption('a', "option-a", CmdLineParser::E_NO_ARG, "Option \"A\"");

	unitAssertNoThrow(parser.parse(String("progname -A").tokenize(), CmdLineParser::E_NON_OPTION_ARGS_INVALID));
	unitAssertNoThrow(parser.parse(String("progname --Option-A").tokenize(), CmdLineParser::E_NON_OPTION_ARGS_INVALID));
	unitAssertNoThrow(parser.parse(String("progname -a").tokenize(), CmdLineParser::E_NON_OPTION_ARGS_INVALID));
	unitAssertNoThrow(parser.parse(String("progname --option-a").tokenize(), CmdLineParser::E_NON_OPTION_ARGS_INVALID));

	// Case insensitive argument conflict with -a, --option-a
	unitAssertThrows(parser.addOption('A', "Option-A", CmdLineParser::E_NO_ARG, "Option \"A\""));
	unitAssertNoThrow(parser.addOption('B', "Option-B", CmdLineParser::E_NO_ARG, "Option \"B\""));
}


AUTO_UNIT_TEST(CmdLineParser_testAlternateSwitches)
{
	// LogAppenderScope lgr(new CerrAppender);

	// Windows-style switches.  Yes, there is ambiguity between short and long options.
	CmdLineParser parser('/', "/", ':', CmdLineParser::E_CASE_INSENSITIVE);
	parser.addOption('a', "ay", "default", "0");
	parser.addOption('b', "bee", CmdLineParser::E_REQUIRED_ARG, "1");
	parser.addOption('c', "see", CmdLineParser::E_NO_ARG, "2");
	parser.addOption('\0', "d", CmdLineParser::E_NO_ARG, "3");

	{
		String usage =
			"Options:\n"
			"  /a, /ay [arg]             0 (default is default)\n"
			"  /b, /bee <arg>            1\n"
			"  /c, /see                  2\n"
			"  /d                        3\n";

		unitAssertEquals(usage, parser.getUsage());
	}

	// Long option conflicts with an existing short option
	unitAssertThrows(parser.addOption('\0', "a", "", ""));
	// Long option conflicts with its own short option
	unitAssertThrows(parser.addOption('z', "z", "", ""));
	// Short option conflicts with another long option
	unitAssertThrows(parser.addOption('d', "dee", "", ""));
	unitAssertThrows(parser.addOption('A', "\0", "", ""));
	unitAssertThrows(parser.addOption('\0', "A", "", ""));
	unitAssertThrows(parser.addOption('D', "\0", "", ""));

	// required arg not present
	unitAssertThrows(parser.parse(String("progname /b").tokenize(), CmdLineParser::E_NON_OPTION_ARGS_INVALID));
	unitAssertThrows(parser.parse(String("progname /bee").tokenize(), CmdLineParser::E_NON_OPTION_ARGS_INVALID));
	unitAssertThrows(parser.parse(String("progname -a").tokenize(), CmdLineParser::E_NON_OPTION_ARGS_INVALID));
	unitAssertThrows(parser.parse(String("progname --ay").tokenize(), CmdLineParser::E_NON_OPTION_ARGS_INVALID));
	unitAssertNoThrow(parser.parse(String("progname /ay /").tokenize(), CmdLineParser::E_NON_OPTION_ARGS_INVALID));

	// Case insensitivity so these options are perfectly legal
	unitAssertNoThrow(parser.parse(String("progname /BeE value").tokenize(), CmdLineParser::E_NON_OPTION_ARGS_INVALID));
	unitAssertNoThrow(parser.parse(String("progname /sEe").tokenize(), CmdLineParser::E_NON_OPTION_ARGS_INVALID));
	unitAssertNoThrow(parser.parse(String("progname /A /a").tokenize(), CmdLineParser::E_NON_OPTION_ARGS_INVALID));
	unitAssertNoThrow(parser.parse(String("progname /D /d").tokenize(), CmdLineParser::E_NON_OPTION_ARGS_INVALID));

	{
		parser.parse(String("progname /b:1").tokenize(), CmdLineParser::E_NON_OPTION_ARGS_INVALID);
		unitAssert(parser.isSet("/b"));
		unitAssertEquals(parser.getOptionValue(parser.getOptionID("/b")), "1");
	}
	{
		parser.parse(String("progname /b 2").tokenize(), CmdLineParser::E_NON_OPTION_ARGS_INVALID);
		unitAssert(parser.isSet("/b"));
		unitAssertEquals(parser.getOptionValue(parser.getOptionID("/b")), "2");
	}
}

AUTO_UNIT_TEST(CmdLineParser_invalidOptionsStatic)
{
	//	LogAppenderScope lgr(new CerrAppender);

	enum
	{
		opt1,
		opt2,
		opt3,
		invalidOpt
	};

	CmdLineParser parser;

	{
		CmdLineParser::Option options[] =
		{
			{opt1, '1', "one", CmdLineParser::E_NO_ARG, 0, "first description"},
			{opt2, '1', "two", CmdLineParser::E_OPTIONAL_ARG, "optional", "second description"},
			{opt3, '3', "three", CmdLineParser::E_REQUIRED_ARG, 0, "third description"},
			{0, 0, 0, CmdLineParser::E_NO_ARG, 0, 0}
		};

		unitAssertThrows(parser = CmdLineParser(options));
	}
	{
		CmdLineParser::Option options[] =
		{
			{opt1, '1', "one", CmdLineParser::E_NO_ARG, 0, "first description"},
			{opt2, '2', "one", CmdLineParser::E_OPTIONAL_ARG, "optional", "second description"},
			{opt3, '3', "three", CmdLineParser::E_REQUIRED_ARG, 0, "third description"},
			{0, 0, 0, CmdLineParser::E_NO_ARG, 0, 0}
		};

		unitAssertThrows(parser = CmdLineParser(options));
	}
	{
		CmdLineParser::Option options[] =
		{
			{opt1, '1', "one", CmdLineParser::E_NO_ARG, 0, "first description"},
			{opt2, '2', "two", CmdLineParser::E_OPTIONAL_ARG, 0, "second description"},
			{opt3, '3', "three", CmdLineParser::E_REQUIRED_ARG, 0, "third description"},
			{0, 0, 0, CmdLineParser::E_NO_ARG, 0, 0}
		};

		unitAssertThrows(parser = CmdLineParser(options));
	}
}

AUTO_UNIT_TEST(CmdLineParser_invalidOptionsDynamic)
{
	//	LogAppenderScope lgr(new CerrAppender);

	{
		// Duplicated long option
		CmdLineParser parser;
		unitAssertNoThrow(parser.addOption('1', "one", CmdLineParser::E_NO_ARG, "first description"));
		unitAssertThrows(parser.addOption('2', "one", CmdLineParser::E_NO_ARG, "second description"));
	}
	{
		// Duplicated short option
		CmdLineParser parser;
		unitAssertNoThrow(parser.addOption('1', "one", CmdLineParser::E_NO_ARG, "first description"));
		unitAssertThrows(parser.addOption('1', "two", CmdLineParser::E_NO_ARG, "second description"));
	}
	{
		// Optional argument with no default
		CmdLineParser parser;
		unitAssertNoThrow(parser.addOption('1', "one", CmdLineParser::E_NO_ARG, "first description"));
		unitAssertThrows(parser.addOption('2', "two", CmdLineParser::E_OPTIONAL_ARG, "second description"));
	}
	{
		// Long and short are both empty
		CmdLineParser parser;
		unitAssertNoThrow(parser.addOption('1', "one", CmdLineParser::E_NO_ARG, "first description"));
		unitAssertThrows(parser.addOption('\0', "", CmdLineParser::E_NO_ARG, "second description"));
	}
}

AUTO_UNIT_TEST(CmdLineParser_addOption)
{
	//	LogAppenderScope lgr(new CerrAppender);
	Logger logger(TEST_COMPONENT_NAME);
	CmdLineParser parser;

	StringArray testOptions = String("<name> --foo").tokenize();

	unitAssertThrows(parser.getOptionID("--foo"));
	unitAssertThrows(parser.getOptionID("foo"));
	unitAssertThrows(parser.parse(testOptions, CmdLineParser::E_NON_OPTION_ARGS_INVALID));

	int id = parser.addOption('\0', "foo", CmdLineParser::E_NO_ARG, "just an option");
	BLOCXX_LOG_DEBUG3(logger, Format("Option foo was assigned ID %1", id));

	unitAssertNoThrow(parser.getOptionID("--foo"));
	unitAssertNoThrow(parser.getOptionID("foo"));
	unitAssertNoThrow(parser.parse(testOptions, CmdLineParser::E_NON_OPTION_ARGS_INVALID));
	unitAssert(parser.isSet(id));

	int zooId = 100;
	unitAssertNoThrow(parser.addOptionWithID(zooId, 'z', "zoo", CmdLineParser::E_NO_ARG, "The zoo is for animals"));
	unitAssertNoThrow(parser.parse(String("<name> -z").tokenize(), CmdLineParser::E_NON_OPTION_ARGS_INVALID));
	unitAssert(parser.isSet(zooId));
}

AUTO_UNIT_TEST(CmdLineParser_addOptionOptional)
{
	//	LogAppenderScope lgr(new CerrAppender);

	CmdLineParser parser;
	int id = parser.addOption('f', "foo", "ick", "An option for \"foo\"");

	{
		StringArray testOptions = String("<name> --foo=1==1").tokenize();
		unitAssertNoThrow(parser.parse(testOptions, CmdLineParser::E_NON_OPTION_ARGS_INVALID));
		unitAssert(parser.isSet(id));
		unitAssert(parser.isSet("--foo"));
		unitAssert(parser.isSet("-f"));
		unitAssertThrows(parser.isSet("--ick"));
		unitAssertThrows(parser.isSet("-n"));
		unitAssertEquals("1==1", parser.getOptionValue(id));
	}
	{
		StringArray testOptions = String("<name> -f1==1").tokenize();
		unitAssertNoThrow(parser.parse(testOptions, CmdLineParser::E_NON_OPTION_ARGS_INVALID));
		unitAssert(parser.isSet(id));
		unitAssertEquals("1==1", parser.getOptionValue(id));
	}
	{
		StringArray testOptions = String("<name> --foo").tokenize();
		unitAssertNoThrow(parser.parse(testOptions, CmdLineParser::E_NON_OPTION_ARGS_INVALID));
		unitAssert(parser.isSet(id));
		unitAssertEquals("ick", parser.getOptionValue(id));
	}
	{
		StringArray testOptions = String("<name> --foo bar").tokenize();
		unitAssertNoThrow(parser.parse(testOptions, CmdLineParser::E_NON_OPTION_ARGS_INVALID));
		unitAssert(parser.isSet(id));
		unitAssertEquals("bar", parser.getOptionValue(id));
	}
	{
		StringArray testOptions = String("<name> --foo=baz").tokenize();
		unitAssertNoThrow(parser.parse(testOptions, CmdLineParser::E_NON_OPTION_ARGS_INVALID));
		unitAssert(parser.isSet(id));
		unitAssertEquals("baz", parser.getOptionValue(id));
	}
	{
		StringArray testOptions = String("<name> --foo=").tokenize();
		unitAssertNoThrow(parser.parse(testOptions, CmdLineParser::E_NON_OPTION_ARGS_INVALID));
		unitAssert(parser.isSet(id));
		unitAssertEquals("", parser.getOptionValue(id));
	}
	{
		StringArray testOptions = String("<name> --foo").tokenize();
		testOptions.append("");
		unitAssertNoThrow(parser.parse(testOptions, CmdLineParser::E_NON_OPTION_ARGS_INVALID));
		unitAssert(parser.isSet(id));
		unitAssertEquals("", parser.getOptionValue(id));
	}
	{
		StringArray testOptions = String("<name> -f").tokenize();
		testOptions.append("");
		unitAssertNoThrow(parser.parse(testOptions, CmdLineParser::E_NON_OPTION_ARGS_INVALID));
		unitAssert(parser.isSet(id));
		unitAssertEquals("", parser.getOptionValue(id));
	}
	{
		StringArray testOptions = String("<name> -f").tokenize();
		unitAssertNoThrow(parser.parse(testOptions, CmdLineParser::E_NON_OPTION_ARGS_INVALID));
		unitAssert(parser.isSet(id));
		unitAssertEquals("ick", parser.getOptionValue(id));
	}
	{
		StringArray testOptions = String("<name> -fquux").tokenize();
		unitAssertNoThrow(parser.parse(testOptions, CmdLineParser::E_NON_OPTION_ARGS_INVALID));
		unitAssert(parser.isSet(id));
		unitAssertEquals("quux", parser.getOptionValue(id));
	}
	{
		StringArray testOptions = String("<name> -f=zzyzx").tokenize();
		unitAssertNoThrow(parser.parse(testOptions, CmdLineParser::E_NON_OPTION_ARGS_INVALID));
		unitAssert(parser.isSet(id));
		unitAssertEquals("zzyzx", parser.getOptionValue(id));
	}
	{
		StringArray testOptions = String("<name> -f=").tokenize();
		unitAssertNoThrow(parser.parse(testOptions, CmdLineParser::E_NON_OPTION_ARGS_INVALID));
		unitAssert(parser.isSet(id));
		unitAssertEquals("", parser.getOptionValue(id));
	}
	{
		StringArray testOptions = String("<name> -f xyzzx").tokenize();
		unitAssertNoThrow(parser.parse(testOptions, CmdLineParser::E_NON_OPTION_ARGS_INVALID));
		unitAssert(parser.isSet(id));
		unitAssertEquals("xyzzx", parser.getOptionValue(id));
	}
	{
		StringArray testOptions = String("<name> -f --foo").tokenize();
		unitAssertNoThrow(parser.parse(testOptions, CmdLineParser::E_NON_OPTION_ARGS_INVALID));
		unitAssert(parser.isSet(id));
		unitAssertEquals("ick", parser.getOptionValue(id));
	}
	{
		StringArray testOptions = String("<name> -f junk --foo").tokenize();
		unitAssertNoThrow(parser.parse(testOptions, CmdLineParser::E_NON_OPTION_ARGS_INVALID));
		unitAssert(parser.isSet(id));
		unitAssertEquals("ick", parser.getOptionValue(id));
	}
	{
		StringArray testOptions = String("<name> -f=bar --foo=baz -f bar --foo").tokenize();
		unitAssertNoThrow(parser.parse(testOptions, CmdLineParser::E_NON_OPTION_ARGS_INVALID));
		unitAssert(parser.isSet(id));
		unitAssertEquals("ick", parser.getOptionValue(id));
	}
}

AUTO_UNIT_TEST(CmdLineParser_usage)
{
	//	LogAppenderScope lgr(new CerrAppender);
	CmdLineParser parser;
	parser.addOption('1', "one", CmdLineParser::E_NO_ARG, "first description");
	parser.addOption('2', "two", "optional", "second description");
	parser.addOption('3', "three", CmdLineParser::E_REQUIRED_ARG, "third description");
	parser.addOption('4', "four", CmdLineParser::E_NO_ARG, "fourth description", CmdLineParser::E_OPTION_NOT_VISIBLE);

	const String header = "Options:\n";
	const String one = "  -1, --one                 first description\n";
	const String two = "  -2, --two [arg]           second description (default is optional)\n";
	const String three = "  -3, --three <arg>         third description\n";
	const String four = "  -4, --four                fourth description\n";

	unitAssertEquals(header + one + two + three, parser.getUsage());
	unitAssertEquals(header + one + two + three + four, parser.getUsage(80, CmdLineParser::E_USAGE_SHOW_ALL));

	parser.addOption('\0', 0, CmdLineParser::E_GROUPING, "The following are grouped");

	parser.addOption('5', "five", "default value", "fifth description, but this one is fairly long and should easily cause some form of wrapping because of the output limit", CmdLineParser::E_OPTION_NOT_VISIBLE);
	parser.addOption('6', "six", CmdLineParser::E_REQUIRED_ARG, "sixth description");

	const String group =
		"\n"
		"  The following are grouped\n"
		"\n";

	const String five =
		"  -5, --five [arg]          fifth description, but this one is fairly long and\n"
		"                            should easily cause some form of wrapping because of\n"
		"                            the output limit (default is \"default value\")\n";

	const String six = "  -6, --six <arg>           sixth description\n";

	unitAssertEquals(header + one + two + three + four + group + five + six, parser.getUsage(80, CmdLineParser::E_USAGE_SHOW_ALL));
	unitAssertEquals(header + one + two + three + group + six, parser.getUsage(80, CmdLineParser::E_USAGE_SHOW_VISIBLE));


	// Test some small widths.
	unitAssertEquals(
		"Options:\n"
		"  -1, --one                 first\n"
		"                            description\n"
		"  -2, --two [arg]           second\n"
		"                            description\n"
		"                            (default is\n"
		"                            optional)\n"
		"  -3, --three <arg>         third\n"
		"                            description\n"
		"  -4, --four                fourth\n"
		"                            description\n"
		"\n"
		"  The following are grouped\n"
		"\n"
		"  -5, --five [arg]          fifth\n"
		"                            description,\n"
		"                            but this one\n"
		"                            is fairly\n"
		"                            long and\n"
		"                            should\n"
		"                            easily cause\n"
		"                            some form of\n"
		"                            wrapping\n"
		"                            because of\n"
		"                            the output\n"
		"                            limit\n"
		"                            (default is\n"
		"                            \"default\n"
		"                            value\")\n"
		"  -6, --six <arg>           sixth\n"
		"                            description\n",
		parser.getUsage(40, CmdLineParser::E_USAGE_SHOW_ALL));

	unitAssertEquals(
		"Options:\n"
		"  -1, --one                 first description\n"
		"  -2, --two [arg]           second description (default is\n"
		"                            optional)\n"
		"  -3, --three <arg>         third description\n"
		"  -4, --four                fourth description\n"
		"\n"
		"  The following are grouped\n"
		"\n"
		"  -5, --five [arg]          fifth description, but this one is\n"
		"                            fairly long and should easily cause\n"
		"                            some form of wrapping because of the\n"
		"                            output limit (default is \"default\n"
		"                            value\")\n"
		"  -6, --six <arg>           sixth description\n",
		parser.getUsage(65, CmdLineParser::E_USAGE_SHOW_ALL));


	CmdLineParser copyOfParser1(parser);
	unitAssertEquals(parser.getUsage(), copyOfParser1.getUsage());
	CmdLineParser copyOfParser2;
	copyOfParser2 = parser;
	unitAssertEquals(parser.getUsage(), copyOfParser2.getUsage());
}
