/**
 * @author Norm Paxton (npaxton@novell.com)
 */

/* ConditionRunnable.hpp
*
*  Threading classes demonstrated include:
*    Runnable
*    Condition
*
*  Other classes demonstrated include:
*    Logger
*    LoggerRef
*    NonRecursiveMutex
*    NonRecursiveMutexLock
*    String
*
*
*/

#include <blocxx/BLOCXX_config.h>
#include "conditionRunnable.hpp"
#include <blocxx/NonRecursiveMutex.hpp>
#include <blocxx/NonRecursiveMutexLock.hpp>
#include <blocxx/Condition.hpp>
#include <blocxx/Logger.hpp>
#include <blocxx/String.hpp>

using namespace BLOCXX_NAMESPACE;

extern Condition cond;

ConditionRunnable::ConditionRunnable(LoggerRef logger,
							  unsigned int threadNum)
   : Runnable()
	  , m_logger(logger)
	  , m_threadNum(threadNum)
     , m_bShutdown(false)
{
   String msg;
	msg.format("Creating thread [%d]", m_threadNum);
   m_logger->logDebug(msg);
}

ConditionRunnable::~ConditionRunnable()
{
   String msg;
	msg.format("Destructing thread [%d]", m_threadNum);
	m_logger->logDebug(msg);
}

void ConditionRunnable::run()
{
   String msg;
   msg.format("Thread [%d] starting:  Waiting on mutex.",
               m_threadNum);
	m_logger->logDebug(msg);

   NonRecursiveMutexLock lock(g_mutex);
   msg.format("Thread [%d] starting:  Waiting on condition.",
               m_threadNum);
   m_logger->logDebug(msg);
   cond.wait(lock);
   lock.release();

   msg.format("Thread [%d] received condition.   Falling out.",
               m_threadNum);
   m_logger->logDebug(msg);
}

void ConditionRunnable::shutdown()
{
   String msg;
	msg.format("Thread [%d] Shutdown called", m_threadNum);
	m_logger->logDebug(msg);

   m_bShutdown = true;
}


void ConditionRunnable::doCooperativeCancel()
{
   String msg;
	msg.format("Thread [%d] doCooperativeCancel called", m_threadNum);
	m_logger->logDebug(msg);

   m_bShutdown = true;
}

void ConditionRunnable::doDefinitiveCancel()
{
   String msg;
	msg.format("Thread [%d] doDefinitiveCancel called", m_threadNum);
	m_logger->logDebug(msg);

   m_bShutdown = true;
}
