/**
 * @author Norm Paxton (npaxton@novell.com)
 */

/* DemoRunnable.hpp
*
*  Threading classes demonstrated include:
*    Runnable
*    ThreadOnce
*
*  Other classes demonstrated include:
*    Logger
*    LoggerRef
*
*
*/

#include <blocxx/BLOCXX_config.h>
#include "demoRunnable.hpp"
#include <blocxx/Mutex.hpp>
#include <blocxx/MutexLock.hpp>
#include <unistd.h>

using namespace BLOCXX_NAMESPACE;

extern UInt32 existingThreadCount;
extern UInt32 runningThreadCount;

namespace // anonymous
{

bool g_inited = false;
LoggerRef gLogger;

// this demonstrates ThreadOnce - this is once globally
static void initGlobalThreading()
{
   String msg;
   if (g_inited == true)
   {
      msg.format("ERROR:  Got into initGlobalThreading() twice !!!!");
      gLogger->logError(msg);
   }
   else
   {
      g_inited = true;
      msg.format("In the initGlobalThreading() call - should only get here ONCE");
      gLogger->logInfo(msg);
   }
}

// this demonstrates ThreadOnce - this is once per thread
static void initLocalThreading()
{
   String msg;
   msg.format("In the initLocalThreading() call - should get here ONCE per thread ...");
   gLogger->logInfo(msg);
}

// demonstrate using Mutex and MutexLock to thread-safe access the threadCounts
Mutex tsMutex;
static void tsInc(UInt32 &val)
{
   MutexLock lock(tsMutex);
   val++;
}

static void tsDec(UInt32 &val)
{
   MutexLock lock(tsMutex);
   val--;
}

} // end anonymous namespace


static OnceFlag g_once = BLOCXX_ONCE_INIT;

// This will create 'threadNum' threads, each looping 'numLoops' times
//  outputting thread and loop information and waiting for 'waitTime' seconds in each iteration
// different output is allowed by setting -v/--verbosity to different levels on commmand-line, which
// gets passed in here.
// This also will show the initGlobalThreading to be run only once, even though it's called once for each thread
DemoRunnable::DemoRunnable(LoggerRef logger,
							  unsigned int threadNum,
							  unsigned int numLoops,
							  unsigned int waitTime,
                       OnceFlag &globalOnceFlag)
   : Runnable()
	  , m_logger(logger)
	  , m_threadNum(threadNum)
	  , m_numLoops(numLoops)
	  , m_waitTime(waitTime)
	  , m_bShutdown(false)
	 , m_localOnceFlag(g_once)
{
   tsInc(existingThreadCount);
   String msg;
	msg.format("Creating thread [%d]", m_threadNum);
   m_logger->logDebug(msg);
   gLogger = m_logger;
   callOnce(globalOnceFlag, initGlobalThreading);
}

DemoRunnable::~DemoRunnable()
{
   String msg;
	msg.format("Destructing thread [%d]", m_threadNum);
	m_logger->logDebug(msg);
   tsDec(existingThreadCount);
}

void DemoRunnable::run()
{
   tsInc(runningThreadCount);
   String msg;
   unsigned int uiCurLoop = 0;
   msg.format("Thread [%d] starting:  [%d] loops.       Existing Threads: %d     Running Threads: %d",
               m_threadNum,
               m_numLoops,
               existingThreadCount,
               runningThreadCount);
	m_logger->logDebug(msg);

   callOnce(m_localOnceFlag, initLocalThreading);

	while( (uiCurLoop <= m_numLoops) && (m_bShutdown==false) )
	{
	   msg.format("Thread [%d]:  Loop [%d].     Existing Threads: %d    Running Threads: %d",
                  m_threadNum,
                  uiCurLoop,
                  existingThreadCount,
                  runningThreadCount);
		m_logger->logInfo(msg);
		sleep(m_waitTime);
		uiCurLoop++;
	}

   tsDec(runningThreadCount);
}

void DemoRunnable::shutdown()
{
   String msg;
	msg.format("Thread [%d] Shutdown called", m_threadNum);
	m_logger->logDebug(msg);

   m_bShutdown = true;
}


void DemoRunnable::doCooperativeCancel()
{
   String msg;
	msg.format("Thread [%d] doCooperativeCancel called", m_threadNum);
	m_logger->logDebug(msg);

   m_bShutdown = true;
}

void DemoRunnable::doDefinitiveCancel()
{
   String msg;
	msg.format("Thread [%d] doDefinitiveCancel called", m_threadNum);
	m_logger->logDebug(msg);

   m_bShutdown = true;
}
