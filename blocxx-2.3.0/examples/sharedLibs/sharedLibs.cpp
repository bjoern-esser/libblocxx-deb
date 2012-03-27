/**
 * @author Norm Paxton (npaxton@novell.com)
 */

/* sharedLibs.cpp
**
** This demonstrates how to use the various sharedLibs classes of BloCxx
*  SharedLibs classes demonstrated include:
*
*  Other classes demonstrated include:
*    CmdLineParserException
*    Exception
*    String
*    StringArray
*
*/
/* sharedLibs.cpp
**
** This is a sample of how to use various classes in the BLOCXX project.
** The classes demonstrated include:
**    SharedLibrary
*/


#include <blocxx/BLOCXX_config.h>
#include <blocxx/SharedLibrary.hpp>
#include <blocxx/SharedLibraryLoader.hpp>
#include <blocxx/CerrLogger.hpp>
#include "dummyLib.hpp"
#include <iostream>

using namespace BLOCXX_NAMESPACE;
using namespace std;


// ********************************************************************
// Define the expected return types
// ********************************************************************
typedef const char* (*fpVersion)(void);
typedef DummyLib* (*fpWithValType)(int);
typedef DummyLib* (*fpNoValType)(void);
typedef void (*fpVoid)(void);


// ********************************************************************
// The main function... This demonstrates the SharedLibrary class
// ********************************************************************
int main(int argc, char* argv[])
{
   String strLibName = "./libdummylib.so";
   if (argc == 2)
   {
      strLibName.format("%s/libdummylib.so", argv[1]);
   }
   try
   {
      fpVersion fpVer;
      fpWithValType fp;
      fpNoValType fpnv;
      fpVoid fpv;
      DummyLib *dl = NULL;


      // ********************************************************************
      // Create a shared library loader ref, to use to load libraries
      // ********************************************************************
      SharedLibraryLoaderRef sllr = SharedLibraryLoader::createSharedLibraryLoader();

      // ********************************************************************
      // Load a non-existent NotFoundLib library
      //  Expected result:  log error to logger, return NULL ref
      // ********************************************************************
      SharedLibraryRef slrNotFound = sllr->loadSharedLibrary( "NotFoundLib");
      if (slrNotFound)
      {
         cout << "UNExpected :  found non-existent library" << endl;
      }
      else
      {
         cout << "Expected: didn't find non-existent library:  " << endl;
      }


      // ********************************************************************
      // Load the DummyLib library
      // ********************************************************************
      SharedLibraryRef slr = sllr->loadSharedLibrary( strLibName );

      if (slr)
      {
         cout << "Found shared library" << endl;

         // ********************************************************************
         // Call a non-existent method
         // ********************************************************************
         if (true == slr->getFunctionPointer( "nonExistentMethod", fpv ))
         {
            cout << "Got UNexpected non-null on non-existent method" << endl;
         }
         else
         {
            cout << "Got expected null on non-existent method" << endl;
         }


         // ********************************************************************
         // Call an api returning a string
         // ********************************************************************
         if (true == slr->getFunctionPointer( "getVersion", fpVer ))
         {
            cout << "Got fp for Version:  " << fpVer() << endl;
         }
         else
         {
            cout << "Got UNexpected null on getVersion" << endl;
         }


         // ********************************************************************
         // create and use DummyLib with the val constructor
         // ********************************************************************
         if (true == slr->getFunctionPointer( "createDummyLibWithVal", fp ) )
         {
            cout << "got fp for createDummyLibWithVal" << endl;
            dl = fp(3);
            if (dl)
            {
               cout << "Expected Val: 3         Actual: " << dl->getVal() << endl;
               dl->setVal(5);
               cout << "Expected Val: 5         Actual: " << dl->getVal() << endl;

               delete(dl);
            }
         }
         else
         {
            cout << "Got UNexpected null on createDummyLibWithVal" << endl;
         }

         // ********************************************************************
         // create and use DummyLib with the default constructor
         // ********************************************************************
         if (true == slr->getFunctionPointer( "createDummyLib", fpnv ) )
         {
            cout << "got fp for createDummyLib" << endl;
            dl = fpnv();
            if (dl)
            {
               cout << "Expected Val: 0         Actual: " << dl->getVal() << endl;
               dl->setVal(5);
               cout << "Expected Val: 5         Actual: " << dl->getVal() << endl;

               delete(dl);
            }
         }
         else
         {
            cout << "Got UNexpected null on createDummyLib" << endl;
         }
      }
      else
      {
         cout << "UNExpected :  didn't find shared library" << endl <<
                 "  Perhaps you need to specify a path on the command line:" << endl <<
                 "  Usage: sharedLib [pathToSharedLibrary]" << endl;;
      }

		return 0;
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
