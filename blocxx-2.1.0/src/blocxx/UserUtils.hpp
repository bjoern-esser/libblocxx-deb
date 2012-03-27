/*******************************************************************************
* Copyright (C) 2005, Vintela, Inc. All rights reserved.
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
*       Vintela, Inc., 
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

#ifndef BLOCXX_USER_UTILS_HPP_INCLUDE_GUARD
#define BLOCXX_USER_UTILS_HPP_INCLUDE_GUARD

#include "blocxx/BLOCXX_config.h"
#include "blocxx/String.hpp"

#ifdef BLOCXX_WIN32
BLOCXX_COMMON_API BLOCXX_NAMESPACE::UserId geteuid(void);
#endif

namespace BLOCXX_NAMESPACE
{
    /// Facade encapsulating OS specific user functionality.
	namespace UserUtils
	{
#ifdef BLOCXX_WIN32
		typedef PSID UserID;
#else
		typedef uid_t UserID;
		const UserID INVALID_USERID = UserID(~0);
#endif


		/**
		 * Get the effective user id.  On POSIX platforms this calls geteuid().
		 */
		BLOCXX_COMMON_API String getEffectiveUserId();
		BLOCXX_COMMON_API String getCurrentUserName();
		/**
		 * If the username is invalid, or if getUserName() fails for any other
		 * reason, 'success' will be set to false. On success, 'success' is
		 * set to true.  
		 */
		BLOCXX_COMMON_API String getUserName(UserID uid, bool& success);

		/**
		 * Convert a textual username into a platform native user type.
		 * @param userName The user name to convert.
		 * @param validUserName Out param set to true if the conversion was successful, false otherwise.
		 * @return The user id corresponding to userName.
		 */
		BLOCXX_COMMON_API UserID getUserId(const String& userName, bool& validUserName);

	} // end namespace UserUtils
} // end namespace BLOCXX_NAMESPACE

#endif


