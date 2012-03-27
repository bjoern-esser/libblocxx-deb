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


#ifndef BLOCXX_DESCRIPTOR_UTILS_NOEXCEPT_HPP_INCLUDE_GUARD_
#define BLOCXX_DESCRIPTOR_UTILS_NOEXCEPT_HPP_INCLUDE_GUARD_

/**
 * @author Kevin Van Horn
 * @author Dan Nuffer
 */

#include "blocxx/BLOCXX_config.h"
#include "blocxx/Types.hpp"
#include "blocxx/AutoDescriptor.hpp"

namespace BLOCXX_NAMESPACE
{

/**
 * Sends a @c Descriptor to the peer.  Guaranteed not to throw an exception.
 * @param descriptor The @c Descriptor to send.
 * @return -1 on I/O error. errno will indicate the specific error.

 */
int passDescriptor(Descriptor streamPipe, Descriptor descriptor, ProcId targetProcess = 0);

/**
 * Gets a @c Descriptor from the peer.  Guaranteed not to throw an exception.
 *
 * @pre @a errbuf is an array of at least @a bufsz elements.
 *
 * @return The @c Descriptor sent by the peer, if there is no error.
 * On I/O error, or if the peer does not send a @c Descriptor, returns -1
 * and writes a null-terminated error message into @a errbuf.
 */
AutoDescriptor receiveDescriptor(Descriptor streamPipe, char * errbuf, size_t bufsz);

} // end namespace BLOCXX_NAMESPACE


#endif



