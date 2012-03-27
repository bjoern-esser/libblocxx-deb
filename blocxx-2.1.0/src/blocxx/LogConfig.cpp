/*******************************************************************************
* Copyright (C) 2004 Novell, Inc. All rights reserved.
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
#include "blocxx/LogConfig.hpp"
#include "blocxx/String.hpp"
#include <cstring>

namespace BLOCXX_NAMESPACE
{

namespace LogConfigOptions
{

const char* const LOG_1_LOCATION_opt = "log.%1.location";
const char* const LOG_1_MAX_FILE_SIZE_opt = "log.%1.max_file_size";
const char* const LOG_1_MAX_BACKUP_INDEX_opt = "log.%1.max_backup_index";
const char* const LOG_1_FLUSH_opt = "log.%1.flush";
const char* const LOG_1_SYSLOG_IDENTITY_opt = "log.%1.identity";
const char* const LOG_1_SYSLOG_FACILITY_opt = "log.%1.facility";


const NameAndDefault g_defaults[] = 
{
	{ LOG_1_FLUSH_opt, BLOCXX_DEFAULT_LOG_1_FLUSH },
	{ LOG_1_LOCATION_opt,  "" },
	{ LOG_1_MAX_BACKUP_INDEX_opt, BLOCXX_DEFAULT_LOG_1_MAX_BACKUP_INDEX },
	{ LOG_1_MAX_FILE_SIZE_opt,  BLOCXX_DEFAULT_LOG_1_MAX_FILE_SIZE },
	{ LOG_1_SYSLOG_IDENTITY_opt, BLOCXX_DEFAULT_LOG_1_SYSLOG_IDENTITY },
	{ LOG_1_SYSLOG_FACILITY_opt, BLOCXX_DEFAULT_LOG_1_SYSLOG_FACILITY },
	{ "zz_end" , "garbage" }
}; 

const NameAndDefault* const g_defaultsEnd = &g_defaults[0] + 
	(sizeof(g_defaults)/sizeof(*g_defaults)) - 1; 

} // end of namespace LogConfigOptions


} // end namespace blocxx

