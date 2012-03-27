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

#ifndef BLOCXX_SELECTABLE_CALLBACKIFC_HPP_INCLUDE_GUARD_
#define BLOCXX_SELECTABLE_CALLBACKIFC_HPP_INCLUDE_GUARD_
#include "blocxx/BLOCXX_config.h"
#include "blocxx/SelectableIFC.hpp"
#include "blocxx/IntrusiveCountableBase.hpp"
#include "blocxx/IntrusiveReference.hpp"

// The classes and functions defined in this file are not meant for general
// use, they are internal implementation details.  They may change at any time.

namespace BLOCXX_NAMESPACE
{

class BLOCXX_COMMON_API SelectableCallbackIFC : virtual public IntrusiveCountableBase
{
public:

	enum EEventType
	{
		// bits that can be or'd together
		E_READ_EVENT = 1,
		E_ACCEPT_EVENT = 1, // E_ACCEPT_EVENT aliases E_READ_EVENT due to select() semantics
		E_WRITE_EVENT = 2
	};

	virtual ~SelectableCallbackIFC();
	void selected(Select_t& selectedObject, EEventType eventType)
	{
		doSelected(selectedObject, eventType);
	}
protected:
	virtual void doSelected(Select_t& selectedObject, EEventType eventType) = 0;
};

typedef IntrusiveReference<SelectableCallbackIFC> SelectableCallbackIFCRef;

} // end namespace BLOCXX_NAMESPACE

#endif
