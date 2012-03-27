/*******************************************************************************
* Copyright (C) 2004, Quest Software, Inc. All rights reserved.
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
 */

#if       !defined(BLOCXX_TEST_LOG_UTILS_HPP_INCLUDE_GUARD_)
#define            BLOCXX_TEST_LOG_UTILS_HPP_INCLUDE_GUARD_

#include "blocxx/BLOCXX_config.h"
#include "blocxx/String.hpp"
#include "blocxx/Logger.hpp"
#include "blocxx/Format.hpp"

#define GENERIC_LOG_DEBUG3(logger, P, X) do { BLOCXX_LOG_DEBUG3(logger, ::blocxx::String(P) + (X)); } while(0)
#define GENERIC_LOG_DEBUG2(logger, P, X) do { BLOCXX_LOG_DEBUG2(logger, ::blocxx::String(P) + (X)); } while(0)
#define GENERIC_LOG_DEBUG(logger, P, X) do { BLOCXX_LOG_DEBUG(logger, ::blocxx::String(P) + (X)); } while(0)
#define GENERIC_LOG_INFO(logger, P, X) do { BLOCXX_LOG_INFO(logger, ::blocxx::String(P) + (X)); } while(0)
#define GENERIC_LOG_ERROR(logger, P, X) do { BLOCXX_LOG_ERROR(logger, ::blocxx::String(P) + (X)); } while(0)

// This macro looks nasty, but the constant for level should usually be known
// at compile time and the rest of the body should be discarded by the
// optimizer.  Note that the check for logger.getLogLevel() >= level is handled
// indirectly by the macros that this macro invokes.  This may cause a little
// code bloat in the cases where level is not known at compile time (eg. a
// parameter to a function), but this should be uncommon
#define GENERIC_LOG_LEVEL(logger, level, P, X) \
	do \
	{ \
		if( level >= ::blocxx::E_DEBUG3_LEVEL ) { GENERIC_LOG_DEBUG3(logger, P, X); } \
		else if( level >= ::blocxx::E_DEBUG2_LEVEL ) { GENERIC_LOG_DEBUG2(logger, P, X); } \
		else if( level >= ::blocxx::E_DEBUG_LEVEL ) { GENERIC_LOG_DEBUG(logger, P, X); } \
		else if( level >= ::blocxx::E_INFO_LEVEL ) { GENERIC_LOG_INFO(logger, P, X); } \
		else if( level >= ::blocxx::E_ERROR_LEVEL ) { GENERIC_LOG_ERROR(logger, P, X); } \
	} while(0)


#define STANDARD_LOG_DEBUG3(P, X) GENERIC_LOG_DEBUG3(logger, P, X)
#define STANDARD_LOG_DEBUG2(P, X) GENERIC_LOG_DEBUG2(logger, P, X)
#define STANDARD_LOG_DEBUG(P, X) GENERIC_LOG_DEBUG(logger, P, X)
#define STANDARD_LOG_INFO(P, X) GENERIC_LOG_INFO(logger, P, X)
#define STANDARD_LOG_ERROR(P, X) GENERIC_LOG_ERROR(logger, P, X)
#define STANDARD_LOG_LEVEL(level, P, X) GENERIC_LOG_LEVEL(logger, level, P, X)

namespace BLOCXX_NAMESPACE
{
	namespace LogUtils
	{
		/*
		 * Send the given garbage to the given logger, each line prefixed by the given prefix.
		 *
		 * @param garbage
		 *   The garbage which will be dumped to the log.
		 * @param prefix
		 *   The text to prepend to each line of the output.
		 * @param debug
		 *   If true, send output to the debug log, otherwise use the error log.
		 */
		template <class T>
		void dumpToLog(const blocxx::Array<T>& garbage, const blocxx::Logger& logger, const blocxx::String& prefix, ELogLevel level = E_DEBUG_LEVEL)
		{
			GENERIC_LOG_LEVEL(logger, level, prefix, Format(" [%2 elements]", garbage.size()));
			for(
				typename blocxx::Array<T>::const_iterator foo = garbage.begin();
				foo != garbage.end();
				++foo )
			{
				GENERIC_LOG_LEVEL(logger, level, prefix, *foo);
			}
		}
	} // end namespace LogUtils
} // end namespace BLOCXX_NAMESPACE

#endif /* !defined(BLOCXX_TEST_LOG_UTILS_HPP_INCLUDE_GUARD_) */
