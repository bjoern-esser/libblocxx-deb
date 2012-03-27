/*******************************************************************************
* Copyright (C) 2005, 2009, Quest Software, Inc. All rights reserved.
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
 * @author Joel Smith - posix group support
 */

#ifndef BLOCXX_USER_UTILS_HPP_INCLUDE_GUARD
#define BLOCXX_USER_UTILS_HPP_INCLUDE_GUARD

#include "blocxx/BLOCXX_config.h"
#include "blocxx/CommonFwd.hpp"
#include "blocxx/Types.hpp"

#ifdef BLOCXX_WIN32
BLOCXX_COMMON_API BLOCXX_NAMESPACE::UserId geteuid(void);
#endif

namespace BLOCXX_NAMESPACE
{
	/// Facade encapsulating OS specific user functionality.
	namespace UserUtils
	{
#if !defined( BLOCXX_WIN32 )
		const UserId INVALID_USERID = UserId(~0);
		const GroupId INVALID_GROUPID = GroupId(~0);
#endif


		/**
		 * Get the effective user id.  On POSIX platforms this calls geteuid().
		 *
		 * @return A string representation of the user id.  On POSIX
		 *   platforms, this can be converted to a UInt64.
		 */
		BLOCXX_COMMON_API String getEffectiveUserId();
		/**
		 * Get the effective group id.  On POSIX platforms this calls getegid().
		 *
		 * @return A string representation of the group id.  On POSIX
		 *   platforms, this can be converted to a UInt64.
		 */
		BLOCXX_COMMON_API String getEffectiveGroupId();
		/**
		 * Return the user name for the current user.
		 */
		BLOCXX_COMMON_API String getCurrentUserName();
		/**
		 * Return the group name for the current user.
		 */
		BLOCXX_COMMON_API String getCurrentGroupName();
		/**
		 * If the username is invalid, or if getUserName() fails for any other
		 * reason, 'success' will be set to false. On success, 'success' is
		 * set to true.
		 * @param uid The id to query for user name
		 * @return The user name corresponding to the requested uid, or
		 *   an empty string on failure.  Check success before relying
		 *   on the return result.
		 *
		 * \example utils.cpp
		 */
		BLOCXX_COMMON_API String getUserName(UserId uid, bool& success);
		/**
		 * If the group is invalid, or if getGropuName() fails for any other
		 * reason, 'success' will be set to false. On success, 'success' is
		 * set to true.
		 * @param gid The id to query for group name
		 * @return The group name corresponding to the requested gid, or
		 *   an empty string on failure.  Check success before relying
		 *   on the return result.
		 *
		 * \example utils.cpp
		 */
		BLOCXX_COMMON_API String getGroupName(GroupId gid, bool& success);

		/**
		 * Convert a textual username into a platform native user type.
		 * @param userName The user name to convert.
		 * @param validUserName Out param set to true if the conversion was successful, false otherwise.
		 * @return The user id corresponding to userName.
		 *
		 * \example utils.cpp
		 */
		BLOCXX_COMMON_API UserId getUserId(const String& userName, bool& validUserName);
		/**
		 * Convert a textual group name into a platform native group type.
		 * @param groupName The group name to convert.
		 * @param validGroupName Out param set to true if the conversion was successful, false otherwise.
		 * @return The group id corresponding to groupName.
		 *
		 * \example utils.cpp
		 */
		BLOCXX_COMMON_API UserId getGroupId(const String& groupName, bool& validGroupName);

	} // end namespace UserUtils
} // end namespace BLOCXX_NAMESPACE

#endif


