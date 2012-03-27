#ifndef BLOCXX_MUTEXED_LOGGER_HPP_INCLUDE_GUARD_
#define BLOCXX_MUTEXED_LOGGER_HPP_INCLUDE_GUARD_

#include <blocxx/BLOCXX_config.h>
#include <blocxx/Logger.hpp>
#include <blocxx/Mutex.hpp>

using namespace BLOCXX_NAMESPACE;

/// This logger sends all log messages to cerr (stderr)
class MutexedLogger : public Logger
{
public:
	MutexedLogger(Mutex &guard);

	virtual void doProcessLogMessage(const LogMessage&) const;

	virtual LoggerRef doClone() const;

private:
   Mutex *pGuard;
};


#endif
