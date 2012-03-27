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
 * @author Dan Nuffer
 * @author Kevin Harris
 */

#include "blocxx/BLOCXX_config.h"
#include "blocxx/Format.hpp"
#include "blocxx/ExceptionIds.hpp"
#include <limits>

namespace BLOCXX_NAMESPACE
{
BLOCXX_DEFINE_EXCEPTION_WITH_ID(Format);

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

Format::~Format()
{
}

Format::Format()
	: oss()
{
}

Format::Flags::Flags()
	: flags(0)
	, width(0)
	, precision(-1)
	, fill(' ')
	, savedprecision(0)
	, savedwidth(0)
	, savedfill(' ')
{
}

void Format::useFlags(std::ostream& o, Flags& flags)
{
	flags.savedprecision = o.precision();
	flags.savedwidth = o.width();
	flags.savedfill = o.fill();

	long filtered = flags.flags & ~(std::ios::hex|std::ios::oct);
	o << std::setiosflags(std::ios::fmtflags(filtered));

	// std::setiosflags seems to ignore ios::hex and ios::oct.  Use a separate
	// stream manipulator to set them.
	if( flags.flags & std::ios::hex )
	{
		o << std::hex;
	}
	if( flags.flags & std::ios::oct )
	{
		o << std::oct;
	}
	o << std::setw(flags.width);
	if( flags.precision >= 0 )
	{
		o << std::setprecision(flags.precision);
	}
	o << std::setfill(flags.fill);
}

void Format::clearFlags(std::ostream& o, Flags& flags)
{
	if( flags.flags & (std::ios::hex|std::ios::oct) )
	{
		o << std::dec;
	}
	o << std::resetiosflags(std::ios::fmtflags(flags.flags));
	o << std::setprecision(flags.savedprecision);
	o << std::setw(flags.savedwidth);
	o << std::setfill(flags.savedfill);


	flags.savedprecision = 0;
	flags.savedwidth = 0;
	flags.savedfill = ' ';
}

namespace
{
	bool scanNumber(const String& str, size_t& offset, int& result, int& sign, bool& leadingZero)
	{
		int c = 0;
		int len = str.length();
		int i = offset;
		bool havedigit = false;
		sign = 0;
		leadingZero = false;

		if( i < len )
		{
			if( str[i] == '-' )
			{
				sign = -1;
				++i;
			}
			else if( str[i] == '+' )
			{
				sign = 1;
				++i;
			}
			if( (i < len) && (str[i] == '0') )
			{
				leadingZero = true;
			}
		}

		while( i < len && isdigit(str[i]) )
		{
			havedigit = true;
			c = c * 10 + (str[i] - '0');
			++i;
		}

		if ( sign < 0 )
		{
			c *= -1;
		}

		if( !havedigit )
		{
			return false;
		}

		result = c;
		offset = i;

		return true;
	}

