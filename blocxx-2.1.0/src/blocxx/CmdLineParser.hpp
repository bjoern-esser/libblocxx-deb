/*******************************************************************************
* Copyright (C) 2005, Vintela, Inc. All rights reserved.
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
*       Vintela, Inc., 
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
	enum EArgumentTypeFlag
	{
		E_NO_ARG,       //!< the option does not take an argument
		E_REQUIRED_ARG, //!< the option requires an argument
		E_OPTIONAL_ARG  //!< the option might have an argument
	};

	// errors codes that may be specified when a CmdLineParserException is thrown
	enum EErrorCodes
	{
		E_INVALID_OPTION,         //!< an unknown option was specified
		E_MISSING_ARGUMENT,       //!< an option for which argtype == E_REQUIRED_ARG did not have an argument
		E_INVALID_NON_OPTION_ARG, //!< a non-option argument was specified, but they are not allowed
		E_MISSING_OPTION          //!< the option wasn't specified
	};

	struct Option
	{
		int id;                    //!< unique option id, used to retrieve option values
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

	/**
	 * @param argc Count of pointers in argv.  Pass value from main().
	 * @param argv Arguments.  Pass value from main(). Value is not saved.
	 * @param options An array of Option terminated by a final entry that
	 *        has a '\\0' shortopt && 0 longopt. Value is not saved.
	 * @param allowNonOptionArgs Indicate whether the presense of
	 *        non-option arguments is an error.
	 * @throws CmdLineParserException if the given command line is invalid.
	 */
	CmdLineParser(int argc, char const* const* const argv, const Option* options, EAllowNonOptionArgsFlag allowNonOptionArgs);

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
	 * @param options An array of Option terminated by a final entry that
	 *        has a '\\0' shortopt && 0 longopt.
	 * @param maxColumns Wrap the descriptions so no line of the usage
	 *        string exceeds the specified number of columns.
	 */
	static String getUsage(const Option* options, unsigned int maxColumns = 80);


private:

#ifdef BLOCXX_WIN32
#pragma warning (push)
#pragma warning (disable: 4251)
#endif

	// key is Option::id, value is the value(s) specified by the user
	typedef SortedVectorMap<int, StringArray> optionsMap_t;
	optionsMap_t m_parsedOptions;
	StringArray m_nonOptionArgs;

#ifdef BLOCXX_WIN32
#pragma warning (pop)
#endif

};

} // end namespace BLOCXX_NAMESPACE

#endif


