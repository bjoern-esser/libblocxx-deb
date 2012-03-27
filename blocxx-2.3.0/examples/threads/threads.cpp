/**
 * @author Norm Paxton (npaxton@novell.com)
 */

/* threads.cpp
**
** This demonstrates how to use the various threading classes of BloCxx
*  Threading classes demonstrated include:
*    ThreadPool
*    ThreadOnce
*    Runnable
*    Mutex
*    MutexLock
*    Thread
*    Condition
*    ThreadBarrier
*
*  Other classes demonstrated include:
*    CmdLineParserException
*    Exception
*    String
*    StringArray
*
*
* Tips: To demonstrate behaviour of different threads, try running from the command-line
*  and making queueSize < totalThreads, changing poolType, etc.
*
*/


#include <blocxx/BLOCXX_config.h>
#include <blocxx/Types.hpp>
#include <blocxx/ThreadPool.hpp>
#include <blocxx/Thread.hpp>
#include <blocxx/ThreadOnce.hpp>
#include <blocxx/ThreadBarrier.hpp>
#include <blocxx/Condition.hpp>
#include <blocxx/CerrLogger.hpp>
#include <blocxx/CmdLineParser.hpp>
#include <blocxx/Exception.hpp>
#include <blocxx/Timeout.hpp>
//#include "demoThread.hpp"
#include "demoRunnable.hpp"
#include "barrierRunnable.hpp"
#include "conditionRunnable.hpp"
#include <fstream>
#include <iostream>
#include <unistd.h>

using namespace BLOCXX_NAMESPACE;
using namespace std;

#define SAMPLE_NAME     "threads"
#define SAMPLE_VERSION  "Version 0.1"
#define SAMPLE_AUTHOR   "Norm Paxton"
#define SAMPLE_EMAIL    "npaxton@novell.com"

// these counts will be used in demoRunnable, and accessed through mutex.
UInt32 existingThreadCount = 0;
UInt32 runningThreadCount = 0;

// these indicate the number of threads for conditionRunnable or barrierRunnable
// demos.  Not command-line-configurable, because changing these values doesn't really
// change the behavior, just shortens or lengthens it.
#define NUM_CONDITION_THREADS 5
#define NUM_BARRIER_THREADS 5

// ********************************************************************
// Used to demonstrate using Condition
//    Condition:  each thread will sit and wait on a condition.
//                the primary thread will notify one or many of the threads
//                when notified, a thread will continue
// ********************************************************************
Condition cond;
// ********************************************************************
// Used to demonstrate using ThreadBarrier
//    ThreadBarrier:  when initialized, tell it how many barriers are expected
//                    each thread will wait on the threadBarrier.
//                    once the specified number of barriers are waiting, they
//                    are all released to continue execution
// ********************************************************************
ThreadBarrier barrier(NUM_BARRIER_THREADS + 1);


