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
 * @author Bart Whiteley
 * @author Dan Nuffer
 */


#include "blocxx/BLOCXX_config.h"

#include <iostream>
#include <stdlib.h>

using std::cerr;
using std::endl;

/*
BLOCXX_UNDERRUN=1
BLOCXX_OVERRUN=2
BLOCXX_UNKNOWN_ADDR=3
BLOCXX_DOUBLE_DELETE=4
BLOCXX_DOUBLE_DELETE_NOFREE=5
*/

void Usage(const char* cmd)
{
	cerr <<
		"Usage: " << cmd << " <mode>\n"
		"	1 = under run\n"
		"	2 = over run\n"
		"	3 = unknown address\n"
		"	4 = double delete\n"
		"	7 = aggressive" << endl;
}

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		Usage(argv[0]);
		return 9;
	}

	int mode = atoi(argv[1]);

	char* buf = NULL;
	char* buf2 = NULL;

	switch (mode)
	{
		case 1: // under run
			buf = new char[10];
			buf--;
			(*buf) = 's';
			buf++;
			delete[] buf;
			break;
		case 2: // over run
			buf = new char[10];
			buf[10] = 's';
			delete[] buf;
			break;
		case 4: // double delete
			buf = new char[10];
			delete[] buf;
			delete[] buf;
			break;
		case 3: // unknown address
			buf = new char[10];
			buf += 5;
			delete[] buf;
			break;
		case 7:
			buf = new char[10];
			buf[10] = 's';
			buf2 = new char[10];
			break;
		default:
			Usage(argv[0]);
			return 9;
			break;
	}

	return 0;
}


