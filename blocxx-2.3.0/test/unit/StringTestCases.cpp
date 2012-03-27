/*******************************************************************************
* Copyright (C) 2009, Quest Software, Inc. All rights reserved.
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
 * @author Richard Holden
 */

#include "blocxx/BLOCXX_config.h"
#define PROVIDE_AUTO_TEST_MAIN
#include "AutoTest.hpp"

#include "blocxx/String.hpp"
#include "blocxx/Array.hpp"
#include "blocxx/Types.hpp"
#include "blocxx/Format.hpp"
#include "blocxx/StringJoin.hpp"
#include <cstring>  // for strcmp
#include <math.h> // for isnan
#include <limits>

using namespace blocxx;

AUTO_UNIT_TEST(StringTestCases_testErase)
{
	String s = "abc";
	s.erase();
	unitAssertEquals( s , "" );
	unitAssertEquals( s.length() , 0U );
	unitAssertEquals( String("12345").erase(2) , "12" );
	unitAssertEquals( String("12345").erase(2, 1) , "1245" );
}

AUTO_UNIT_TEST(StringTestCases_testEqualsIgnoreCase)
{
	String s = "abc";
	unitAssert(s.equalsIgnoreCase("abc"));
	s = "";
	unitAssert(s.equalsIgnoreCase(""));

	unitAssertFail(s.equalsIgnoreCase("a"));
}

AUTO_UNIT_TEST(StringTestCases_testSubstring)
{
	String s = "abc";
	unitAssertEquals( s.substring(0, 0) , "" );
	unitAssertEquals( s.substring(0, 0).length() , 0U );
	unitAssertEquals( s.substring(static_cast<UInt32>(-1)) , "" );
	unitAssertEquals( s.substring(static_cast<UInt32>(-1)).length() , 0U );
	unitAssertEquals( s.substring(1) , "bc" );
	unitAssertEquals( s.substring(1, 1) , "b" );
	unitAssertEquals( s.substring(2, 3) , "c" );
}

AUTO_UNIT_TEST(StringTestCases_testNumbers)
{
	String s = "-1";
	unitAssertEquals(s.toInt32() , -1);
	String uls("5000000000000");
	UInt64 ul = uls.toUInt64();
	unitAssertEquals(ul , 5000000000000ULL);
	String uls2(ul);
	unitAssert(uls.equals(uls2));

	Real64 rv = 4.56e+80;
	String rs(rv);
	Real64 r = rs.toReal64();
	unitAssertEquals(r, rv);
	String rs2(r);

	// This test depends entirely on what format
	unitAssert(rs.equals(rs2));
}

#if !defined(BLOCXX_HAVE_ISINF)
#if !defined(BLOCXX_WIN32)
# include <ieeefp.h>
#endif

#if defined(BLOCXX_WIN32)
#define finite _finite
#endif

static int isinf(double x) { return !finite(x) && x==x; }
#endif

#if defined(BLOCXX_WIN32)
#define isnan _isnan
#endif

AUTO_UNIT_TEST(StringTestCases_testRealConversionWithInfinity)
{
	unitAssert(isinf(String("INF").toReal32()));
	unitAssert(isinf(String("INF").toReal64()));
	unitAssert(isinf(String("+INF").toReal32()));
	unitAssert(isinf(String("+INF").toReal64()));

	unitAssertEquals(std::numeric_limits<Real32>::infinity(), String("+INF").toReal32());
	unitAssertEquals(std::numeric_limits<Real64>::infinity(), String("+INF").toReal64());

	unitAssert(isinf(String("inf").toReal32()));
	unitAssert(isinf(String("inf").toReal64()));
	unitAssert(isinf(String("INFINITY").toReal32()));
	unitAssert(isinf(String("INFINITY").toReal64()));
	unitAssert(isinf(String("infinity").toReal32()));
	unitAssert(isinf(String("infinity").toReal64()));
}

