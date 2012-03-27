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

/* iconv_test1.cpp
**
** This example demonstrates how to use the IConv_t
** class (later) and the IConv namespace functions.
**
** FIXME: document this stuff.
**
*/


#include <blocxx/BLOCXX_config.h>
#include <blocxx/Exception.hpp>
#include <blocxx/String.hpp>
#include <blocxx/IConv.hpp>
#include <iostream>
#include <cstring>  // for strlen
#include <langinfo.h>

using namespace BLOCXX_NAMESPACE;
using namespace std;

void test_list();
void test_conv1();
void test_conv2();

int  main(int argc, char* argv[])
{
	//test_list();
	test_conv1();
	test_conv2();

	return 0;
}

void test_conv1()
{
	std::cout << "=== test_conv1 ==============================="
	          << std::endl;
	const char str[] = {'\xfc',	// ü
	                    '\xf6',	// ö
	                    '\xe4',	// ä
	                    '\xdc',	// Ü
	                    '\xd6',	// Ö
	                    '\xc4',	// Ä
	                    '\xdf',	// ß
	                    '\xa4',	// €
	                    '\xa2',	// ¢
	                    '\x00'};

	String out = IConv::fromByteString("ISO-8859-15", str, strlen(str));
	std::cout << "Src: 'üöäÜÖÄß€¢'"      << std::endl;
	std::cout << "Out: '" << out << "'"  << std::endl;
	std::cout << std::endl;
}

void test_conv2()
{
#if BLOCXX_HAVE_STD_WSTRING
	std::cout << "=== test_conv2 ==============================="
	          << std::endl;

	try
	{
		String       src("üöä ÜÖÄ €¢ß");
		std::cout << "Src: '" << src << "'" << std::endl;
		std::wstring wch = IConv::toWideString("WCHAR_T", src);
		String       tmp = IConv::fromWideString("WCHAR_T", wch);
		std::cout << "Tmp: '" << tmp << "'" << std::endl;
		std::string  out = IConv::toByteString("ISO-8859-15", tmp);
		std::cout << "Out: '" << IConv::fromByteString("ISO-8859-15",
		                                               out)
		          << "'"      << std::endl;
		std::cout << std::endl;

		std::cout << "Converting of Latin2 characters into Latin9 "
		          << "will throw an exception:" << std::endl;

		src = "ÿýóïłźĄ";  // (utf8 encoded, but latin2 characters!
		std::cout << "Src: '" << src << "'" << std::endl;
		out = out = IConv::toByteString("ISO-8859-15", src);
	}
	catch(const Exception &e)
	{
		std::cerr << "Exception (" << e.getFile()    << ":"
		                           << e.getLine()    << "): "
		                           << e.getMessage() << "!"
					   << std::endl;
	}
#endif
}

void test_list()
{
#if 0
	std::cout << "=== test_list ================================"
	          << std::endl;
	StringArray list;
	try
	{
		list = IConv::encodings();
	}
	catch(const Exception &e)
	{
		std::cerr << "Exception (" << e.getFile() << ":"
		                           << e.getLine() << "): "
		                           << e.getMessage() << std::endl;
	}
	if( !list.empty())
	{
		StringArray::const_iterator i=list.begin();
		for( ; i != list.end(); ++i)
		{
			std::cout << *i << std::endl;
		}
	} else {
		std::cout << "ERROR" << std::endl;
	}
#endif
}

