/*******************************************************************************
* Copyright (C) 2009, Quest Software, Inc. All rights reserved.
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
 * @author Richard Holden
 */

#include "blocxx/BLOCXX_config.h"
#include "UserUtilsTestCases.hpp"

#define PROVIDE_AUTO_TEST_MAIN
#include "AutoTest.hpp"
AUTO_UNIT_TEST_SUITE_NAMED(UserUtilsTestCases, "UserUtils");
#include "TestSuite.hpp"
#include "TestCaller.hpp"



#ifdef BLOCXX_HAVE_PWD_H
#include <pwd.h>
#endif

#ifdef BLOCXX_HAVE_GRP_H
#include <grp.h>
#endif

using namespace blocxx;

//Stuff up to 5 users and groups into a map for use in the tests
void UserUtilsTestCases::setUp()
{
#ifdef BLOCXX_WIN32
#pragma message ("Not implemented")
#else
	const int pws_to_get = 5;
	const int grs_to_get = 5;

	::setpwent();

	for (int i = 0; i < pws_to_get; i++)
	{
		passwd* result = ::getpwent();

		//Break out if there are no more
		if (!result)
			break;
		if( result->pw_uid != UserUtils::INVALID_USERID )
		{
			users.insert(std::make_pair(result->pw_uid, result->pw_name));
		}
	}
	::endpwent();

	::setgrent();

	for (int i = 0; i < grs_to_get; i++)
	{
		group* result = ::getgrent();

		//Break out if there are no more
		if (!result)
			break;
		if( result->gr_gid != UserUtils::INVALID_GROUPID )
		{
			groups.insert(std::make_pair(result->gr_gid, result->gr_name));
		}
	}
	::endgrent();
#endif
}

void UserUtilsTestCases::tearDown()
{
}

void UserUtilsTestCases::testGetEffectiveUserId()
{
	// There's not really a way to test this since we don't know the right answer
	// so we'll just call it, and make sure that it's not an empty string
	// If we passed a known euid or uid in, it could be tested
	unitAssert(!UserUtils::getEffectiveUserId().empty());
}

void UserUtilsTestCases::testGetEffectiveGroupId()
{
	// There's not really a way to test this since we don't know the right answer
	// so we'll just call it, and make sure that it's not an empty string
	// If we passed a known euid or uid in, it could be tested
#ifdef BLOCXX_WIN32
#pragma message ("Not implemented")
#else
	unitAssert(!UserUtils::getEffectiveGroupId().empty());
#endif
}

void UserUtilsTestCases::testGetCurrentUserName()
{
	// There's not really a way to test this since we don't know the right answer
	// so we'll just call it, and make sure that it's not an empty string
	// If we passed a known euid or uid in, it could be tested
	unitAssert(!UserUtils::getCurrentUserName().empty());
}

void UserUtilsTestCases::testGetCurrentGroupName()
{
	// There's not really a way to test this since we don't know the right answer
	// so we'll just call it, and make sure that it's not an empty string
	// If we passed a known euid or uid in, it could be tested
#ifdef BLOCXX_WIN32
#pragma message ("Not implemented")
#else
	unitAssert(!UserUtils::getCurrentGroupName().empty());
#endif
}

void UserUtilsTestCases::testGetUserName()
{
	bool success;
	for (UserMap::iterator iter(users.begin()), iterend(users.end()); iter != iterend; iter++)
	{
		// Note: this code could fail on systems where there isn't a 1-1 mapping of names to uids
		unitAssertEquals(UserUtils::getUserName(iter->first, success), iter->second);
		unitAssert(success);
	}
#ifdef BLOCXX_WIN32
#pragma message ("Not implemented")
#else
	UserUtils::getGroupName(UserUtils::INVALID_USERID, success);
	unitAssert(!success);
#endif
}

void UserUtilsTestCases::testGetGroupName()
{
	bool success;
	for (GroupMap::iterator iter(groups.begin()), iterend(groups.end()); iter != iterend; iter++)
	{
#ifdef BLOCXX_WIN32
#pragma message ("Not implemented")
#else
		// Note: this code could fail on systems where there isn't a 1-1 mapping of group names to gids
		unitAssertEquals(UserUtils::getGroupName(iter->first, success), iter->second);
		unitAssert(success);
#endif
	}

#ifdef BLOCXX_WIN32
#pragma message ("Not implemented")
#else
	UserUtils::getGroupName(UserUtils::INVALID_GROUPID, success);
	unitAssert(!success);
#endif
}

void UserUtilsTestCases::testGetUserId()
{
	bool success;
	for (UserMap::iterator iter(users.begin()), iterend(users.end()); iter != iterend; iter++)
	{
		// Note: this code could fail on systems where there isn't a 1-1 mapping of names to uids
		unitAssertEquals(UserUtils::getUserId(iter->second, success), iter->first);
		unitAssert(success);
	}
}

void UserUtilsTestCases::testGetGroupId()
{
	bool success;
	for (GroupMap::iterator iter(groups.begin()), iterend(groups.end()); iter != iterend; iter++)
	{
#ifdef BLOCXX_WIN32
#pragma message ("Not implemented")
#else
		// Note: this code could fail on systems where there isn't a 1-1 mapping of group names to gids
		unitAssertEquals(UserUtils::getGroupId(iter->second, success), iter->first);
		unitAssert(success);
#endif
	}
}

Test* UserUtilsTestCases::suite()
{
	TestSuite *testSuite = new TestSuite ("UserUtils");

	ADD_TEST_TO_SUITE(UserUtilsTestCases, testGetEffectiveUserId);
	ADD_TEST_TO_SUITE(UserUtilsTestCases, testGetEffectiveGroupId);
	ADD_TEST_TO_SUITE(UserUtilsTestCases, testGetCurrentUserName);
	ADD_TEST_TO_SUITE(UserUtilsTestCases, testGetCurrentGroupName);
	ADD_TEST_TO_SUITE(UserUtilsTestCases, testGetUserName);
	ADD_TEST_TO_SUITE(UserUtilsTestCases, testGetGroupName);
	ADD_TEST_TO_SUITE(UserUtilsTestCases, testGetUserId);
	ADD_TEST_TO_SUITE(UserUtilsTestCases, testGetGroupId);

	return testSuite;
}

