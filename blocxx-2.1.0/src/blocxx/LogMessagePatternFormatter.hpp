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

#ifndef BLOCXX_LOG_MESSAGE_PATTERN_FORMATTER_IFC_HPP_INCLUDE_GUARD_
#define BLOCXX_LOG_MESSAGE_PATTERN_FORMATTER_IFC_HPP_INCLUDE_GUARD_
#include "blocxx/BLOCXX_config.h"
#include "blocxx/CommonFwd.hpp"
#include "blocxx/Array.hpp"
#include "blocxx/Exception.hpp"
#include "blocxx/GlobalString.hpp"

namespace BLOCXX_NAMESPACE
{

BLOCXX_DECLARE_APIEXCEPTION(LogMessagePatternFormatter, BLOCXX_COMMON_API);

class BLOCXX_COMMON_API LogMessagePatternFormatter
{
public:

	static const GlobalString STR_DEFAULT_MESSAGE_PATTERN;

	enum EErrorCodes
	{
		E_INVALID_PATTERN_NO_DIGIT_AFTER_DOT,
		E_INVALID_PATTERN_PRECISION_NOT_AN_INTEGER,
		E_INVALID_PATTERN_UNSUPPORTED_CONVERSION
	};

	/**
	 * @throws LogMessagePatternFormatterException if the pattern is invalid.
	 */
	LogMessagePatternFormatter(const String& pattern);
	~LogMessagePatternFormatter();

	void formatMessage(const LogMessage& message, StringBuffer& output) const;

public: // implementation details
	class Converter;
	typedef IntrusiveReference<Converter> ConverterRef;

private:

#ifdef BLOCXX_WIN32
#pragma warning (push)
#pragma warning (disable: 4251)
#endif

	Array<ConverterRef> m_patternConverters;

#ifdef BLOCXX_WIN32
#pragma warning (pop)
#endif

	// non-copyable
	LogMessagePatternFormatter(const LogMessagePatternFormatter&);
	LogMessagePatternFormatter& operator=(const LogMessagePatternFormatter&);
};

} // end namespace BLOCXX_NAMESPACE

#endif



