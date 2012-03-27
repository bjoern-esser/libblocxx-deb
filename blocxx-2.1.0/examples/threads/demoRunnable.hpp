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
*    String
*
*/
#ifndef BLOCXX_EXAMPLES_DEMO_RUNNABLE
#define BLOCXX_EXAMPLES_DEMO_RUNNABLE

#include <blocxx/BLOCXX_config.h>
#include <blocxx/Types.hpp>
#include <blocxx/CommonFwd.hpp>
#include <blocxx/ThreadOnce.hpp>
#include <blocxx/Runnable.hpp>
#include <blocxx/Logger.hpp>
#include <blocxx/String.hpp>

using namespace BLOCXX_NAMESPACE;

class DemoRunnable : public Runnable
{
   public:
	   DemoRunnable(LoggerRef logger,
		           unsigned int threadNum,
					  unsigned int numLoops, 
					  unsigned int waitTime,
                 OnceFlag &globalOnceFlag);
		~DemoRunnable();

		virtual void run();
		void shutdown();

	private:
	   void doCooperativeCancel();
		void doDefinitiveCancel();

      LoggerRef m_logger;
		unsigned int m_threadNum;
		unsigned int m_numLoops;
		unsigned int m_waitTime;
      
      bool m_bShutdown;

      OnceFlag m_localOnceFlag;
};

#endif
