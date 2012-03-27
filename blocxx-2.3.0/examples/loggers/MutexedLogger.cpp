#include <blocxx/BLOCXX_config.h>
#include "MutexedLogger.hpp"
#include <blocxx/LogMessage.hpp>
#include <blocxx/MutexLock.hpp>

#include <iostream>

using namespace std;

/////////////////////////////////////////////////////////////////////////////
MutexedLogger::MutexedLogger(Mutex &guard)
	: Logger("sampleLogger", E_ALL_LEVEL)
     , pGuard(&guard)
{
}


/////////////////////////////////////////////////////////////////////////////
void
MutexedLogger::doProcessLogMessage(const LogMessage& message) const
{
	MutexLock l(*pGuard);
	std::cerr << message.message << std::endl;
}

/////////////////////////////////////////////////////////////////////////////
LoggerRef
MutexedLogger::doClone() const
{
	return LoggerRef(new MutexedLogger(*this));
}
