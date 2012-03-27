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

#ifndef BLOCXX_HASH_MULTI_MAP_HPP_INCLUDE_GUARD_
#define BLOCXX_HASH_MULTI_MAP_HPP_INCLUDE_GUARD_
#include "blocxx/BLOCXX_config.h"
#ifdef BLOCXX_HAVE_HASH_MAP
	#include <hash_map>
	#define BLOCXX_HASH_MAP_NS std
	#define HashMultiMap std::hash_multimap
#elif BLOCXX_HAVE_EXT_HASH_MAP
	#include <ext/hash_map>
	#define BLOCXX_HASH_MAP_NS __gnu_cxx
	#define HashMultiMap __gnu_cxx::hash_multimap
#else
	// TODO: Write a real hash multi map
	#include <map>
	#define BLOCXX_HASH_MAP_NS std
	#define HashMultiMap std::multimap
#endif

#ifndef BLOCXX_HASH_SPECIALIZED_INCLUDE_GUARD_
#define BLOCXX_HASH_SPECIALIZED_INCLUDE_GUARD_
#if defined(BLOCXX_HAVE_HASH_MAP) || defined(BLOCXX_HAVE_EXT_HASH_MAP)
#include "blocxx/String.hpp"

// need to specialize hash
namespace BLOCXX_HASH_MAP_NS
{
template<> struct hash<BLOCXX_NAMESPACE::String>
{
	size_t operator()(const BLOCXX_NAMESPACE::String& s) const
	{
		return hash<const char*>()(s.c_str());
	}
};
}

#endif
#endif
#undef BLOCXX_HASH_MAP_NS
#endif