AUTO_UNIT_TEST(StringTestCases_testRealConversionWithNegativeInfinity)
{
	unitAssert(isinf(String("-INF").toReal32()));
	unitAssert(isinf(String("-INF").toReal64()));

	unitAssertEquals(-std::numeric_limits<Real32>::infinity(), String("-INF").toReal32());
	unitAssertEquals(-std::numeric_limits<Real64>::infinity(), String("-INF").toReal64());
}
#if 0
/*
 * These should be tested but currently the re2c generated code does not
 * handle this correctly.
 */
AUTO_UNIT_TEST(StringTestCases_testRealConversionWithInvalidInfinityRepresentation)
{
	unitAssertThrowsEx(String("INFIN").toReal32(), StringConversionException);
	unitAssertThrowsEx(String("INFIN").toReal64(), StringConversionException);

	unitAssertThrowsEx(String("infin").toReal32(), StringConversionException);
	unitAssertThrowsEx(String("infin").toReal64(), StringConversionException);
}
#endif

AUTO_UNIT_TEST(StringTestCases_testRealConversionWithNAN)
{
	unitAssert(isnan(String("NAN").toReal32()));
	unitAssert(isnan(String("NAN").toReal64()));
	unitAssert(isnan(String("nan").toReal32()));
	unitAssert(isnan(String("nan").toReal64()));
	unitAssert(isnan(String("NAN()").toReal32()));
	unitAssert(isnan(String("NAN()").toReal64()));
}

AUTO_UNIT_TEST(StringTestCases_testNumberConversionException)
{
	// More tests should go here...
	unitAssertThrowsEx(String().toUInt32(), StringConversionException);
	unitAssertThrowsEx(String("").toUInt32(), StringConversionException);

	/*
	 * This breaks on all platforms except Linux, proper behavior should
	 * probably be to throw an exception when converting a negative value
	 * to an unsigned type.
	unitAssertThrowsEx(String("-1234").toUInt32(), StringConversionException);
	*/

	unitAssertThrowsEx(String().toReal32(), StringConversionException);
	unitAssertThrowsEx(String("").toReal32(), StringConversionException);

#if !defined(BLOCXX_SOLARIS) && !defined(BLOCXX_HPUX)
	// Solaris and HP-UX convert this to Infinity instead of
	// throwing the expected exception.
	{
		// Larger than the largest float exponent.
		int exponent = std::numeric_limits<Real32>::max_exponent10 + 1;
		String out_of_bounds_real32 = Format("1e%1", exponent);
		unitAssertThrowsEx(out_of_bounds_real32.toReal32(), StringConversionException);
	}
#endif
	{
		// Larger than the largest double exponent.
		int exponent = std::numeric_limits<Real64>::max_exponent10 + 1;
		String out_of_bounds_real64 = Format("1e%1", exponent);
		unitAssertThrowsEx(out_of_bounds_real64.toReal64(), StringConversionException);
	}
}


AUTO_UNIT_TEST(StringTestCases_testTokenizeSingleTokenBetweenValues)
{
	String teststring1 = "0.1.2.3.4";
	StringArray tokenized1 = teststring1.tokenize( "." );
	unitAssertEquals( tokenized1.size() , 5U );
	unitAssertEquals( tokenized1[0] , "0" );
	unitAssertEquals( tokenized1[1] , "1" );
	unitAssertEquals( tokenized1[2] , "2" );
	unitAssertEquals( tokenized1[3] , "3" );
	unitAssertEquals( tokenized1[4] , "4" );

	tokenized1 = teststring1.tokenize( ".", String::E_RETURN_DELIMITERS );
	unitAssertEquals( tokenized1.size() , 9U );
	unitAssertEquals( tokenized1[0] , "0" );
	unitAssertEquals( tokenized1[1] , "." );
	unitAssertEquals( tokenized1[2] , "1" );
	unitAssertEquals( tokenized1[3] , "." );
	unitAssertEquals( tokenized1[4] , "2" );
	unitAssertEquals( tokenized1[5] , "." );
	unitAssertEquals( tokenized1[6] , "3" );
	unitAssertEquals( tokenized1[7] , "." );
	unitAssertEquals( tokenized1[8] , "4" );

	tokenized1 = teststring1.tokenize( ".", String::E_DISCARD_DELIMITERS, String::E_RETURN_EMPTY_TOKENS );
	unitAssertEquals( tokenized1.size() , 5U );
	unitAssertEquals( tokenized1[0] , "0" );
	unitAssertEquals( tokenized1[4] , "4" );
}

