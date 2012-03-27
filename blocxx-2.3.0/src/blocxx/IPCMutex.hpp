/*******************************************************************************
* Copyright (C) 2005 Novell, Inc. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of source code must retain the above copyright notice,
*    this list of conditions and the following disclaimer.
*
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
*
*  - Neither the name of Novell, Inc., nor the names of its
*    contributors may be used to endorse or promote products derived from this
*    software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL Novell, Inc., OR THE
* CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
* OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

/**
 * @author Bart Whiteley
 */

#ifndef BLOCXX_IPC_MUTEX_HPP_INCLUDE_GUARD_
#define BLOCXX_IPC_MUTEX_HPP_INCLUDE_GUARD_

#include "blocxx/BLOCXX_config.h"
// This class only works on Linux.  Apparently, it only works on older versions
// of linux.  It  is known to NOT work with kernel  version 2.6.28.  If someone
// really needs this, they will need to fix it.
#if defined(BLOCXX_GNU_LINUX)
#include <linux/version.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
#if !defined (BLOCXX_HAVE_SYS_IPC_H) && !defined (BLOCXX_HAVE_SYS_SEM_H)
 #error "Port me!"
#else

#define BLOCXX_IPC_MUTEX_ENABLED 1

#include "blocxx/Exception.hpp"

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>


namespace BLOCXX_NAMESPACE
{

BLOCXX_DECLARE_APIEXCEPTION(IPCMutex, BLOCXX_COMMON_API);

/**
 * WARNING: This class only works on Linux. Also the tests are disabled.
 * @todo Fix it to work on modern versions of Linux and other platforms.
 */
class IPCMutex
{
public:
	/**
	 * Construct an IPCMutex instance.  The first time this ctor is
	 * called with a given semId, the process semaphore will be created.
	 * When the semaphore is no longer needed, IPCMutex::free() should
	 * be called with the same semId.
	 *
	 * @param semId A unique identifier for the process mutex.
	 *
	 * @throws IPCMutexException if unable to create a process semaphore
	 */
	IPCMutex(int semId);
	~IPCMutex() {};

	/**
	 * Wait on the process semaphore
	 */
	void wait();

	/**
	 * Signal the process semaphore.
	 */
	void signal();
	/**
	 * Get the process semaphore ID
	 * @return The process semaphore ID
	 */
	int getId() { return m_semid;}

	/**
	 * Free the system resources associated with the process semaphore
	 * @param semKey The key to the process semaphore resources to free
	 */
	static void free(int semKey);
private:
	int m_semid;
	struct sembuf m_sbuf;
	union semun
	{
		int val;
		struct sumid_ds* buf;
		unsigned short* array;
	} m_arg;

};

//////////////////////////////////////////////////////////////////////////////

/**
 * A class to simplify the use of IPCMutex, and make it exception safe.
 * Example:
 *
 *    IPCMutex guard(1234);
 *    {
 *       IPCMutexLock mlock(guard);
 *       // critical section code;
 *    }
 *
 * Note that you don't have to keep guard around, just use the same key
 * value each time.
 */
class IPCMutexLock
{
public:
	IPCMutexLock(IPCMutex& sem);
	~IPCMutexLock();
private:
	IPCMutex& m_sem;
};

} // end namespace

#endif // #if !defined (BLOCXX_HAVE_SYS_IPC_H) && !defined (BLOCXX_HAVE_SYS_SEM_H)
#endif // #if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
#endif // #ifdef BLOCXX_GNU_LINUX
#endif // #ifndef BLOCXX_IPC_MUTEX_HPP_INCLUDE_GUARD_

