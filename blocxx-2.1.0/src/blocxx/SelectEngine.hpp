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

#ifndef BLOCXX_SELECT_ENGINE_HPP_
#define BLOCXX_SELECT_ENGINE_HPP_
#include "blocxx/BLOCXX_config.h"
#include "blocxx/SelectableIFC.hpp"
#include "blocxx/SelectableCallbackIFC.hpp"
#include "blocxx/SortedVectorMap.hpp"
#include "blocxx/Exception.hpp"

// The classes and functions defined in this file are not meant for general
// use, they are internal implementation details.  They may change at any time.

namespace BLOCXX_NAMESPACE
{

BLOCXX_DECLARE_APIEXCEPTION(Select, BLOCXX_COMMON_API)
class BLOCXX_COMMON_API SelectEngine
{
public:
	/**
	 * Add an object to the set.
	 * If obj is already added, the previous values for cb and eventType will be overwritten
	 */
	void addSelectableObject(const Select_t& obj,
		const SelectableCallbackIFCRef& cb, SelectableCallbackIFC::EEventType eventType);
	/// return indicates if obj was removed
	bool removeSelectableObject(const Select_t& obj, SelectableCallbackIFC::EEventType eventType);
	void go(const Timeout& timeout); // Throws SelectException on error
	void stop();
private:

	struct Data
	{
		Data(const SelectableCallbackIFCRef& callback_,
			SelectableCallbackIFC::EEventType eventType_)
			: callback(callback_)
			, eventType(eventType_)
		{
		}

		SelectableCallbackIFCRef callback;
		SelectableCallbackIFC::EEventType eventType;

	};

	SortedVectorMap<Select_t, Data> m_table; // the key is Data::selectObj->getSelectObj()

	bool m_stopFlag;
};

class BLOCXX_COMMON_API SelectEngineStopper : public SelectableCallbackIFC
{
public:
	SelectEngineStopper(SelectEngine& engine);
protected:
	virtual void doSelected(Select_t& selectedObject, EEventType eventType);
private:
	SelectEngine& m_engine;
};

} // end namespace BLOCXX_NAMESPACE

#endif
