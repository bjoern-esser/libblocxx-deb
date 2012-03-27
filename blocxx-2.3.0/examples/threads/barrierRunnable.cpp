/**
 * @author Norm Paxton (npaxton@novell.com)
 */

/* BarrierRunnable.hpp
*
*  Threading classes demonstrated include:
*    Runnable
*    ThreadBarrier
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


// NOTE: Currently identical to conditionRunnable...

#include <blocxx/BLOCXX_config.h>
#include "barrierRunnable.hpp"
#include <blocxx/ThreadBarrier.hpp>
#include <blocxx/NonRecursiveMutex.hpp>
#include <blocxx/NonRecursiveMutexLock.hpp>
#include <blocxx/String.hpp>
#include <blocxx/Logger.hpp>


using namespace BLOCXX_NAMESPACE;

extern ThreadBarrier barrier;

BarrierRunnable::BarrierRunnable(LoggerRef logger,
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

BarrierRunnable::~BarrierRunnable()
{
   String msg;
	msg.format("Destructing thread [%d]", m_threadNum);
	m_logger->logDebug(msg);
}

void BarrierRunnable::run()
{
   String msg;
   msg.format("Thread [%d] starting:  Waiting on barrier.",
               m_threadNum);
	m_logger->logDebug(msg);

   barrier.wait();

   msg.format("Thread [%d] received barrier.   Falling out.",
               m_threadNum);
   m_logger->logDebug(msg);
}

void BarrierRunnable::shutdown()
{
   String msg;
	msg.format("Thread [%d] Shutdown called", m_threadNum);
	m_logger->logDebug(msg);

   m_bShutdown = true;
}


void BarrierRunnable::doCooperativeCancel()
{
   String msg;
	msg.format("Thread [%d] doCooperativeCancel called", m_threadNum);
	m_logger->logDebug(msg);

   m_bShutdown = true;
}

void BarrierRunnable::doDefinitiveCancel()
{
   String msg;
	msg.format("Thread [%d] doDefinitiveCancel called", m_threadNum);
	m_logger->logDebug(msg);

   m_bShutdown = true;
}