AUTO_UNIT_TEST(StringTestCases_testTokenizeMultipleTokensBetweenValues)
{
	String teststring2 = "0..1.2.3..4";
	StringArray tokenized2 = teststring2.tokenize( "." );
	unitAssertEquals( tokenized2.size() , 5U );
	unitAssertEquals( tokenized2[0] , "0" );
	unitAssertEquals( tokenized2[1] , "1" );
	unitAssertEquals( tokenized2[2] , "2" );
	unitAssertEquals( tokenized2[3] , "3" );
	unitAssertEquals( tokenized2[4] , "4" );

	tokenized2 = teststring2.tokenize( ".", String::E_RETURN_DELIMITERS );
	unitAssertEquals( tokenized2.size() , 11U );
	unitAssertEquals( tokenized2[0] , "0" );
	unitAssertEquals( tokenized2[1] , "." );
	unitAssertEquals( tokenized2[2] , "." );
	unitAssertEquals( tokenized2[3] , "1" );
	unitAssertEquals( tokenized2[4] , "." );
	unitAssertEquals( tokenized2[5] , "2" );
	unitAssertEquals( tokenized2[6] , "." );
	unitAssertEquals( tokenized2[7] , "3" );
	unitAssertEquals( tokenized2[8] , "." );
	unitAssertEquals( tokenized2[9] , "." );
	unitAssertEquals( tokenized2[10] , "4" );

	tokenized2 = teststring2.tokenize( ".", String::E_DISCARD_DELIMITERS, String::E_RETURN_EMPTY_TOKENS );
	unitAssertEquals( tokenized2.size() , 7U );
	unitAssertEquals( tokenized2[0] , "0" );
	unitAssertEquals( tokenized2[1] , "" );
	unitAssertEquals( tokenized2[2] , "1" );
	unitAssertEquals( tokenized2[3] , "2" );
	unitAssertEquals( tokenized2[4] , "3" );
	unitAssertEquals( tokenized2[5] , "" );
	unitAssertEquals( tokenized2[6] , "4" );

	tokenized2 = teststring2.tokenize( ".", String::E_RETURN_DELIMITERS, String::E_RETURN_EMPTY_TOKENS );
	unitAssertEquals( tokenized2.size() , 13U );
	unitAssertEquals( tokenized2[0] , "0" );
	unitAssertEquals( tokenized2[1] , "." );
	unitAssertEquals( tokenized2[2] , "" );
	unitAssertEquals( tokenized2[3] , "." );
	unitAssertEquals( tokenized2[4] , "1" );
	unitAssertEquals( tokenized2[5] , "." );
	unitAssertEquals( tokenized2[6] , "2" );
	unitAssertEquals( tokenized2[7] , "." );
	unitAssertEquals( tokenized2[8] , "3" );
	unitAssertEquals( tokenized2[9] , "." );
	unitAssertEquals( tokenized2[10] , "" );
	unitAssertEquals( tokenized2[11] , "." );
	unitAssertEquals( tokenized2[12] , "4" );
}

