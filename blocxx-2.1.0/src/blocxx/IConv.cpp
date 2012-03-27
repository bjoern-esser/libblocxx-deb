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
*  - Neither the name of Novell, Inc., nor the names of its
*    contributors may be used to endorse or promote products derived from this
*    software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL Novell, Inc., OR THE 
* CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
* OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/
/**
 * @author Marius Tomaschewski (mt@suse.de)
 */

#include "blocxx/IConv.hpp"

#if defined(BLOCXX_HAVE_ICONV_SUPPORT)
#include "blocxx/Assertion.hpp"
#include "blocxx/Format.hpp"
#include "blocxx/Exec.hpp"

#include <cwchar>
#include <cwctype>

#include <errno.h>

namespace BLOCXX_NAMESPACE
{

// -------------------------------------------------------------------
IConv_t::IConv_t()
	: m_iconv(iconv_t(-1))
{
}


// -------------------------------------------------------------------
IConv_t::IConv_t(const String &fromEncoding, const String &toEncoding)
{
	m_iconv = ::iconv_open(toEncoding.c_str(), fromEncoding.c_str());
	if( m_iconv == iconv_t(-1))
	{
		BLOCXX_THROW(StringConversionException,
		             Format("Unable to convert from \"%1\" to \"%2\"",
		                    fromEncoding, toEncoding).c_str());
	}
}


// -------------------------------------------------------------------
IConv_t::~IConv_t()
{
	close();
}


// -------------------------------------------------------------------
bool
IConv_t::open(const String &fromEncoding, const String &toEncoding)
{
	close();
	m_iconv = ::iconv_open(toEncoding.c_str(), fromEncoding.c_str());
	return ( m_iconv != iconv_t(-1));
}


// -------------------------------------------------------------------
size_t
IConv_t::convert(char **istr, size_t *ibytesleft,
               char **ostr, size_t *obytesleft)
{
#if defined(BLOCXX_ICONV_INBUF_CONST)
	BLOCXX_ASSERT(istr != NULL); 
	const char *ptr = *istr;
	int ret = ::iconv(m_iconv, &ptr, ibytesleft, ostr, obytesleft); 
	*istr = const_cast<char*>(ptr); 
	return ret; 
#else
	return ::iconv(m_iconv, istr, ibytesleft, ostr, obytesleft);
#endif
}


// -------------------------------------------------------------------
bool
IConv_t::close()
{
	bool ret = true;
	int  err = errno;

	if( m_iconv != iconv_t(-1))
	{
		if( ::iconv_close(m_iconv) == -1)
			ret = false;
		m_iconv = iconv_t(-1);
	}

	errno = err;
	return ret;
}


// *******************************************************************
namespace IConv
{

// -------------------------------------------------------------------
static inline void
mayThrowStringConversionException()
{
	switch( errno)
	{
		case E2BIG:
		break;

		case EILSEQ:
			BLOCXX_THROW(StringConversionException,
			"Invalid character or multibyte sequence in the input");
		break;

		case EINVAL:
		default:
			BLOCXX_THROW(StringConversionException,
			"Incomplete multibyte sequence in the input");
		break;
	}
}

// -------------------------------------------------------------------
String
fromByteString(const String &enc, const char *str, size_t len)
{
	if( !str || len == 0)
		return String();

	IConv_t      iconv(enc, "UTF-8"); // throws error
	String       out;
	char         obuf[4097];
	char        *optr;
	size_t       olen;

	char        *sptr = (char *)str;
	size_t       slen = len;

	while( slen > 0)
	{
		obuf[0] = '\0';
		optr = (char *)obuf;
		olen = sizeof(obuf) - sizeof(obuf[0]);

		size_t ret = iconv.convert(&sptr, &slen, &optr, &olen);
		if( ret == size_t(-1))
		{
			mayThrowStringConversionException();
		}
		*optr = '\0';
		out  += obuf;
	}

	return out;
}


// -------------------------------------------------------------------
String
fromByteString(const String &enc, const std::string  &str)
{
	return fromByteString(enc, str.c_str(), str.length());
}


#ifdef BLOCXX_HAVE_STD_WSTRING
// -------------------------------------------------------------------
String
fromWideString(const String &enc, const std::wstring &str)
{
	if( str.empty())
		return String();

	IConv_t      iconv(enc, "UTF-8"); // throws error
	String       out;
	char         obuf[4097];
	char        *optr;
	size_t       olen;

	char        *sptr = (char *)str.c_str();
	size_t       slen = str.length() * sizeof(wchar_t);

	while( slen > 0)
	{
		obuf[0] = '\0';
		optr = (char *)obuf;
		olen = sizeof(obuf) - sizeof(obuf[0]);

		size_t ret = iconv.convert(&sptr, &slen, &optr, &olen);
		if( ret == size_t(-1))
		{
			mayThrowStringConversionException();
		}
		*optr = '\0';
		out  += obuf;
	}

	return out;
}
#endif

// -------------------------------------------------------------------
std::string
toByteString(const String &enc, const String &utf8)
{
	if( utf8.empty())
		return std::string();

	IConv_t      iconv("UTF-8", enc); // throws error
	std::string  out;
	char         obuf[4097];
	char        *optr;
	size_t       olen;

	char        *sptr = (char *)utf8.c_str();
	size_t       slen = utf8.length();

	while( slen > 0)
	{
		obuf[0] = '\0';
		optr = (char *)obuf;
		olen = sizeof(obuf) - sizeof(obuf[0]);

		size_t ret = iconv.convert(&sptr, &slen, &optr, &olen);
		if( ret == size_t(-1))
		{
			mayThrowStringConversionException();
		}
		*optr = '\0';
		out  += obuf;
	}

	return out;
}

#ifdef BLOCXX_HAVE_STD_WSTRING
// -------------------------------------------------------------------
std::wstring
toWideString(const String &enc, const String &utf8)
{
	if( utf8.empty())
		return std::wstring();

	IConv_t      iconv("UTF-8", enc); // throws error
	std::wstring out;
	wchar_t      obuf[1025];
	char        *optr;
	size_t       olen;

	char        *sptr = (char *)utf8.c_str();
	size_t       slen = utf8.length();

	while( slen > 0)
	{
		obuf[0] = L'\0';
		optr = (char *)obuf;
		olen = sizeof(obuf) - sizeof(obuf[0]);

		size_t ret = iconv.convert(&sptr, &slen, &optr, &olen);
		if( ret == size_t(-1))
		{
			mayThrowStringConversionException();
		}
		*((wchar_t *)optr) = L'\0';
		out += obuf;
	}

	return out;
}
#endif


#if 0
// -------------------------------------------------------------------
StringArray
encodings()
{
	StringArray   command;
	String        output;
	int           status = -1;

	command.push_back("/usr/bin/iconv");
	command.push_back("--list");

	try
	{
		Exec::executeProcessAndGatherOutput(command, output, status);
	}
	catch(...)
	{
	}

	if(status == 0)
	{
		return output.tokenize("\r\n");
	}
	return StringArray();
}
#endif


}  // End of IConv namespace
}  // End of BLOCXX_NAMESPACE

#endif // BLOCXX_HAVE_ICONV_SUPPORT

/* vim: set ts=8 sts=8 sw=8 ai noet: */

