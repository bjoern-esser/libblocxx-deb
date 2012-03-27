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
 * @author Jon Carey
 * @author Dan Nuffer
 */

#include "blocxx/BLOCXX_config.h"
#include "blocxx/SelectEngine.hpp"
#include "blocxx/Select.hpp"
#include "blocxx/ExceptionIds.hpp"
#include "blocxx/Timeout.hpp"
#include "blocxx/TimeoutTimer.hpp"

namespace BLOCXX_NAMESPACE
{

BLOCXX_DEFINE_EXCEPTION_WITH_ID(Select);
//////////////////////////////////////////////////////////////////////////////
void
SelectEngine::addSelectableObject(const Select_t& obj,
	const SelectableCallbackIFCRef& cb, SelectableCallbackIFC::EEventType eventType)
{
	m_table.insert(std::make_pair(obj, Data(cb, eventType)));
}

//////////////////////////////////////////////////////////////////////////////
bool 
SelectEngine::removeSelectableObject(const Select_t& obj, SelectableCallbackIFC::EEventType eventType)
{
	return m_table.erase(obj);
}

//////////////////////////////////////////////////////////////////////////////
void
SelectEngine::go(const Timeout& timeout)
{
	TimeoutTimer timer(timeout);
	timer.start();
	m_stopFlag = false;
	do
	{
		Select::SelectObjectArray selObjs;
		typedef SortedVectorMap<Select_t, Data>::const_iterator citer_t;
		for (citer_t iter = m_table.begin(); iter != m_table.end(); ++iter)
		{
			Select::SelectObject so(iter->first);
			if (iter->second.eventType & SelectableCallbackIFC::E_READ_EVENT)
			{
				so.waitForRead = true;
			}
			if (iter->second.eventType & SelectableCallbackIFC::E_WRITE_EVENT)
			{
				so.waitForWrite = true;
			}
			selObjs.push_back(so);
		}
		
		if (selObjs.empty())
		{
			return;
		}

		int selected = Select::selectRW(selObjs, timer.asTimeout());
		if (selected == Select::SELECT_ERROR)
		{
			BLOCXX_THROW_ERRNO_MSG(SelectException, "Select Error");
		}
		else if (selected == Select::SELECT_TIMEOUT)
		{
			BLOCXX_THROW(SelectException, "Select Timeout");
		}
		else
		{
			for (size_t i = 0; i < selObjs.size() && selected > 0; ++i)
			{
				const Select::SelectObject& selObj(selObjs[i]);
				if (selObj.readAvailable || selObj.writeAvailable)
				{
					--selected;
					typedef SortedVectorMap<Select_t, Data>::iterator iter_t;
					iter_t iter = m_table.find(selObj.s);
					if (selObj.readAvailable)
					{
						iter->second.callback->selected(iter->first, SelectableCallbackIFC::E_READ_EVENT);
					}
					if (selObj.writeAvailable)
					{
						iter->second.callback->selected(iter->first, SelectableCallbackIFC::E_WRITE_EVENT);
					}
				}
			}
		}
		timer.resetOnLoop();
	} while (!m_stopFlag);
}
//////////////////////////////////////////////////////////////////////////////
void
SelectEngine::stop()
{
	m_stopFlag = true;
}

//////////////////////////////////////////////////////////////////////////////
SelectEngineStopper::SelectEngineStopper(SelectEngine& engine)
	: SelectableCallbackIFC()
	, m_engine(engine)
{
}

//////////////////////////////////////////////////////////////////////////////
void 
SelectEngineStopper::doSelected(Select_t& selectedObject, EEventType eventType)
{
	m_engine.stop();
}
	
} // end namespace BLOCXX_NAMESPACE

