/*******************************************************************************
* Copyright (C) 2004 Quest Software, Inc. All rights reserved.
* Copyright (C) 2005 Novell, Inc. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of source code must retain the above copyright notice,
*    this list of conditions and the following disclaimer.
*
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
*
*  - Neither the name of Quest Software, Inc., Novell, Inc., nor the names of its
*    contributors may be used to endorse or promote products derived from this
*    software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL Quest Software, Inc., Novell, Inc., OR THE
* CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
* OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/


/**
 * @author Jon Carey
 * @author Dan Nuffer
 */

#ifndef BLOCXX_LIST_HPP_INCLUDE_GUARD_
#define BLOCXX_LIST_HPP_INCLUDE_GUARD_
#include "blocxx/BLOCXX_config.h"
#include "blocxx/COWReference.hpp"
#include <list>

namespace BLOCXX_NAMESPACE
{

// forward declarations are necessary for template friends.
template<class T> class List;

template <class T>
inline bool operator==(const List<T>& x, const List<T>& y);

template <class T>
inline bool operator<(const List<T>& x, const List<T>& y);


/**
 * This class is a wrapper around std::list<> and adds COW capabilities.
 */
template<class T> class List
{
private:
	typedef std::list<T> L;
	COWReference<L> m_impl;
public:
	typedef typename L::value_type value_type;
	typedef typename L::pointer pointer;
	typedef typename L::const_pointer const_pointer;
	typedef typename L::reference reference;
	typedef typename L::const_reference const_reference;
	typedef typename L::size_type size_type;
	typedef typename L::difference_type difference_type;
	typedef typename L::iterator iterator;
	typedef typename L::const_iterator const_iterator;
	typedef typename L::reverse_iterator reverse_iterator;
	typedef typename L::const_reverse_iterator const_reverse_iterator;

	/**
	 * Default Constructor
	 */
	List() : m_impl(new L) {}
	/**
	 * Constructor
	 * @param toWrap The std::list to wrap with this List object.
	 */
	explicit List(L* toWrap) : m_impl(toWrap)
	{
	}
	/**
	 * Construct a List from a range specified with InputIterators.
	 * @param first The iterator for the beginning of the range.
	 * @param last The iterator for the end of the range.
	 */
	template<class InputIterator>
	List(InputIterator first, InputIterator last)
		: m_impl(new L(first, last))
	{
	}
	/**
	 * Construct a List that consist of a specified number of elements
	 * that are copies of a given object.
	 * @param n Number of elements the List will contain.
	 * @param value The value every element of the List will be
	 *              initialized to.
	 */
	List(size_type n, const T& value) : m_impl(new L(n, value))
	{
	}
	/**
	 * Construct a List that consist of a specified number of elements
	 * that are copies of a given object.
	 * @param n Number of elements the List will contain.
	 * @param value The value every element of the List will be
	 *              initialized to.
	 */
	List(int n, const T& value) : m_impl(new L(n, value))
	{
	}
	/**
	 * Construct a List that consist of a specified number of elements
	 * that are copies of a given object.
	 * @param n Number of elements the List will contain.
	 * @param value The value every element of the List will be
	 *              initialized to.
	 */
	List(long n, const T& value) : m_impl(new L(n, value))
	{
	}
	/**
	 * Construct a List that consist of a specified number of elements
	 * that have be constructed using the default constructor of class T.
	 * @param n Number of elements the List will contain.
	 */
	explicit List(size_type n) : m_impl(new L(n))
	{
	}
	/**
	 * @doctodo
	 * @return Pointer to the std::list object used to implement the List class.
	 */
	L* getImpl()
	{
		return &*m_impl;
	}
	/**
	 * @return A read/write iterator that points to the first element
	 *         in the List. Iteration is done in the normal order
	 *         (1st to last) with the returned iterator.
	 */
	iterator begin()
	{
		return m_impl->begin();
	}
	/**
	 * @return A read only iterator that points to the first element
	 *         in the List. Iteration is done in the normal order
	 *         (1st to last) with the returned iterator.
	 */
	const_iterator begin() const
	{
		return m_impl->begin();
	}
	/**
	 * @return A read/write iterator that points to one past the last
	 *         element in the List. Iteration is done in the normal
	 *         order (1st to last) with the returned iterator.
	 */
	iterator end()
	{
		return m_impl->end();
	}
	/**
	 * @return A read only iterator that points to one past the last
	 *         element in the List. Iteration is done in the normal
	 *         order (1st to last) with the returned iterator.
	 */
	const_iterator end() const
	{
		return m_impl->end();
	}
	/**
	 * @return A read/write reverse iterator that points to the last
	 *         element in the List. Iteration is done in the reverse
	 *         order (last to 1st) with the returned iterator.
	 */
	reverse_iterator rbegin()
	{
		return m_impl->rbegin();
	}
	/**
	 * @return A read only reverse iterator that points to the last
	 *         element in the List. Iteration is done in the reverse
	 *         order (last to 1st) with the returned iterator.
	 */
	const_reverse_iterator rbegin() const
	{
		return m_impl->rbegin();
	}
	/**
	 * @return A read/write reverse iterator that points to one before
	 *         the first element in the List. Iteration is done in the
	 *         reverse order (last to 1st) with the returned iterator.
	 */
	reverse_iterator rend()
	{
		return m_impl->rend();
	}
	/**
	 * @return A read only reverse iterator that points to one before
	 *         the first element in the List. Iteration is done in the
	 *         reverse order (last to 1st) with the returned iterator.
	 */
	const_reverse_iterator rend() const
	{
		return m_impl->rend();
	}
	/**
	 * @return true if the List is empty (contains zero elements)
	 */
	bool empty() const
	{
		return m_impl->empty();
	}
	/**
	 * @return The number of elements in the List.
	 */
	size_type size() const
	{
		return m_impl->size();
	}
	/**
	 * @return The maximal number of elements the List can hold.
	 */
	size_type max_size() const
	{
		return m_impl->max_size();
	}
	/**
	 * @return A read/write reference to the first element in the List.
	 */
	reference front()
	{
		return m_impl->front();
	}
	/**
	 * @return A read only reference to the first element in the List.
	 */
	const_reference front() const
	{
		return m_impl->front();
	}
	/**
	 * @return A read/write reference to the last element in the List.
	 */
	reference back()
	{
		return m_impl->back();
	}
	/**
	 * @return A read only reference to the last element in the List.
	 */
	const_reference back() const
	{
		return m_impl->back();
	}
	/**
	 * Exchanges the elements of the current list with those of another.
	 * @param x The another List, this List will exchange its elements with.
	 */
	void swap(List<T>& x)
	{
		m_impl.swap(x.m_impl);
	}
	/**
	 * Insert an element to the List before the element specified
	 * by the iterator.
	 * @param position An iterator that points to the insertion point.
	 *                 The element will be inserted before this point.
	 * @param x        The element to insert into the List.
	 * @return An iterator that points to the newly inserted element.
	 */
	iterator insert(iterator position, const T& x)
	{
		return m_impl->insert(position, x);
	}
	/**
	 * Insert an default-constructed element to the List before the
	 * element specified by the iterator.
	 * @param position An iterator that points to the insertion point.
	 *                 The element will be inserted before this point.
	 * @return An iterator that points to the newly inserted element.
	 */
	iterator insert(iterator position)
	{
		return m_impl->insert(position);
	}
	/**
	 * Insert a range of elements before a given position in the List.
	 * @param position The position to insert the elements at. The
	 *                 insertion will be done before this position.
	 * @param first The beginning of the range of elements to insert.
	 * @param last The end of the range of elements to insert.
	 */
	template<class InputIterator>
	void insert(iterator position, InputIterator first, InputIterator last)
	{
		m_impl->insert(position, first, last);
	}
	/**
	 * Insert a specified number of elements that are copies of a given
	 * object before the given position in the List.
	 *
	 * @param pos   The position to insert the elements before.
	 * @param n     Number of elements to insert.
	 * @param x     The value every newly inserted element of the
	 *              List will be initialized to.
	 */
	void insert(iterator pos, size_type n, const T& x)
	{
		m_impl->insert(pos, n, x);
	}
	/**
	 * Insert a specified number of elements that are copies of a given
	 * object before the given position in the List.
	 *
	 * @param pos   The position to insert the elements before.
	 * @param n     Number of elements to insert.
	 * @param x     The value every newly inserted element of the
	 *              List will be initialized to.
	 */
	void insert(iterator pos, int n, const T& x)
	{
		m_impl->insert(pos, n, x);
	}
	/**
	 * Insert a specified number of elements that are copies of a given
	 * object before the given position in the List.
	 *
	 * @param pos   The position to insert the elements before.
	 * @param n     Number of elements to insert.
	 * @param x     The value every newly inserted element of the
	 *              List will be initialized to.
	 */
	void insert(iterator pos, long n, const T& x)
	{
		m_impl->insert(pos, n, x);
	}
	/**
	 * Prepend the specified element at the front of the List.
	 *
	 * @param x     The element to prepend to the front of the List.
	 */
	void push_front(const T& x)
	{
		m_impl->push_front(x);
	}
	/**
	 * Append the specified element to the end of the List.
	 *
	 * @param x     The element to append to the end of the List.
	 */
	void push_back(const T& x)
	{
		m_impl->push_back(x);
	}
	/**
	 * Remove an element from the List specified with an iterator.
	 *
	 * @param position An iterator that points to the element to be removed.
	 * @return An iterator that points to the element that was following
	 *         the removed element in the List.
	 */
	iterator erase(iterator position)
	{
		return m_impl->erase(position);
	}
	/**
	 * Remove elements from the List specified by a beginning and
	 * ending iterator.
	 *
	 * @param first An iterator that specifies the first element to remove.
	 * @param last An iterator that specifies the last element to remove.
	 * @return An iterator that points to the element that was following
	 *         the last removed element in the List.
	 */
	iterator erase(iterator first, iterator last)
	{
		return m_impl->erase(first, last);
	}
	/**
	 * Ensure the List has a given size.
	 *
	 * @param new_size The new size of the List.
	 * @param x An object to append to the end of the List if it is enlarged.
	 */
	void resize(size_type new_size, const T& x)
	{
		m_impl->resize(new_size, x);
	}
	/**
	 * Ensure the List has a given size appending a default-constructed
	 * object to the end of the List if it is enlarged.
	 *
	 * @param new_size The new size of the List.
	 */
	void resize(size_type new_size)
	{
		m_impl->resize(new_size);
	}
	/**
	 * Remove all items from the List. The size() of the List should be
	 * zero after calling this method.
	 */
	void clear()
	{
		m_impl->clear();
	}
	/**
	 * Find element x in the list range specified by the first and last
	 * iterators.
	 * @param  x     The element to seach for.
	 * @param  first The first position iterator; begin of the range.
	 * @param  last  The last position iterator; end of the range.
	 * @return An read only iterator pointing to the found element or
	 * the end iterator pointing one past the last element in the list.
	 */
	const_iterator find(const T &x, const_iterator first,
	                                const_iterator last) const
	{
		for( ; first != end(); ++first)
		{
			if( x == *first)
				return first;
			if( first == last)
				break;
		}
		return end();
	}
	/**
	 * Find element x in the list.
	 * @param  x    The element to seach for.
	 * @return An read only iterator pointing to the found element or
	 * the end iterator pointing one past the last element in the list.
	 */
	const_iterator find(const T &x) const
	{
		return find(x, begin(), end());
	}
	/**
	 * Find element x in the list range specified by the first and last
	 * iterators.
	 * @param  x     The element to seach for.
	 * @param  first The first position iterator; begin of the range.
	 * @param  last  The last position iterator; end of the range.
	 * @return An read/write iterator pointing to the found element or
	 * the end iterator pointing one past the last element in the list.
	 */
	iterator       find(const T &x, iterator first, iterator last)
	{
		for( ; first != end(); ++first)
		{
			if( x == *first)
				return first;
			if( first == last)
				break;
		}
		return end();
	}
	/**
	 * Find element x in the list.
	 * @param  x    The element to seach for.
	 * @return An read/write iterator pointing to the found element or
	 * the end iterator pointing one past the last element in the list.
	 */
	iterator       find(const T &x)
	{
		return find(x, begin(), end());
	}
	/**
	 * Determine if element x is contained in the list range specified
	 * by the first and last iterators.
	 * @param  x     The element to seach for.
	 * @param  first The first position iterator; begin of the range.
	 * @param  last  The last position iterator; end of the range.
	 * @return true if the element x is contained in the specified
	 *         range of the list.
	 */
	bool contains(const T& x, const_iterator first,
	                          const_iterator last) const
	{
		return find(x, first, last) != end();
	}
	/**
	 * Determine if element x is contained in the list.
	 * @param  x    The element to seach for.
	 * @return true if the element x is contained in the list.
	 */
	bool contains(const T& x) const
	{
		return find(x, begin(), end()) != end();
	}
	/**
	 * Remove the first element in the List.
	 */
	void pop_front()
	{
		m_impl->pop_front();
	}
	/**
	 * Remove the last element in the List.
	 */
	void pop_back()
	{
		m_impl->pop_back();
	}
	/**
	 * @doctodo
	 * Move the specified list into the current list at the given position.
	 * @param position The iterator pointing to the insert position.
	 * @param x The list to move elements from.
	 */
	void splice(iterator position, List& x)
	{
		m_impl->splice(position, *x.m_impl);
	}
	/**
	 * @doctodo
	 * Move the specified element from list x pointed to by iterator i
	 * into the current list at the given position.
	 * @param position The iterator pointing to the insert position.
	 * @param x        The List to insert element from.
	 * @param i        The element in List x to move.
	 */
	void splice(iterator position, List& x, iterator i)
	{
		m_impl->splice(position, *x.m_impl, i);
	}
	/**
	 * @doctodo
	 * Move the elements from list x specified by first and last iterators
	 * into the current list at the given position.
	 * @param position The iterator pointing to the insert position.
	 * @param x        The list to insert element from.
	 * @param first    The first element in List x to move.
	 * @param last     The last element in List x to move.
	 */
	void splice(iterator position, List& x, iterator first, iterator last)
	{
		m_impl->splice(position, *x.m_impl, first, last);
	}
	/**
	 * Remove the specified element from the List.
	 * @param value The element to remove.
	 */
	void remove(const T& value)
	{
		m_impl->remove(value);
	}
	/**
	 * Remove all duplicate elements from the List.
	 */
	void unique()
	{
		m_impl->unique();
	}
	/**
	 * Merge the current and specified lists, producing a combined list
	 * that is ordered with respect to the < operator.
	 * @param x The list to merge with.
	 */
	void merge(List& x)
	{
		m_impl->merge(*x.m_impl);
	}
	/**
	 * Reverse the order of elements in the list.
	 */
	void reverse()
	{
		m_impl->reverse();
	}
	/**
	 * Sort the list using the < operator to compare elements.
	 */
	void sort()
	{
		m_impl->sort();
	}
	/**
	 * Removes all elements from the list for which the unary
	 * predicate p is true.
	 * @param p The unary predicate
	 */
	template<class Predicate> void remove_if (Predicate p)
	{
		m_impl->remove_if (p);
	}
	/**
	 * Remove all elements from the List for which the binary
	 * predicate bp is true.
	 * @param bp The binary predicate
	 */
	template<class BinaryPredicate> void unique(BinaryPredicate bp)
	{
		m_impl->unique(bp);
	}
	/**
	 * Merge the current and specified list, producing a combined list
	 * that is ordered with respect to the specified comparisation class.
	 * @param swo The comparisation functor class.
	 */
	template<class StrictWeakOrdering> void merge(List& x, StrictWeakOrdering swo)
	{
		m_impl->merge(*x.m_impl, swo);
	}
	/**
	 * Sort the list using the specified comparisation class.
	 * @param swo The comparisation functor class.
	 */
	template<class StrictWeakOrdering> void sort(StrictWeakOrdering swo)
	{
		m_impl->sort(swo);
	}
	/**
	 * Determine equality of two Lists comparing the size of both lists
	 * and all elements in the same position using the elements "=="
	 * operator.
	 * @param x The first List in the comparison.
	 * @param x The second List in the comparison.
	 * @return true if the Lists are equal.
	 */
	friend bool operator== <>(const List<T>& x, const List<T>& y);
	/**
	 * Determine if one Lists is less than another comparing the size
	 * of both lists and all their elements.
	 * @param x The first List in the comparison.
	 * @param x The second List in the comparison.
	 * @return true if the List x is less than List y.
	 */
	friend bool operator< <>(const List<T>& x, const List<T>& y);

	/** \example listDemo.cpp */
};
template <class T>
inline bool operator==(const List<T>& x, const List<T>& y)
{
	return *x.m_impl == *y.m_impl;
}
template <class T>
inline bool operator<(const List<T>& x, const List<T>& y)
{
	return *x.m_impl < *y.m_impl;
}
template <class T>
inline void swap(List<T>& x, List<T>& y)
{
	x.swap(y);
}
template <class T>
std::list<T>* COWReferenceClone(std::list<T>* obj)
{
	return new std::list<T>(*obj);
}

} // end namespace BLOCXX_NAMESPACE

#endif
