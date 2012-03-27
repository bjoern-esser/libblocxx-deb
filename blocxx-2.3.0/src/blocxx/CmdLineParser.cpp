/*******************************************************************************
* Copyright (C) 2009, Quest Software, Inc. All rights reserved.
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
 * @author Kevin Harris
 */

#include "blocxx/BLOCXX_config.h"
#include "blocxx/CmdLineParser.hpp"
#include "blocxx/Array.hpp"
#include "blocxx/ExceptionIds.hpp"
#include "blocxx/StringBuffer.hpp"
#include "blocxx/Assertion.hpp"
#include "blocxx/Cstr.hpp"
#include "blocxx/Logger.hpp"
#include "blocxx/Format.hpp"

#include <algorithm>


namespace BLOCXX_NAMESPACE
{

	BLOCXX_DEFINE_EXCEPTION_WITH_ID(CmdLineParser)
	namespace
	{
		const char* const COMPONENT_NAME = "blocxx.CmdLineParser";
	}

// Similar to CmdLineParser::Option, but is used internally to the
// CmdLineParser to avoid source compatibility breakage for some new features.
struct ParserOptionImpl : public IntrusiveCountableBase
{
	ParserOptionImpl();
	// Convert the exposed CmdLineParser::Option to the internal implementation
	ParserOptionImpl(const CmdLineParser::Option& opt);

	ParserOptionImpl(int optID, char shortOption, const String& longOption, CmdLineParser::EArgumentTypeFlag argType, bool haveDefault, const String& defValue, const String& desc, CmdLineParser::EOptionVisibilityTypeFlag visFlag);

	int id;
	char shortopt;
	String longopt;
	CmdLineParser::EArgumentTypeFlag argtype;
	bool haveDefaultValue;
	String defaultValue;
	String description;
	CmdLineParser::EOptionVisibilityTypeFlag visibility;
};

ParserOptionImpl::ParserOptionImpl()
	: id(-1)
	, shortopt('\0')
	, haveDefaultValue(false)
	, visibility(CmdLineParser::E_OPTION_VISIBLE)
{
}

ParserOptionImpl::ParserOptionImpl(const CmdLineParser::Option& opt)
	: id(opt.id)
	, shortopt(opt.shortopt)
	, longopt(opt.longopt)
	, argtype(opt.argtype)
	, haveDefaultValue((argtype == CmdLineParser::E_OPTIONAL_ARG) && (opt.defaultValue != 0))
	, defaultValue(opt.defaultValue)
	, description(opt.description)
	, visibility(CmdLineParser::E_OPTION_VISIBLE)
{
}

ParserOptionImpl::ParserOptionImpl(int optID, char shortOption, const String& longOption, CmdLineParser::EArgumentTypeFlag argType, bool haveDefault, const String& defValue, const String& desc, CmdLineParser::EOptionVisibilityTypeFlag visFlag)
	: id(optID)
	, shortopt(shortOption)
	, longopt(longOption)
	, argtype(argType)
	, haveDefaultValue(haveDefault)
	, defaultValue(defValue)
	, description(desc)
	, visibility(visFlag)
{
}

namespace
{
/////////////////////////////////////////////////////////////////////////////
	struct longOptIs
	{
		longOptIs(const String& longOpt, CmdLineParser::ECaseSensitivityFlag flag)
			: m_longOpt(longOpt)
			, m_caseSensitivity(flag)
		{
		}

		bool operator()(const ParserOptionImplRef& x) const
		{
			if( m_caseSensitivity == CmdLineParser::E_CASE_SENSITIVE )
			{
				return m_longOpt.equals(x->longopt);
			}
			else // CmdLineParser::E_CASE_INSENSITIVE
			{
				return m_longOpt.equalsIgnoreCase(x->longopt);
			}
		}

		String m_longOpt;
		CmdLineParser::ECaseSensitivityFlag m_caseSensitivity;
	};

/////////////////////////////////////////////////////////////////////////////
	struct shortOptIs
	{
		shortOptIs(char shortOpt, CmdLineParser::ECaseSensitivityFlag flag)
			: m_shortOpt(shortOpt)
			,  m_caseSensitivity(flag)
		{
			if( m_caseSensitivity == CmdLineParser::E_CASE_INSENSITIVE )
			{
				m_shortOpt = toupper(m_shortOpt);
			}
		}

