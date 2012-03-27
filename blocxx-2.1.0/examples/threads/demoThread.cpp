/**
 * @author Norm Paxton (npaxton@novell.com)
 */

/* DemoThread.hpp
* 
*  Threading classes demonstrated include:
*    Runnable
*    ThreadOnce
*    Mutex
*    MutexLock
*    ThreadOnce
* 
*  Other classes demonstrated include:
*    Logger
*    LoggerRef
*    String
*    
*
*/

#include "demoThread.hpp"
#include <blocxx/Mutex.hpp>
#include <blocxx/MutexLock.hpp>

using namespace BLOCXX_NAMESPACE;

extern UInt32 existingThreadCount;
extern UInt32 runningThreadCount;

namespace // anonymous
{
   
bool g_inited = false;
LoggerRef gLogger;

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

static void initLocalThreading()
{
   String msg;
   msg.format("In the initLocalThreading() call - should get here ONCE per thread ...");
   gLogger->logInfo(msg);
}

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


DemoThread::DemoThread(LoggerRef logger,
							  unsigned int threadNum, 
							  unsigned int numLoops, 
							  unsigned int waitTime,
                       OnceFlag &globalOnceFlag)
   : Runnable()
	  , m_logger(logger)
	  , m_threadNum(threadNum)
	  , m_numLoops(numLoops)
	  , m_waitTime(waitTime)
     , m_localOnceFlag(BLOCXX_ONCE_INIT)
{
   tsInc(existingThreadCount);
   String msg;
	msg.format("Creating thread [%d]", m_threadNum);
   m_logger->logDebug(msg);
   gLogger = m_logger;
   callOnce(globalOnceFlag, initGlobalThreading);
}

DemoThread::~DemoThread()
{
   String msg;
	msg.format("Destructing thread [%d]", m_threadNum);
	m_logger->logDebug(msg);
   tsDec(existingThreadCount);
}

void DemoThread::run()
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

	while( uiCurLoop <= m_numLoops)
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

void DemoThread::shutdown()
{
   String msg;
	msg.format("Thread [%d] Shutdown called", m_threadNum);
	m_logger->logDebug(msg);
}


void DemoThread::doCooperativeCancel()
{
   String msg;
	msg.format("Thread [%d] doCooperativeCancel called", m_threadNum);
	m_logger->logDebug(msg);
}

void DemoThread::doDefinitiveCancel()
{
   String msg;
	msg.format("Thread [%d] doDefinitiveCancel called", m_threadNum);
	m_logger->logDebug(msg);
}
