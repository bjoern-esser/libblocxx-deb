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


#include "blocxx/BLOCXX_config.h"
#include "TestSuite.hpp"
#include "TestCaller.hpp"
#include "COWIntrusiveReferenceTestCases.hpp"
#include "blocxx/COWIntrusiveReference.hpp"
#include "blocxx/COWIntrusiveCountableBase.hpp"

using namespace blocxx;



//
//  intrusive_ptr_test.cpp
//
//  Copyright (c) 2002, 2003 Peter Dimov
//
//  Permission to copy, use, modify, sell and distribute this software
//  is granted provided this copyright notice appears in all copies.
//  This software is provided "as is" without express or implied
//  warranty, and with no claim as to its suitability for any purpose.
//

#include <algorithm>
#include <functional>

struct X: public virtual COWIntrusiveCountableBase
{
	X* clone() const
	{
		return new X(*this);
	}
	int use_count() const
	{
		return getRefCount().get();
	}
};

struct Y: public X
{
	Y* clone() const
	{
		return new Y(*this);
	}
};

void f(X &)
{
}

void COWIntrusiveReferenceTestCases::n_element_type()
{
	typedef COWIntrusiveReference<X>::element_type T;
	T t;
	f(t);
}

void COWIntrusiveReferenceTestCases::default_constructor()
{
	COWIntrusiveReference<X> px;
	unitAssert(px.getPtr() == 0);
}

void COWIntrusiveReferenceTestCases::pointer_constructor()
{
	{
		COWIntrusiveReference<X> px(0);
		unitAssert(px.getPtr() == 0);
	}

	{
		COWIntrusiveReference<X> px(0, false);
		unitAssert(px.getPtr() == 0);
	}

	{
		X * p = new X;
		unitAssert(p->use_count() == 0);

		COWIntrusiveReference<X> px(p);
		unitAssert(px.getPtr() == p);
		unitAssert(px->use_count() == 1);
	}

	{
		X * p = new X;
		unitAssert(p->use_count() == 0);

		COWIntrusiveReferenceAddRef(p);
		unitAssert(p->use_count() == 1);

		COWIntrusiveReference<X> px(p, false);
		unitAssert(px.getPtr() == p);
		unitAssert(px->use_count() == 1);
	}
}

void COWIntrusiveReferenceTestCases::copy_constructor()
{
	{
		COWIntrusiveReference<X> px;
		COWIntrusiveReference<X> px2(px);
		unitAssert(px2.getPtr() == px.getPtr());
	}

	{
		COWIntrusiveReference<Y> py;
		COWIntrusiveReference<X> px(py);
		unitAssert(px.getPtr() == py.getPtr());
	}

	{
		COWIntrusiveReference<X> px(0);
		COWIntrusiveReference<X> px2(px);
		unitAssert(px2.getPtr() == px.getPtr());
	}

	{
		COWIntrusiveReference<Y> py(0);
		COWIntrusiveReference<X> px(py);
		unitAssert(px.getPtr() == py.getPtr());
	}

	{
		COWIntrusiveReference<X> px(0, false);
		COWIntrusiveReference<X> px2(px);
		unitAssert(px2.getPtr() == px.getPtr());
	}

	{
		COWIntrusiveReference<Y> py(0, false);
		COWIntrusiveReference<X> px(py);
		unitAssert(px.getPtr() == py.getPtr());
	}

	{
		COWIntrusiveReference<X> px(new X);
		COWIntrusiveReference<X> px2(px);
		unitAssert(px2.getPtr() == px.getPtr());
	}

	{
		COWIntrusiveReference<Y> py(new Y);
		COWIntrusiveReference<X> px(py);
		unitAssert(px.getPtr() == py.getPtr());
	}
}

void COWIntrusiveReferenceTestCases::n_destructor()
{
	COWIntrusiveReference<X> px(new X);
	unitAssert(px->use_count() == 1);

	{
		const COWIntrusiveReference<X> px2(px);
		unitAssert(px2->use_count() == 2);
		// now calling the non-const operator-> will cause them to become unique
		px->use_count();
		unitAssert(px->use_count() == 1);
		unitAssert(px2->use_count() == 1);
	}

	unitAssert(px->use_count() == 1);
}

void COWIntrusiveReferenceTestCases::n_access()
{
	{
		COWIntrusiveReference<X> px;
		unitAssert(px? false: true);
		unitAssert(!px);

		//unitAssert(getPtr_pointer(px) == px.getPtr());
	}

	{
		COWIntrusiveReference<X> px(0);
		unitAssert(px? false: true);
		unitAssert(!px);

		//unitAssert(getPtr_pointer(px) == px.getPtr());
	}

	{
		COWIntrusiveReference<X> px(new X);
		unitAssert(px? true: false);
		unitAssert(!!px);
		unitAssert(&*px == px.getPtr());
		unitAssert(px.operator ->() == px.getPtr());

		//unitAssert(getPtr_pointer(px) == px.getPtr());
	}
}

