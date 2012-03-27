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


#include "TestSuite.hpp"
#include "TestCaller.hpp"
#include "MD5TestCases.hpp"
#include "blocxx/MD5.hpp"
#include "blocxx/MD5.hpp"
#include "blocxx/String.hpp"


using namespace blocxx;

void MD5TestCases::setUp()
{
}

void MD5TestCases::tearDown()
{
}

void MD5TestCases::testSomething()
{
	MD5 md5("");
	unitAssert(md5.toString().equals("d41d8cd98f00b204e9800998ecf8427e"));

	md5.init("a");
	unitAssert(md5.toString().equals("0cc175b9c0f1b6a831c399e269772661"));

	md5.init("abc");
	unitAssert(md5.toString().equals("900150983cd24fb0d6963f7d28e17f72"));

	md5.init("");
	md5.update("a");
	md5.update("b");
	md5.update("c");
	unitAssert(md5.toString().equals("900150983cd24fb0d6963f7d28e17f72"));

	md5.init("message digest");
	unitAssert(md5.toString().equals("f96b697d7cb7938d525a2f31aaf161d0"));
	unitAssert(md5.toString().equals("f96b697d7cb7938d525a2f31aaf161d0"));

	md5.init("abcdefghijklmnopqrstuvwxyz");
	unitAssert(md5.toString().equals("c3fcd3d76192e4007dfb496cca67e13b"));
	
	md5.init("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
	unitAssert(md5.toString().equals("d174ab98d277d9f5a5611c2c9f419d9f"));

	md5.init("1234567890123456789012345678901234567890"
			"1234567890123456789012345678901234567890");
	unitAssert(md5.toString().equals("57edf4a22be3c955ac49da2e2107b67a"));

	// Now test ostream stuff. 
	md5.init("");
	unitAssert(md5.toString().equals("d41d8cd98f00b204e9800998ecf8427e"));

	md5.init("");
	md5 << "a";
	unitAssert(md5.toString().equals("0cc175b9c0f1b6a831c399e269772661"));

	md5.init("");
	md5 << "abc";
	unitAssert(md5.toString().equals("900150983cd24fb0d6963f7d28e17f72"));

	md5.init("");
	md5 << "a" << "b" << "c";
	unitAssert(md5.toString().equals("900150983cd24fb0d6963f7d28e17f72"));

	md5.init("");
	md5 << "message digest";
	unitAssert(md5.toString().equals("f96b697d7cb7938d525a2f31aaf161d0"));
	unitAssert(md5.toString().equals("f96b697d7cb7938d525a2f31aaf161d0"));

	md5.init("");
	md5 << "abcdefghijklmnopqrstuvwxyz";
	unitAssert(md5.toString().equals("c3fcd3d76192e4007dfb496cca67e13b"));
	
	md5.init("");
	md5 << "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
	unitAssert(md5.toString().equals("d174ab98d277d9f5a5611c2c9f419d9f"));

	md5.init("");
	md5 << "1234567890123456789012345678901234567890"
		<<	"1234567890123456789012345678901234567890";
	unitAssert(md5.toString().equals("57edf4a22be3c955ac49da2e2107b67a"));

	// test formatting
	md5.init("12345");
	String s1 = md5.toString();
	md5.init("");
	md5 << 12345;
	unitAssert(md5.toString().equals(s1));

}
/*
A.5 Test suite

	The MD5 test suite (driver option "-x") should print the following
	results:

MD5 test suite:
MD5 ("") = d41d8cd98f00b204e9800998ecf8427e
MD5 ("a") = 0cc175b9c0f1b6a831c399e269772661
MD5 ("abc") = 900150983cd24fb0d6963f7d28e17f72
MD5 ("message digest") = f96b697d7cb7938d525a2f31aaf161d0
MD5 ("abcdefghijklmnopqrstuvwxyz") = c3fcd3d76192e4007dfb496cca67e13b
MD5 ("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789") =
d174ab98d277d9f5a5611c2c9f419d9f
MD5 ("123456789012345678901234567890123456789012345678901234567890123456
78901234567890") = 57edf4a22be3c955ac49da2e2107b67a
*/

Test* MD5TestCases::suite()
{
	TestSuite *testSuite = new TestSuite ("MD5");

	testSuite->addTest (new TestCaller <MD5TestCases> 
			("testSomething", 
			&MD5TestCases::testSomething));

	return testSuite;
}