AUTO_UNIT_TEST(StringTestCases_testTokenizeDefaultTokens)
{
	String teststring3 = "a b c\nd e f\n\r\tg";
	StringArray tokenized3 = teststring3.tokenize();
	unitAssertEquals( tokenized3.size() , 7U );
	unitAssertEquals( tokenized3[0] , "a" );
	unitAssertEquals( tokenized3[1] , "b" );
	unitAssertEquals( tokenized3[2] , "c" );
	unitAssertEquals( tokenized3[3] , "d" );
	unitAssertEquals( tokenized3[4] , "e" );
	unitAssertEquals( tokenized3[5] , "f" );
	unitAssertEquals( tokenized3[6] , "g" );

	tokenized3 = teststring3.tokenize(" \n\r\t",  String::E_RETURN_DELIMITERS);
	unitAssertEquals( tokenized3.size() , 15U );
	unitAssertEquals( tokenized3[0] , "a" );
	unitAssertEquals( tokenized3[1] , " " );
	unitAssertEquals( tokenized3[2] , "b" );
	unitAssertEquals( tokenized3[3] , " " );
	unitAssertEquals( tokenized3[4] , "c" );
	unitAssertEquals( tokenized3[5] , "\n");
	unitAssertEquals( tokenized3[6] , "d" );
	unitAssertEquals( tokenized3[7] , " " );
	unitAssertEquals( tokenized3[8] , "e" );
	unitAssertEquals( tokenized3[9] , " " );
	unitAssertEquals( tokenized3[10] , "f" );
	unitAssertEquals( tokenized3[11] , "\n" );
	unitAssertEquals( tokenized3[12] , "\r" );
	unitAssertEquals( tokenized3[13] , "\t" );
	unitAssertEquals( tokenized3[14] , "g" );

	tokenized3 = teststring3.tokenize(" \n\r\t",  String::E_DISCARD_DELIMITERS, String::E_RETURN_EMPTY_TOKENS);
	unitAssertEquals( tokenized3.size() , 9U );
	unitAssertEquals( tokenized3[0] , "a" );
	unitAssertEquals( tokenized3[1] , "b" );
	unitAssertEquals( tokenized3[2] , "c" );
	unitAssertEquals( tokenized3[3] , "d" );
	unitAssertEquals( tokenized3[4] , "e" );
	unitAssertEquals( tokenized3[5] , "f" );
	unitAssertEquals( tokenized3[6] , "" );
	unitAssertEquals( tokenized3[7] , "" );
	unitAssertEquals( tokenized3[8] , "g" );
}

AUTO_UNIT_TEST(StringTestCases_testTokenizeMultipleDefaultTokensBetweenValues)
{
	String teststring4 = "foo bar        baz  quux  flarp   snoz  blarf                    zzyzx   veryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryveryvery_looooooooooonnnnnnnnggggggg_word";
	StringArray tokenized4 = teststring4.tokenize();
	unitAssertEquals( tokenized4.size() , 9U );
	unitAssertEquals( tokenized4[0] , "foo" );
	unitAssertEquals( tokenized4[1] , "bar" );
	unitAssertEquals( tokenized4[2] , "baz" );
	unitAssertEquals( tokenized4[3] , "quux" );
	unitAssertEquals( tokenized4[4] , "flarp" );
	unitAssertEquals( tokenized4[5] , "snoz" );
	unitAssertEquals( tokenized4[6] , "blarf" );
	unitAssertEquals( tokenized4[7] , "zzyzx" );
	unitAssertEquals( tokenized4[8].indexOf("veryvery") , size_t(0) );

	// Test tokenization where empty tokens should be returned, and there should
	// be one at the end of the string (because it ends with a token).
	String teststring5 = "a\nb\nc\n";
	StringArray tokenized5 = teststring5.tokenize("\n",  String::E_DISCARD_DELIMITERS, String::E_RETURN_EMPTY_TOKENS);
	unitAssertEquals( size_t(4), tokenized5.size() );
	unitAssertEquals( "a", tokenized5[0] );
	unitAssertEquals( "b", tokenized5[1] );
	unitAssertEquals( "c", tokenized5[2] );
	unitAssertEquals( "", tokenized5[3] );

	String teststring6 = "a\nb\nc\n\n";
	StringArray tokenized6 = teststring6.tokenize("\n",  String::E_DISCARD_DELIMITERS, String::E_RETURN_EMPTY_TOKENS);
	unitAssertEquals( size_t(5), tokenized6.size() );
	unitAssertEquals( "a", tokenized6[0] );
	unitAssertEquals( "b", tokenized6[1] );
	unitAssertEquals( "c", tokenized6[2] );
	unitAssertEquals( "", tokenized6[3] );
	unitAssertEquals( "", tokenized6[4] );
}

