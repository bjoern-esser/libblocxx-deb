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
*  - Neither the name of Novell, Inc. nor the names of its
*    contributors may be used to endorse or promote products derived from this
*    software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL Novell, Inc. OR THE CONTRIBUTORS
* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

#include "blocxx/BLOCXX_config.h"
#include "IPCMutexTestCases.hpp"

#define PROVIDE_AUTO_TEST_MAIN
#include "AutoTest.hpp"
#include "TestSuite.hpp"
#include "TestCaller.hpp"
AUTO_UNIT_TEST_SUITE_NAMED(IPCMutexTestCases, "IPCMutex");
#include "blocxx/IPCMutex.hpp"
#include "blocxx/Thread.hpp"
#ifndef BLOCXX_WIN32
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#endif
#include <signal.h>
#if defined(BLOCXX_HAVE_SYS_WAIT_H)
#include <sys/wait.h>
#endif

#include <iostream>
#include <cstdlib>  // for getenv
#include <cstring>  // for strcmp,strcpy
using namespace std; 


static const int IPCMUTEX_KEYVAL = 1234;

using namespace blocxx;


struct  SharedStuff_t
{
	int num;
	char buf[256];
};

static const char* const parent1 = "Parent One";
static const char* const parent2 = "Parent Two";
static const char* const parent3 = "Parent Three";
static const char* const child1 = "Child One";
static const char* const child2 = "Child Two";
static const char* const child3 = "Child Three";



namespace
{
	struct PIDWaiter
	{
		PIDWaiter(pid_t pid): m_pid(pid) {}
		~PIDWaiter()
		{
			if( m_pid != pid_t(-1) )
			{
				doWait();
			}
		}

		int doWait()
		{
			int status = 0;
			pid_t childval;
			do
			{
				childval = waitpid(m_pid, &status, 0);
			}
			while( (childval == -1) && (errno == EINTR) );

			m_pid = pid_t(-1);

			cout << "Child quit with status " << WEXITSTATUS(status) << endl;

			return status;
		}

		pid_t m_pid;
	};
}


void IPCMutexTestCases::setUp()
{

#if defined(BLOCXX_IPC_MUTEX_ENABLED)
	IPCMutex::free(IPCMUTEX_KEYVAL);
#endif
}

void IPCMutexTestCases::tearDown()
{
#if defined(BLOCXX_IPC_MUTEX_ENABLED)
	IPCMutex::free(IPCMUTEX_KEYVAL);
#endif
}

void IPCMutexTestCases::testSomething()
{
#if defined(BLOCXX_IPC_MUTEX_ENABLED)
	// This test is way too slow for the fast unit tests.
	if (!getenv("BLOCXXLONGTEST"))
	{
		return;
	}

	const int syncWaitTime = 3; // seconds

	size_t sz = sizeof(SharedStuff_t);
	pid_t pid = fork();
	unitAssertNotEquals(-1, pid);
	if (pid == 0)
	{
		// child
		cout << endl << "Child stopping itself" << endl;
		pid_t own = getpid();
		unitAssertEquals(0, kill(own, SIGSTOP));

		int shmid = shmget(IPCMUTEX_KEYVAL, sz, 0666);
		unitAssertNotEquals(-1, shmid);

		SharedStuff_t* ssp = (SharedStuff_t*) shmat(shmid, 0, 0);

		IPCMutex sem(IPCMUTEX_KEYVAL);
		unitAssertNotEquals((void*)-1, ssp);
		{
			cout << "Child acquiring semaphore..." << endl;
			IPCMutexLock sl(sem);
			cout << "Child acquired semaphore" << endl;
			unitAssertEquals(11, ssp->num);
			unitAssert(strcmp(ssp->buf, parent1) == 0);
			ssp->num = 21;
			strcpy(ssp->buf, child1);

			cout << "Child waiting for parent to attempt an acquire" << endl;
			sleep(syncWaitTime);

			cout << "Child releasing semaphore" << endl;
		}

		// This is wrong, but our old versions of linux don't appear to actually
		// signal the child when doing the semop() called (indirectly) from
		// ~IPCMutex().  Attaching a debugger or using strace to debug this both
		// produce bad results that differ from executing it normally.
		sleep(syncWaitTime);

		{
			cout << "Child acquiring semaphore..." << endl;
			IPCMutexLock sl(sem);
			cout << "Child acquired semaphore" << endl;
			unitAssertEquals(12, ssp->num);
			unitAssert(strcmp(ssp->buf, parent2) == 0);
			ssp->num = 22;
			strcpy(ssp->buf, child2);

			cout << "Child waiting for parent to attempt an acquire" << endl;
			sleep(syncWaitTime);

			cout << "Child releasing semaphore" << endl;
		}
		_exit(0);
	}
	else
	{
		PIDWaiter childCollector(pid);

		// parent
		cout << "Parent waits for child " << endl;

		sleep(syncWaitTime);
		cout << "Parent continues now " << endl;

		unitAssertEquals(256 + sizeof(int), sz);
		int shmid = shmget(IPCMUTEX_KEYVAL, sz,
						   IPC_CREAT | 0666);
		unitAssertNotEquals(-1, shmid);
		SharedStuff_t* ssp = (SharedStuff_t*) shmat(shmid, 0, 0);
		IPCMutex sem(IPCMUTEX_KEYVAL);
		unitAssertNotEquals((void*)-1, ssp);
		{
			cout << "Parent acquiring semaphore..." << endl;
			IPCMutexLock sl(sem);
			cout << "Parent acquired semaphore.  Waking child." << endl;

			unitAssertEquals(0, kill(pid, SIGCONT)); // Wake up the child

			ssp->num = 11;
			strcpy(ssp->buf, parent1);

			cout << "Parent waiting for child to attempt an acquire" << endl;
			sleep(syncWaitTime);

			cout << "Parent releasing semaphore" << endl;
		}

		// BAD (see above)
		sleep(syncWaitTime);

		{
			cout << "Parent acquiring semaphore..." << endl;
			IPCMutexLock sl(sem);
			cout << "Parent acquired semaphore" << endl;
			unitAssertEquals(21, ssp->num);
			unitAssert(strcmp(ssp->buf, child1) == 0);
			ssp->num = 12;
			strcpy(ssp->buf, parent2);

			cout << "Parent waiting for child to attempt an acquire" << endl;
			sleep(syncWaitTime);

			cout << "Parent releasing semaphore" << endl;
		}

		// BAD (see above)
		sleep(syncWaitTime);

		{
			cout << "Parent acquiring semaphore..." << endl;
			IPCMutexLock sl(sem);
			cout << "Parent acquired semaphore" << endl;
			unitAssertEquals(22, ssp->num);
			unitAssert(strcmp(ssp->buf, child2) == 0);

			cout << "Parent waiting for child to attempt an acquire" << endl;
			sleep(syncWaitTime);

			cout << "Parent releasing semaphore" << endl;
		}

		// Make sure the child didn't exit badly.
		{
			int status = childCollector.doWait();
			unitAssert(WIFEXITED(status));
			unitAssertEquals(0, WEXITSTATUS(status));
		}
	}
#endif // defined(BLOCXX_IPC_MUTEX_ENABLED)
}

Test* IPCMutexTestCases::suite()
{
	TestSuite *testSuite = new TestSuite ("IPCMutex");

	ADD_TEST_TO_SUITE(IPCMutexTestCases, testSomething);

	return testSuite;
}

