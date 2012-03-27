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

#ifndef BLOCXX_MAP_HPP_INCLUDE_GUARD_
#define BLOCXX_MAP_HPP_INCLUDE_GUARD_
#include "blocxx/BLOCXX_config.h"
#include "blocxx/COWReference.hpp"
#include <map>
#include <functional>

// This class is a wrapper around std::map<> and adds COW capabilities.
namespace BLOCXX_NAMESPACE
{


// forward declarations are necessary for template friends.
template<class Key, class T, class Compare > class Map;

template<class Key, class T, class Compare>
inline bool operator==(const Map<Key, T, Compare>& x,
	const Map<Key, T, Compare>& y);

template<class Key, class T, class Compare>
inline bool operator<(const Map<Key, T, Compare>& x,
	const Map<Key, T, Compare>& y);


template<class Key, class T, class Compare = std::less<Key> > class Map
{
	typedef std::map<Key, T, Compare > M;
	COWReference<M> m_impl;
public:
	typedef typename M::key_type key_type;
	typedef typename M::mapped_type mapped_type;
	typedef typename M::value_type value_type;
	typedef typename M::key_compare key_compare;
	typedef typename M::value_compare value_compare;
	typedef typename M::pointer pointer;
	typedef typename M::const_pointer const_pointer;
	typedef typename M::reference reference;
	typedef typename M::const_reference const_reference;
	typedef typename M::iterator iterator;
	typedef typename M::const_iterator const_iterator;
	typedef typename M::reverse_iterator reverse_iterator;
	typedef typename M::const_reverse_iterator const_reverse_iterator;
	typedef typename M::size_type size_type;
	typedef typename M::difference_type difference_type;
	Map() : m_impl(new M) {  }
	explicit Map(M* toWrap) : m_impl(toWrap)
		{ }
	explicit Map(const Compare& comp)
		: m_impl(new M(comp)) {  }
	template <class InputIterator>
	Map(InputIterator first, InputIterator last) :
		m_impl(new M(first, last))
	{
	}
	template <class InputIterator>
	Map(InputIterator first, InputIterator last, const Compare& comp) :
		m_impl(new M(first, last, comp))
	{
	}
	M* getImpl()
	{
		return &*m_impl;
	}
	key_compare key_comp() const
	{
		return m_impl->key_comp();
	}
	value_compare value_comp() const
	{
		return m_impl->value_comp();
	}
	iterator begin()
	{
		return m_impl->begin();
	}
	const_iterator begin() const
	{
		return m_impl->begin();
	}
	iterator end()
	{
		return m_impl->end();
	}
	const_iterator end() const
	{
		return m_impl->end();
	}
	reverse_iterator rbegin()
	{
		return m_impl->rbegin();
	}
	const_reverse_iterator rbegin() const
	{
		return m_impl->rbegin();
	}
	reverse_iterator rend()
	{
		return m_impl->rend();
	}
	const_reverse_iterator rend() const
	{
		return m_impl->rend();
	}
	bool empty() const
	{
		return m_impl->empty();
	}
	size_type size() const
	{
		return m_impl->size();
	}
	size_type max_size() const
	{
		return m_impl->max_size();
	}
	T& operator[](const key_type& k)
	{
		return m_impl->operator[](k);
	}
	void swap(Map<Key, T, Compare>& x)
	{
		m_impl.swap(x.m_impl);
	}
	std::pair<iterator, bool> insert(const value_type& x)
	{
		return m_impl->insert(x);
	}
	iterator insert(iterator position, const value_type& x)
	{
		return m_impl->insert(position, x);
	}
	template <class InputIterator>
	void insert(InputIterator first, InputIterator last)
	{
		m_impl->insert(first, last);
	}
	void erase(iterator position)
	{
		m_impl->erase(position);
	}
	size_type erase(const key_type& x)
	{
		return m_impl->erase(x);
	}
	void erase(iterator first, iterator last)
	{
		m_impl->erase(first, last);
	}
	void clear()
	{
		m_impl->clear();
	}
	iterator find(const key_type& x)
	{
		return m_impl->find(x);
	}
	const_iterator find(const key_type& x) const
	{
		return m_impl->find(x);
	}
	size_type count(const key_type& x) const
	{
		return m_impl->count(x);
	}
	iterator lower_bound(const key_type& x)
	{
		return m_impl->lower_bound(x);
	}
	const_iterator lower_bound(const key_type& x) const
	{
		return m_impl->lower_bound(x);
	}
	iterator upper_bound(const key_type& x)
	{
		return m_impl->upper_bound(x);
	}
	const_iterator upper_bound(const key_type& x) const
	{
		return m_impl->upper_bound(x);
	}
	std::pair<iterator, iterator> equal_range(const key_type& x)
	{
		return m_impl->equal_range(x);
	}
	std::pair<const_iterator, const_iterator>
		equal_range(const key_type& x) const
	{
		return m_impl->equal_range(x);
	}
	friend bool operator== <>(const Map<Key, T, Compare>& x,
		const Map<Key, T, Compare>& y);
	friend bool operator< <>(const Map<Key, T, Compare>& x,
		const Map<Key, T, Compare>& y);

	/** \example mapDemo.cpp */
};
template <class Key, class T, class Compare>
std::map<Key, T, Compare>* COWReferenceClone(std::map<Key, T, Compare>* obj)
{
	return new std::map<Key, T, Compare>(*obj);
}
template<class Key, class T, class Compare>
inline bool operator==(const Map<Key, T, Compare>& x,
	const Map<Key, T, Compare>& y)
{
	return *x.m_impl == *y.m_impl;
}
template<class Key, class T, class Compare>
inline bool operator<(const Map<Key, T, Compare>& x,
	const Map<Key, T, Compare>& y)
{
	return *x.m_impl < *y.m_impl;
}
template <class Key, class T, class Compare>
inline void swap(Map<Key, T, Compare>& x,
	Map<Key, T, Compare>& y)
{
	x.swap(y);
}

} // end namespace BLOCXX_NAMESPACE

#endif
