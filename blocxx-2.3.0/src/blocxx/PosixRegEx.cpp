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
 * @author Marius Tomaschewski
 */

#include "blocxx/BLOCXX_config.h"
#include "blocxx/PosixRegEx.hpp"
#ifdef BLOCXX_HAVE_REGEX
#ifdef BLOCXX_HAVE_REGEX_H

#include "blocxx/Exception.hpp"
#include "blocxx/ExceptionIds.hpp"
#include "blocxx/Format.hpp"


namespace BLOCXX_NAMESPACE
{

namespace
{
// the REG_NOERROR enum value from linux's regex.h is non-standard, so don't use it.
const int REG_NOERROR = 0;
}

// -------------------------------------------------------------------
static String
substitute_caps(const PosixRegEx::MatchArray &sub,
                const String &str, const String &rep)
{
	static const char *cap_refs[] = {
		NULL,  "\\1", "\\2", "\\3", "\\4",
		"\\5", "\\6", "\\7", "\\8", "\\9", NULL
	};

	String res( rep);
	size_t pos;

	for(size_t i=1; cap_refs[i] != NULL; i++)
	{
		String cap;

		if( i < sub.size() && sub[i].rm_so >= 0 && sub[i].rm_eo >= 0)
		{
			cap = str.substring(sub[i].rm_so, sub[i].rm_eo
			                                - sub[i].rm_so);
		}

		pos = res.indexOf(cap_refs[i]);
		while( pos != String::npos)
		{
			size_t quotes = 0;
			size_t at = pos;

			while( at > 0 && res.charAt(--at) == '\\')
				quotes++;

			if( quotes % 2)
			{
				quotes = (quotes + 1) / 2;

				res = res.erase(pos - quotes, quotes);

				pos = res.indexOf(cap_refs[i],
				                  pos + 2 - quotes);
			}
			else
			{
				quotes = quotes / 2;

				res = res.substring(0, pos - quotes) +
				      cap +
				      res.substring(pos + 2);

				pos = res.indexOf(cap_refs[i],
				                  pos + cap.length() - quotes);
			}
		}
	}
	return res;
}


// -------------------------------------------------------------------
static inline String
getError(const regex_t *preg, const int code)
{
	char err[256] = { '\0'};
	::regerror(code, preg, err, sizeof(err));
	return String(err);
}


// -------------------------------------------------------------------
PosixRegEx::PosixRegEx()
	: compiled(false)
	, m_flags(0)
	, m_ecode(REG_NOERROR)
{
}


// -------------------------------------------------------------------
PosixRegEx::PosixRegEx(const String &regex, int cflags)
	: compiled(false)
	, m_flags(0)
	, m_ecode(REG_NOERROR)
{
	if( !compile(regex, cflags))
	{
		BLOCXX_THROW_ERR(RegExCompileException,
			errorString().c_str(), m_ecode);
	}
}


// -------------------------------------------------------------------
PosixRegEx::PosixRegEx(const PosixRegEx &ref)
	: compiled(false)
	, m_flags(ref.m_flags)
	, m_ecode(REG_NOERROR)
	, m_rxstr(ref.m_rxstr)
{
	if( ref.compiled && !compile(ref.m_rxstr, ref.m_flags))
	{
		BLOCXX_THROW_ERR(RegExCompileException,
			errorString().c_str(), m_ecode);
	}
}


// -------------------------------------------------------------------
PosixRegEx::~PosixRegEx()
{
	if( compiled)
	{
		regfree(&m_regex);
	}
}


// -------------------------------------------------------------------
PosixRegEx &
PosixRegEx::operator = (const PosixRegEx &ref)
{
	if( !ref.compiled)
	{
		m_ecode = REG_NOERROR;
		m_error.erase();
		m_flags = ref.m_flags;
		m_rxstr = ref.m_rxstr;
		if( compiled)
		{
			regfree(&m_regex);
			compiled = false;
		}
	}
	else if( !compile(ref.m_rxstr, ref.m_flags))
	{
		BLOCXX_THROW_ERR(RegExCompileException,
			errorString().c_str(), m_ecode);
	}
	return *this;
}


// -------------------------------------------------------------------
bool
PosixRegEx::compile(const String &regex, int cflags)
{
	if( compiled)
	{
		regfree(&m_regex);
		compiled = false;
	}

	m_rxstr = regex;
	m_flags = cflags;
	m_ecode = ::regcomp(&m_regex, regex.c_str(), cflags);
	if( m_ecode == REG_NOERROR)
	{
		compiled = true;
		m_error.erase();
		return true;
	}
	else
	{
		m_error = getError(&m_regex, m_ecode);
		return false;
	}
}


// -------------------------------------------------------------------
int
PosixRegEx::errorCode()
{
	return m_ecode;
}


// -------------------------------------------------------------------
String
PosixRegEx::errorString() const
{
	return m_error;
}


// -------------------------------------------------------------------
String
PosixRegEx::patternString() const
{
	return m_rxstr;
}


// -------------------------------------------------------------------
int
PosixRegEx::compileFlags() const
{
	return m_flags;
}


// -------------------------------------------------------------------
bool
PosixRegEx::isCompiled() const
{
	return compiled;
}


// -------------------------------------------------------------------
bool
PosixRegEx::execute(MatchArray &sub, const String &str,
               size_t index, size_t count, int eflags)
{
	if( !compiled)
	{
		BLOCXX_THROW(RegExCompileException,
			"Regular expression is not compiled");
	}

	if( index > str.length())
	{
		BLOCXX_THROW(OutOfBoundsException,
			Format("String index out of bounds ("
			       "length = %1, index = %2).",
			       str.length(), index
			).c_str());
	}

	if( count == 0)
	{
		count = m_regex.re_nsub + 1;
	}
	AutoPtrVec<regmatch_t> rsub(new regmatch_t[count]);
	rsub[0].rm_so = -1;
	rsub[0].rm_eo = -1;

	sub.clear();
	m_ecode = ::regexec(&m_regex, str.c_str() + index,
	                    count, rsub.get(), eflags);
	if( m_ecode == REG_NOERROR)
	{
		m_error.erase();
		if( m_flags & REG_NOSUB)
		{
			return true;
		}

		sub.resize(count);
		for(size_t n = 0; n < count; n++)
		{
			if( rsub[n].rm_so < 0 || rsub[n].rm_eo < 0)
			{
				sub[n] = rsub[n];
			}
			else
			{
				rsub[n].rm_so += index;
				rsub[n].rm_eo += index;
				sub[n] = rsub[n];
			}
		}
		return true;
	}
	else
	{
		m_error = getError(&m_regex, m_ecode);
		return false;
	}
}


// -------------------------------------------------------------------
StringArray
PosixRegEx::capture(const String &str, size_t index, size_t count, int eflags)
{
	if( !compiled)
	{
		BLOCXX_THROW(RegExCompileException,
			"Regular expression is not compiled");
	}

	MatchArray  rsub;
	StringArray ssub;

	bool match = execute(rsub, str, index, count, eflags);
	if( match)
	{
		if( rsub.empty())
		{
			BLOCXX_THROW(RegExCompileException,
				"Non-capturing regular expression");
		}

		MatchArray::const_iterator i=rsub.begin();
		for( ; i != rsub.end(); ++i)
		{
			if( i->rm_so >= 0 && i->rm_eo >= 0)
			{
				ssub.push_back(str.substring(i->rm_so,
			                       i->rm_eo - i->rm_so));
			}
			else
			{
				ssub.push_back(String(""));
			}
		}
	}
	else if(m_ecode != REG_NOMATCH)
	{
		BLOCXX_THROW_ERR(RegExExecuteException,
			errorString().c_str(), m_ecode);
	}
	return ssub;
}


// -------------------------------------------------------------------
blocxx::String
PosixRegEx::replace(const String &str, const String &rep,
                    bool global, int eflags)
{
	if( !compiled)
	{
		BLOCXX_THROW(RegExCompileException,
			"Regular expression is not compiled");
	}

	MatchArray  rsub;
	bool        match;
	size_t      off = 0;
	String      out = str;

	do
	{
		match = execute(rsub, out, off, 0, eflags);
		if( match)
		{
			if( rsub.empty()      ||
			    rsub[0].rm_so < 0 ||
			    rsub[0].rm_eo < 0)
			{
				// only if empty (missused as guard).
				BLOCXX_THROW(RegExCompileException,
					"Non-capturing regular expression");
			}

			String res = substitute_caps(rsub, out, rep);

			out = out.substring(0, rsub[0].rm_so) +
			      res + out.substring(rsub[0].rm_eo);

			off = rsub[0].rm_so + res.length();
		}
		else if(m_ecode == REG_NOMATCH)
		{
			m_ecode = REG_NOERROR;
			m_error.erase();
		}
		else
		{
			BLOCXX_THROW_ERR(RegExExecuteException,
				errorString().c_str(), m_ecode);
		}
	} while(global && match && out.length() > off);

	return out;
}

// -------------------------------------------------------------------
StringArray
PosixRegEx::split(const String &str, bool empty, int eflags)
{
	if( !compiled)
	{
		BLOCXX_THROW(RegExCompileException,
			"Regular expression is not compiled");
	}

	MatchArray  rsub;
	StringArray ssub;
	bool        match;
	size_t      off = 0;
	size_t      len = str.length();

	do
	{
		match = execute(rsub, str, off, 1, eflags);
		if( match)
		{
			if( rsub.empty()      ||
			    rsub[0].rm_so < 0 ||
			    rsub[0].rm_eo < 0)
			{
				BLOCXX_THROW(RegExCompileException,
					"Non-capturing regular expression");
			}

			if( empty || ((size_t)rsub[0].rm_so > off))
			{
				ssub.push_back(str.substring(off,
				                   rsub[0].rm_so - off));
			}
			off = rsub[0].rm_eo;
		}
		else if(m_ecode == REG_NOMATCH)
		{
			String tmp = str.substring(off);
			if( empty || !tmp.empty())
			{
				ssub.push_back(tmp);
			}
			m_ecode = REG_NOERROR;
			m_error.erase();
		}
		else
		{
			BLOCXX_THROW_ERR(RegExExecuteException,
				errorString().c_str(), m_ecode);
		}
	} while(match && len > off);

	return ssub;
}


// -------------------------------------------------------------------
StringArray
PosixRegEx::grep(const StringArray &src, int eflags)
{
	if( !compiled)
	{
		BLOCXX_THROW(RegExCompileException,
			"Regular expression is not compiled");
	}

	m_ecode = REG_NOERROR;
	m_error.erase();

	StringArray out;
	if( !src.empty())
	{
		StringArray::const_iterator i=src.begin();
		for( ; i != src.end(); ++i)
		{
			int ret = ::regexec(&m_regex, i->c_str(),
			                    0, NULL, eflags);
			if( ret == REG_NOERROR)
			{
				out.push_back(*i);
			}
			else if(ret != REG_NOMATCH)
			{
				m_ecode = ret;
				m_error = getError(&m_regex, m_ecode);
				BLOCXX_THROW_ERR(RegExExecuteException,
					errorString().c_str(), m_ecode);
			}
		}
	}

	return out;
}


// -------------------------------------------------------------------
bool
PosixRegEx::match(const String &str, size_t index, int eflags) const
{
	if( !compiled)
	{
		BLOCXX_THROW(RegExCompileException,
			"Regular expression is not compiled");
	}

	if( index > str.length())
	{
		BLOCXX_THROW(OutOfBoundsException,
			Format("String index out of bounds ("
			       "length = %1, index = %2).",
			       str.length(), index
			).c_str());
	}

	m_ecode = ::regexec(&m_regex, str.c_str() + index,
	                    0, NULL, eflags);

	if( m_ecode == REG_NOERROR)
	{
		m_error.erase();
		return true;
	}
	else if(m_ecode == REG_NOMATCH)
	{
		m_error = getError(&m_regex, m_ecode);
		return false;
	}
	else
	{
		m_error = getError(&m_regex, m_ecode);
		BLOCXX_THROW_ERR(RegExExecuteException,
			errorString().c_str(), m_ecode);
	}
}


// -------------------------------------------------------------------
} // namespace BLOCXX_NAMESPACE

#endif // BLOCXX_HAVE_REGEX_H
#endif // BLOCXX_HAVE_REGEX

/* vim: set ts=8 sts=8 sw=8 ai noet: */

