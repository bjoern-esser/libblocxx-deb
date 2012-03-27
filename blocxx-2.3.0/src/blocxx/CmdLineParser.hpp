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

#ifndef BLOCXX_CMD_LINE_PARSER_HPP_INCLUDE_GUARD_
#define BLOCXX_CMD_LINE_PARSER_HPP_INCLUDE_GUARD_
#include "blocxx/BLOCXX_config.h"
#include "blocxx/String.hpp"
#include "blocxx/SortedVectorMap.hpp"
#include "blocxx/Array.hpp"
#include "blocxx/Exception.hpp"

namespace BLOCXX_NAMESPACE
{

BLOCXX_DECLARE_APIEXCEPTION(CmdLineParser, BLOCXX_COMMON_API)

struct ParserOptionImpl;
typedef IntrusiveReference<ParserOptionImpl> ParserOptionImplRef;

/**
 * Do command line parsing.
 *
 * Thread safety: read/write
 * Copy semantics: Value
 * Exception safety: Strong
 */
class BLOCXX_COMMON_API CmdLineParser
{
public:
	enum ECaseSensitivityFlag
	{
		E_CASE_SENSITIVE,  //!< the CommandLine parser is case sensitive (-a and -A are distinct)
		E_CASE_INSENSITIVE //!< the CommandLine parser is case insensitive (-a and -A are equal)
	};

	enum EArgumentTypeFlag
	{
		E_NO_ARG,       //!< the option does not take an argument
		E_REQUIRED_ARG, //!< the option requires an argument
		E_OPTIONAL_ARG, //!< the option might have an argument
		E_GROUPING      //!< no option is accepted, provided for grouping only
	};

	enum EOptionVisibilityTypeFlag
	{
		E_OPTION_VISIBLE,    //!< The option will appear in the default help output.
		E_OPTION_NOT_VISIBLE //!< The option will not appear in the help output.
	};

	//!< errors codes that may be specified when a CmdLineParserException is thrown
	enum EErrorCodes
	{
		E_INVALID_OPTION,         //!< an unknown option was specified
		E_MISSING_ARGUMENT,       //!< an option for which argtype == E_REQUIRED_ARG did not have an argument
		E_INVALID_NON_OPTION_ARG, //!< a non-option argument was specified, but they are not allowed
		E_MISSING_OPTION          //!< the option wasn't specified
	};

	/**
	 * A structure for static CmdLineParser initialization.
	 */
	struct Option
	{
		int id;                    //!< unique option id, used to retrieve option values.  Negative numbers are reserved for internal use.
		char shortopt;             //!< short option char.  Set to '\\0' for none.
		const char* longopt;       //!< long option string.  Set to 0 for none.
		EArgumentTypeFlag argtype; //!< specifies constraints for the option's argument
		const char* defaultValue;  //!< if argtype == E_OPTIONAL_ARG and no argument is specified, this value will be returned.  Set to 0 for none.
		const char* description;   //!< description used by getUsage().  May be 0.
	};

	enum EAllowNonOptionArgsFlag
	{
		E_NON_OPTION_ARGS_ALLOWED, //!< Non-option arguments are allowed
		E_NON_OPTION_ARGS_INVALID  //!< Non-option arguments are invalid
	};

	enum EUsageVisibilityFlag
	{
		E_USAGE_SHOW_VISIBLE, //!< Show visible options only.
		E_USAGE_SHOW_ALL      //!< Show all options (including non-visible options).
	};

	CmdLineParser(
		char shortOptionPrefix = '-',
		const String& longOptionPrefix = "--",
		char valueSeparator = '=',
		ECaseSensitivityFlag caseSensitivity = E_CASE_SENSITIVE);
	CmdLineParser(const CmdLineParser& p);
	~CmdLineParser();
	CmdLineParser& operator=(const CmdLineParser& p);

	/**
	 * @param argc Count of pointers in argv.  Pass value from main().
	 * @param argv Arguments.  Pass value from main(). Value is not saved.
	 * @param options An array of Option terminated by a final entry that
	 *        has a '\\0' shortopt && 0 longopt.
	 * @param allowNonOptionArgs Indicate whether the presense of
	 *        non-option arguments is an error.
	 * @throws CmdLineParserException if the given command line is invalid.
	 */
	CmdLineParser(int argc, char const* const* const argv, const Option* options, EAllowNonOptionArgsFlag allowNonOptionArgs,
		char shortOptionPrefix = '-',
		const String& longOptionPrefix = "--",
		char valueSeparator = '=',
		ECaseSensitivityFlag caseSensitivity = E_CASE_SENSITIVE);

	/**
	 * Create a command line parser for the specified options.
	 * @throws CmdLineParserException if the any option is invalid due to
	 *         parameter duplication or other error.
	 */
	CmdLineParser(const Option* options,
		char shortOptionPrefix = '-',
		const String& longOptionPrefix = "--",
		char valueSeparator = '=',
		ECaseSensitivityFlag caseSensitivity = E_CASE_SENSITIVE);

