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
 */

#include "blocxx/BLOCXX_config.h"
#define PROVIDE_AUTO_TEST_MAIN
#include "AutoTest.hpp"

#include "blocxx/Format.hpp"
#define _USE_MATH_DEFINES // For windows to define M_PI, shouldn't hurt any other platform.
#include <cmath>

using namespace blocxx;

AUTO_UNIT_TEST(FormatTestCases_testFormat)
{
	unitAssertEquals( "1 != 2", Format("%1 != %2", 1, 2) );
	unitAssertEquals( "good != food", Format("%1 != %2", "good", "food") );
	unitAssertEquals( "1 != 2", Format("%1 != %2", 1, 2).toString() );
	unitAssertEquals( "1,2,3,4,5,6,7,8,9", Format("%1,%2,%3,%4,%5,%6,%7,%8,%9",
				1,2,3,4,5,6,7,8,9).toString() );
	unitAssertEquals( "1 != 2", String(Format("%1 != %2", 1, 2).c_str()) );
	unitAssertEquals( "0", Format("%1", false));

	unitAssertEquals( "sdrawkcab", Format("%9%8%7%6%5%4%3%2%1", 'b', 'a', 'c', 'k', 'w', 'a', 'r', 'd', 's') );

	// This test is for backwards compatibility.  Previously format only allowed
	// 9 format arguments and a single digit after the '%'.  Make sure this
	// behavior has not changed.
	unitAssertEquals( "abcdefghia0a1a2a3a4", Format("%1%2%3%4%5%6%7%8%9%10%11%12%13%14", 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i') );


#if defined(BLOCXX_USE_CXX_0X)
	// Using variadic templates, we are able to have any number of arguments...
	// This is unreadable, but the feature is nice.
	unitAssertEquals( "txet sdrawkcab|backwards text",
		Format("%<14>%<13>%<12>%<11>%<10>%<9>%<8>%<7>%<6>%<5>%<4>%<3>%<2>%<1>|%<1>%<2>%<3>%<4>%<5>%<6>%<7>%<8>%<9>%<10>%<11>%<12>%<13>%<14>",
			'b','a','c','k','w','a','r','d','s',' ','t','e','x','t') );
#endif
}

namespace
{
	class Foo
	{
	public:
		Foo(int a, int b) : m_a(a), m_b(b) { }

		friend std::ostream& operator<<(std::ostream& o, const Foo& f)
		{
			o << "{ a=" << f.m_a << ", b=" << f.m_b << " }";
			return o;
		}
	private:
		int m_a;
		int m_b;
	};
}

AUTO_UNIT_TEST(FormatTestCases_paramReuse)
{
	unitAssertEquals( "Hello and Goodbye but then Hello again.", Format("%1 and %2 but then %1 again.", "Hello", "Goodbye"));
	unitAssertEquals( "Hello Goodbye Goodbye Hello", Format("%1 %2 %2 %1", "Hello", "Goodbye"));
}

AUTO_UNIT_TEST(FormatTestCases_notCorrectDataTypeModifiers)
{
	// Empty modifier string, or strings that don't apply for the
	// supplied data type.
	unitAssertEquals( "hello", Format("%<1>", "hello") );
	unitAssertEquals( "hello", Format("%<1:x>", "hello") );
	unitAssertEquals( "hello", Format("%<1:!>", "hello") );
	unitAssertEquals( "hello", Format("%<1:.10>", "hello") );
	unitAssertEquals( "hello", Format("%<1:.10x!>", "hello") );

}

AUTO_UNIT_TEST(FormatTestCases_alignmentModifiers)
{
	// Alignment
	unitAssertEquals( "           1 != 2", Format("%<1:12> != %2", 1, 2) );
	unitAssertEquals( "1            != 2", Format("%<1:-12> != %2", 1, 2) );
	unitAssertEquals( "|   -->|", Format("|%<1:+7>", "-->|"));
	unitAssertEquals( "|<--   |", Format("%<1:-7>|", "|<--"));
	unitAssertEquals( "|<--   |   -->|", Format("%<1:-7>|%<2:7>", "|<--", "-->|") );
	// Outer alignment of structures (not inner)
	unitAssertEquals( "{ a=1, b=2 }", Format("%1", Foo(1,2)) );
	unitAssertEquals( "{ a=1, b=2 }   ", Format("%<1:-15>", Foo(1,2)) );
	unitAssertEquals( "   { a=1, b=2 }", Format("%<1:15>", Foo(1,2)) );

	unitAssertEquals( "123 in oct is     173", Format("%1 in oct is %<1:7o>", 123) );
	unitAssertEquals( "123 in oct is 0173   ", Format("%1 in oct is %<1:-7o!>", 123) );

	unitAssertEquals( "123 in hex is    0x7b", Format("%1 in hex is %<1:7x!>", 123) );
	unitAssertEquals( "123 in hex is 0x7b   ", Format("%1 in hex is %<1:-7x!>", 123) );
	unitAssertEquals( "123 in hex is      7b", Format("%1 in hex is %<1:7x>", 123) );
	unitAssertEquals( "123 in hex is 7b     ", Format("%1 in hex is %<1:-7x>", 123) );

	unitAssertEquals( "Hex number:    face", Format("Hex number: %<1:7x>", 0xFACE) );
	unitAssertEquals( "Hex number: face   ", Format("Hex number: %<1:-7x>", 0xFACE) );
	unitAssertEquals( "Hex number:    FACE", Format("Hex number: %<1:7X>", 0xFACE) );
	unitAssertEquals( "Hex number: FACE   ", Format("Hex number: %<1:-7X>", 0xFACE) );
	unitAssertEquals( "Hex number:    0xface", Format("Hex number: %<1:9x!>", 0xFACE) );
	unitAssertEquals( "Hex number: 0xface   ", Format("Hex number: %<1:-9x!>", 0xFACE) );
	unitAssertEquals( "Hex number:    0XFACE", Format("Hex number: %<1:9X!>", 0xFACE) );
	unitAssertEquals( "Hex number: 0XFACE   ", Format("Hex number: %<1:-9X!>", 0xFACE) );
	unitAssertEquals( " 0x123", Format("%<1:6x!>", 291) );
	unitAssertEquals( "0x123 ", Format("%<1:-6x!>", 291) );
}

AUTO_UNIT_TEST(FormatTestCases_boolRepresentation)
{
	// bool (0,1) or (false,true)
	unitAssertEquals( "0", Format("%1", false));
	unitAssertEquals( "1", Format("%1", true));
	unitAssertEquals( "false", Format("%<1:!>", false));
	unitAssertEquals( "true", Format("%<1:!>", true) );
	unitAssertEquals( "0 false 1 true", Format("%1 %<2:!> %3 %<4:!>", false, false, true, true));
}

AUTO_UNIT_TEST(FormatTestCases_otherBases)
{
	// hex and octal
	unitAssertEquals( "123 in oct is 0173", Format("%1 in oct is %<1:o!>", 123) );
	unitAssertEquals( "123 in oct is 173", Format("%1 in oct is %<1:o>", 123) );
	unitAssertEquals( "123 in oct is 0173   ", Format("%1 in oct is %<1:-7o!>", 123) );
	unitAssertEquals( "123 in hex is 0x7b", Format("%1 in hex is %<1:x!>", 123) );
	unitAssertEquals( "123 in hex is 7b", Format("%1 in hex is %<1:x>", 123) );

	unitAssertEquals( "Hex number: face", Format("Hex number: %<1:x>", 0xFACE) );
	unitAssertEquals( "Hex number: FACE", Format("Hex number: %<1:X>", 0xFACE) );
	unitAssertEquals( "Hex number: 0xface", Format("Hex number: %<1:x!>", 0xFACE) );
	unitAssertEquals( "Hex number: 0XFACE", Format("Hex number: %<1:X!>", 0xFACE) );
	unitAssertEquals( " 0x123", Format("%<1:6x!>", 291) );
}

AUTO_UNIT_TEST(FormatTestCases_zeroPadding)
{
	// Padding
	unitAssertEquals( "000000000001 != 2", Format("%<1:012> != %2", 1, 2) );
	unitAssertEquals( "1            != 2", Format("%<1:-012> != %2", 1, 2) );
	unitAssertEquals( "0000001.5000 != 2.7", Format("%<1:012.4> != %2", 1.5f, 2.7f) );
	unitAssertEquals( "1.5000       != 2.7", Format("%<1:-012.4> != %2", 1.5f, 2.7f) );

	unitAssertEquals( "|   -->|", Format("|%<1:+07>", "-->|"));
	unitAssertEquals( "|<--   |", Format("%<1:-07>|", "|<--"));
	unitAssertEquals( "|<--   |   -->|", Format("%<1:-7>|%<2:07>", "|<--", "-->|") );

	// Bool ignores leading zero padding
	unitAssertEquals( " 0", Format("%<1:02>", false));
	unitAssertEquals( " 1", Format("%<1:02>", true));
	unitAssertEquals( "false", Format("%<1:02!>", false));
	unitAssertEquals( "true", Format("%<1:02!>", true) );

	unitAssertEquals( "123 in oct is 0000173", Format("%1 in oct is %<1:07o!>", 123) );
	unitAssertEquals( "123 in oct is 0173   ", Format("%1 in oct is %<1:-07o!>", 123) );
	unitAssertEquals( "123 in oct is 0000173", Format("%1 in oct is %<1:07o>", 123) );
	unitAssertEquals( "123 in oct is 173    ", Format("%1 in oct is %<1:-07o>", 123) );

	unitAssertEquals( "123 in hex is 0000x7b", Format("%1 in hex is %<1:07x!>", 123) );
	unitAssertEquals( "123 in hex is 0x7b   ", Format("%1 in hex is %<1:-07x!>", 123) );
	unitAssertEquals( "123 in hex is 000007b", Format("%1 in hex is %<1:07x>", 123) );
	unitAssertEquals( "123 in hex is 7b     ", Format("%1 in hex is %<1:-07x>", 123) );

	unitAssertEquals( "Hex number: 000face", Format("Hex number: %<1:07x>", 0xFACE) );
	unitAssertEquals( "Hex number: face   ", Format("Hex number: %<1:-07x>", 0xFACE) );
	unitAssertEquals( "Hex number: 000FACE", Format("Hex number: %<1:07X>", 0xFACE) );
	unitAssertEquals( "Hex number: FACE   ", Format("Hex number: %<1:-07X>", 0xFACE) );
	unitAssertEquals( "Hex number: 0000xface", Format("Hex number: %<1:09x!>", 0xFACE) );
	unitAssertEquals( "Hex number: 0xface   ", Format("Hex number: %<1:-09x!>", 0xFACE) );
	unitAssertEquals( "Hex number: 0000XFACE", Format("Hex number: %<1:09X!>", 0xFACE) );
	unitAssertEquals( "Hex number: 0XFACE   ", Format("Hex number: %<1:-09X!>", 0xFACE) );
	unitAssertEquals( "00x123", Format("%<1:06x!>", 291) );
	unitAssertEquals( "0x123 ", Format("%<1:-06x!>", 291) );

	// Outer alignment of structures (not inner)
	unitAssertEquals( "{ a=1, b=2 }   ", Format("%<1:-015>", Foo(1,2)) );
	unitAssertEquals( "   { a=1, b=2 }", Format("%<1:015>", Foo(1,2)) );
}

AUTO_UNIT_TEST(FormatTestCases_testPrecision)
{
	// precision
	unitAssertEquals( "2", Format("%1", 2.00000000000000000001));
	unitAssertEquals( "2.", Format("%<1:.0>", 2.00000000000000000001));
	unitAssertEquals( "2.0000", Format("%<1:0.4>", 2.00000000000000000001));
	unitAssertEquals( "2.0000000000", Format("%<1:0.10>", 2.00000000000000000001));
	unitAssertEquals( "31.4", Format("%<1:0.1>", 10 * M_PI) );
	unitAssertEquals( "3.1", Format("%<1:0.1>", M_PI) );
	unitAssertEquals( "3.1416", Format("%<1:0.4>", M_PI) );
	unitAssertEquals( "3.1415926536", Format("%<1:0.10>", M_PI) );
	unitAssertEquals( "3.1415926536   ", Format("%<1:-15.10>", M_PI) );
	unitAssertEquals( "   3.1415926536", Format("%<1:15.10>", M_PI) );
}

AUTO_UNIT_TEST(FormatTestCases_testFormatModifiers)
{
	// Yes, void pointers can be output, but their output depends on the
	// particular implementation.  For most platforms this works, but a few
	// print it out with a whole bunch more digits (leading zeroes):
	//	unitAssertEquals( "0x123", Format("%<1:x!>", (void*)291) );

	// base tags being used inside a structure's << operator
	unitAssertEquals( "{ a=0x1, b=0x2 } { a=1, b=2 }", Format("%<1:15x!> %1", Foo(1,2)) );
	unitAssertEquals( "  { a=0x1, b=0x2 } 123", Format("%<1:18x!> %2", Foo(1,2), 123) );
	unitAssertEquals( "{ a=0x1, b=0x2 }   123", Format("%<1:-18x!> %2", Foo(1,2), 123) );
}

AUTO_UNIT_TEST(FormatTestCases_testPercent)
{
	unitAssertEquals( Format("%1%%%2", "abc", "def").toString(), "abc%def" );
	unitAssertEquals( Format("%2%%%1%3%2", "one", "two", '%').toString(),
			"two%one%two" );
}

AUTO_UNIT_TEST(FormatTestCases_testError)
{
	unitAssertEquals( "1,\n*** Parameter specifier 200 is too large (>1)\n*** Error in format string at \"%<200>\"\n", Format("%1,%<200>xx", 1).toString() );
	unitAssertEquals( "1,\n*** Parameter specifier 2 is too large (>1)\n*** Error in format string at \"%2\"\n", Format("%1,%2xx", 1).toString() );
	unitAssertEquals( "\n*** Error in format string at \"%x\"\n", Format("%xABC", 'e').toString() );
	unitAssertEquals( "\n*** Error in format string at \"%<>\"\n", Format("%<>", 'e').toString() );
	unitAssertEquals( "\n*** Error in format string at \"%<b\"\n", Format("%<bad>", 'e').toString() );
	unitAssertEquals( "\n*** Parameter specifier 0 must be >= 1\n*** Error in format string at \"%0\"\n", Format("%0ABC", 'e').toString() );
}
