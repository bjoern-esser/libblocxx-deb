/*******************************************************************************
* Copyright (C) 2009, Quest Software, Inc. All rights reserved.
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
 * @author Kevin Harris
 */

#ifndef BLOCXX_LAZY_GLOBAL_HPP_INCLUDE_GUARD_
#define BLOCXX_LAZY_GLOBAL_HPP_INCLUDE_GUARD_

#include "blocxx/BLOCXX_config.h"
#include "blocxx/ThreadOnce.hpp"
#include "blocxx/GlobalPtr.hpp"
#include "blocxx/Exception.hpp"

#ifdef BLOCXX_HAVE_STD_TR1_IS_POD
#include "blocxx/StaticAssert.hpp"

#include <tr1/type_traits>
#endif

namespace BLOCXX_NAMESPACE
{

	template <typename T, typename PODType>
	struct DefaultVariableConstructorFactory
	{
		static T* create(const PODType& t)
		{
			return new T(t);
		}
	};

	/**
	 * This class can be used to store a global variable that is lazily initialized in a thread safe manner. It is
	 * intended as a solution to the common C++ issue with the ordering of constructor invocations during the dynamic
	 * initialization phase. See http://www.parashift.com/c++-faq-lite/ctors.html#faq-10.12 for a description and for
	 * some possible solutions. Unfortunately the soultions presented in the C++ FAQ are not thread-safe on all
	 * platforms (some versions of g++ & C standard library can initialize function-scope static variables in a
	 * thread-safe manner) and can't be used by BloCxx.
	 *
	 * A LazyGlobal instance consists of a pointer to T that is lazily constructed and deleted when the global variable
	 * goes out of scope, an instance of PODType that is used to pass to the constructor of T when it is newed, and a
	 * ThreadOnce instance that is used to ensure the pointer is constructed in a thread safe manner.
	 *
	 * LazyGlobal is a POD type with a destructor, and thus must be initialized using POD initialization syntax:
	 *   LazyGlobal<T, PODType> myGlobalVar = BLOCXX_LAZY_GLOBAL_INIT(pod);
	 * Doing initialization in this manner happens during the static initialization phase, before any code is run, and
	 * so avoids the ordering problem. Take care that the argument is truly static data and not the result of running a
	 * function or constructor.
	 *
	 * The pointer will be lazily constructed. The first time get() or a conversion operator is called, a new T (or
	 * derived) will be instantiated by a call to FactoryT::create(m_pod). The default type for FactoryT allocates a new
	 * instance of T passing the variable m_pod of type PODType to the constructor. The initialization is done in a
	 * thread safe manner.  The double checked locking pattern is used, which doesn't have a large overhead.
	 *
	 * No copy or assignment of LazyGlobal variables should ever be done.  This cannot be enforced while maintaining
	 * the POD initialization of the class.
	 *
	 * Conversion operators to T& and const T& are provided so that a LazyGlobal can be used wherever a T would be used.
	 *
	 * All calls to get() or the conversion operators are thread safe.
	 *
	 * Gotchas to be aware of:
	 * - This class does help a bit with the static deinitialization situation, because if the object has already been
	 *   destroyed, it will be safely recreated if get() is called. However, if this happens, the destructor will not
	 *   run again, and so the global variable will not be deleted.
	 * - Be careful to not use a function or constructor as an argument to BLOCXX_LAZY_GLOBAL_INIT. This will cause the
	 *   initialization to happen during the dynamic initialization phase and negate the benefit of using LazyGlobal. If
	 *   you erroneously use dynamic code, the compiler will not issue a warning or error.
	 *
	 * @param T The type of the pointer.
	 * @param PODType The type of the POD data that can be passed to the constructor of T. This must be a POD type.  If
	 *                the compiler has a functional std::tr1::is_pod<>, then this will be enforced at compile time.
	 * @param FactoryT To create the T*, FactoryT::create() will be called with 1 parameter, the m_pod variable of type
	 *                 PODType. The return type must be convertible to a T*. The default
	 *                 DefaultVariableConstructorFactory will return a new T passing m_pod as a single argument to the
	 *                 constructor. The factory function should not access instances that may not be initialized, such
	 *                 as a dynamic global or static object, because the factory function may be run at any point during
	 *                 the dynamic initialization phase and there is no guarantee that the instance is already
	 *                 initialized. It is best to only refer to the POD data passed as an argument.
	 */
	template <typename T, typename PODType, typename FactoryT = DefaultVariableConstructorFactory<T, PODType> >
	class LazyGlobal
	{
#ifdef BLOCXX_HAVE_STD_TR1_IS_POD
		BLOCXX_STATIC_ASSERT(std::tr1::is_pod<PODType>::value);
#endif
	private:
		struct InitPtr
		{
			InitPtr(T*& p, const PODType& str)
				: m_p(p)
				, m_pod(str)
			{ }
			T*& m_p;
			const PODType& m_pod;

