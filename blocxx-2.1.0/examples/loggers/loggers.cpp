/**
 * @author Norm Paxton (npaxton@novell.com)
 */

/* loggers.cpp
**
** This demonstrates how to use the various logging classes of BloCxx
*  Logger classes demonstrated include:
*       LogAppender / LogAppenderRef
*       AppenderLogger
*       Logger / LoggerRef
* 
*  Other classes demonstrated include:
*
*/

#include <blocxx/LogAppender.hpp>
#include <blocxx/AppenderLogger.hpp>
#include <blocxx/Logger.hpp>
#include <blocxx/LogConfig.hpp>
#include <blocxx/String.hpp>
#include <blocxx/Mutex.hpp>
#include <blocxx/MutexLock.hpp>

#include <blocxx/BLOCXX_config.h>
#include <iostream>

#include "MutexedLogger.hpp"

using namespace BLOCXX_NAMESPACE;
using namespace std;

#define SAMPLE_VERSION  "Version 0.1"
#define SAMPLE_AUTHOR   "Norm Paxton"
#define SAMPLE_EMAIL    "npaxton@novell.com"


// ********************************************************************
// Show how to use LogAppender to append to stderr
// ********************************************************************
void logAppender(String name, const String type, ELogLevel level, String location)
{
   try
   {
      LoggerConfigMap configItems; // empty for syslog
      String strPath;
      if (type == LogAppender::TYPE_FILE)
      {
         String strKey;
         strKey.format("log.%s.location", name.c_str());
         strPath.format("%s/%s", location.c_str(), name.c_str());
         configItems[strKey] = strPath;
      }
      LogAppenderRef genericAppender = LogAppender::createLogAppender(name,
                                                                     LogAppender::ALL_COMPONENTS,
                                                                     LogAppender::ALL_CATEGORIES,
                                                                     LogAppender::STR_TTCC_MESSAGE_FORMAT,
                                                                     type,
                                                                     configItems );
                                                                     
      LoggerRef genericLogger(new AppenderLogger("genericLoggerTest", level, genericAppender));
      
      //genericLogger->logMessage("This is a dummy message log msg."); // should it go to output?
      genericLogger->logDebug("This is a dummy debug log msg."); // should it go to output?
      genericLogger->logInfo("This is a dummy info log msg."); // should it go to output?
      genericLogger->logError("This is a dummy error log msg."); // should it go to output?
      genericLogger->logFatalError("This is a dummy fatalError log msg."); // should it go to output?
   }
   catch (LoggerException &e)
   {
      cout << "Exception (perhaps you need to be superUser): " << endl << e.what() << endl;
   }
}


// ********************************************************************
// The main function... This demonstrates the Logger classes
// ********************************************************************
int main(int argc, char* argv[])
{
   Mutex guard;
   
   try
   {
      // the following examples all implement various logging using AppenderLogger
      logAppender("syslog", LogAppender::TYPE_SYSLOG, E_ERROR_LEVEL, "");
      logAppender("stderr", LogAppender::TYPE_STDERR, E_ERROR_LEVEL, "");
      logAppender("testFileName", LogAppender::TYPE_FILE, E_ERROR_LEVEL, "/var/log");
      logAppender("", LogAppender::TYPE_NULL, E_ERROR_LEVEL, "");

      LoggerRef log(new MutexedLogger(guard));
      log->logInfo("This is a thread-safe msg sent through MutexedLogger");
      
      MutexLock lock(guard);
      cerr << "This is a thread-safe msg sent through cerr" << endl;
      lock.release();

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