// ********************************************************************
// Print any command line exception to cerr
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
   RUNNABLE_OPT,
   CONDITION_OPT,
   BARRIER_OPT,
   THREAD_OPT,
	INTERVAL_OPT,
	THREADCOUNT_OPT,
	NUMTHREADS_OPT,
   QUEUE_OPT,
   POOLTYPE_OPT,
	LOOP_OPT,
	LOOP_WAIT_OPT,
	NOBLOCK_OPT,
   SIGNAL_OPT,
	VERBOSE_OPT,
	THREADPOOL_LOGGER_OPT,
   SEPARATOR_OPER,
   SEPARATOR_ALL,
   SEPARATOR_RUNNABLE,
   SEPARATOR_CONDITION
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
   {VERSION_OPT, '\0', "version", CmdLineParser::E_NO_ARG, 0, "Show version information."},
   {SEPARATOR_OPER, '\0', "",              CmdLineParser::E_GROUPING,  0, "OPERATIONS:"},
   {RUNNABLE_OPT,    'r', "runnable",      CmdLineParser::E_NO_ARG,  0, "Demonstrate Runnable (Default operation)"},
   {CONDITION_OPT,   'c', "condition",     CmdLineParser::E_NO_ARG,  0, "Demonstrate Condition"},
   {BARRIER_OPT,     'b', "barrier",       CmdLineParser::E_NO_ARG,  0, "Demonstrate ThreadBarrier"},
   {THREAD_OPT,      't', "thread",        CmdLineParser::E_NO_ARG,  0, "Demonstrate Thread"},
   {SEPARATOR_ALL,  '\0', "",              CmdLineParser::E_GROUPING,  0, "The following apply to RUNNABLE, CONDITION, and THREADBARRIER"},
   {NOBLOCK_OPT,     'x', "noblock",       CmdLineParser::E_NO_ARG,        0, "Don't block when adding work to queue.  (Default = block)  With threadPools, you can add work in two ways:\n1. tryAddWork, which if the queue is full it fails\n2. addWork, which if the queue is full, it blocks until there is space to add the work.\nThis allows for demonstrating both ways"},
   {VERBOSE_OPT,     'v', "verbosity",     CmdLineParser::E_REQUIRED_ARG,  0, "Verbosity of logger output.  Options are:  NONE, FATAL, ERROR, INFO, DEBUG, ALL  (Default=INFO)"},
   {THREADPOOL_LOGGER_OPT,'\0', "tpLogger",CmdLineParser::E_NO_ARG,        0, "Include the threadpool output in the logger output  (Default = don't include)"},
   {SEPARATOR_RUNNABLE,'\0', "",           CmdLineParser::E_GROUPING,  0, "RUNNABLE options:"},
   {THREADCOUNT_OPT, 'n', "totalThreads",  CmdLineParser::E_OPTIONAL_ARG, "12", "Total number of threads to use (put into the threadpool queue, threads to create, etc).  Default=12"},
   {NUMTHREADS_OPT,  'a', "activeThreads", CmdLineParser::E_OPTIONAL_ARG, "5", "ThreadPool Threads to use at one time.  Default=5"},
   {QUEUE_OPT,       'q', "queueSize",     CmdLineParser::E_OPTIONAL_ARG, "8", "ThreadPool Queue size.  Default = 8.  Must be > number of Active Threads"},
   {POOLTYPE_OPT,    'p', "poolType",      CmdLineParser::E_OPTIONAL_ARG, "1", "ThreadPool Type (0, 1, or 2).  0=Fixed, 1=Dynamic (Default),  2=DynamicNoQueue"},
   {LOOP_OPT,        'l', "loop",          CmdLineParser::E_OPTIONAL_ARG, "10", "How many times each thread should loop before it is quits (Default = 10)"},
   {INTERVAL_OPT,    'i', "interval",      CmdLineParser::E_OPTIONAL_ARG, "1", "Interval (in seconds) between creation of each thread.  Default = 1."},
   {LOOP_WAIT_OPT,   'w', "loopwait",      CmdLineParser::E_OPTIONAL_ARG, "1", "How long to wait (in seconds) between each loop in the thread.  Default = 1."},
   {SEPARATOR_CONDITION,'\0', "",          CmdLineParser::E_GROUPING,  0, "CONDITION options:"},
   {SIGNAL_OPT,      's', "signal",        CmdLineParser::E_REQUIRED_ARG,  0, "Number of threads to signal.  Default = 2"},
   {0, 0, 0, CmdLineParser::E_NO_ARG, 0, 0}
};


// ********************************************************************
// output the usage options -  This is facilitated by the CmdLineParser::getUsage call.
// ********************************************************************
void Usage()
{
	cout << "Usage: threads [options]\n\n";
   cout << CmdLineParser::getUsage(g_options) << endl;
}