void COWIntrusiveReferenceTestCases::n_swap()
{
	{
		COWIntrusiveReference<X> px;
		COWIntrusiveReference<X> px2;

		px.swap(px2);

		unitAssert(px.getPtr() == 0);
		unitAssert(px2.getPtr() == 0);

		std::swap(px, px2);

		unitAssert(px.getPtr() == 0);
		unitAssert(px2.getPtr() == 0);
	}

	{
		X * p = new X;
		COWIntrusiveReference<X> px;
		COWIntrusiveReference<X> px2(p);
		COWIntrusiveReference<X> px3(px2);

		px.swap(px2);

		unitAssert(px.getPtr() == p);
		unitAssert(px->use_count() == 1);
		unitAssert(px2.getPtr() == 0);
		unitAssert(px3.getPtr() == p);
		unitAssert(px3->use_count() == 1);

		std::swap(px, px2);

		unitAssert(px.getPtr() == 0);
		unitAssert(px2->use_count() == 1);
		unitAssert(px3.getPtr() == p);
		unitAssert(px3->use_count() == 1);
	}

	{
		X * p1 = new X;
		X * p2 = new X;
		COWIntrusiveReference<X> px(p1);
		COWIntrusiveReference<X> px2(p2);
		COWIntrusiveReference<X> px3(px2);

		px.swap(px2);

		unitAssert(px.getPtr() == p2);
		unitAssert(px->use_count() == 1);
		unitAssert(px2.getPtr() == p1);
		unitAssert(px2->use_count() == 1);
		unitAssert(px3.getPtr() == p2);
		unitAssert(px3->use_count() == 1);

		std::swap(px, px2);

		unitAssert(px.getPtr() == p1);
		unitAssert(px->use_count() == 1);
		//unitAssert(px2.getPtr() == p2);
		unitAssert(px2->use_count() == 1);
		unitAssert(px3.getPtr() == p2);
		unitAssert(px3->use_count() == 1);
	}
}

template<class T, class U> void COWIntrusiveReferenceTestCases::test2(COWIntrusiveReference<T> const & p, COWIntrusiveReference<U> const & q)
{
	unitAssert((p == q) == (p.getPtr() == q.getPtr()));
	unitAssert((p != q) == (p.getPtr() != q.getPtr()));
}

template<class T> void COWIntrusiveReferenceTestCases::test3(COWIntrusiveReference<T> const & p, COWIntrusiveReference<T> const & q)
{
	unitAssert((p == q) == (p.getPtr() == q.getPtr()));
	unitAssert((p.getPtr() == q) == (p.getPtr() == q.getPtr()));
	unitAssert((p == q.getPtr()) == (p.getPtr() == q.getPtr()));
	unitAssert((p != q) == (p.getPtr() != q.getPtr()));
	unitAssert((p.getPtr() != q) == (p.getPtr() != q.getPtr()));
	unitAssert((p != q.getPtr()) == (p.getPtr() != q.getPtr()));

	// 'less' moved here as a g++ 2.9x parse error workaround
	std::less<const T*> less;
	unitAssert((p < q) == less(p.getPtr(), q.getPtr()));
}

void COWIntrusiveReferenceTestCases::n_comparison()
{
	{
		COWIntrusiveReference<X> px;
		test3(px, px);

		COWIntrusiveReference<X> px2;
		test3(px, px2);

		COWIntrusiveReference<X> px3(px);
		test3(px3, px3);
		test3(px, px3);
	}

	{
		COWIntrusiveReference<X> px;

		COWIntrusiveReference<X> px2(new X);
		test3(px, px2);
		test3(px2, px2);

		COWIntrusiveReference<X> px3(new X);
		test3(px2, px3);

		COWIntrusiveReference<X> px4(px2);
		test3(px2, px4);
		test3(px4, px4);
	}

	{
		COWIntrusiveReference<X> px(new X);

		COWIntrusiveReference<Y> py(new Y);
		test2(px, py);

		COWIntrusiveReference<X> px2(py);
		test2(px2, py);
		test3(px, px2);
		test3(px2, px2);
	}
}

struct X2: public COWIntrusiveCountableBase
{
	COWIntrusiveReference<X2> next;

	X2* clone() const
	{
		return new X2(*this);
	}
};

void COWIntrusiveReferenceTestCases::n_transitive()
{
	COWIntrusiveReference<X2> p(new X2);
	p->next = COWIntrusiveReference<X2>(new X2);
	unitAssert(!p->next->next);
	p = p->next;
	unitAssert(!p->next);
}

class foo: public COWIntrusiveCountableBase
{
public: 
#ifdef BLOCXX_WIN32
#pragma warning (push)
#pragma warning (disable : 4355)
#endif
	foo(): m_self(this)
	{
	} 

	foo(const foo& x): COWIntrusiveCountableBase(), m_self(this)
	{
	}
#ifdef BLOCXX_WIN32
#pragma warning (pop)
#endif

	void suicide()
	{
		m_self = 0;
	}

	foo* clone() const
	{
		return new foo(*this);
	}
private:

	COWIntrusiveReference<foo> m_self;
}; 

void COWIntrusiveReferenceTestCases::n_report_1()
{
	foo * foo_ptr = new foo;
	foo_ptr->suicide();
} 

void COWIntrusiveReferenceTestCases::setUp()
{
}

void COWIntrusiveReferenceTestCases::tearDown()
{
}

Test* COWIntrusiveReferenceTestCases::suite()
{
	TestSuite *testSuite = new TestSuite ("COWIntrusiveReference");

	ADD_TEST_TO_SUITE(COWIntrusiveReferenceTestCases, n_element_type);
	ADD_TEST_TO_SUITE(COWIntrusiveReferenceTestCases, default_constructor);
	ADD_TEST_TO_SUITE(COWIntrusiveReferenceTestCases, pointer_constructor);
	ADD_TEST_TO_SUITE(COWIntrusiveReferenceTestCases, copy_constructor);
	ADD_TEST_TO_SUITE(COWIntrusiveReferenceTestCases, n_destructor);
	ADD_TEST_TO_SUITE(COWIntrusiveReferenceTestCases, n_access);
	ADD_TEST_TO_SUITE(COWIntrusiveReferenceTestCases, n_swap);
	ADD_TEST_TO_SUITE(COWIntrusiveReferenceTestCases, n_comparison);
	ADD_TEST_TO_SUITE(COWIntrusiveReferenceTestCases, n_transitive);
	ADD_TEST_TO_SUITE(COWIntrusiveReferenceTestCases, n_report_1);

	return testSuite;
}

