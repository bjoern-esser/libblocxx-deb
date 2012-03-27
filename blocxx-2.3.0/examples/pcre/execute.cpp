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
** This example demonstrates how to use the PerlRegEx::execute method.
*/


#include <blocxx/BLOCXX_config.h>
#include <blocxx/PerlRegEx.hpp>
#include <iostream>

using namespace blocxx;
using namespace std;

int main(int argc, char* argv[])
{
#ifndef BLOCXX_HAVE_PCRE
	std::cerr << "Sorry, no perl regex avaliable in blocxx" << std::endl;
	return 1;
#else
	String                 str("http://user:pass@www.some.name:33/path/to/file.html#target");
	PerlRegEx              reg;
#if use_match_array
	PerlRegEx::MatchArray  sub;
#else
	PerlRegEx::MatchVector sub;
#endif

	try
	{
	    reg = PerlRegEx("^(([^:/?#]+)://)?([^/?#]*)?([^?#]*)?(\\?([^#]*))?(#(.*))?");
	}
	catch(const blocxx::Exception &e)
	{
	    std::cerr << "Compilation failed: " << e << std::endl;
	    return 1;
	}

	std::cout << "Executing regex '" << reg.patternString() << "'"
	          << std::endl
	          << "to match string '" << str << "'"
		  << std::endl;
	try
	{
	    reg.execute(sub, str);
	}
	catch(const blocxx::Exception &e)
	{
	    std::cerr << "Execute failed: " << e << std::endl;
	    return 1;
	}

#if use_match_array
        std::cout << "Found "       << sub.size()
                  << " match(es): " << std::endl;

        PerlRegEx::MatchArray::const_iterator i=sub.begin();
        size_t                                 n=0;

        std::cout << "{" << std::endl;
        while( i != sub.end())
        {
            std::cout << "\tsub[" << n << "] = { "
                      << i->rm_so << ", "
                      << i->rm_eo << " }";
            if( ++i != sub.end())
                std::cout << ",";
            std::cout << std::endl;

            n++;
        }
        std::cout << "}" << std::endl;
#else

	size_t n, matches = sub.size() / 2;

	std::cout << "Found "       << matches
	          << " match(es): " << std::endl;

	std::cout << "{" << std::endl;
	for( n = 0; n < matches; n++)
	{
	    std::cout << "\tsub[" << n << "] = { "
	              << sub[ ( n * 2) ]     << ", "
		      << sub[ ( n * 2) + 1 ] << " }";
	    if( (n + 1) < matches)
		std::cout << ",";
	    std::cout << std::endl;
	}
	std::cout << "}" << std::endl;
#endif

	return 0;
#endif
}