		bool operator()(const ParserOptionImplRef& x) const
		{
			char opt = x->shortopt;
			if( m_caseSensitivity == CmdLineParser::E_CASE_INSENSITIVE )
			{
				opt = toupper(opt);
			}
			return m_shortOpt == opt;
		}

		char m_shortOpt;
		CmdLineParser::ECaseSensitivityFlag m_caseSensitivity;
	};

/////////////////////////////////////////////////////////////////////////////
	struct optIsID
	{
		optIsID(int id) : m_id(id) {}

		bool operator()(const ParserOptionImplRef& x) const
		{
			return m_id == x->id;
		}

		int m_id;
	};

	bool argIsEmpty(char* arg)
	{
		return arg[0] == '\0';
	}

	bool argIsEmpty(const String& arg)
	{
		return arg.empty();
	}

	///////////////////////////////////////////////////////////////////////////
	// Split an option (eg. --foo=bar) into the base option and value (eg, base="foo" and value="bar")
	enum EOptionType { E_OPTION_INVALID = -1, E_OPTION_SHORT, E_OPTION_LONG };
	EOptionType splitArgument(const String& option, String& baseOption, String& value,
		bool& valueAttached, char shortPrefix, const String& longPrefix, char valueSeparator)
	{
		EOptionType retval = E_OPTION_INVALID;
		Logger logger(COMPONENT_NAME);
		BLOCXX_LOG_DEBUG3(logger, Format("splitArgument called for \"%1\" with short prefix='%2', long prefix=\"%3\", separator='%4'",
				option, shortPrefix, longPrefix, valueSeparator));

		const size_t valueIndex = option.indexOf(valueSeparator);
		if( valueIndex != String::npos )
		{
			value = option.substring(valueIndex + 1);
			valueAttached = true;
		}
		else
		{
			value = String();
			valueAttached = false;
		}

		String optionWithoutValue = option.substring(0, valueIndex);

		if( optionWithoutValue.startsWith(longPrefix) )
		{
			// The horrible case of ambiguity between long and short options being equal in switch character
			if( (optionWithoutValue.length() == 2) && (longPrefix.length() == 1) && (longPrefix.charAt(0) == shortPrefix) )
			{
				baseOption = optionWithoutValue.substring(1,1);
				retval = E_OPTION_SHORT;
			}
			else
			{
				baseOption = optionWithoutValue.substring(longPrefix.length());
				retval = E_OPTION_LONG;
			}
		}
		else if( optionWithoutValue.startsWith(shortPrefix) )
		{
			baseOption = optionWithoutValue.substring(1,1);
			retval = E_OPTION_SHORT;
		}

		if( retval == E_OPTION_SHORT )
		{
			// A short option can have the value together with it
			// (e.g. -I/opt/vintela/include).  This needs to exclude the
			// "-f=abc=def" case.
			if( (option.length() > 2) && (option[2] != valueSeparator) )
			{
				value = option.substring(2);
				valueAttached = true;
			}
		}

		BLOCXX_LOG_DEBUG3(logger, Format("splitArgument returning %1. base=\"%2\", value=\"%3\", attached=%<4:!>", retval, baseOption, value, valueAttached));

		return retval;
	}

/////////////////////////////////////////////////////////////////////////////
	template <typename T>
	void parseCmdLine(T argvBegin, T argvEnd, CmdLineParser::EAllowNonOptionArgsFlag allowNonOptionArgs,
		Array<ParserOptionImplRef>& options,
		SortedVectorMap<int, StringArray>& parsedOptions,
		StringArray& nonOptionArgs,
		char shortOptionPrefix,
		const String& longOptionPrefix,
		char valueSeparator,
		CmdLineParser::ECaseSensitivityFlag caseSensitivity
	)
	{
		Logger logger(COMPONENT_NAME);
		parsedOptions.clear();
		nonOptionArgs.clear();

		// Determine if there is ambiguity between short and long options
		bool ambiguityExists = (longOptionPrefix.length() == 1) && (longOptionPrefix.charAt(0) == shortOptionPrefix);

		T argv = argvBegin;

		// skip the first argv, which is the program name and loop through the rest
		BLOCXX_LOG_DEBUG3(logger, Format("Skipping argv[0] \"%1\"", *argv));
		for( ++argv; argv != argvEnd; ++argv )
		{
			const String argAsSupplied(*argv);
			BLOCXX_LOG_DEBUG3(logger, Format("Examining argument \"%1\"", argAsSupplied));

			String baseOption;
			String value;
			bool valueAttached = false;

			// look for an option
			EOptionType optionType = splitArgument(argAsSupplied, baseOption, value, valueAttached, shortOptionPrefix, longOptionPrefix, valueSeparator);

			String optionWithoutValue;
			if( optionType == E_OPTION_SHORT )
			{
				optionWithoutValue = String(shortOptionPrefix) + baseOption.substring(0, 1);
			}
			else
			{
				optionWithoutValue = longOptionPrefix + baseOption;
			}

			Array<ParserOptionImplRef>::const_iterator theOpt = options.end();
			if( (optionType == E_OPTION_LONG) && !baseOption.empty() )
			{
				theOpt = std::find_if (options.begin(), options.end(), longOptIs(baseOption, caseSensitivity));
				// We do not need to check for ambiguity with short options here.
				// If the long option was only one character long, the split would
				// have returned it as a short option.
			}
			else if( (optionType == E_OPTION_SHORT) && !baseOption.empty() )
			{
				theOpt = std::find_if (options.begin(), options.end(), shortOptIs(baseOption[0], caseSensitivity));

				if( theOpt == options.end() )
				{
					// Since the short option did not exist, we need to check the
					// ambiguous short/long option case
					if( ambiguityExists )
					{
						theOpt = std::find_if (options.begin(), options.end(), longOptIs(baseOption, caseSensitivity));
					}
				}
			}
			else
			{
				// non-option argument
				if (allowNonOptionArgs == CmdLineParser::E_NON_OPTION_ARGS_INVALID)
				{
					String text = Format("Invalid non-option argument \"%1\"", argAsSupplied);
					BLOCXX_LOG_DEBUG3(logger, text.c_str());
					BLOCXX_THROW_ERR(CmdLineParserException, text.c_str(), CmdLineParser::E_INVALID_NON_OPTION_ARG);
				}
				else
				{
					nonOptionArgs.push_back(argAsSupplied);
					BLOCXX_LOG_DEBUG3(logger, Format("Accepting non-option argument \"%1\"", argAsSupplied));
					continue;
				}
			}

			if (theOpt == options.end())
			{
				String errText = Format("Option \"%1\" is not defined", optionWithoutValue);
				BLOCXX_LOG_DEBUG3(logger, errText);
				BLOCXX_THROW_ERR(CmdLineParserException, errText.c_str(), CmdLineParser::E_INVALID_OPTION);
			}

			BLOCXX_LOG_DEBUG3(logger, Format("Argument \"%1\" has id %2", optionWithoutValue, (*theOpt)->id));

			if ((*theOpt)->argtype == CmdLineParser::E_NO_ARG)
			{
				// It is an error to attach a value to something that accepts no arguments.
				if( valueAttached )
				{
					String errText = "Option \"" + optionWithoutValue + "\" does not take a value";
					BLOCXX_THROW_ERR(CmdLineParserException, errText.c_str(), CmdLineParser::E_INVALID_OPTION);
				}

				parsedOptions[(*theOpt)->id]; // if one is already there, don't modify it, else insert a new one
				continue;
			}

			bool defaultRequired = false;
			if (valueAttached)
			{
				BLOCXX_LOG_DEBUG3(logger, Format("Using attached value \"%1\" for argument \"%2\"", value, baseOption));
			}
			else if( (argv + 1) == argvEnd )
			{
				defaultRequired = true;
				BLOCXX_LOG_DEBUG3(logger, Format("Ran out of arguments for \"%1\".  Default required.", optionWithoutValue));
			}
			else
			{
				String nextArgString = *(argv + 1);
				bool useNext = false;
				if( nextArgString == String(shortOptionPrefix) )
				{
					useNext = true;
				}
				else if( !nextArgString.startsWith(longOptionPrefix) && !nextArgString.startsWith(shortOptionPrefix) )
				{
					useNext = true;
				}
				else
				{
					defaultRequired = true;
					BLOCXX_LOG_DEBUG3(logger, Format("Argument after \"%1\" (\"%2\") is another option. Default required.", optionWithoutValue, nextArgString));
				}

				if( useNext )
				{
					value = nextArgString;
					++argv;
					BLOCXX_LOG_DEBUG3(logger, Format("Using value \"%1\" for argument \"%2\"", value, optionWithoutValue));
				}
			}

			if( defaultRequired )
			{
				if (((*theOpt)->argtype == CmdLineParser::E_OPTIONAL_ARG) && (*theOpt)->haveDefaultValue)
				{
					value = (*theOpt)->defaultValue;
					BLOCXX_LOG_DEBUG3(logger, Format("Using default value \"%1\" for argument \"%2\"", value, optionWithoutValue));
				}
				else
				{
					BLOCXX_LOG_DEBUG3(logger, Format("No argument value for \"%1\"", optionWithoutValue));
					BLOCXX_THROW_ERR(CmdLineParserException, optionWithoutValue.c_str(), CmdLineParser::E_MISSING_ARGUMENT);
				}
			}

			parsedOptions[(*theOpt)->id].push_back(value);
		}
	}
}


CmdLineParser::CmdLineParser(char shortOptionPrefix, const String& longOptionPrefix, char valueSeparator, ECaseSensitivityFlag caseSensitivity)
	: m_lowestInternalID(-1)
	, m_shortOptionPrefix(shortOptionPrefix)
	, m_longOptionPrefix(longOptionPrefix)
	, m_valueSeparator(valueSeparator)
	, m_caseSensitivity(caseSensitivity)
{
}

CmdLineParser::CmdLineParser(const CmdLineParser& p)
	: m_parsedOptions(p.m_parsedOptions)
	, m_nonOptionArgs(p.m_nonOptionArgs)
	, m_options(p.m_options)
	, m_lowestInternalID(p.m_lowestInternalID)
	, m_shortOptionPrefix(p.m_shortOptionPrefix)
	, m_longOptionPrefix(p.m_longOptionPrefix)
	, m_valueSeparator(p.m_valueSeparator)
	, m_caseSensitivity(p.m_caseSensitivity)
{
}

CmdLineParser::~CmdLineParser()
{
}

CmdLineParser& CmdLineParser::operator=(const CmdLineParser& p)
{
	m_parsedOptions = p.m_parsedOptions;
	m_nonOptionArgs = p.m_nonOptionArgs;
	m_options = p.m_options;
	m_lowestInternalID = p.m_lowestInternalID;
	m_shortOptionPrefix = p.m_shortOptionPrefix;
	m_longOptionPrefix = p.m_longOptionPrefix;
	m_valueSeparator = p.m_valueSeparator;
	m_caseSensitivity = p.m_caseSensitivity;

	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CmdLineParser::CmdLineParser(int argc, char const* const* const argv, const Option* options, EAllowNonOptionArgsFlag allowNonOptionArgs, char shortOptionPrefix, const String& longOptionPrefix, char valueSeparator, ECaseSensitivityFlag caseSensitivity)
	: m_lowestInternalID(-1)
	, m_shortOptionPrefix(shortOptionPrefix)
	, m_longOptionPrefix(longOptionPrefix)
	, m_valueSeparator(valueSeparator)
	, m_caseSensitivity(caseSensitivity)
{
	addOptions(options);
	parse(argc, argv, allowNonOptionArgs);
}

CmdLineParser::CmdLineParser(const Option* options, char shortOptionPrefix, const String& longOptionPrefix, char valueSeparator, ECaseSensitivityFlag caseSensitivity)
	: m_lowestInternalID(-1)
	, m_shortOptionPrefix(shortOptionPrefix)
	, m_longOptionPrefix(longOptionPrefix)
	, m_valueSeparator(valueSeparator)
	, m_caseSensitivity(caseSensitivity)
{
	addOptions(options);
}


void CmdLineParser::verifyOptionValidity(const ParserOptionImplRef& opt) const
{
	Logger logger(COMPONENT_NAME);

	// Both long and short cannot be empty.
	BLOCXX_LOG_DEBUG3(logger, "Checking both empty");
	if( opt->argtype != CmdLineParser::E_GROUPING )
	{
		if( opt->longopt.empty() && (opt->shortopt == 0) )
		{
			const char* errorText = "Both long and short options cannot be empty";
			BLOCXX_LOG_DEBUG3(logger, errorText);
			BLOCXX_THROW_ERR(CmdLineParserException, errorText, E_INVALID_OPTION);
		}
	}
	else if( !opt->longopt.empty() || (opt->shortopt != 0) )
	{
		const char* errorText = "Grouping options cannot have long or short option values";
		BLOCXX_LOG_DEBUG3(logger, errorText);
		BLOCXX_THROW_ERR(CmdLineParserException, errorText, E_INVALID_OPTION);
	}

	bool ambiguityExists = (m_longOptionPrefix.length() == 1) && (m_longOptionPrefix.charAt(0) == m_shortOptionPrefix);

	// Duplicated option values are not allowed.
	if( !opt->longopt.empty() )
	{
		BLOCXX_LOG_DEBUG3(logger, Format("Checking for existing long option \"%1%2\"", m_longOptionPrefix, opt->longopt));
		if( std::find_if(m_options.begin(), m_options.end(),
				longOptIs(opt->longopt, m_caseSensitivity)) != m_options.end() )
		{
			String errorText = Format("Long option \"%1%2\" is already in use", m_longOptionPrefix, opt->longopt);
			BLOCXX_LOG_DEBUG3(logger, errorText);
			BLOCXX_THROW_ERR(CmdLineParserException, errorText.c_str(), E_INVALID_OPTION);
		}
		if( ambiguityExists && (opt->longopt.length() == 1) )
		{
			if( std::find_if(m_options.begin(), m_options.end(),
					shortOptIs(opt->longopt.charAt(0), m_caseSensitivity)) != m_options.end() )
			{
				String errorText = Format("Long \"%1%2\" conflicts with a short option", m_longOptionPrefix, opt->longopt);
				BLOCXX_LOG_DEBUG3(logger, errorText);
				BLOCXX_THROW_ERR(CmdLineParserException, errorText.c_str(), E_INVALID_OPTION);
			}
		}
	}
	if( opt->shortopt != 0 )
	{
		BLOCXX_LOG_DEBUG3(logger, Format("Checking for existing short option \"%1%2\"", m_shortOptionPrefix, opt->shortopt));
		if( std::find_if(m_options.begin(), m_options.end(),
				shortOptIs(opt->shortopt, m_caseSensitivity)) != m_options.end() )
		{
			String errorText = Format("Short option \"%1%2\" is already in use", m_shortOptionPrefix, opt->shortopt);
			BLOCXX_LOG_DEBUG3(logger, errorText);
			BLOCXX_THROW_ERR(CmdLineParserException, errorText.c_str(), E_INVALID_OPTION);
		}

		if( ambiguityExists )
		{
			if( std::find_if(m_options.begin(), m_options.end(),
					longOptIs(String(opt->shortopt), m_caseSensitivity)) != m_options.end() )
			{
				String errorText = Format("Short option \"%1%2\" conflicts with a long option", m_shortOptionPrefix, opt->shortopt);
				BLOCXX_LOG_DEBUG3(logger, errorText);
				BLOCXX_THROW_ERR(CmdLineParserException, errorText.c_str(), E_INVALID_OPTION);
			}
			// Long and short conflict with themselves
			if( (opt->longopt.length() == 1) && (opt->longopt.charAt(0) == opt->shortopt) )
			{
				String errorText = Format("Short option \"%1%2\" conflicts its own long option \"%3%2\"", m_shortOptionPrefix, opt->shortopt, m_longOptionPrefix);
				BLOCXX_LOG_DEBUG3(logger, errorText);
				BLOCXX_THROW_ERR(CmdLineParserException, errorText.c_str(), E_INVALID_OPTION);
			}
		}
	}

	// An option specified as optional must have a default value.
	if( opt->argtype == E_OPTIONAL_ARG )
	{
		String optname = (opt->longopt.empty()?m_shortOptionPrefix + String(opt->shortopt):m_longOptionPrefix + opt->longopt);
		BLOCXX_LOG_DEBUG3(logger, Format("Checking for invalid optional value for \"%1\"", optname));
		if( !opt->haveDefaultValue )
		{
			String errorText = Format("Argument \"%1\" must have default value to be optional", optname);
			BLOCXX_LOG_DEBUG3(logger, errorText);
			BLOCXX_THROW_ERR(CmdLineParserException, errorText.c_str(), E_INVALID_OPTION);
		}
	}

	BLOCXX_LOG_DEBUG3(logger, "Option has no apparent errors.");
}

void CmdLineParser::addOptions(const Option* options)
{
	BLOCXX_ASSERT(options != 0);

	// m_options is an array terminated by a final entry that has a '\0' shortopt && 0 longopt.
	const Option* optionsEnd(options);
	while (optionsEnd->shortopt != '\0' || optionsEnd->longopt != 0)
	{
		ParserOptionImplRef opt(new ParserOptionImpl(*optionsEnd++));

		verifyOptionValidity(opt);

		m_options.append(opt);
	}
}

int CmdLineParser::addOption(char shortOption, const String& longOption, EArgumentTypeFlag argType, const String& description, EOptionVisibilityTypeFlag visibility)
{
	// If an exception is thrown in addOptionWithID, no changes are made to the
	// class, so the id decrement needs to be done after it is added.
	addOptionWithID(m_lowestInternalID - 1, shortOption, longOption, argType, description, visibility);
	return --m_lowestInternalID;
}

int CmdLineParser::addOption(char shortOption, const String& longOption, const String& defaultValue, const String& description, EOptionVisibilityTypeFlag visibility)
{
	// If an exception is thrown in addOptionWithID, no changes are made to the
	// class, so the id decrement needs to be done after it is added.
	addOptionWithID(m_lowestInternalID - 1, shortOption, longOption, defaultValue, description, visibility);
	return --m_lowestInternalID;
}

void CmdLineParser::addOptionWithID(int id, char shortOption, const String& longOption, EArgumentTypeFlag argType, const String& description, EOptionVisibilityTypeFlag visibility)
{
	ParserOptionImplRef opt(new ParserOptionImpl(id, shortOption, longOption,
			argType, false, String(), description, visibility));

	// Check for user error.
	verifyOptionValidity(opt);

	m_options.append(opt);
}

void CmdLineParser::addOptionWithID(int id, char shortOption, const String& longOption, const String& defaultValue, const String& description, EOptionVisibilityTypeFlag visibility)
{
	ParserOptionImplRef opt(new ParserOptionImpl(id, shortOption, longOption,
			E_OPTIONAL_ARG, true, defaultValue, description, visibility));

	// Check for user error.
	verifyOptionValidity(opt);

	m_options.append(opt);
}

void CmdLineParser::parse(const StringArray& arguments, EAllowNonOptionArgsFlag allowNonOptionArgs)
{
	BLOCXX_ASSERT(arguments.size() > 0); // have to get at least the name
	parseCmdLine(arguments.begin(), arguments.end(), allowNonOptionArgs, m_options, m_parsedOptions, m_nonOptionArgs, m_shortOptionPrefix, m_longOptionPrefix, m_valueSeparator, m_caseSensitivity);
}


int CmdLineParser::getOptionID(const String& option) const
{
	Array<ParserOptionImplRef>::const_iterator theOpt = m_options.end();

	if( option.empty() )
	{
		BLOCXX_THROW_ERR(CmdLineParserException, "<empty option>", E_MISSING_OPTION);
	}

	if( option.startsWith(m_longOptionPrefix) )
	{
		if( option.length() > m_longOptionPrefix.size() )
		{
			theOpt = std::find_if (m_options.begin(), m_options.end(),
				longOptIs(option.substring(m_longOptionPrefix.length()), m_caseSensitivity));

			// If we didn't find it as a long option, but some ambiguity exists, recheck as a short.
			bool ambiguityExists = (m_longOptionPrefix.length() == 1) && (m_longOptionPrefix.charAt(0) == m_shortOptionPrefix);
			if( (theOpt == m_options.end()) && ambiguityExists && (option.length() == 2) )
			{
				theOpt = std::find_if (m_options.begin(), m_options.end(),
					shortOptIs(option.charAt(1), m_caseSensitivity));
			}
		}
	}
	else if( option.startsWith(m_shortOptionPrefix) )
	{
		if( option.length() == 2 )
		{
			theOpt = std::find_if (m_options.begin(), m_options.end(),
				shortOptIs(option.charAt(1), m_caseSensitivity));
		}
	}
	// Allow searching by the option without the leading '-' or "--"
	else if( option.length() > 1 )
	{
		theOpt = std::find_if (m_options.begin(), m_options.end(), longOptIs(option, m_caseSensitivity));
	}
	else if( option.length() == 1 )
	{
		theOpt = std::find_if (m_options.begin(), m_options.end(),
			shortOptIs(option.charAt(0), m_caseSensitivity));
	}

	if( theOpt == m_options.end() )
	{
		BLOCXX_THROW_ERR(CmdLineParserException, Format("Invalid option \"%1\"", option).c_str(), E_INVALID_OPTION);
	}

	return (*theOpt)->id;
}

void CmdLineParser::parse(int argc, char const* const* const argv, EAllowNonOptionArgsFlag allowNonOptionArgs)
{
	BLOCXX_ASSERT(argc > 0); // have to get at least the name
	BLOCXX_ASSERT(argv != 0);

	char const* const* argvBegin = argv;
	char const* const* argvEnd = argv + argc;

	parseCmdLine(argvBegin, argvEnd, allowNonOptionArgs, m_options, m_parsedOptions, m_nonOptionArgs, m_shortOptionPrefix, m_longOptionPrefix, m_valueSeparator, m_caseSensitivity);
}

/////////////////////////////////////////////////////////////////////////////
String
CmdLineParser::getUsage(unsigned int maxColumns, EUsageVisibilityFlag visibility) const
{
// looks like this:
//     "Options:\n"
//     "  -1, --one                 first description\n"
//     "  -2, --two [arg]           second description (default is optional)\n"
//     "  -3, --three <arg>         third description\n"

	const unsigned int NUM_OPTION_COLUMNS = 28;
	StringBuffer usage("Options:\n");

	const String optionSpaces = padOnRight("", NUM_OPTION_COLUMNS, ' ');
	const String initialIndent = "  ";

	for (Array<ParserOptionImplRef>::const_iterator optionIter = m_options.begin();
		optionIter != m_options.end();
		++optionIter)
	{
		const ParserOptionImplRef& curOption = *optionIter;
		const char* const groupingNewline = ( curOption->argtype == E_GROUPING )? "\n" : "";

		if( (curOption->visibility == E_OPTION_NOT_VISIBLE) && (visibility != E_USAGE_SHOW_ALL) )
		{
			// This option is invisible.  Skip it.
			continue;
		}

		usage += groupingNewline;

		StringBuffer curLine;
		curLine += initialIndent;

		if (curOption->shortopt != '\0')
		{
			curLine += m_shortOptionPrefix;
			curLine += curOption->shortopt;
			if (!curOption->longopt.empty())
			{
				curLine += ", ";
			}
		}
		if (!curOption->longopt.empty())
		{
			curLine += m_longOptionPrefix;
			curLine += curOption->longopt;
		}

		if (curOption->argtype == E_REQUIRED_ARG)
		{
			curLine += " <arg>";
		}
		else if (curOption->argtype == E_OPTIONAL_ARG)
		{
			curLine += " [arg]";
		}

		if( curOption->argtype != E_GROUPING )
		{
			size_t bufferlen = (curLine.length() >= NUM_OPTION_COLUMNS-1) ? 1 : (NUM_OPTION_COLUMNS - curLine.length());
			curLine += optionSpaces.substring(0, bufferlen);
		}

		if (!curOption->description.empty())
		{
			curLine += curOption->description;
		}

		if (curOption->haveDefaultValue)
		{
			curLine += " (default is ";
			String quote;
			if( curOption->defaultValue.indexOf(" ") != String::npos )
			{
				quote = "\"";
			}
			curLine += quote + curOption->defaultValue + quote;
			curLine += ')';
		}

		// now if curLine is too long or contains newlines, we need to wrap it.
		while (curLine.length() > maxColumns || curLine.toString().indexOf('\n') != String::npos)
		{
			String curLineStr(curLine.toString());
			// first we look for a \n to cut at
			size_t newlineIdx = curLineStr.indexOf('\n');

			// next look for the last space to cut at
			size_t lastSpaceIdx = curLineStr.lastIndexOf(' ', maxColumns);

			size_t cutIdx = 0;
			size_t nextLineBeginIdx = 0;
			if (newlineIdx <= maxColumns)
			{
				cutIdx = newlineIdx;
				nextLineBeginIdx = newlineIdx + 1; // skip the newline
			}
			else if (lastSpaceIdx > NUM_OPTION_COLUMNS)
			{
				cutIdx = lastSpaceIdx;
				nextLineBeginIdx = lastSpaceIdx + 1; // skip the space
			}
			else
			{
				// no space to cut it, just cut it in the middle of a word
				cutIdx = maxColumns;
				nextLineBeginIdx = maxColumns;
			}

			// found a place to cut, so do it.
			usage += curLineStr.substring(0, cutIdx);
			usage += '\n';

			String spaces;
			// Grouping arguments do not need to be indented to the option level,
			// only wrapped and indented at the initial indent.
			if( curOption->argtype == E_GROUPING )
			{
				spaces = initialIndent;
			}
			else
			{
				spaces = optionSpaces;
			}
			// cut out the line from curLine
			curLine = spaces + curLineStr.substring(nextLineBeginIdx);
		}

		curLine += '\n';
		usage += curLine;
		usage += groupingNewline;
	}
	return usage.releaseString();
}
/////////////////////////////////////////////////////////////////////////////
// static
String
CmdLineParser::getUsage(const Option* options, unsigned int maxColumns, EUsageVisibilityFlag visibility)
{
	return CmdLineParser(options).getUsage(maxColumns, visibility);
}

namespace // anonymous
{
	ParserOptionImplRef getOptionByID(const Array<ParserOptionImplRef>& options, int id)
	{
		Array<ParserOptionImplRef>::const_iterator theOpt = std::find_if (options.begin(), options.end(), optIsID(id));
		if( theOpt == options.end() )
		{
			BLOCXX_THROW_ERR(CmdLineParserException, Format("No such option id: %1", id).c_str(), CmdLineParser::E_INVALID_OPTION);
		}
		return *theOpt;
	}
}

Array<int> CmdLineParser::getAllOptionIDs() const
{
	Array<int> retval;
	retval.reserve(m_options.size());
	for( Array<ParserOptionImplRef>::const_iterator opt = m_options.begin(); opt != m_options.end(); ++opt )
	{
		retval.push_back((*opt)->id);
	}
	return retval;
}

String CmdLineParser::getShortOptionByID(int id) const
{
	return String(getOptionByID(m_options, id)->shortopt);
}

String CmdLineParser::getLongOptionByID(int id) const
{
	return String(getOptionByID(m_options, id)->longopt);
}

String CmdLineParser::getDescriptionByID(int id) const
{
	return String(getOptionByID(m_options, id)->description);
}

/////////////////////////////////////////////////////////////////////////////
String
CmdLineParser::getOptionValue(int id, const char* defaultValue) const
{
	optionsMap_t::const_iterator ci = m_parsedOptions.find(id);
	if (ci != m_parsedOptions.end() && ci->second.size() > 0)
	{
		// grab the last one
		return ci->second[ci->second.size()-1];
	}
	return defaultValue;
}

/////////////////////////////////////////////////////////////////////////////
String
CmdLineParser::mustGetOptionValue(int id, const char* exceptionMessage) const
{
	optionsMap_t::const_iterator ci = m_parsedOptions.find(id);
	if (ci != m_parsedOptions.end() && ci->second.size() > 0)
	{
		// grab the last one
		return ci->second[ci->second.size()-1];
	}
	BLOCXX_THROW_ERR(CmdLineParserException, exceptionMessage, E_MISSING_OPTION);
}

/////////////////////////////////////////////////////////////////////////////
StringArray
CmdLineParser::getOptionValueList(int id) const
{
	StringArray rval;
	optionsMap_t::const_iterator ci = m_parsedOptions.find(id);
	if (ci != m_parsedOptions.end() && ci->second.size() > 0)
	{
		rval = ci->second;
	}
	return rval;
}

/////////////////////////////////////////////////////////////////////////////
StringArray
CmdLineParser::mustGetOptionValueList(int id, const char* exceptionMessage) const
{
	optionsMap_t::const_iterator ci = m_parsedOptions.find(id);
	if (ci != m_parsedOptions.end() && ci->second.size() > 0)
	{
		return ci->second;
	}
	BLOCXX_THROW_ERR(CmdLineParserException, exceptionMessage, E_MISSING_OPTION);
}

/////////////////////////////////////////////////////////////////////////////
bool
CmdLineParser::isSet(int id) const
{
	return m_parsedOptions.count(id) > 0;
}

/////////////////////////////////////////////////////////////////////////////
bool
CmdLineParser::isSet(const String& option) const
{
	return isSet(getOptionID(option));
}

/////////////////////////////////////////////////////////////////////////////
size_t
CmdLineParser::getNonOptionCount () const
{
	return m_nonOptionArgs.size();
}

/////////////////////////////////////////////////////////////////////////////
String
CmdLineParser::getNonOptionArg(size_t n) const
{
	return m_nonOptionArgs[n];
}

/////////////////////////////////////////////////////////////////////////////
StringArray
CmdLineParser::getNonOptionArgs() const
{
	return m_nonOptionArgs;
}

} // end namespace BLOCXX_NAMESPACE



