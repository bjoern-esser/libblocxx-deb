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
 * @author Marius Tomaschewski
 */

#ifndef  BLOCXX_ICONV_HPP_INCLUDE_GUARD_HPP_
#define  BLOCXX_ICONV_HPP_INCLUDE_GUARD_HPP_
#include "blocxx/BLOCXX_config.h"

#if defined(BLOCXX_HAVE_ICONV_SUPPORT)
#include "blocxx/String.hpp"
#include "blocxx/Array.hpp"
#include <string>
#include <iconv.h>

namespace BLOCXX_NAMESPACE
{

/**
 * The IConv_t class is a wrapper to the UNIX98 iconv(3) functions.
 * See also the IConv namespace for some utility functions.
 */
class BLOCXX_COMMON_API IConv_t
{
public:
	/**
	 * Create an IConv object
	 */
	IConv_t();

	/**
	 * Create an IConv object and initializes its handle
	 * to convert between the specified character encodings.
	 *
	 * Throws an error if the requested conversion is not
	 * available.
	 *
	 * @param  fromEncoding source encoding name
	 * @param  toEncoding   destination encoding name
	 * @throws StringConversionException if the conversion
	 *         is not supported.
	 */
	IConv_t(const String &fromEncoding, const String &toEncoding);

	/**
	 * Frees all resources and destroys the object.
	 */
	~IConv_t();

	/**
	 * Initializes the IConv object handle to convert between
	 * the specified character encodings.
	 *
	 * See "man 3 iconv_open" for more detailed description.
	 *
	 * @param  fromEncoding source encoding name
	 * @param  toEncoding   destination encoding name
	 * @return true on success, false and errno code on failure
	 */
	bool         open(const String &fromEncoding, const String &toEncoding);

	/**
	 * Converts the text in the input buffer and stores the
	 * result text in the output buffer.
	 *
	 * See "man 3 iconv" for more detailed description.
	 *
	 * @param  istr         pointer to input text buffer
	 * @param  ibytesleft   number of bytes in ibuf
	 * @param  ostr         pointer to output text buffer
	 * @param  obytesleft   number of bytes in obuf
	 *
	 * @return 0, number of non-reversible conversions
	 *         or (size_t)-1 with reason in errno.
	 */
	size_t       convert(char **istr, size_t *ibytesleft,
	                     char **ostr, size_t *obytesleft);

	/**
	 * Frees all object resources.
	 *
	 * See "man 3 iconv_close" for more detailed description.
	 *
	 * @return  true on success, false and errno code on failure
	 */
	bool         close();

private:
	iconv_t m_iconv;
};

/**
 * The IConv namespace contains some utility functions
 * based on the IConv_t class.
 */
namespace IConv
{
	/**
	 * Converts a C string from the specified encoding
	 * into a String object using UTF-8 encoding.
	 *
	 * @param  enc          source encoding name
	 * @param  str          source string pointer
	 * @param  len          source string length
	 * @return new, UTF-8 encoded String object
	 * @throws StringConversionException if the conversion
	 *         is not supported or incomplete or invalid
	 *         character or multibyte sequence was found.
	 */
	BLOCXX_COMMON_API String
	fromByteString(const String &enc, const char *str, size_t len);

	/**
	 * Converts a std::string from the specified encoding
	 * into a String object using UTF-8 encoding.
	 *
	 * @param  enc          source encoding name
	 * @param  str          source std::string
	 * @return new, UTF-8 encoded String object
	 * @throws StringConversionException if the conversion
	 *         is not supported or incomplete or invalid
	 *         character or multibyte sequence was found.
	 */
	BLOCXX_COMMON_API String
	fromByteString(const String &enc, const std::string  &str);

#ifdef BLOCXX_HAVE_STD_WSTRING
	/**
	 * Converts a std::wstring from the specified encoding
	 * into a String object using UTF-8 encoding.
	 *
	 * @param  enc          source encoding name
	 * @param  str          source std::wstring
	 * @return new, UTF-8 encoded String object
	 * @throws StringConversionException if the conversion
	 *         is not supported or incomplete or invalid
	 *         character or multibyte sequence was found.
	 */
	BLOCXX_COMMON_API String
	fromWideString(const String &enc, const std::wstring &str);
#endif

	/**
	 * Converts a UTF-8 String to the specified encoding
	 * casted as a (char based) std::string.
	 *
	 * @param  enc          encoding name
	 * @param  utf8         source UTF-8 encoded String
	 * @return new, encoded std::string object
	 * @throws StringConversionException if the conversion
	 *         is not supported or incomplete or invalid
	 *         character or multibyte sequence was found.
	 */
	BLOCXX_COMMON_API std::string
	toByteString(const String &enc, const String &utf8);

#ifdef BLOCXX_HAVE_STD_WSTRING
	/**
	 * Convert a UTF-8 String to the specified encoding
	 * casted as a (wchar_t based) std::wstring.
	 *
	 * @param  enc          encoding name
	 * @param  utf8         source UTF-8 encoded String
	 * @return new, encoded std::wstring object
	 * @throws StringConversionException if the conversion
	 *         is not supported or incomplete or invalid
	 *         character or multibyte sequence was found.
	 */
	BLOCXX_COMMON_API std::wstring
	toWideString(const String &enc, const String &utf8);
#endif

#if 0
	/*
	 * Retrieve an array of supported encoding names.
	 *
	 * @return   array with supported encoding names.
	 */
	BLOCXX_COMMON_API StringArray
	encodings();
#endif

}	// End of IConv namespace
}	// End of BLOCXX_NAMESPACE

#endif  // BLOCXX_HAVE_ICONV_SUPPORT
#endif	// INCLUDE_GUARD_HPP_

/* vim: set ts=8 sts=8 sw=8 ai noet: */

