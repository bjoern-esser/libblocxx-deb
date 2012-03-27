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
 * @author Dan Nuffer
 */

#ifndef BLOCXX_FORMAT_HPP
#define BLOCXX_FORMAT_HPP
#include "blocxx/BLOCXX_config.h"
#include <iosfwd>
#include "blocxx/StringStream.hpp"
#include "blocxx/String.hpp"

namespace BLOCXX_NAMESPACE
{

//  Format class declaration  -----------------------------------------------//
class BLOCXX_COMMON_API Format
{
public:
	
	operator String() const;
	String toString() const;
	const char* c_str() const;
	// generic templated constructors
	template<typename A>
	Format(const char* ca, const A& a);
	template<typename A, typename B>
	Format(const char* ca, const A& a, const B& b);
	template<typename A, typename B, typename C>
	Format(const char* ca, const A& a, const B& b, const C& c);
	template<typename A, typename B, typename C, typename D>
	Format(const char* ca, const A& a, const B& b, const C& c, const D& d);
	template<typename A, typename B, typename C, typename D, typename E>
		Format(const char* ca, const A& a, const B& b, const C& c, const D& d, const E& e);
	template<typename A, typename B, typename C, typename D, typename E, typename F>
	Format(const char* ca, const A& a, const B& b, const C& c, const D& d, const E& e, const F& f);
	template<typename A, typename B, typename C, typename D, typename E, typename F, typename G>
	Format(const char* ca, const A& a, const B& b, const C& c, const D& d, const E& e, const F& f, const G& g);
	template<typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H>
	Format(const char* ca, const A& a, const B& b, const C& c, const D& d, const E& e, const F& f, const G& g, const H& h);
	template<typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I>
	Format(const char* ca, const A& a, const B& b, const C& c, const D& d, const E& e, const F& f, const G& g, const H& h, const I& i);
	// These specific versions are to help prevent template bloat
	Format(const char* ca, const String& a);
	Format(const char* ca, const String& a, const String& b);
	Format(const char* ca, const String& a, const String& b, const String& c);
private:
	OStringStream oss;
	char process(String& f, char c0);
	template<typename T> void put(const T& t);
	// These are to help prevent template bloat
	void put (const String& t);
	void put (char t);
	void put (unsigned char t);
	void put (short t);
	void put (unsigned short t);
	void put (int t);
	void put (unsigned int t);
	void put (long t);
	void put (unsigned long t);
	void put (long long t);
	void put (unsigned long long t);
public:
	friend BLOCXX_COMMON_API std::ostream& operator<<(std::ostream& os, const Format& f);
}; // class Format

template<typename T>
void Format::put(const T& t)
{ // t is inserted into oss
	if (!oss.good())
		return;
	oss << t;
}

template<typename A>
Format::Format(const char* ca, const A& a) : oss()
{
	String fmt(ca);
	while (!fmt.empty())
	{
		switch (process(fmt, '1'))
		{
			case '1': put(a); break;
		}
	}
}
template<typename A, typename B>
Format::Format(const char* ca, const A& a, const B& b) : oss()
{
	String fmt(ca);
	while (!fmt.empty())
	{
		switch (process(fmt, '2'))
		{
			case '1': put(a); break;
			case '2': put(b); break;
		}
	}
}
template<typename A, typename B, typename C>
Format::Format(const char* ca, const A& a, const B& b, const C& c) : oss()
{
	String fmt(ca);
	while (!fmt.empty())
	{
		switch (process(fmt, '3'))
		{
			case '1': put(a); break;
			case '2': put(b); break;
			case '3': put(c); break;
		}
	}
}
template<typename A, typename B, typename C, typename D>
Format::Format(const char* ca, const A& a, const B& b, const C& c, const D& d) : oss()
{
	String fmt(ca);
	while (!fmt.empty())
	{
		switch (process(fmt, '4'))
		{
			case '1': put(a); break;
			case '2': put(b); break;
			case '3': put(c); break;
			case '4': put(d); break;
		}
	}
}
template<typename A, typename B, typename C, typename D, typename E>
Format::	Format(const char* ca, const A& a, const B& b, const C& c, const D& d, const E& e) : oss()
{
	String fmt(ca);
	while (!fmt.empty())
	{
		switch (process(fmt, '5'))
		{
			case '1': put(a); break;
			case '2': put(b); break;
			case '3': put(c); break;
			case '4': put(d); break;
			case '5': put(e); break;
		}
	}
}
template<typename A, typename B, typename C, typename D, typename E, typename F>
Format::Format(const char* ca, const A& a, const B& b, const C& c, const D& d, const E& e, const F& f) : oss()
{
	String fmt(ca);
	while (!fmt.empty())
	{
		switch (process(fmt, '6'))
		{
			case '1': put(a); break;
			case '2': put(b); break;
			case '3': put(c); break;
			case '4': put(d); break;
			case '5': put(e); break;
			case '6': put(f); break;
		}
	}
}
template<typename A, typename B, typename C, typename D, typename E, typename F, typename G>
Format::Format(const char* ca, const A& a, const B& b, const C& c, const D& d, const E& e, const F& f, const G& g) : oss()
{
	String fmt(ca);
	while (!fmt.empty())
	{
		switch (process(fmt, '7'))
		{
			case '1': put(a); break;
			case '2': put(b); break;
			case '3': put(c); break;
			case '4': put(d); break;
			case '5': put(e); break;
			case '6': put(f); break;
			case '7': put(g); break;
		}
	}
}
template<typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H>
Format::Format(const char* ca, const A& a, const B& b, const C& c, const D& d, const E& e, const F& f, const G& g, const H& h) : oss()
{
	String fmt(ca);
	while (!fmt.empty())
	{
		switch (process(fmt, '8'))
		{
			case '1': put(a); break;
			case '2': put(b); break;
			case '3': put(c); break;
			case '4': put(d); break;
			case '5': put(e); break;
			case '6': put(f); break;
			case '7': put(g); break;
			case '8': put(h); break;
		}
	}
}
template<typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I>
Format::Format(const char* ca, const A& a, const B& b, const C& c, const D& d, const E& e, const F& f, const G& g, const H& h, const I& i) : oss()
{
	String fmt(ca);
	while (!fmt.empty())
	{
		switch (process(fmt, '9'))
		{
			case '1': put(a); break;
			case '2': put(b); break;
			case '3': put(c); break;
			case '4': put(d); break;
			case '5': put(e); break;
			case '6': put(f); break;
			case '7': put(g); break;
			case '8': put(h); break;
			case '9': put(i); break;
		}
	}
}

} // end namespace BLOCXX_NAMESPACE

#endif

