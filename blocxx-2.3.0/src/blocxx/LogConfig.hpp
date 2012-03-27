/*******************************************************************************
* Copyright (C) 2004 Quest Software, Inc. All rights reserved.
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
*  - Neither the name of Quest Software, Inc., Novell, Inc., nor the names of its
*    contributors may be used to endorse or promote products derived from this
*    software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL Quest Software, Inc., Novell, Inc., OR THE
* CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
* OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/


/**
 * @author Dan Nuffer
 */

#ifndef BLOCXX_LOG_CONFIG_HPP_INCLUDE_GUARD_
#define BLOCXX_LOG_CONFIG_HPP_INCLUDE_GUARD_

#include "blocxx/BLOCXX_config.h"
#include "blocxx/SortedVectorMap.hpp"

namespace BLOCXX_NAMESPACE
{

enum EOverwritePreviousConfigItemFlag
{
	E_PRESERVE_PREVIOUS_CONFIG_ITEM,
	E_OVERWRITE_PREVIOUS_CONFIG_ITEM
};

typedef SortedVectorMap<String, String> LoggerConfigMap;


#ifndef BLOCXX_DEFAULT_LOG_1_MAX_FILE_SIZE
#define BLOCXX_DEFAULT_LOG_1_MAX_FILE_SIZE "0"
#endif

#ifndef BLOCXX_DEFAULT_LOG_1_MAX_BACKUP_INDEX
#define BLOCXX_DEFAULT_LOG_1_MAX_BACKUP_INDEX "1"
#endif

#ifndef BLOCXX_DEFAULT_LOG_1_FLUSH
#define BLOCXX_DEFAULT_LOG_1_FLUSH "true"
#endif

#ifndef BLOCXX_DEFAULT_LOG_1_SYSLOG_IDENTITY
#define BLOCXX_DEFAULT_LOG_1_SYSLOG_IDENTITY BLOCXX_PACKAGE_PREFIX"blocxx"
#endif

#ifndef BLOCXX_DEFAULT_LOG_1_SYSLOG_FACILITY
#define BLOCXX_DEFAULT_LOG_1_SYSLOG_FACILITY "user"
#endif

namespace LogConfigOptions
{
	extern const char* const LOG_1_LOCATION_opt;
	extern const char* const LOG_1_MAX_FILE_SIZE_opt;
	extern const char* const LOG_1_MAX_BACKUP_INDEX_opt;
	extern const char* const LOG_1_FLUSH_opt;
	extern const char* const LOG_1_SYSLOG_IDENTITY_opt;
	extern const char* const LOG_1_SYSLOG_FACILITY_opt;

	struct NameAndDefault
	{
		const char* name;
		const char* defaultValue;
	};
	extern const NameAndDefault g_defaults[];
	extern const NameAndDefault* const g_defaultsEnd;

} // end namespace LogConfigOptions


} // end namespace BLOCXX_NAMESPACE

#endif // BLOCXX_LOGCONFIG_HPP_
