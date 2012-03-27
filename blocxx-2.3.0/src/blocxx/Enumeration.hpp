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
 * @author Dan Nuffer
 */

#ifndef BLOCXX_ENUMERATION_HPP_INCLUDE_GUARD_
#define BLOCXX_ENUMERATION_HPP_INCLUDE_GUARD_
#include "blocxx/BLOCXX_config.h"
#include "blocxx/TempFileEnumerationImplBase.hpp"
#include "blocxx/IntrusiveReference.hpp"

#include <iterator> // for the iterator tags
#include <cstddef> // for ptrdiff_t

namespace BLOCXX_NAMESPACE
{

template <class T>
class TempFileEnumerationImpl : public TempFileEnumerationImplBase
{
public:
	TempFileEnumerationImpl()
	{
	}
	virtual ~TempFileEnumerationImpl()
	{
	}
	void nextElement(T& out)
	{
		throwIfEmpty();
		out.readObject(*m_Data.rdbuf());
		--m_size;
	}
	T nextElement()
	{
		throwIfEmpty();
		T retval;
		retval.readObject(*m_Data.rdbuf());
		--m_size;
		return retval;
	}
	void addElement( const T& arg )
	{
		arg.writeObject(*m_Data.rdbuf());
		++m_size;
	}
private:
	// Prevent copying or assignment
	TempFileEnumerationImpl( const TempFileEnumerationImpl<T>& );
	TempFileEnumerationImpl<T>& operator=( const TempFileEnumerationImpl<T>& );
};

template <class T>
class Enumeration
{
public:
	Enumeration()
	: m_impl( new TempFileEnumerationImpl<T> )
	{
	}
	bool hasMoreElements() const
	{
		return m_impl->hasMoreElements();
	}
	void nextElement(T& arg)
	{
		m_impl->nextElement(arg);
	}
	T nextElement()
	{
		return m_impl->nextElement();
	}
	size_t numberOfElements() const
	{
		return m_impl->numberOfElements();
	}
	void addElement(const T& arg)
	{
		m_impl->addElement(arg);
	}
	void clear()
	{
		m_impl->clear();
	}
	bool usingTempFile() const
	{
		return m_impl->usingTempFile();
	}
private:
	IntrusiveReference< TempFileEnumerationImpl<T> > m_impl;
};

template <class T>
class Enumeration_input_iterator
{
public:
	typedef Enumeration<T> enumeration_type;
	typedef std::input_iterator_tag iterator_category;
	typedef T value_type;
	typedef const T* pointer;
	typedef const T& reference;
	typedef ptrdiff_t difference_type;
	Enumeration_input_iterator() : m_enumeration(0), m_ok(false)
	{
	}
	Enumeration_input_iterator(enumeration_type& e) : m_enumeration(&e)
	{
		m_read();
	}

	// compiler generated copy ctor is what we want.
	// compiler generated copy-assignment operator= is what we want.

	reference operator*() const
	{
		return m_value;
	}
	pointer operator->() const
	{
		return &(operator*());
	}
	Enumeration_input_iterator& operator++()
	{
		m_read();
		return *this;
	}
	Enumeration_input_iterator operator++(int)
	{
		Enumeration_input_iterator tmp = *this;
		m_read();
		return tmp;
	}
	bool m_equal(const Enumeration_input_iterator& x) const
	{
		return(m_ok == x.m_ok) && (!m_ok || m_enumeration == x.m_enumeration);
	}
private:
	enumeration_type* m_enumeration;
	T m_value;
	bool m_ok;
	void m_read()
	{
		m_ok = (m_enumeration && m_enumeration->hasMoreElements()) ? true : false;
		if (m_ok)
		{
			m_enumeration->nextElement(m_value);
		}
	}
};

template <class T>
inline bool
operator==(const Enumeration_input_iterator<T>& x,
	const Enumeration_input_iterator<T>& y)
{
	return x.m_equal(y);
}

template <class T>
inline bool
operator!=(const Enumeration_input_iterator<T>& x,
	const Enumeration_input_iterator<T>& y)
{
	return !x.m_equal(y);
}

template <class T>
class Enumeration_insert_iterator
{
public:
	typedef Enumeration<T> enumeration_type;
	typedef std::output_iterator_tag            iterator_category;
	typedef void                           value_type;
	typedef void                           difference_type;
	typedef void                           pointer;
	typedef void                           reference;
	Enumeration_insert_iterator(enumeration_type& e) : m_enumeration(&e)
	{
	}
	Enumeration_insert_iterator<T>& operator=(const T& value)
	{
		m_enumeration->addElement(value);
		return *this;
	}
	Enumeration_insert_iterator<T>& operator*()
	{
		return *this;
	}
	Enumeration_insert_iterator<T>& operator++()
	{
		return *this;
	}
	Enumeration_insert_iterator<T>& operator++(int)
	{
		return *this;
	}
private:
	enumeration_type* m_enumeration;
};

template <class Container>
inline Enumeration_insert_iterator<Container> Enumeration_inserter(Enumeration<Container>& x)
{
	return Enumeration_insert_iterator<Container>(x);
}

} // end namespace BLOCXX_NAMESPACE

#endif
