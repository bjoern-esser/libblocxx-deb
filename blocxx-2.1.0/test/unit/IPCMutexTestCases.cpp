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

#include <blocxx/BLOCXX_config.h>
#include "TestSuite.hpp"
#include "TestCaller.hpp"
#include "IPCMutexTestCases.hpp"
#include "blocxx/IPCMutex.hpp"
#ifndef BLOCXX_WIN32
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#endif
#include <signal.h>

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




void IPCMutexTestCases::setUp()
{
#ifdef BLOCXX_GNU_LINUX
	IPCMutex::free(IPCMUTEX_KEYVAL); 
#endif
}

void IPCMutexTestCases::tearDown()
{
#ifdef BLOCXX_GNU_LINUX
	IPCMutex::free(IPCMUTEX_KEYVAL); 
#endif
}



void IPCMutexTestCases::testSomething()
{
#ifdef BLOCXX_GNU_LINUX
	// This test is way too slow for the fast unit tests.
	if (!getenv("OWLONGTEST"))
	{
		return;
	}

	size_t sz = sizeof(SharedStuff_t); 
	pid_t own, pid = fork(); 
	unitAssert(pid != -1); 
	if (pid == 0)
	{
		// child
		cout << endl; 

		own = getpid();
		unitAssert(kill(own, SIGSTOP) == 0); 

		int shmid = shmget(IPCMUTEX_KEYVAL, sz, 0666); 
		unitAssert(shmid != -1); 
		SharedStuff_t* ssp; 
		ssp = (SharedStuff_t*) shmat(shmid, 0, 0); 
		IPCMutex sem(IPCMUTEX_KEYVAL); 
		unitAssert(ssp != (void*)-1); 
		{
			cout << "Child acquiring semaphore..." << endl;
			IPCMutexLock sl(sem); 
			cout << "Child acquired semaphore" << endl;
			unitAssert(ssp->num == 11); 
			unitAssert(strcmp(ssp->buf, parent1) == 0); 
			ssp->num = 21; 
			strcpy(ssp->buf, child1); 

			sleep(3); // give parent a chance to acquire
			          // before we reacquire in next block
			cout << "Child releasing semaphore" << endl;
		}
		{
			cout << "Child acquiring semaphore..." << endl;
			IPCMutexLock sl(sem); 
			cout << "Child acquired semaphore" << endl;
			unitAssert(ssp->num == 12); 
			unitAssert(strcmp(ssp->buf, parent2) == 0); 
			ssp->num = 22; 
			strcpy(ssp->buf, child2); 
			cout << "Child releasing semaphore" << endl;
		}
		_exit(0); 
	} else
	{
		// parent
		cout << "Parent waits for child " << endl;
		sleep(3);
		cout << "Parent continues now " << endl;

		SharedStuff_t* ssp; 
		unitAssert(sz == 256 + sizeof(int)); 
		int shmid = shmget(IPCMUTEX_KEYVAL, sz, 
						   IPC_CREAT | 0666); 
		unitAssert(shmid != -1); 
		ssp = (SharedStuff_t*) shmat(shmid, 0, 0); 
		IPCMutex sem(IPCMUTEX_KEYVAL); 
		unitAssert(ssp != (void*)-1); 
		{
			cout << "Parent acquiring semaphore..." << endl;
			IPCMutexLock sl(sem); 
			cout << "Parent acquired semaphore" << endl;

			unitAssert(kill(pid, SIGCONT) == 0); // wake up client

			ssp->num = 11; 
			strcpy(ssp->buf, parent1); 

			sleep(3); // give client a chance to acquire
			          // before we reacquire in next block
			cout << "Parent releasing semaphore" << endl;
		}
		{
			cout << "Parent acquiring semaphore..." << endl;
			IPCMutexLock sl(sem); 
			cout << "Parent acquired semaphore" << endl;
			unitAssert(ssp->num == 21); 
			unitAssert(strcmp(ssp->buf, child1) == 0); 
			ssp->num = 12; 
			strcpy(ssp->buf, parent2); 

			sleep(3); // give client a chance to acquire
			          // before we reacquire in next block
			cout << "Parent releasing semaphore" << endl;
		}
		{
			cout << "Parent acquiring semaphore..." << endl;
			IPCMutexLock sl(sem); 
			cout << "Parent acquired semaphore" << endl;
			unitAssert(ssp->num == 22); 
			unitAssert(strcmp(ssp->buf, child2) == 0); 
			cout << "Parent releasing semaphore" << endl;
		}
	}
#endif // #ifdef BLOCXX_GNU_LINUX
}

Test* IPCMutexTestCases::suite()
{
	TestSuite *testSuite = new TestSuite ("IPCMutex");

	ADD_TEST_TO_SUITE(IPCMutexTestCases, testSomething);

	return testSuite;
}

