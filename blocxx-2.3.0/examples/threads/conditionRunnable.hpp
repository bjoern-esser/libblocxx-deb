/**
 * @author Norm Paxton (npaxton@novell.com)
 */

/* ConditionRunnable.hpp
*
*  Threading classes demonstrated include:
*    Condition
*    Runnable
*
*  Other classes demonstrated include:
*    Logger
*    LoggerRef
*    String
*    NonRecursiveMutex
*    NonRecursiveMutexLock
*
*/
#ifndef BLOCXX_EXAMPLES_CONDITION_RUNNABLE_INCLUDE_GUARD_
#define BLOCXX_EXAMPLES_CONDITION_RUNNABLE_INCLUDE_GUARD_

#include <blocxx/BLOCXX_config.h>
#include <blocxx/CommonFwd.hpp>
#include <blocxx/Runnable.hpp>
#include <blocxx/NonRecursiveMutex.hpp>

using namespace BLOCXX_NAMESPACE;

static NonRecursiveMutex g_mutex;

class ConditionRunnable : public Runnable
{
   public:
	   ConditionRunnable(LoggerRef logger,
		           unsigned int threadNum);
		~ConditionRunnable();

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
