/**
 * @author Norm Paxton (npaxton@novell.com)
 */

/* DemoThread.hpp
*
*  Threading classes demonstrated include:
*    Runnable
*
*  Other classes demonstrated include:
*    Logger
*    LoggerRef
*
*/
#ifndef BLOCXX_EXAMPLES_DEMO_THREAD_INCLUDE_GUARD_
#define BLOCXX_EXAMPLES_DEMO_THREAD_INCLUDE_GUARD_

#include <blocxx/BLOCXX_config.h>
#include <blocxx/Types.hpp>
#include <blocxx/CommonFwd.hpp>
#include <blocxx/Thread.hpp>
#include <blocxx/ThreadOnce.hpp>
#include <blocxx/Runnable.hpp>
#include <blocxx/Logger.hpp>
#include <blocxx/String.hpp>

using namespace BLOCXX_NAMESPACE;

class DemoThread : public Runnable
{
   public:
	   DemoThread(LoggerRef logger,
		           unsigned int threadNum,
					  unsigned int numLoops,
					  unsigned int waitTime,
                 OnceFlag &globalOnceFlag);
		~DemoThread();

		virtual void run();
		void shutdown();

	private:
	   void doCooperativeCancel();
		void doDefinitiveCancel();

      LoggerRef m_logger;
		unsigned int m_threadNum;
		unsigned int m_numLoops;
		unsigned int m_waitTime;

      OnceFlag m_localOnceFlag;
};

#endif