// ********************************************************************
// demoRunnable:  demonstrate the Runnable/ThreadPool classes
// ********************************************************************
void demoRunnable(CmdLineParser &parser)
{
   String strInterval;
   String strThreadCount;
   String strActiveThreads;
   String strQueueSize;
   String strPoolType;
   String strLoop;
   String strWait;
   String strVerbosity;

   try
   {
      strInterval = parser.getOptionValue(INTERVAL_OPT, "1");
      strThreadCount = parser.getOptionValue(THREADCOUNT_OPT, "12");
      strActiveThreads = parser.getOptionValue(NUMTHREADS_OPT, "5");
      strQueueSize = parser.getOptionValue(QUEUE_OPT, "8");
      strPoolType = parser.getOptionValue(POOLTYPE_OPT, "1");
      strLoop = parser.getOptionValue(LOOP_OPT, "10");
      strWait = parser.getOptionValue(LOOP_WAIT_OPT, "1");
      strVerbosity = parser.getOptionValue(VERBOSE_OPT, "DEBUG");

      UInt32 uiInterval = strInterval.toUInt32();
      UInt32 uiThreadCount = strThreadCount.toUInt32();
      UInt32 uiActiveThreads = strActiveThreads.toUInt32();
      UInt32 uiQueueSize = strQueueSize.toUInt32();
      UInt32 uiPoolType = strPoolType.toUInt32();
      UInt32 uiLoop = strLoop.toUInt32();
      UInt32 uiWait = strWait.toUInt32();

      LoggerRef logger = LoggerRef(new CerrLogger());
      logger->setLogLevel(strVerbosity);
/*
      LoggerRef tpLogger;
      if (parser.isSet(THREADPOOL_LOGGER_OPT))
      {
         tpLogger = logger;
      }
*/

      String msg;
      msg.format("Running threads with the following options:\r\n\tVerbosity: %s\r\n\tTotal Threads: %d\r\n\tActive Threads: %d\r\n\tThreadpool Type: %d\r\n\tQueue Size: %d\r\n\tThreadCreate Interval: %d\r\n\tNum Thread Loops: %d\r\n\tThread Loop Wait: %d\r\n\tAdd Work with Blocking: %d\r\n\tThreadPool Logger: %d",
                        strVerbosity.c_str(),
                        uiThreadCount,
                        uiActiveThreads,
                        uiPoolType,
                        uiQueueSize,
                        uiInterval,
                        uiLoop,
                        uiWait,
                        parser.isSet(NOBLOCK_OPT)?0:1,
                        parser.isSet(THREADPOOL_LOGGER_OPT)?1:0);
      logger->logMessage(String("CATEGORY"), msg);

      ThreadPoolRef tPool = ThreadPoolRef(new ThreadPool((ThreadPool::PoolType)uiPoolType, uiActiveThreads, uiQueueSize, *logger, "DemoRunnablePool"));

      OnceFlag globalOnceFlag = BLOCXX_ONCE_INIT;

      for (unsigned int ui=0; ui<uiThreadCount; ui++)
      {
         msg.format("Adding thread [%d] to the threadpool", ui);
         logger->logDebug(msg);
         // create new thread here and add it to the threadpool
         RunnableRef thisThread  = RunnableRef(
                   new DemoRunnable(logger,
                                    ui,
                                    uiLoop,
                                    uiWait,
                                    globalOnceFlag)
                                              );
         if (!parser.isSet(NOBLOCK_OPT))
         {
            tPool->addWork(thisThread);
         }
         else
         {
            if (!tPool->tryAddWork(thisThread))
            {
               msg.format("Failed to add thread [%d] to threadpool", ui);
               logger->logError(msg);
            }
         }
         sleep(uiInterval);
      }

      msg.format("All Threads have been added to the threadPool, now causing a shutdown");
      logger->logInfo(msg);

      // don't wait for the queue to empty, cause a shutdown
      //tPool->waitForEmptyQueue();
      tPool->shutdown(ThreadPool::E_DISCARD_WORK_IN_QUEUE, Timeout::relative(2.0));
   }
   catch (CmdLineParserException &e)
   {
      printCmdLineParserExceptionMessage(e);
      Usage();
   }
   catch (StringConversionException &e)
   {
      cout << "Invalid parameter option - must be a numeric value" << endl << e << endl;
   }
}


// ********************************************************************
// demoCondition:  demonstrate the Condition class
// ********************************************************************
void demoCondition(CmdLineParser &parser)
{
   String strPoolType;
   String strVerbosity;
   String strSignal;

   try
   {
      strPoolType = parser.getOptionValue(POOLTYPE_OPT, "1");
      strVerbosity = parser.getOptionValue(VERBOSE_OPT, "DEBUG");
      strSignal = parser.getOptionValue(SIGNAL_OPT, "4");

      UInt32 uiPoolType = strPoolType.toUInt32();
      UInt32 uiSignal = strSignal.toUInt32();

      LoggerRef logger = LoggerRef(new CerrLogger());
      logger->setLogLevel(strVerbosity);

	  /*
      LoggerRef tpLogger;
      if (parser.isSet(THREADPOOL_LOGGER_OPT))
      {
         tpLogger = logger;
      }
	  */

      ThreadPoolRef tPool = ThreadPoolRef(new ThreadPool((ThreadPool::PoolType)uiPoolType, NUM_CONDITION_THREADS, NUM_CONDITION_THREADS, *logger, "ConditionRunnablePool"));

      String msg;

      logger->logInfo("Starting to add threads to threadpool");

      for (unsigned int ui=0; ui<NUM_CONDITION_THREADS; ui++)
      {
         msg.format("Adding thread [%d] to the threadpool", ui);
         logger->logDebug(msg);

         // create new thread here and add it to the threadpool
         RunnableRef thisThread  = RunnableRef(
                     new ConditionRunnable(  logger,
                                             ui )  );
         if (!parser.isSet(NOBLOCK_OPT))
         {
            tPool->addWork(thisThread);
         }
         else
         {
            if (!tPool->tryAddWork(thisThread))
            {
               msg.format("Failed to add thread [%d] to threadpool", ui);
               logger->logError(msg);
            }
         }
      }

      msg.format("About to sleep for 2 seconds");
      logger->logInfo(msg);
      sleep(2);

      msg.format("All Threads have been added to the threadPool, now signalling the first %d threads", uiSignal);
      logger->logInfo(msg);

      if (uiSignal > NUM_CONDITION_THREADS)
         uiSignal = NUM_CONDITION_THREADS;

      for (unsigned int kill = 0; kill < uiSignal; kill++)
      {
         cond.notifyOne();
      }
      sleep(1);

      msg.format("About to sleep for 2 seconds");
      logger->logInfo(msg);
      sleep(2);

      msg.format("Now signalling all the remainder of the threads.");
      logger->logInfo(msg);
      cond.notifyAll();
      msg.format("All threads have been signalled");
      logger->logInfo(msg);

      tPool->waitForEmptyQueue();
      tPool->shutdown();
   }
   catch (CmdLineParserException &e)
   {
      printCmdLineParserExceptionMessage(e);
      Usage();
   }
   catch (StringConversionException &e)
   {
      cout << "Invalid parameter option - must be a numeric value" << endl << e << endl;
   }
}


