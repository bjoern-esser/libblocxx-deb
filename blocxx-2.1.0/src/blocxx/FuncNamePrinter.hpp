/*******************************************************************************
* Copyright (C) 2005, Vintela, Inc. All rights reserved.
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
*       Vintela, Inc., 
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

#ifndef BLOCXX_FUNC_NAME_PRINTER_HPP
#define BLOCXX_FUNC_NAME_PRINTER_HPP
#include "blocxx/BLOCXX_config.h"
#include "blocxx/Format.hpp"
#include <stdio.h>
#include <unistd.h>

// The classes and functions defined in this file are not meant for general
// use, they are internal implementation details.  They may change at any time.

// For printing function names during debug
#ifdef BLOCXX_PRINT_FUNC_DEBUG
#define PRINT_FUNC_NAME BLOCXX_FuncNamePrinter fnp##__LINE__ (__PRETTY_FUNCTION__)

#define PRINT_FUNC_NAME_ARGS1(a) BLOCXX_FuncNamePrinter fnp##__LINE__ ( __PRETTY_FUNCTION__ , a)
#define PRINT_FUNC_NAME_ARGS2(a, b) BLOCXX_FuncNamePrinter fnp##__LINE__ ( __PRETTY_FUNCTION__ , a, b)
#define PRINT_FUNC_NAME_ARGS3(a, b, c) BLOCXX_FuncNamePrinter fnp##__LINE__ ( __PRETTY_FUNCTION__ , a, b, c)
#define PRINT_FUNC_NAME_ARGS4(a, b, c, d) BLOCXX_FuncNamePrinter fnp##__LINE__ ( __PRETTY_FUNCTION__ , a, b, c, d)
#define PRINT_FUNC_NAME_ARGS5(a, b, c, d, e) BLOCXX_FuncNamePrinter fnp##__LINE__ ( __PRETTY_FUNCTION__ , a, b, c, d, e)
#define PRINT_FUNC_NAME_ARGS6(a, b, c, d, e, f) BLOCXX_FuncNamePrinter fnp##__LINE__ ( __PRETTY_FUNCTION__ , a, b, c, d, e, f)
#define PRINT_FUNC_NAME_ARGS7(a, b, c, d, e, f, g) BLOCXX_FuncNamePrinter fnp##__LINE__ ( __PRETTY_FUNCTION__ , a, b, c, d, e, f, g)
#define PRINT_FUNC_NAME_ARGS8(a, b, c, d, e, f, g, h) BLOCXX_FuncNamePrinter fnp##__LINE__ ( __PRETTY_FUNCTION__ , a, b, c, d, e, f, g, h)
#define PRINT_FUNC_NAME_ARGS9(a, b, c, d, e, f, g, h, i) BLOCXX_FuncNamePrinter fnp##__LINE__ ( __PRETTY_FUNCTION__ , a, b, c, d, e, f, g, h, i)
#define PRINT_FUNC_NAME_ARGS10(a, b, c, d, e, f, g, h, i, j) BLOCXX_FuncNamePrinter fnp##__LINE__ ( __PRETTY_FUNCTION__ , a, b, c, d, e, f, g, h, i, j)
#else
#define PRINT_FUNC_NAME_ARGS1(a)
#define PRINT_FUNC_NAME_ARGS2(a, b)
#define PRINT_FUNC_NAME_ARGS3(a, b, c)
#define PRINT_FUNC_NAME_ARGS4(a, b, c, d)
#define PRINT_FUNC_NAME_ARGS5(a, b, c, d, e)
#define PRINT_FUNC_NAME_ARGS6(a, b, c, d, e, f)
#define PRINT_FUNC_NAME_ARGS7(a, b, c, d, e, f, g)
#define PRINT_FUNC_NAME_ARGS8(a, b, c, d, e, f, g, h)
#define PRINT_FUNC_NAME_ARGS9(a, b, c, d, e, f, g, h, i)
#define PRINT_FUNC_NAME_ARGS10(a, b, c, d, e, f, g, h, i, j)
#define PRINT_FUNC_NAME
#endif /* #ifdef BLOCXX_PRINT_FUNC_DEBUG */

namespace BLOCXX_NAMESPACE
{

class FuncNamePrinter
{
private:
	const char* m_funcName;
public:
	
