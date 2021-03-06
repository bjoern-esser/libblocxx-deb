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

/*
** This example demonstrates how to use the PosixRegEx::split method.
*/


#include <blocxx/BLOCXX_config.h>
#include <blocxx/PosixRegEx.hpp>
#include <iostream>

using namespace blocxx;
using namespace std;

int main(int argc, char* argv[])
{
#ifndef BLOCXX_HAVE_REGEX
	std::cerr << "Sorry, no posix regex avaliable in blocxx" << std::endl;
	return 1;
#else
	String             str("1.23, .50 , , 71.00 , 6.00");
	PosixRegEx         reg;
	StringArray        out;

	try
	{
	    reg = PosixRegEx("([ \t]*,[ \t]*)");
	}
	catch(const blocxx::Exception &e)
	{
	    std::cerr << "Compilation failed: " << e << std::endl;
	    return 1;
	}

	std::cout << "Splitting string '" << str
	          << "' using regex '" << reg.patternString()
		  << "' to match separators" << std::endl;
	try
	{
	    out = reg.split(str);
	}
	catch(const blocxx::Exception &e)
	{
	    std::cerr << "Split failed: " << e << std::endl;
	    return 1;
	}

	std::cout << "Splitted into " << out.size()
	          << " substrings: " << std::endl;

	StringArray::const_iterator i=out.begin();
	std::cout << "{" << std::endl;
	while( i != out.end())
	{
	    std::cout << "\t\"" << *i << "\"";
	    if( ++i != out.end())
	        std::cout << ",";
	    std::cout << std::endl;
	}
	std::cout << "}" << std::endl;

	return 0;
#endif
}

