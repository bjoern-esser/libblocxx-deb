/*******************************************************************************
* Copyright (C) 2009 Quest Software, Inc. All rights reserved.
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
*  - Neither the name of Quest Software, Inc., nor the names of its
*    contributors may be used to endorse or promote products derived from this
*    software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL Quest Software, Inc., OR THE
* CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
* OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

/**
 * @author Kevin Harris
 */

#include "blocxx/BLOCXX_config.h"
#include "blocxx/CmdLineParser.hpp"
#include "blocxx/Exception.hpp"
#include "blocxx/Array.hpp"
#include "blocxx/Format.hpp"
#include <iostream>
#include <stdexcept>

void handleParseResults(const blocxx::CmdLineParser& parser)
{
	// Display all possible options, show if they were supplied, and show any
	// values attached to them.
	blocxx::Array<int> optionValues = parser.getAllOptionIDs();
	for( blocxx::Array<int>::const_iterator optval = optionValues.begin(); optval != optionValues.end(); ++optval )
	{
		const blocxx::String shortopt = parser.getShortOptionByID(*optval);
		const blocxx::String longopt = parser.getLongOptionByID(*optval);
		const blocxx::String description = parser.getDescriptionByID(*optval);
		const blocxx::String optinfo = blocxx::Format("Option id %1 (%2, %3, \"%4\")", *optval, shortopt, longopt, description);

		if( parser.isSet(*optval) )
		{
			blocxx::StringArray values = parser.getOptionValueList(*optval);

			if( !values.empty() )
			{
				for( blocxx::StringArray::const_iterator val = values.begin(); val != values.end(); ++val )
				{
					std::cout << blocxx::Format("%1 was supplied with value \"%2\"", optinfo, *val) << std::endl;
				}
			}
			else
			{
				std::cout << blocxx::Format("%1 was supplied with no value", optinfo, *optval) << std::endl;
			}
		}
		else
		{
			std::cout << blocxx::Format("%1 was not supplied", optinfo, *optval) << std::endl;
		}
	}

	// Display non-option arguments
	blocxx::StringArray nonOptionValues = parser.getNonOptionArgs();
	for( blocxx::StringArray::const_iterator value = nonOptionValues.begin(); value != nonOptionValues.end(); ++value )
	{
		std::cout << blocxx::Format("Non-option argument supplied: \"%1\"", *value) << std::endl;
	}

}

int main(int argc, const char** argv)
{
	// Create an empty command line parser.
	blocxx::CmdLineParser parser;

	try
	{
		// Add options to the parser.
		parser.addOption('h', "help", blocxx::CmdLineParser::E_NO_ARG, "Show this help");
		parser.addOption('f', "simple-option", blocxx::CmdLineParser::E_NO_ARG, "A simple option");
		parser.addOption('g', "longer-option", blocxx::CmdLineParser::E_REQUIRED_ARG, "A longer option.  Requires a value.");


		// Parse the user-supplied command line
		std::cout << "Parsing with the actual command line arguments." << std::endl;
		parser.parse(argc, argv, blocxx::CmdLineParser::E_NON_OPTION_ARGS_ALLOWED);

		if( parser.isSet("help") )
		{
			std::cout << "The help option was requested.  Displaying usage:" << std::endl;
			std::cout << parser.getUsage() << std::endl;
			return 0;
		}

		// Print some command line parsing results
		handleParseResults(parser);

		std::cout << std::endl;
		std::cout << std::endl;

		// Reparse with fake command line values.
		std::cout << "Reparsing with some fake command line arguments." << std::endl;
		// This will attach the value of "2", "4", "6", "8", "10" to the list of
		// values attached to the "-g" or "--longer-option" argument.  The odd
		// numbers are accepted as non-option arguments.
		const blocxx::String argtext = "dynamicCmdLineParser 1 -g2 3 -g 4 5 -g=6 7 --longer-option 8 9 --longer-option=10";
		std::cout << "Fake arguments are: " << argtext << std::endl;
		blocxx::StringArray arguments = argtext.tokenize();
		parser.parse(arguments, blocxx::CmdLineParser::E_NON_OPTION_ARGS_ALLOWED);

		// Print the results of the fake arguments.
		handleParseResults(parser);

		return 0;
	}
	catch(const blocxx::Exception& e)
	{
		std::cerr << "Error: " << e << std::endl;
	}
	catch(const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
	}
	catch(...)
	{
		std::cerr << "Unknown error occurred." << std::endl;
	}
	return 1;
}
