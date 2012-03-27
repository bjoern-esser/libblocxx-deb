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
 * @author Kevin Harris
 * @author Anton Afanasiev - for Win
 */

#include "blocxx/BLOCXX_config.h"
#include "TestSuite.hpp"
#include "TestCaller.hpp"
#include "PathSecurityTestCases.hpp"
#include "blocxx/FileSystem.hpp"
#include "blocxx/RandomNumber.hpp"

using namespace blocxx;

void PathSecurityTestCases::setUp()
{
}

void PathSecurityTestCases::tearDown()
{
}

void PathSecurityTestCases::testSecurePaths()
{
	// These paths should be secure.  If they aren't, your install is probably broken.
#ifndef BLOCXX_WIN32
	const char* const 
#else
	char*
#endif
		secure_paths[] =
	{
#ifndef BLOCXX_WIN32
		"/",
		"/etc",
		"/usr/sbin"
#else
		new char[MAX_PATH + 1]
#endif
	};

#ifdef BLOCXX_WIN32
	// retrieve system environment variables
	secure_paths[0][0] = 0;
	strcat(secure_paths[0], "C:/blocxx_securitytest_");
	RandomNumber randomGenerator;
 	strcat(secure_paths[0], String(randomGenerator.getNextNumber()).c_str());
	/* by default it creates unsecured directory: */
	unitAssertFail( !FileSystem::makeDirectory( secure_paths[0], 774 )) ;

#endif

	for( size_t i = 0; i < sizeof(secure_paths) / sizeof(*secure_paths); ++i )
	{
		if( FileSystem::exists(secure_paths[i]) )
		{
			// Check the security of the file...
			std::pair<FileSystem::Path::ESecurity, String> results;
			// The directory exists, so this shouldn't throw.
			unitAssertNoThrow(results = FileSystem::Path::security(secure_paths[i]));
			unitAssertEquals(results.first, FileSystem::Path::E_SECURE_DIR);
		}
#ifdef BLOCXX_WIN32
		FileSystem::removeDirectory(secure_paths[i]);
		delete(secure_paths[i]);
#endif
	}
}

void PathSecurityTestCases::testInsecurePaths()
{
	// These paths should be secure.  If they aren't, your install is probably broken.
#ifndef BLOCXX_WIN32
	const char* const 
#else
	char*
#endif
		insecure_paths[] =
	{
#ifndef BLOCXX_WIN32
		"/tmp",
		"/var/tmp"
#else
		new char[MAX_PATH+1] 
#endif
	};

#ifdef BLOCXX_WIN32
	insecure_paths[0][0] = 0;
	strcat(insecure_paths[0], "C:/blocxx_securitytest_");
	RandomNumber randomGenerator;
 	strcat(insecure_paths[0], String(randomGenerator.getNextNumber()).c_str());
	/* by default it creates unsecured directory: */
	unitAssertFail( !FileSystem::makeDirectory( insecure_paths[0], 777 )) ;
#endif

	for( size_t i = 0; i < sizeof(insecure_paths) / sizeof(*insecure_paths); ++i )
	{
		if( FileSystem::exists(insecure_paths[i]) )
		{
			// Check the security of the file...
			std::pair<FileSystem::Path::ESecurity, String> results;
			// The directory exists, so this shouldn't throw.
			unitAssertNoThrow(results = FileSystem::Path::security(insecure_paths[i]));
			unitAssertEquals(results.first, FileSystem::Path::E_INSECURE);
		}
#ifdef BLOCXX_WIN32
		FileSystem::removeDirectory(insecure_paths[i]);
		delete(insecure_paths[i]);
#endif
	}
}

void PathSecurityTestCases::testInvalidPaths()
{
	// These paths should be secure.  If they aren't, your install is probably broken.
	const char* const invalid_paths[] =
	{
#ifndef BLOCXX_WIN32
		"/this/should/not/exist",
		"/if/this/is/here/someone/is/twisted"
#else
		"C:/this/should/not/exist"
#endif
	};

	for( size_t i = 0; i < sizeof(invalid_paths) / sizeof(*invalid_paths); ++i )
	{
		if( !FileSystem::exists(invalid_paths[i]) )
		{
			// The directory should NOT exist so this should throw.
			unitAssertThrowsSpecific(FileSystemException, FileSystem::Path::security(invalid_paths[i]));
		}
	}
}

Test* PathSecurityTestCases::suite()
{
	TestSuite *testSuite = new TestSuite ("PathSecurity");

	ADD_TEST_TO_SUITE(PathSecurityTestCases, testSecurePaths);
	ADD_TEST_TO_SUITE(PathSecurityTestCases, testInsecurePaths);
	ADD_TEST_TO_SUITE(PathSecurityTestCases, testInvalidPaths);

	return testSuite;
}

