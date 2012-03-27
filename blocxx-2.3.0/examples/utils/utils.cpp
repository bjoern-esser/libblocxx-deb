/**
 * @author Norm Paxton (npaxton@novell.com)
 */

/* utils.cpp
**
** This demonstrates how to use the various utility classes / functions of BloCxx
*  Utility classes and functions demonstrated include:
*    UserUtils
*    EnvVars
*    GetPass

*  Still need to find a way to demonstrate
*    DateTime
*    Exec
*    Format
*    RandomNumber
*    UnnamedPipe
*    UTF8Utils
*    UUID
*
*  Other classes demonstrated include:
*    CmdLineParserException
*    Exception
*    String
*    StringArray
*
*/


#include <blocxx/BLOCXX_config.h>
#include <blocxx/CmdLineParser.hpp>
#include <blocxx/UserUtils.hpp>
#include <blocxx/Exception.hpp>
#include <blocxx/EnvVars.hpp>
#include <blocxx/GetPass.hpp>
#include <blocxx/Format.hpp>
#include <fstream>
#include <iostream>

using namespace BLOCXX_NAMESPACE;
using namespace UserUtils;
using namespace std;

#define SAMPLE_VERSION  "Version 0.1"
#define SAMPLE_AUTHOR   "Norm Paxton"
#define SAMPLE_EMAIL    "npaxton@novell.com"


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
		ENVVARS_OPT,
		SEPARATOR_USERINFO,
		USERS_OPT,
		UID_OPT,
		SEPARATOR_ENVVARS,
		ENV_SIZE_OPT,
		ENV_VAL_OPT,
		SEPARATOR_GENERAL,
		GETPASS_OPT,
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
**    Description  -- describe what the option will do.
**                NOTE that the array is terminated by an empty element
*/
// ********************************************************************
CmdLineParser::Option g_options[] =
	{
		{HELP_OPT,    'h', "help",    CmdLineParser::E_NO_ARG, 0, "Show this help information about options."},
		{VERSION_OPT, 'v', "version", CmdLineParser::E_NO_ARG, 0, "Show version information."},
		// OPERATIONS
		{SEPARATOR_OPERATIONS,  '\0', "\0",      CmdLineParser::E_GROUPING, 0, "OPERATIONS (mutually exclusive, default = userInfo)"},
		{USERINFO_OPT,'u', "userInfo",   CmdLineParser::E_NO_ARG, 0, "Output user information"},
		{ENVVARS_OPT, 'e', "envVars",    CmdLineParser::E_NO_ARG, 0, "Demonstrate envVars classes."},
		// USERINFO CONFIG
		{SEPARATOR_USERINFO,  '\0', "\0",      CmdLineParser::E_GROUPING, 0, "USERINFO options - only applicable if --userInfo indicated"},
		{USERS_OPT,   'n', "names",   CmdLineParser::E_REQUIRED_ARG, 0, "The names for which to lookup UserIDs.  Comma-separated, multivalued"},
		{UID_OPT,     'i', "ids",     CmdLineParser::E_REQUIRED_ARG, 0, "The userIds for which to lookup userNames.  Comma-separated, multivalued.  Default = '0,1000'"},
		// ENVVARS CONFIG
		{SEPARATOR_ENVVARS,  '\0', "\0",      CmdLineParser::E_GROUPING, 0, "EnvVars options - only applicable if --envVars indicated"},
		{ENV_SIZE_OPT,'s', "size",    CmdLineParser::E_NO_ARG, 0, "Display the size of the current environment variable list"},
		{ENV_VAL_OPT, 'l', "lookup",  CmdLineParser::E_OPTIONAL_ARG, "", "Lookup the value for the specified key(s) (multi-value by comma-separated list).  If no key is specified, show all."},
		// GENERAL_OPTIONS
		{SEPARATOR_GENERAL,  '\0', "\0",      CmdLineParser::E_GROUPING, 0, "General options"},
		{GETPASS_OPT, 'g', "getPass",    CmdLineParser::E_NO_ARG, 0, "Get the user's password"},
		// OUTPUT CONFIG
		{SEPARATOR_LOGGING, '\0',"\0",   CmdLineParser::E_GROUPING, 0, "LOGGING AND OUTPUT CONFIGURATION"},
		{VERBOSE_OPT, 'b', "verbose",    CmdLineParser::E_NO_ARG, 0, "Verbose output"},
		{0, 0, 0, CmdLineParser::E_NO_ARG, 0, 0}
	};

// ********************************************************************
// Demonstrates the use of UserInfo, StringArray, and some String
// ********************************************************************
void ShowUserInfo(String &inUserNames, String & inStrUids)
{
   StringArray nameList = inUserNames.tokenize(",");;
	StringArray uidList = inStrUids.tokenize(",");

   cout << "**** User Info **** " << endl;
	cout << Format("  effectiveUserId:              %1", getEffectiveUserId()).toString() << endl;
	cout << Format("  currentUserName:              %1", getCurrentUserName()).toString() << endl;

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
	for (unsigned int i=0; i<nameList.size(); i++)
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
	for (unsigned int i=0; i<uidList.size(); i++)
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
	cout << "Usage: utils [options]\n\n";
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
		   String inUserName;
			String inUid;

			inUserName = parser.getOptionValue(USERS_OPT);     // demonstrating no default value
			inUid = parser.getOptionValue(UID_OPT, "0,1000");	// demonstrating default value
		   ShowUserInfo(inUserName, inUid);
		}
		else if (parser.isSet(ENVVARS_OPT))
		{
         if (parser.isSet(ENV_SIZE_OPT))
         {
            EnvVars vars(EnvVars::E_CURRENT_ENVIRONMENT);
            cout << "There are " << vars.size() << " in the current environment map." << endl;
         }
         if (parser.isSet(ENV_VAL_OPT))
         {
            EnvVars vars(EnvVars::E_CURRENT_ENVIRONMENT);
            String vals = parser.getOptionValue(ENV_VAL_OPT);
            cout << "Current Environment Variables:" << endl;
            if (vals.empty())
            {
               // show all
               const char* const* allValues = vars.getenvp();
               int i=0;
               while(allValues[i] != NULL)
               {
                  String thisVal = (char *)allValues[i];
                  cout << "   " << thisVal << endl;
                  i++;
               }
            }
            else
            {
               StringArray valArray = vals.tokenize(",");
               for (size_t i=0; i<valArray.size(); i++)
               {
                  String thisVal = valArray[i];
                  cout << "  Value for environment variable [" << thisVal << "]: " << vars.getValue(thisVal) << endl;
               }
            }
         }
		}
      else if (parser.isSet(GETPASS_OPT))
      {
         String pwd = GetPass::getPass("Enter a password.  For this demo, it will be printed back out to you, so don't use a real password\r\n");
         cout << "You entered the following password: " << pwd << endl;
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
