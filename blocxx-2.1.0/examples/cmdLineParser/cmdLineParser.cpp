/*******************************************************************************
* Copyright (C) 2005 Novell, Inc. All rights reserved.
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
*  - Neither the name of Novell, Inc., nor the names of its
*    contributors may be used to endorse or promote products derived from this
*    software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL Novell, Inc., OR THE
* CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
* OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

/**
 * @author Norm Paxton (npaxton@novell.com)
 */

/* cmdLineParser.cpp
**
** This demonstrates how to use the cmdLineParser class.
** Other classes demonstrated include:
**    CmdLineParserException
**    UserUtils
**    Exception
**    String
**    StringArray
**
*/


#include <blocxx/BLOCXX_config.h>
#include <blocxx/CmdLineParser.hpp>
#include <blocxx/UserUtils.hpp>
#include <blocxx/Exception.hpp>
#include <fstream>
#include <iostream>

using namespace BLOCXX_NAMESPACE;
using namespace UserUtils;
using namespace std;

#define SAMPLE_VERSION  "Version 0.1"
#define SAMPLE_AUTHOR   "Norm Paxton"
#define SAMPLE_EMAIL    "npaxton@novell.com"


// ********************************************************************
// Print out command line errors / exceptions to cerr
// ********************************************************************
void printCmdLineParserExceptionMessage(CmdLineParserException& e)
{
   switch (e.getErrorCode())
   {
      case CmdLineParser::E_INVALID_OPTION:
         cerr << "unknown option: " << e.getMessage() << '\n';
      break;
      case CmdLineParser::E_MISSING_ARGUMENT:
         cerr << "missing argument for option: " << e.getMessage() << '\n';
      break;
      case CmdLineParser::E_INVALID_NON_OPTION_ARG:
         cerr << "invalid non-option argument: " << e.getMessage() << '\n';
      break;
      case CmdLineParser::E_MISSING_OPTION:
         cerr << "missing required option: " << e.getMessage() << '\n';
      break;
      default:
         cerr << "failed parsing command line options: " << e << "\n";
      break;
   }
}


// ********************************************************************
// These are the optionTags that will be used by the CmdLineParser and the array of options below
// ********************************************************************
enum tag_OPTIONS
{
   HELP_OPT,
   VERSION_OPT,
	SEPARATOR_OPERATIONS,
	USERINFO_OPT,
	CMDLINE_OPT,
	SEPARATOR_USERINFO,
	USERS_OPT,
	UID_OPT,
	SEPARATOR_CMDLINE,
	DEFAULT_OPT,
	REQUIRED_OPT,
	OPTIONAL_OPT,
	STRING_OPT,
	NUMERIC_OPT,
	SEPARATOR_LOGGING,
	VERBOSE_OPT
};

