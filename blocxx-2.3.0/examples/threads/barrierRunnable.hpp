/**
 * @author Norm Paxton (npaxton@novell.com)
 */

/* BarrierRunnable.hpp
*
*  Threading classes demonstrated include:
*    ThreadBarrier
*    Runnable
*
*  Other classes demonstrated include:
*    Logger
*    LoggerRef
*    String
*
*/


// NOTE: Currently identical to conditionRunnable...


#ifndef BLOCXX_EXAMPLES_BARRIER_RUNNABLE_INCLUDE_GUARD_
#define BLOCXX_EXAMPLES_BARRIER_RUNNABLE_INCLUDE_GUARD_

#include <blocxx/BLOCXX_config.h>
#include <blocxx/CommonFwd.hpp>
#include <blocxx/Runnable.hpp>


using namespace BLOCXX_NAMESPACE;

class BarrierRunnable : public Runnable
{
   public:
	   BarrierRunnable(LoggerRef logger,
		           unsigned int threadNum);
		~BarrierRunnable();

		virtual void run();
		void shutdown();

	private:
	   void doCooperativeCancel();
		void doDefinitiveCancel();

      LoggerRef m_logger;
		unsigned int m_threadNum;

      bool m_bShutdown;
};

#endif
