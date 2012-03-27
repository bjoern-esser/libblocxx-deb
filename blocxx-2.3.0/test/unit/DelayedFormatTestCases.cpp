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


#include "blocxx/BLOCXX_config.h"
#define PROVIDE_AUTO_TEST_MAIN
#include "AutoTest.hpp"

#include "blocxx/DelayedFormat.hpp"

using namespace blocxx;

AUTO_UNIT_TEST(DelayedFormatTestCases_testDelayedFormat0)
{
	DelayedFormat fmt("empty");

	unitAssert(fmt == "empty");
}

AUTO_UNIT_TEST(DelayedFormatTestCases_testDelayedFormat1)
{
	int a = 0;
	DelayedFormat fmt("a=%1", a);
	unitAssert(fmt == "a=0");
	a = 1; unitAssert(fmt == "a=1");
	a = 2; unitAssert(fmt == "a=2");
	a = 0; unitAssert(fmt == "a=0");
}

AUTO_UNIT_TEST(DelayedFormatTestCases_testDelayedFormat2)
{
	int a = 0;
	int b = 1;
	DelayedFormat fmt("a=%1 b=%2", a, b);
	unitAssert(fmt == "a=0 b=1");
	a = 1; unitAssert(fmt == "a=1 b=1");
	b = 2; unitAssert(fmt == "a=1 b=2");
	a = b = 0;
	unitAssert(fmt == "a=0 b=0");
}

AUTO_UNIT_TEST(DelayedFormatTestCases_testDelayedFormat3)
{
	int a = 0;
	int b = 1;
	int c = 2;
	DelayedFormat fmt("a=%1 b=%2 c=%3", a, b, c);
	unitAssert(fmt == "a=0 b=1 c=2");
	a = 1; unitAssert(fmt == "a=1 b=1 c=2");
	b = 2; unitAssert(fmt == "a=1 b=2 c=2");
	c = 3; unitAssert(fmt == "a=1 b=2 c=3");
	a = b = c = 0;
	unitAssert(fmt == "a=0 b=0 c=0");
}

AUTO_UNIT_TEST(DelayedFormatTestCases_testDelayedFormat4)
{
	int a = 0;
	int b = 1;
	int c = 2;
	int d = 3;
	DelayedFormat fmt("a=%1 b=%2 c=%3 d=%4", a, b, c, d);
	unitAssert(fmt == "a=0 b=1 c=2 d=3");
	a = 1; unitAssert(fmt == "a=1 b=1 c=2 d=3");
	b = 2; unitAssert(fmt == "a=1 b=2 c=2 d=3");
	c = 3; unitAssert(fmt == "a=1 b=2 c=3 d=3");
	d = 4; unitAssert(fmt == "a=1 b=2 c=3 d=4");
	a = b = c = d = 0;
	unitAssert(fmt == "a=0 b=0 c=0 d=0");
}

AUTO_UNIT_TEST(DelayedFormatTestCases_testDelayedFormat5)
{
	int a = 0;
	int b = 1;
	int c = 2;
	int d = 3;
	int e = 4;
	DelayedFormat fmt("a=%1 b=%2 c=%3 d=%4 e=%5", a, b, c, d, e);
	unitAssert(fmt == "a=0 b=1 c=2 d=3 e=4");
	a = 1; unitAssert(fmt == "a=1 b=1 c=2 d=3 e=4");
	b = 2; unitAssert(fmt == "a=1 b=2 c=2 d=3 e=4");
	c = 3; unitAssert(fmt == "a=1 b=2 c=3 d=3 e=4");
	d = 4; unitAssert(fmt == "a=1 b=2 c=3 d=4 e=4");
	e = 5; unitAssert(fmt == "a=1 b=2 c=3 d=4 e=5");
	a = b = c = d = e = 0;
	unitAssert(fmt == "a=0 b=0 c=0 d=0 e=0");
}

AUTO_UNIT_TEST(DelayedFormatTestCases_testDelayedFormat9)
{
	int a = 0;
	int b = 1;
	int c = 2;
	int d = 3;
	int e = 4;
	String f = "foo";
	DelayedFormat g("a=%1", a); // Yes, we're going to embed a delayed format
	                            // into the delayed format.  Neat.
	String h = "bar";
	DelayedFormat i("g=%1", g);

	DelayedFormat fmt("a=%1 b=%2 c=%3 d=%4 e=%5 f=%6 g=%7 h=%8 i=%9", a, b, c, d, e, f, g, h, i);
	unitAssert(fmt == "a=0 b=1 c=2 d=3 e=4 f=foo g=a=0 h=bar i=g=a=0");
	a = 1; unitAssert(fmt == "a=1 b=1 c=2 d=3 e=4 f=foo g=a=1 h=bar i=g=a=1");
	b = 2; unitAssert(fmt == "a=1 b=2 c=2 d=3 e=4 f=foo g=a=1 h=bar i=g=a=1");
	c = 3; unitAssert(fmt == "a=1 b=2 c=3 d=3 e=4 f=foo g=a=1 h=bar i=g=a=1");
	d = 4; unitAssert(fmt == "a=1 b=2 c=3 d=4 e=4 f=foo g=a=1 h=bar i=g=a=1");
	e = 5; unitAssert(fmt == "a=1 b=2 c=3 d=4 e=5 f=foo g=a=1 h=bar i=g=a=1");
	a = b = c = d = e = 0;
	unitAssert(fmt == "a=0 b=0 c=0 d=0 e=0 f=foo g=a=0 h=bar i=g=a=0");
	a = 10;
	h = "Enough Already";
	unitAssert(fmt == "a=10 b=0 c=0 d=0 e=0 f=foo g=a=10 h=Enough Already i=g=a=10");
}