// ********************************************************************
/* This is the array of options used by the CmdLineParser... Note that it includes the following:
**    optionTag    -- see the tag_OPTIONS defined above
**    shortOption  -- the one-letter option  - can be '\0' if no shortOption is to be allowed
**    longOption   -- the string option - can be '\0' or 0 if no longOption is to be allowed
**                NOTE that this example uses separators to group the options.  In this case, both the short
**                     and the long options are '\0'
**    arg requirements -- whether the arguments are required, optional, or none expected
**    Default Value -- this is the default that will be set if the option is provided without a value.
**                NOTE: this default will only be applied if the arg requirement is E_OPTIONAL_ARG
**                      this default will not apply if the option is not specified on the command-line
**                      see the -d/--default option for example (both in the g_options table and the getOptionValue() call)
**                      to accomplish this, supply a default with the "parser.getOptionValue()" call, see below
**    Description  -- describe what the option will do.
**                NOTE that the array is terminated by an empty element
*/
// ********************************************************************
CmdLineParser::Option g_options[] =
{
   {HELP_OPT,    'h', "help",    CmdLineParser::E_NO_ARG, 0, "Show this help information about options."},
   {VERSION_OPT, 'v', "version", CmdLineParser::E_NO_ARG, 0, "Show version information.\r\n"},

   // OPERATIONS
   {SEPARATOR_OPERATIONS,  '\0', "\0", CmdLineParser::E_NO_ARG, 0, "OPERATIONS (mutually exclusive, default = userInfo\r\n"},
   {USERINFO_OPT,'u', "userInfo",      CmdLineParser::E_NO_ARG, 0, "Demonstrate the UserUtils utility functions"},
   {CMDLINE_OPT, 'c', "cmdline",       CmdLineParser::E_NO_ARG, 0, "Demonstrate CmdLineParser class."},

   // USERINFO CONFIG   
   {SEPARATOR_USERINFO,  '\0', "\0",   CmdLineParser::E_NO_ARG, 0, "\r\nUSERINFO options - only applicable if --userInfo indicated\r\n"},
   {USERS_OPT,   'n', "names",         CmdLineParser::E_REQUIRED_ARG, 0, "The names for which to lookup UserIDs.  Comma-separated, multivalued"},
   {UID_OPT,     'i', "ids",           CmdLineParser::E_REQUIRED_ARG, 0, 
	                                    "The userIds for which to lookup userNames.  Comma-separated, multivalued.  Default = '0,1000'\r\n"},
   
	// CMDLINE CONFIG   
   {SEPARATOR_CMDLINE,  '\0', "\0",    CmdLineParser::E_NO_ARG, 0, "THREADPOOL options - only applicable if --threadpool indicated\r\n"},
     // This option demonstrates the 'default value'	
   {DEFAULT_OPT, 'd', "default",       CmdLineParser::E_OPTIONAL_ARG, "SpecifiedDefault", "Demonstrate cmdLineParser defaults.\r\n"},
     // This option demonstrates the 'required_arg'	
   {REQUIRED_OPT,'r', "required",      CmdLineParser::E_REQUIRED_ARG, 0, "Required argument"},
       // This option demonstrates the 'optional_arg'	
   {OPTIONAL_OPT,'o', "optional",      CmdLineParser::E_OPTIONAL_ARG, 0, "Optional argument"},
	    // This option demonstrates the 'optional_arg'	
   {STRING_OPT,  's', "string",        CmdLineParser::E_REQUIRED_ARG, 0, "Optional argument"},
	    // This option demonstrates the 'optional_arg'	
   {NUMERIC_OPT, '#', "numeric",       CmdLineParser::E_REQUIRED_ARG, 0, "Optional argument"},
	
	// OUTPUT CONFIG   
   {SEPARATOR_LOGGING, '\0',"\0",      CmdLineParser::E_NO_ARG, 0, "LOGGING AND OUTPUT CONFIGURATION\r\n"},
   {VERBOSE_OPT, 'v', "verbose",       CmdLineParser::E_NO_ARG, 0, "Verbose output"},
   {0, 0, 0, CmdLineParser::E_NO_ARG, 0, 0}
};

// ********************************************************************
// Demonstrates the use of UserInfo, StringArray, and some String
//   Included primarily for demonstrating categories through CmdLineParser
// ********************************************************************
void ShowUserInfo(String &inUserNames, String & inStrUids)
{
   StringArray nameList = inUserNames.tokenize(",");;
	StringArray uidList = inStrUids.tokenize(",");

   cout << "**** User Info **** " << endl;
	cout << "  effectiveUserId:              " << getEffectiveUserId() << endl;
	cout << "  currentUserName:              " << getCurrentUserName() << endl;
	
	if (nameList.size() == 0)
	{
	   nameList.append("root");
	}
	// should never get into the next 'if' clause because of default value.
	if (uidList.size() == 0)
	{
		uidList.append("1000");
	}

	bool bSuccess = true;
	unsigned int i = 0;
	for (i=0; i<nameList.size(); i++)
	{
	   String name = nameList[i];
		unsigned int uId = getUserId(name, bSuccess);
		if (bSuccess)
		{
		   cout << "  userId for name '"<<name<<"':     " << uId << endl;
		}
		else
		{
		   cout << "  userId for name '"<<name<<"':        UnSuccessful" << endl;
		}
	}
	for (i=0; i<uidList.size(); i++)
	{
	   unsigned int uid = uidList[i].toUInt16();
		String uName = getUserName(uid, bSuccess);
		if (bSuccess)
		{
		   cout << "  userName for id '"<<uid<<"':     " << uName << endl;
		}
		else
		{
		   cout << "  userName for id '"<<uid<<"':        UnSuccessful" << endl;
		}
	}
	cout << " **** End User Info **** " << endl;
}

