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

#include "blocxx/BLOCXX_config.h"
#include "blocxx/Format.hpp"

namespace BLOCXX_NAMESPACE
{

/////////////////////////////////////////////////////////////////////////////
Format::operator String() const
{
	return oss.toString();
}
/////////////////////////////////////////////////////////////////////////////
String Format::toString() const
{
	return oss.toString();
}
/////////////////////////////////////////////////////////////////////////////
const char* Format::c_str() const
{
	return oss.c_str();
}
/////////////////////////////////////////////////////////////////////////////
char Format::process(String& str, char numArgs)
{
	int len(str.length());
	char c(' ');
	bool err = false;
	int i = 0;
	while (i < len && c == ' ' && !err)
	{
		switch (str[i])
		{
			case '%':
				if (i + 1 < len)
				{
					++i;
					switch (str[i])
					{
						case '1': case '2': case '3': case '4': case '5': 
						case '6': case '7': case '8': case '9':
							c = str[i]; 
							break;
						case '%': 
							oss << str[i]; 
							break;
						default: 
							err = true;
					} // inner switch
				} else err = true; 
				break;
			default: 
				oss << str[i];
				break;
		} // outer switch
		++i;
	} // for
	if ( i <= len && c > numArgs )
	{
		oss << "\n*** Parameter specifier too large.";
		err = true;
	}
	if (err)
	{
		oss << "\n*** Error in format string at \"" << str.substring(i-1) << "\".\n";
		str.erase();
		return '0';
	}
	str.erase(0, i);
	return c;
} // process
/////////////////////////////////////////////////////////////////////////////
std::ostream&
operator<<(std::ostream& os, const Format& f)
{
	os.write(f.oss.c_str(), f.oss.length());
	return os;
}
/////////////////////////////////////////////////////////////////////////////
void Format::put(const String& t)
{ // t is inserted into oss
	if (!oss.good())
	{
		return;
	}
	oss << t;
}
/////////////////////////////////////////////////////////////////////////////
#define BLOCXX_DEFINE_PUT(type) \
void Format::put(type t) \
{ \
\
	if (!oss.good()) \
	{ \
		return; \
	} \
\
	oss << t; \
}
BLOCXX_DEFINE_PUT(char);
BLOCXX_DEFINE_PUT(unsigned char);
BLOCXX_DEFINE_PUT(short);
BLOCXX_DEFINE_PUT(unsigned short);
BLOCXX_DEFINE_PUT(int);
BLOCXX_DEFINE_PUT(unsigned int);
BLOCXX_DEFINE_PUT(long);
BLOCXX_DEFINE_PUT(unsigned long);
BLOCXX_DEFINE_PUT(long long);
BLOCXX_DEFINE_PUT(unsigned long long);
#undef BLOCXX_DEFINE_PUT

Format::Format(const char* ca, const String& a) : oss()
{
	String fmt(ca);
	while (!fmt.empty())
	{
		switch (process(fmt, '1'))
		{
			case '1': put(a); break;
		}
	}
}
Format::Format(const char* ca, const String& a, const String& b) : oss()
{
	String fmt(ca);
	while (!fmt.empty())
	{
		switch (process(fmt, '2'))
		{
			case '1': put(a); break;
			case '2': put(b); break;
		}
	}
}
Format::Format(const char* ca, const String& a, const String& b, const String& c) : oss()
{
	String fmt(ca);
	while (!fmt.empty())
	{
		switch (process(fmt, '3'))
		{
			case '1': put(a); break;
			case '2': put(b); break;
			case '3': put(c); break;
		}
	}
}

} // end namespace BLOCXX_NAMESPACE

