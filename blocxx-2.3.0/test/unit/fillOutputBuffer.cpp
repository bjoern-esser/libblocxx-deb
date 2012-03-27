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

#include <iostream>
#include <ostream>
#include <vector>
#include <algorithm>
#include <stdlib.h>

// This program exists to dump output to a buffered stream (stdout).  The
// intention is to use this in testing the proper termination of an executed
// process if that process becomes blocked on output.  This would happen if the
// parent process doesn't start reading from its end of the pipe (eg. it is
// waiting for the child process to do the close-on-exec of its status pipe).

int main(int argc, char** argv)
{
	if( argc > 1 )
	{
		int bytes_to_write = atoi(argv[1]);
		std::vector<char> buffer(16384);
		for( size_t i = 0; i < buffer.size(); ++i )
		{
			buffer[i] = char((i % 26) + 'A');
		}

		while( std::cout && (bytes_to_write > 0) )
		{
			size_t count = std::min<size_t>(bytes_to_write, buffer.size());
			std::cout.write(&buffer[0], count);
			bytes_to_write -= count;
		}

		if( !std::cout )
		{
			return 3;
		}
		std::cout.flush();
		return 0;
	}
	return 1;
}