			void operator()()
			{
				m_p = static_cast<T*>(FactoryT::create(m_pod));
			}
		};

		T* getPtr() const
		{
			callOnce(m_onceFlag, InitPtr(m_p, m_pod));
			return m_p;
		}

	public:
		~LazyGlobal()
		{
			// Don't allow exceptions to escape uncaught.

			const char* unableToHandle = "Caught exception in the LazyGlobal destructor";
			try
			{
				delete m_p;
				// reset these variables in case get() is called after the destructor runs.
				m_p = 0;
				OnceFlag tmp = BLOCXX_ONCE_INIT;
				m_onceFlag = tmp;
			}
			catch(const Exception& e)
			{
				logErroneousException(unableToHandle, e);
			}
			catch(const std::exception& e)
			{
				logErroneousException(unableToHandle, e);
			}
			catch(...)
			{
				logErroneousException(unableToHandle);
			}
		}

		T& get() const
		{
			callOnce(m_onceFlag, InitPtr(m_p, m_pod));
			return *m_p;
		}

		// The two different operators are defined because there may be some
		// global variables marked as const.  We'll want them treated
		// differently.
		operator T&()
		{
			return get();
		}

		operator const T&() const
		{
			return get();
		}

		template<typename T2>
		T& operator=(const T2& rhs)
		{
			// checking for self assignment is handled by type T
			return (get() = rhs);
		}

		// These members should be treated as private. They aren't marked private
		// because if they are, POD initializer syntax can't be used.
		PODType m_pod;
		mutable T* m_p;
		mutable OnceFlag m_onceFlag;
	};

/**
 * Statically initialize a LazyGlobal instance.
 * @param Static POD data convertible to type T. Take care that this is not a function or constructor.
 */
#define BLOCXX_LAZY_GLOBAL_INIT(...) { __VA_ARGS__, 0, BLOCXX_ONCE_INIT }


	/**
	 * A factory to take an unused int as a pod type and do
	 * initialization using the default constructor.  This is suitable
	 * for use in a LazyGlobal.
	 */
	template <typename T>
	struct DefaultConstructedVariableFactory
	{
		typedef int PODType;

		static T* create(const PODType& t)
		{
			(void) t;
			return new T();
		}
	};

	/**
	 * A type wrapper to simplify the creation of a default-constructed
	 * lazy global.  When C++0x template aliases are possible, a clean
	 * and simple alias could be created.
	 *
	 * Example C++0x alias:
	 *   template <typename T>
	 *   using DefaultConstructedLazyGlobal2 = LazyGlobal<T,
	 *     typename DefaultConstructedVariableFactory<T>::PODType,
	 *     DefaultConstructedVariableFactory<T> >;
	 *
	 * Which could then be used as DefaultConstructedLazyGlobal2<MyType>
	 *
	 * The current simplification requires using the embedded typedef
	 * such as DefaultConstructedLazyGlobal<MyType>::type
	 */
	template <typename T>
	struct DefaultConstructedLazyGlobal
	{
		typedef LazyGlobal<T, typename DefaultConstructedVariableFactory<T>::PODType, DefaultConstructedVariableFactory<T> >  type;
	};

/**
 * An initializer for template<> DefaultConstructedLazyGlobal<>::type
 *
 * Example:
 *   DefaultConstructedLazyGlobal<MyType>::type MyGlobal = BLOCXX_LAZY_GLOBAL_DEFAULT_INIT;
 */
#define BLOCXX_LAZY_GLOBAL_DEFAULT_INIT BLOCXX_LAZY_GLOBAL_INIT(0)

} // end namespace BLOCXX_NAMESPACE


#endif