	FuncNamePrinter(const char* funcName) : m_funcName(funcName)
	{
		fprintf(stderr, "%d Entering: %s\n", getpid(), funcName);
	}
	template<typename A>
		FuncNamePrinter(const char* funcName, const A& a) : m_funcName(funcName)
		{
			fprintf(stderr, "%d Entering: %s\n\t%s\n", getpid(), funcName, 
				Format("(%1)", a).c_str());
		}
	template<typename A, typename B>
		FuncNamePrinter(const char* funcName, const A& a, const B& b) : m_funcName(funcName)
		{
			fprintf(stderr, "%d Entering: %s\n\t%s\n", getpid(), funcName, 
				Format("(%1, %2)", a, b).c_str());
		}
	template<typename A, typename B, typename C>
		FuncNamePrinter(const char* funcName, const A& a, const B& b, const C& c) : m_funcName(funcName)
		{
			fprintf(stderr, "%d Entering: %s\n\t%s\n", getpid(), funcName, 
				Format("(%1, %2, %3)", a, b, c).c_str());
		}
	template<typename A, typename B, typename C, typename D>
		FuncNamePrinter(const char* funcName, const A& a, const B& b, const C& c, const D& d) : m_funcName(funcName)
		{
			fprintf(stderr, "%d Entering: %s\n\t%s\n", getpid(), funcName, 
				Format("(%1, %2, %3, %4)", a, b, c, d).c_str());
		}
	template<typename A, typename B, typename C, typename D, typename E>
		FuncNamePrinter(const char* funcName, const A& a, const B& b, const C& c, const D& d, const E& e) : m_funcName(funcName)
		{
			fprintf(stderr, "%d Entering: %s\n\t%s\n", getpid(), funcName, 
				Format("(%1, %2, %3, %4, %5)", a, b, c, d, e).c_str());
		}
	template<typename A, typename B, typename C, typename D, typename E, typename F>
		FuncNamePrinter(const char* funcName, const A& a, const B& b, const C& c, const D& d, const E& e, const F& f) : m_funcName(funcName)
		{
			fprintf(stderr, "%d Entering: %s\n\t%s\n", getpid(), funcName, 
				Format("(%1, %2, %3, %4, %5, %6)", a, b, c, d, e, f).c_str());
		}
	template<typename A, typename B, typename C, typename D, typename E, typename F,
	typename G>
		FuncNamePrinter(const char* funcName, const A& a, const B& b, const C& c, const D& d, const E& e, const F& f, const G& g) : m_funcName(funcName)
		{
			fprintf(stderr, "%d Entering: %s\n\t%s\n", getpid(), funcName, 
				Format("(%1, %2, %3, %4, %5, %6, %7)", a, b, c, d, e, f, g).c_str());
		}
	template<typename A, typename B, typename C, typename D, typename E, typename F,
	typename G, typename H>
		FuncNamePrinter(const char* funcName, const A& a, const B& b, const C& c, const D& d, const E& e, const F& f, const G& g, const H& h) : m_funcName(funcName)
		{
			fprintf(stderr, "%d Entering: %s\n\t%s\n", getpid(), funcName, 
				Format("(%1, %2, %3, %4, %5, %6, %7, %8)", a, b, c, d, e, f, g, h).c_str());
		}
	template<typename A, typename B, typename C, typename D, typename E, typename F,
	typename G, typename H, typename I>
		FuncNamePrinter(const char* funcName, const A& a, const B& b, const C& c, const D& d, const E& e, const F& f, const G& g, const H& h, const I& i) : m_funcName(funcName)
		{
			fprintf(stderr, "%d Entering: %s\n\t%s\n", getpid(), funcName, 
				Format("(%1, %2, %3, %4, %5, %6, %7, %8, %9)", a, b, c, d, e, f, g, h, i).c_str());
		}
	template<typename A, typename B, typename C, typename D, typename E, typename F,
	typename G, typename H, typename I, typename J>
		FuncNamePrinter(const char* funcName, const A& a, const B& b, const C& c, const D& d, const E& e, const F& f, const G& g, const H& h, const I& i, const J& j) : m_funcName(funcName)
		{
			fprintf(stderr, "%d Entering: %s\n\t%s\n", getpid(), funcName, 
				Format("(%1, %2, %3, %4, %5, %6, %7, %8, %9)", a, b, c, d, e, f, g, h, Format("%1, %2", i, j)).c_str());
		}
	~FuncNamePrinter()
		{ fprintf(stderr, "%d Leaving:  %s\n", getpid(), m_funcName); }
}; // class FuncNamePrinter

} // end namespace BLOCXX_NAMESPACE

#endif //#define BLOCXX_FUNC_NAME_PRINTER_HPP
