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

#ifndef BLOCXX_SHARED_LIBRARY_REFERENCE_HPP_
#define BLOCXX_SHARED_LIBRARY_REFERENCE_HPP_
#include "blocxx/BLOCXX_config.h"
#include "blocxx/SharedLibrary.hpp"

namespace BLOCXX_NAMESPACE
{

template <class T>
class SharedLibraryReference
{
public:
	typedef T element_type;

	SharedLibraryReference(const SharedLibraryRef& lib, const T& obj)
	: m_sharedLib(lib), m_obj(obj)
	{}
	SharedLibraryReference(const SharedLibraryRef& lib, typename T::element_type* obj)
	: m_sharedLib(lib), m_obj(T(obj))
	{}
	SharedLibraryReference(const SharedLibraryReference<T>& arg)
	: m_sharedLib(arg.m_sharedLib), m_obj(arg.m_obj)
	{
	}
	/* construct out of a reference to a derived type.  U should be
	derived from T */
	template <class U>
	SharedLibraryReference(const SharedLibraryReference<U>& arg)
	: m_sharedLib(arg.m_sharedLib), m_obj(arg.m_obj)
	{
	}
	SharedLibraryReference()
	: m_sharedLib(), m_obj()
	{}
	SharedLibraryReference<T>& operator=(const SharedLibraryReference<T>& arg)
	{
		m_obj = arg.m_obj;
		m_sharedLib = arg.m_sharedLib;
		return *this;
	}
	~SharedLibraryReference()
	{
		try
		{
			m_obj = 0;
			m_sharedLib = 0;
		}
		catch (...)
		{
			// don't let exceptions escape
		}
	}
	SharedLibraryRef getLibRef() const
	{
		return m_sharedLib;
	}
	typename T::element_type* operator->() const
	{
		return &*m_obj;
	}
	T get() const
	{
		return m_obj;
	}
	
	typedef T SharedLibraryReference::*safe_bool;
	operator safe_bool () const
		{  return (m_obj) ? &SharedLibraryReference::m_obj : 0; }
	bool operator!() const
		{  return !m_obj; }
	void setNull()
	{
		m_obj = 0;
		m_sharedLib = 0;
	}
	
	template <class U>
	SharedLibraryReference<U> cast_to() const
	{
		SharedLibraryReference<U> rval;
		rval.m_sharedLib = m_sharedLib;
		rval.m_obj = m_obj.cast_to<U>();
		return rval;
	}

	BLOCXX_DEPRECATED bool isNull() const // in 3.1.0
	{
		return !m_obj;
	}

#if !defined(__GNUC__) || __GNUC__ > 2 // causes gcc 2.95 to ICE
	/* This is so cast_to will work */
	template <class U> friend class SharedLibraryReference;

private:
#endif

	SharedLibraryRef m_sharedLib;
	T m_obj;
};

} // end namespace BLOCXX_NAMESPACE

#endif