// ********************************************************************
// demoBarrier:  demonstrate the ThreadBarrier class
// ********************************************************************
void demoBarrier(CmdLineParser &parser)
{
   String strPoolType;
   String strVerbosity;
   String strSignal;

   try
   {
      strPoolType = parser.getOptionValue(POOLTYPE_OPT, "1");
      strVerbosity = parser.getOptionValue(VERBOSE_OPT, "DEBUG");
      strSignal = parser.getOptionValue(SIGNAL_OPT, "4");

      UInt32 uiPoolType = strPoolType.toUInt32();
      UInt32 uiSignal = strSignal.toUInt32();

      LoggerRef logger = LoggerRef(new CerrLogger());
      logger->setLogLevel(strVerbosity);

	  /*
      LoggerRef tpLogger;
      if (parser.isSet(THREADPOOL_LOGGER_OPT))
      {
         tpLogger = logger;
      }
	  */

      ThreadPoolRef tPool = ThreadPoolRef(new ThreadPool((ThreadPool::PoolType)uiPoolType, NUM_BARRIER_THREADS, NUM_BARRIER_THREADS, *logger, "BarrierRunnablePool"));

      String msg;

      logger->logInfo("Starting to add threads to threadpool");

      for (unsigned int ui=0; ui<NUM_CONDITION_THREADS; ui++)
      {
         msg.format("Adding thread [%d] to the threadpool", ui);
         logger->logDebug(msg);

         // create new thread here and add it to the threadpool
         RunnableRef thisThread  = RunnableRef(
                     new BarrierRunnable(  logger,
                                           ui )  );
         if (!parser.isSet(NOBLOCK_OPT))
         {
            tPool->addWork(thisThread);
         }
         else
         {
            if (!tPool->tryAddWork(thisThread))
            {
               msg.format("Failed to add thread [%d] to threadpool", ui);
               logger->logError(msg);
            }
         }
      }
      msg.format("All threads added to threadpool.  About to sleep for 2 seconds");
      logger->logInfo(msg);
      sleep(2);

      msg.format("About to signal main thread barrier");
      logger->logInfo(msg);
      barrier.wait();

      msg.format("Main thread received barrier");
      logger->logInfo(msg);

      tPool->waitForEmptyQueue();
      tPool->shutdown();
   }
   catch (CmdLineParserException &e)
   {
      printCmdLineParserExceptionMessage(e);
      Usage();
   }
   catch (StringConversionException &e)
   {
      cout << "Invalid parameter option - must be a numeric value" << endl << e << endl;
   }
}


// ********************************************************************
// demoThread:  demonstrate the Thread class
// ********************************************************************
void demoThread(CmdLineParser &parser)
{
   cout << "The thread operation is not yet implemented" << endl;
}


// ********************************************************************
// The main function... This demonstrates the CmdLineParser class, and depending upon options, calls into other functions
//      Also demonstrates ld/Exceptions
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
         cout << "BloCxx Sample App  '" << SAMPLE_NAME << "': " << SAMPLE_VERSION << endl;
         cout << "Written by " << SAMPLE_AUTHOR << endl;
         cout << SAMPLE_EMAIL << endl;
         return 0;
      }
      else if (parser.isSet(CONDITION_OPT))
      {
         demoCondition(parser);
      }
      else if (parser.isSet(BARRIER_OPT))
      {
         demoBarrier(parser);
      }
      else if (parser.isSet(THREAD_OPT))
      {
         demoThread(parser);
      }
		else // default:  if (parser.iseSet(RUNNABLE_OPT))
		{
         demoRunnable(parser);
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
