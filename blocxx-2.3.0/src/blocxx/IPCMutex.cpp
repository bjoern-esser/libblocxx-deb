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

#include "blocxx/BLOCXX_config.h"
#include "blocxx/IPCMutex.hpp"
#if defined(BLOCXX_IPC_MUTEX_ENABLED)

#include "blocxx/ExceptionIds.hpp"


namespace BLOCXX_NAMESPACE
{

const int ADD_KEY = 1;
const int BLOCK_FOR_KEY = -1;

BLOCXX_DEFINE_EXCEPTION_WITH_ID(IPCMutex);


//////////////////////////////////////////////////////////////////////////////
IPCMutex::IPCMutex(int semKey)
{
	m_sbuf.sem_num = 0;
	m_sbuf.sem_flg = 0;
	m_semid = semget((key_t)semKey, 1, 0666);
	if (m_semid == -1)
	{
		m_semid = semget((key_t)semKey, 1, IPC_CREAT | 0666);
		if (m_semid == -1)
		{
			BLOCXX_THROW(IPCMutexException,
						 "Unable to create semaphore");
			return;
		}
		m_arg.val = 1;
		if (semctl(m_semid, 0, SETVAL, m_arg) != 0)
		{
			BLOCXX_THROW_ERRNO_MSG(IPCMutexException,
				"semctl() failed");
		}
	}
}

//////////////////////////////////////////////////////////////////////////////
void
IPCMutex::wait()
{
	m_sbuf.sem_op = BLOCK_FOR_KEY;
	if (semop(m_semid, &m_sbuf, 1) != 0)
	{
		BLOCXX_THROW_ERRNO_MSG(IPCMutexException,
			"Failed to wait on semaphore");
	}
}

//////////////////////////////////////////////////////////////////////////////
void
IPCMutex::signal()
{
	m_sbuf.sem_op = ADD_KEY;
	if (semop(m_semid, &m_sbuf, 1) != 0)
	{
		BLOCXX_THROW_ERRNO_MSG(IPCMutexException,
			"Failed to signal semaphore");
	}
}

//////////////////////////////////////////////////////////////////////////////
// static
void
IPCMutex::free(int semKey)
{
	int semid = semget((key_t)semKey, 1, 0666);
	if (semid != -1)
	{
		semctl(semid, 1, IPC_RMID, 0);
	}
}

//////////////////////////////////////////////////////////////////////////////
IPCMutexLock::IPCMutexLock(IPCMutex& sem)
: m_sem(sem)
{
	m_sem.wait();
}

//////////////////////////////////////////////////////////////////////////////
IPCMutexLock::~IPCMutexLock()
{
	m_sem.signal();
}

}

#endif // defined(BLOCXX_IPC_MUTEX_ENABLED)