// ********************************************************************
// output the usage options -  This is facilitated by the CmdLineParser::getUsage call.
// ********************************************************************
void Usage()
{
	cout << "Usage: cmdLineParser [options]\n\n";
   cout << CmdLineParser::getUsage(g_options) << endl;
}

// ********************************************************************
// The main function... This demonstrates the CmdLineParser class, and depending upon options, calls into other functions
//      Also demonstrates Exceptions
// ********************************************************************
int main(int argc, char* argv[])
{
   try
   {
      // parse command line
      CmdLineParser parser(argc, argv, g_options, CmdLineParser::E_NON_OPTION_ARGS_INVALID);

		// handle the OPERATIONS options
      if (parser.isSet(HELP_OPT))
      {
         Usage();
         return 0;
      }
      else if (parser.isSet(VERSION_OPT))
      {
         cout << "BloCxx Sample App  " << SAMPLE_VERSION << endl;
         cout << "Written by " << SAMPLE_AUTHOR << endl;
         cout << SAMPLE_EMAIL << endl;
         return 0;
      }
		else if (parser.isSet(USERINFO_OPT))
		{
         // Included primarily to demonstrate options within options, ie category or operation
		   String inUserName;
			String inUid;

			inUserName = parser.getOptionValue(USERS_OPT);     // demonstrating no default value
			inUid = parser.getOptionValue(UID_OPT, "0,1000");	// demonstrating default value
		   ShowUserInfo(inUserName, inUid);
		}
		else if (parser.isSet(CMDLINE_OPT))
		{
		   if (parser.isSet(DEFAULT_OPT))
		   {
		      String value = parser.getOptionValue(DEFAULT_OPT, "optionSpecifiedGetValueDefault"); // should never see this value.
		      cout << "You specified the -d/--default option on the command line.  The Value is: " << value << endl;
   		}
		   else
		   { 	
			   String value = parser.getOptionValue(DEFAULT_OPT, "optionNotSpecifiedDefault");
		      cout << "You did not specify the -d/--default option on the command line.  The default value is: " << value << endl;
		   }


			if (parser.isSet(OPTIONAL_OPT))
		   {
		      String value = parser.getOptionValue(OPTIONAL_OPT);
				if (value.empty())
				{
		         cout << "You specified the -o/--optional option on the command line without the optional arg" << endl;
				}
				else
				{
		         cout << "You specified the -o/--optional option on the command line with an optional arg value of: " << value << endl;
				}
   		}
		   else
		   { 	
		      cout << "You did not specify the -o/--optional option on the command line." << endl;
		   }


         try
			{
		      String strReqVal = parser.mustGetOptionValue(REQUIRED_OPT, "The -r/--required option is mandatory");
				cout << "You specified the mandatory -r/--required option on the command line with the required arg value of: " << strReqVal << endl;
			}
			catch (CmdLineParserException& e)
			{
			   printCmdLineParserExceptionMessage(e);
			}

         if (parser.isSet(STRING_OPT))
			{
				String strString = parser.getOptionValue(STRING_OPT);
				cout << "You specified the -s/--string option with a string value of: " << strString << endl;
			}

			if (parser.isSet(NUMERIC_OPT))
			{
				String strNumber = parser.getOptionValue(NUMERIC_OPT);
				UInt32 iNum = strNumber.toUInt32();
				cout << "You specified the -#/--numeric option with a UInt32 value of: " << iNum << endl;
			}
		}
		else
		{
		   Usage();
			return 1;
	   }	

		return 0;
	}
   catch (CmdLineParserException& e)
   {
      printCmdLineParserExceptionMessage(e);
      Usage();
   }
	catch(Exception& e)
	{
	   cerr << "ERROR:   Exception:" << e << endl;
	}
	catch(std::exception& e)
	{
	   cerr << "ERROR:   sdtException:" << e.what() << endl;
	}
	catch(...)
	{
	   cerr << "ERROR:   UnknownException." << endl;
	}

   return 1;
}