	const char BRACE_OPEN = '<';
	const char BRACE_CLOSE = '>';
}

// BRACE_OPEN [0-9]+ ( ':' ([0-9]* '.' [0-9]* )? ('x' | 'X' | 'o')? '!'? )? BRACE_CLOSE
bool Format::processFormatSpecifier(String& str, size_t& offset, size_t& argnum, Format::Flags& flags)
{
	bool retval = true;

	if( str[offset] == BRACE_OPEN )
	{
		++offset;

		// Get the argument number.
		int arg;
		int argsign;
		bool unused;
		if( scanNumber(str, offset, arg, argsign, unused) )
		{
			// Don't allow signs on the number.
			if( argsign != 0 )
			{
				retval = false;
			}

			argnum = arg;
		}
		else
		{
			retval = false;
		}

		// Get the format width/output specifiers.
		if( str[offset] == ':' )
		{
			++offset;

			int val;
			int sign;
			bool leadingZero;
			if( scanNumber(str, offset, val, sign, leadingZero) )
			{
				if( sign == -1 )
				{
					val *= -1;
					flags.flags |= std::ios::left;
				}
				else if( sign == 1 )
				{
					flags.flags |= std::ios::showpos;
				}

				if( leadingZero && sign != -1)
				{
					flags.fill = '0';
				}
				flags.width = val;
			}

			if( (offset < str.length()) && (str[offset] == '.') )
			{
				size_t j = offset + 1;
				if( scanNumber(str, j, val, sign, leadingZero) )
				{
					flags.precision = val;
					flags.flags |= std::ios::showpoint | std::ios::fixed;
					offset = j;
				}
			}

			if( offset < str.length() )
			{
				if( str[offset] == 'x' )
				{
					flags.flags |= std::ios::hex;
					++offset;
				}
				else if( str[offset] == 'X' )
				{
					flags.flags |= std::ios::hex | std::ios::uppercase;
					++offset;
				}
				else if( str[offset] == 'o' )
				{
					flags.flags |= std::ios::oct;
					++offset;
				}

				if( offset < str.length() && str[offset] == '!' )
				{
					flags.flags |= std::ios::showbase | std::ios::boolalpha;
					++offset;
				}
			}
		}

		if( (offset < str.length()) && (str[offset] == BRACE_CLOSE) )
		{
			++offset;
		}
		else
		{
			// No close brace.
			retval = false;
		}
	}

	return retval;
}

/////////////////////////////////////////////////////////////////////////////
size_t Format::process(String& str, size_t minArg, size_t maxArg, Format::Flags& flags, FormatErrorHandling errortype)
{
	size_t len(str.length());
	size_t c = static_cast<size_t>(-1);
	bool err = false;
	size_t i = 0;
	size_t percentStart = 0;
	for (; (i < len) && (c == size_t(-1)) && !err; ++i)
	{
		if( str[i] == '%' )
		{
			percentStart = i;
			if (i + 1 < len)
			{
				++i;

				if( str[i] == '%' )
				{
					oss << '%';
				}
				else if( str[i] == BRACE_OPEN )
				{
					if( !processFormatSpecifier(str, i, c, flags) )
					{
						// No close brace or some other error.
						err = true;
					}
					else
					{
						// Incremented next loop.
						--i;
					}
				}
				else if( isdigit(str[i]) )
				{
					c = str[i] - '0';
				}
				else
				{
					// Random junk after '%'
					err = true;
				}
			}
			else
			{
				err = true;
			}
		}
		else // anything other than a '%'
		{
			oss << str[i];
		}
	} // for
	if ( (i <= len) && (c != size_t(-1)))
	{
		if( c > maxArg )
		{
			OStringStream error;
			error << "Parameter specifier " << c << " is too large (>" << maxArg << ")";

			if( errortype == E_FORMAT_EXCEPTION )
			{
				BLOCXX_THROW(FormatException, error.c_str());
			}
			oss << "\n*** " << error.c_str();
			err = true;
		}
		else if( c < minArg )
		{
			OStringStream error;
			error << "Parameter specifier " << c << " must be >= " << minArg;
			if( errortype == E_FORMAT_EXCEPTION )
			{
				BLOCXX_THROW(FormatException, error.c_str());
			}
			oss << "\n*** " << error.c_str();

			err = true;
		}
	}
	if (err)
	{
		// Print the percent and all of the text causing the error.
		size_t textlength = std::max(i, percentStart + 1) - percentStart;

		OStringStream error;
		error << "Error in format string at \"" << str.substring(percentStart, textlength) << "\"";

		if( errortype == E_FORMAT_EXCEPTION )
		{
			BLOCXX_THROW(FormatException, error.c_str());
		}
		oss << "\n*** " << error.c_str() << "\n";
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
void Format::put(const String& t, Format::Flags& flags)
{ // t is inserted into oss
	if (!oss.good())
	{
		return;
	}
	useFlags(oss, flags);
	oss << t;
	clearFlags(oss, flags);
}
/////////////////////////////////////////////////////////////////////////////
#define BLOCXX_DEFINE_PUT(type) \
void Format::put(type t, Format::Flags& flags) \
{ \
\
	if (!oss.good()) \
	{ \
		return; \
	} \
\
	useFlags(oss, flags); \
	if (!std::numeric_limits<type>::is_specialized) \
	{ \
		oss << std::setfill(flags.savedfill); \
	} \
	oss << t; \
	clearFlags(oss, flags);	\
}

BLOCXX_DEFINE_PUT(const char*);
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
BLOCXX_DEFINE_PUT(float);
BLOCXX_DEFINE_PUT(double);
BLOCXX_DEFINE_PUT(long double);
#undef BLOCXX_DEFINE_PUT

Format::Format(const char* ca, const String& a) : oss()
{
	String fmt(ca);
	while (!fmt.empty())
	{
		Flags flags;
		switch( process(fmt, 1, 1, flags) )
		{
		case 1: put(a, flags); break;
		}
	}
}
Format::Format(const char* ca, const String& a, const String& b) : oss()
{
	String fmt(ca);
	while (!fmt.empty())
	{
		Flags flags;
		switch( process(fmt, 1, 2, flags) )
		{
		case 1: put(a, flags); break;
		case 2: put(b, flags); break;
		}
	}
}
Format::Format(const char* ca, const String& a, const String& b, const String& c) : oss()
{
	String fmt(ca);
	while (!fmt.empty())
	{
		Flags flags;
		switch( process(fmt, 1, 3, flags) )
		{
		case 1: put(a, flags); break;
		case 2: put(b, flags); break;
		case 3: put(c, flags); break;
		}
	}
}

} // end namespace BLOCXX_NAMESPACE