AUTO_UNIT_TEST(StringTestCases_testRealConstructors)
{
	unitAssert(String(Real32(-32897.238)).startsWith("-32897.238"));
	unitAssert(String(Real64(-32897.23828125)).startsWith("-32897.23828125"));
}

AUTO_UNIT_TEST(StringTestCases_test_cstr)
{
	// This tests for a bug we found and fixed that caused c_str() to behave
	// as if it were not const, resulting in COW problems.  The test mimics
	// the code that manifested the bug.
	String str("[$01 $][$02 $][$03 $][$04 $][$05 $][$06 $][$07 $][$08 $][$09 $][$10 $][$11 $][$12 $][$13 $][$14 $][$15 $][$17 $][$18 $][$19 $][$20 $]");
	char const * cstr = str.c_str();
	String scopy;
	scopy = str;
	char const * cstrcopy = str.c_str();
	unitAssertEquals(cstr , cstrcopy);
	unitAssertEquals(std::strcmp(cstr, "[$01 $][$02 $][$03 $][$04 $][$05 $][$06 $][$07 $][$08 $][$09 $][$10 $][$11 $][$12 $][$13 $][$14 $][$15 $][$17 $][$18 $][$19 $][$20 $]") , 0);
	scopy = str;
	cstrcopy = str.c_str();
	unitAssertEquals(cstr , cstrcopy);
	unitAssertEquals(std::strcmp(cstr, "[$01 $][$02 $][$03 $][$04 $][$05 $][$06 $][$07 $][$08 $][$09 $][$10 $][$11 $][$12 $][$13 $][$14 $][$15 $][$17 $][$18 $][$19 $][$20 $]") , 0);
}

AUTO_UNIT_TEST(StringTestCases_testIndexOf)
{
	String abc("abc");
	unitAssertEquals(1U, abc.indexOf("bc"));
	unitAssert(String::npos == abc.indexOf("bc", 5));
	unitAssert(String::npos == abc.indexOf("c", 3));
	unitAssertEquals(2U, abc.indexOf("c", 2));
	unitAssertEquals(1U, abc.indexOf("", 1));
	unitAssertEquals(0U, abc.indexOf(""));
	unitAssertEquals(1U, abc.indexOf(static_cast<const char*>(0), 1));
	unitAssertEquals(0U, abc.indexOf(static_cast<const char*>(0)));
}

AUTO_UNIT_TEST(StringTestCases_testFreeJoin)
{
	String teststring1 = "0.1.2.3.4";
	StringArray tokenized1 = teststring1.tokenize( "." );

	String s1 = StringJoin(tokenized1.begin(), tokenized1.end(), ".");
	unitAssertEquals(s1, teststring1);

	StringArray::iterator iter = tokenized1.begin();
	iter++;
	String s2 = StringJoin(tokenized1.begin(), iter, ", ");
	unitAssertEquals(s2, "0");

	iter++;
	String s3 = StringJoin(tokenized1.begin(), iter, ", ");
	unitAssertEquals(s3, "0, 1");

	String s4 = StringJoin(tokenized1.begin(), tokenized1.begin(), ", ");
	unitAssert(s4.empty());
}