	/**
	 * Add an option.
	 *
	 * @param shortOption The short option.  If '\0', no short option will
	 *        be used.
	 * @param longOption The long option.  If empty, no long option will be used.
	 * @param argType Specifies constraints for the option's argument.
	 * @param description A description for the option which will be included in
	 *        getUsage().  Empty descriptions are allowed.
	 * @returns The ID of the added option.  This may be used in any
	 *          CmdLineParser function that expects an ID.
	 * @throws CmdLineParserException if the given option is invalid due to
	 *         parameter duplication or other error.
	 */
	int addOption(char shortOption, const String& longOption, EArgumentTypeFlag argType, const String& description, EOptionVisibilityTypeFlag visibility = E_OPTION_VISIBLE);

	/**
	 * Add an option which will allow an optional value.
	 *
	 * @param shortOption The short option.  If '\0', no short option will
	 *        be used.
	 * @param longOption The long option.  If empty, no long option will be used.
	 * @param description A description for the option which will be included in
	 *        getUsage().  Empty descriptions are allowed.
	 * @param defaultValue The default value for the option if no argument is
	 *        specified.
	 * @returns The ID of the added option.  This may be used in any
	 *          CmdLineParser function that expects an ID.
	 * @throws CmdLineParserException if the given option is invalid due to
	 *         parameter duplication or other error.
	 */
	int addOption(char shortOption, const String& longOption, const String& defaultValue, const String& description, EOptionVisibilityTypeFlag visibility = E_OPTION_VISIBLE);

	/**
	 * Add an option which will allow an optional value using the supplied ID.
	 *
	 * @param shortOption The short option.  If '\0', no short option will
	 *        be used.
	 * @param longOption The long option.  If empty, no long option will be used.
	 * @param description A description for the option which will be included in
	 *        getUsage().  Empty descriptions are allowed.
	 * @param defaultValue The default value for the option if no argument is
	 *        specified.
	 * @throws CmdLineParserException if the given option is invalid due to
	 *         parameter duplication or other error.
	 */
	void addOptionWithID(int id, char shortOption, const String& longOption, const String& defaultValue, const String& description, EOptionVisibilityTypeFlag visibility = E_OPTION_VISIBLE);

	/**
	 * Add an option with a supplied ID.
	 *
	 * @param shortOption The short option.  If '\0', no short option will
	 *        be used.
	 * @param longOption The long option.  If empty, no long option will be used.
	 * @param argType Specifies constraints for the option's argument.
	 * @param description A description for the option which will be included in
	 *        getUsage().  Empty descriptions are allowed.
	 * @throws CmdLineParserException if the given option is invalid due to
	 *         parameter duplication or other error.
	 */
	void addOptionWithID(int id, char shortOption, const String& longOption, EArgumentTypeFlag argType, const String& description, EOptionVisibilityTypeFlag visibility = E_OPTION_VISIBLE);

	/**
	 * @param argc Count of pointers in argv.  Pass value from main().
	 * @param argv Arguments.  Pass value from main(). Value is not saved.
	 * @param allowNonOptionArgs Indicate whether the presense of non-option
	 *        arguments is an error.
	 * @throws CmdLineParserException if the given command line is invalid.
	 */
	void parse(int argc, char const* const* const argv, EAllowNonOptionArgsFlag allowNonOptionArgs);

	/**
	 * @param arguments Arguments to parse and verify.
	 * @param allowNonOptionArgs Indicate whether the presense of non-option
	 *        arguments is an error.
	 * @throws CmdLineParserException if the given command line is invalid.
	 */
	void parse(const StringArray& arguments, EAllowNonOptionArgsFlag allowNonOptionArgs);

	/**
	 * Return the ID assigned to the given option.
	 * @throws CmdLineParserException if the supplied option does not exist.
	 */
	int getOptionID(const String& option) const;

	/**
	 * Return an array of all option IDs registered with this CmdLineParser.
	 */
	Array<int> getAllOptionIDs() const;

	/**
	 * Get the short option text for the supplied id.  Will throw a
	 * CmdLineParserException error if the option does not exist.
	 */
	String getShortOptionByID(int id) const;

	/**
	 * Get the long option text for the supplied id.  Will throw a
	 * CmdLineParserException error if the option does not exist.
	 */
	String getLongOptionByID(int id) const;

	/**
	 * Get the description text for the supplied id.  Will throw a
	 * CmdLineParserException error if the option does not exist.
	 */
	String getDescriptionByID(int id) const;

	/**
	 * Read out a string option.
	 * @param id The id of the option.
	 * @param defaultValue The return value if the option wasn't set.
	 * @return The value of the option, if given, otherwise defaultValue.
	 *         If the option was specified more than once, the value
	 *         from the last occurence will be returned.
	 */
	String getOptionValue(int id, const char* defaultValue = "") const;

	/**
	 * Read out a string option.
	 * @param id The id of the option.
	 * @return The value of the option. If the option was specified more
	 *         than once, the value from the last occurence will be
	 *         returned.
	 * @param exceptionMessage If an exception is thrown this string will
	 *        be used as the exception message.
	 * @throws CmdLineParserException with code E_MISSING_OPTION if the
	 *         option wasn't specified.
	 */
	String mustGetOptionValue(int id, const char* exceptionMessage = "") const;

