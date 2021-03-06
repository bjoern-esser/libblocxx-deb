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

#ifndef BLOCXX_MEM_TRACER_HPP_INCLUDE_GUARD_
#define BLOCXX_MEM_TRACER_HPP_INCLUDE_GUARD_
#ifdef __cplusplus
#ifdef BLOCXX_DEBUG_MEMORY

#include "blocxx/BLOCXX_config.h"

#include <new> // for std::bad_alloc

void* operator new(std::size_t size) throw (std::bad_alloc);
void* operator new[](std::size_t size) throw (std::bad_alloc);
void* operator new(std::size_t size, char const* file, int line) throw (std::bad_alloc);
void* operator new[](std::size_t size, char const* file, int line) throw (std::bad_alloc);
void operator delete(void* p);
void operator delete[](void* p);
#ifndef BLOCXX_MEM_TRACER_CPP_INCLUDE_GUARD_
// This is too problematic.
//#define BLOCXX_NEW new(__FILE__, __LINE__)
//#define new BLOCXX_NEW
#endif
#endif	// BLOCXX_DEBUG_MEMORY
#endif // __cplusplus
#endif
