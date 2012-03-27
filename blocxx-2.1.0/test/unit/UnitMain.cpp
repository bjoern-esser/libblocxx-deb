/*******************************************************************************
* Copyright (C) 2005, Quest Software, Inc. All rights reserved.
* Copyright (C) 2006, Novell, Inc. All rights reserved.
* 
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
* 
*     * Redistributions of source code must retain the above copyright notice,
*       this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of 
*       Quest Software, Inc., 
*       nor Novell, Inc., 
*       nor the names of its contributors or employees may be used to 
*       endorse or promote products derived from this software without 
*       specific prior written permission.
* 
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/


/**
 * @author Dan Nuffer
 */


#include "blocxx/BLOCXX_config.h"
#include "blocxx/LogAppender.hpp"
#include "blocxx/CerrAppender.hpp"

#if defined(BLOCXX_DEBUG_MEMORY) && !defined(BLOCXX_WIN32)
#include "MemTracerTestCases.hpp"
#endif

// test cases includes -- DO NOT EDIT THIS COMMENT
#include "IstreamBufIteratorTestCases.hpp"
#include "PathSecurityTestCases.hpp"
#include "AtomicOpsTestCases.hpp"
#include "LogAppenderTestCases.hpp"
#include "DelayedFormatTestCases.hpp"
#include "ScopeLoggerTestCases.hpp"
#include "GlobalPtrTestCases.hpp"
#include "IStringStreamTestCases.hpp"
#include "UnnamedPipeTestCases.hpp"
#include "ProcessTestCases.hpp"
#include "ConditionTestCases.hpp"
#include "TimeoutTimerTestCases.hpp"
#include "ThreadOnceTestCases.hpp"
#include "CryptographicRandomNumberTestCases.hpp"
#include "COWIntrusiveReferenceTestCases.hpp"
#include "CmdLineParserTestCases.hpp"
#include "DateTimeTestCases.hpp"
#include "UTF8UtilsTestCases.hpp"
#include "RandomNumberTestCases.hpp"
#include "UUIDTestCases.hpp"
#include "ThreadPoolTestCases.hpp"
#include "ThreadTestCases.hpp"
#include "ThreadBarrierTestCases.hpp"
#include "ReferenceTestCases.hpp"
#include "SocketUtilsTestCases.hpp"
#include "RWLockerTestCases.hpp"
#include "IPCMutexTestCases.hpp"
#include "FileSystemTestCases.hpp"
#include "ExceptionTestCases.hpp"
#include "StringStreamTestCases.hpp"
#include "MutexTestCases.hpp"
#include "InetAddressTestCases.hpp"
#include "MD5TestCases.hpp"
#include "EnumerationTestCases.hpp"
#include "StackTraceTestCases.hpp"
#include "ExecTestCases.hpp"
#include "LoggerTestCases.hpp"
#include "StringTestCases.hpp"
#include "FormatTestCases.hpp"
#include "LazyGlobalTestCases.hpp"

// includes for this file
#include "TestRunner.hpp"
#include "blocxx/String.hpp"

int main( int argc, char *argv[])
{
	TestRunner runner;

	// add tests to runner -- DO NOT EDIT THIS COMMENT
	runner.addTest( "IstreamBufIterator", IstreamBufIteratorTestCases::suite());
	runner.addTest( "PathSecurity", PathSecurityTestCases::suite());
	runner.addTest( "AtomicOps", AtomicOpsTestCases::suite());

	runner.addTest( "IPCMutex", IPCMutexTestCases::suite());
	runner.addTest( "LogAppender", LogAppenderTestCases::suite());
	runner.addTest( "DelayedFormat", DelayedFormatTestCases::suite());
	runner.addTest( "ScopeLogger", ScopeLoggerTestCases::suite());
	runner.addTest( "GlobalPtr", GlobalPtrTestCases::suite());
	runner.addTest( "IStringStream", IStringStreamTestCases::suite());
	runner.addTest( "UnnamedPipe", UnnamedPipeTestCases::suite());
	runner.addTest( "Process", ProcessTestCases::suite());
	runner.addTest( "Condition", ConditionTestCases::suite());
	runner.addTest( "TimeoutTimer", TimeoutTimerTestCases::suite());
	runner.addTest( "ThreadOnce", ThreadOnceTestCases::suite());
	runner.addTest( "CryptographicRandomNumber", CryptographicRandomNumberTestCases::suite());
	runner.addTest( "COWIntrusiveReference", COWIntrusiveReferenceTestCases::suite());
	runner.addTest( "CmdLineParser", CmdLineParserTestCases::suite());
	runner.addTest( "DateTime", DateTimeTestCases::suite());
	runner.addTest( "UTF8Utils", UTF8UtilsTestCases::suite());
	runner.addTest( "RandomNumber", RandomNumberTestCases::suite());
	runner.addTest( "UUID", UUIDTestCases::suite());
	runner.addTest( "ThreadPool", ThreadPoolTestCases::suite());
	runner.addTest( "Thread", ThreadTestCases::suite());
	runner.addTest( "ThreadBarrier", ThreadBarrierTestCases::suite());
	runner.addTest( "Reference", ReferenceTestCases::suite());
	runner.addTest( "SocketUtils", SocketUtilsTestCases::suite());
	runner.addTest( "RWLocker", RWLockerTestCases::suite());
	runner.addTest( "FileSystem", FileSystemTestCases::suite());
	runner.addTest( "Exception", ExceptionTestCases::suite());
	runner.addTest( "StringStream", StringStreamTestCases::suite());
#if defined(BLOCXX_DEBUG_MEMORY) && !defined(BLOCXX_WIN32)
	runner.addTest( "MemTracer", MemTracerTestCases::suite());
#endif
	runner.addTest( "Mutex", MutexTestCases::suite());
	runner.addTest( "SocketAddress", InetAddressTestCases::suite());
	//runner.addTest( "Base64", Base64TestCases::suite());
	runner.addTest( "MD5", MD5TestCases::suite());
	runner.addTest( "Enumeration", EnumerationTestCases::suite());
#ifdef BLOCXX_ENABLE_STACK_TRACE_ON_EXCEPTIONS
	runner.addTest( "StackTrace", StackTraceTestCases::suite());
#endif

	runner.addTest("Exec", ExecTestCases::suite());


	runner.addTest( "Logger", LoggerTestCases::suite());
	runner.addTest( "String", StringTestCases::suite());
	runner.addTest( "Format", FormatTestCases::suite());
	runner.addTest( "LazyGlobal", LazyGlobalTestCases::suite());

	if ( argc < 2 || ( argc == 2 && blocxx::String("all") == argv[1] ) )
	{
		runner.runAll();
	}
	else
	{
		if (argc == 3)
		{
			blocxx::LogAppender::setDefaultLogAppender(new blocxx::CerrAppender(
				blocxx::LogAppender::ALL_COMPONENTS,
				blocxx::LogAppender::ALL_CATEGORIES,
				blocxx::LogAppender::STR_TTCC_MESSAGE_FORMAT));
		}
		runner.run( argv[1] );
	}
	return 0;
}