	/**
	 * Read out all occurences of a string option.
	 * @param id The id of the option.
	 * @return The value of the option, if given, otherwise an empty
	 *         StringArray.
	 */
 	StringArray getOptionValueList(int id) const;

	/**
	 * Read out all occurences of a string option.
	 * @param id The id of the option.
	 * @return The value of the option.
	 * @param exceptionMessage If an exception is thrown this string will
	 *        be used as the exception message.
	 * @throws CmdLineParserException with code E_MISSING_OPTION if the
	 *         option wasn't specified.
	 */
 	StringArray mustGetOptionValueList(int id, const char* exceptionMessage = "") const;

	/**
	 * Read out a boolean option or check for the presence of string option.
	 */
	bool isSet(int id) const;

	/**
	 * Return if the parsed command line contained the given option. 
	 */
	bool isSet(const String& option) const;

	/**
	 * Read the number of arguments that aren't options (but, for example, filenames).
	 */
	size_t getNonOptionCount () const;

	/**
	 * Read out an non-option argument.
	 * @param n The 0-based index of the argument.  Valid values are 0 to count()-1.
	 */
	String getNonOptionArg(size_t n) const;

	/**
	 * Read out the non-option args
	 */
	StringArray getNonOptionArgs() const;

	/**
	 * Generate a usage string for the options, for example:
	 * @code
	 * "Options:\n"
	 * "  -1, --one                 first description\n"
	 * "  -2, --two [arg]           second description (default is optional)\n"
	 * "  -3, --three <arg>         third description\n"
	 * @endcode
	 *
	 * The E_OPTIONAL_ARG option arguments are indicated by squared
	 * brackets @code "[arg]" @endcode and E_REQUIRED_ARG option
	 * arguments by angle brackets @code "<arg>" @endcode.
	 *
	 * @param maxColumns Wrap the descriptions so no line of the usage
	 *        string exceeds the specified number of columns.
	 * @param visibility If set to E_USAGE_SHOW_VISIBLE, the output will include
	 *        only visible options.  If set to E_USAGE_SHOW_ALL, the output will
	 *        include all options.
	 */
	String getUsage(unsigned int maxColumns = 80, EUsageVisibilityFlag visibility = E_USAGE_SHOW_VISIBLE) const;

	/**
	 * Generate a usage string for the options, for example:
	 * @code
	 * "Options:\n"
	 * "  -1, --one                 first description\n"
	 * "  -2, --two [arg]           second description (default is optional)\n"
	 * "  -3, --three <arg>         third description\n"
	 * @endcode
	 *
	 * The E_OPTIONAL_ARG option arguments are indicated by squared
	 * brackets @code "[arg]" @endcode and E_REQUIRED_ARG option
	 * arguments by angle brackets @code "<arg>" @endcode.
	 *
	 * @param options An array of Option terminated by a final entry that
	 *        has a '\\0' shortopt && 0 longopt.
	 * @param maxColumns Wrap the descriptions so no line of the usage
	 *        string exceeds the specified number of columns.
	 * @param visibility If set to E_USAGE_SHOW_VISIBLE, the output will include
	 *        only visible options.  If set to E_USAGE_SHOW_ALL, the output will
	 *        include all options.
	 */
	static String getUsage(const Option* options, unsigned int maxColumns = 80, EUsageVisibilityFlag visibility = E_USAGE_SHOW_VISIBLE);


private:

#ifdef BLOCXX_WIN32
#pragma warning (push)
#pragma warning (disable: 4251)
#endif

	/**
	 * Add the options to the internally-stored option list.
	 * @param options An array of Option terminated by a final entry that
	 *        has a '\\0' shortopt && 0 longopt.
	 * @throws CmdLineParserException if any option is invalid due to
	 *         parameter duplication or other error.
	 */
	void addOptions(const Option* options);

	/**
	 * Verify the supplied option is not duplicated with existing options and
	 * has no other apparent errors.  This verifies there are no conflicts with
	 * case insensitivity, ambiguous short and long option switches.
	 * @param opt The option to verify.
	 * @throws CmdLineParserException if the option is invalid due to
	 *         parameter duplication or other error.
	 */
	void verifyOptionValidity(const ParserOptionImplRef& opt) const;

	// key is Option::id, value is the value(s) specified by the user
	typedef SortedVectorMap<int, StringArray> optionsMap_t;
	optionsMap_t m_parsedOptions;
	StringArray m_nonOptionArgs;

	Array<ParserOptionImplRef> m_options;

	int m_lowestInternalID;
	char m_shortOptionPrefix;
	String m_longOptionPrefix;
	char m_valueSeparator;
	ECaseSensitivityFlag m_caseSensitivity;

#ifdef BLOCXX_WIN32
#pragma warning (pop)
#endif

	/** \example cmdLineParser.cpp
	 * An example of static CmdLineParser initialization.
	 */
	/** \example dynamicCmdLineParser.cpp
	 * An example of dynamic CmdLineParser initialization.
	 */
};

} // end namespace BLOCXX_NAMESPACE

#endif


