#include <blocxx/BLOCXX_config.h>
#include <blocxx/COWIntrusiveReference.hpp>
#include <blocxx/COWIntrusiveCountableBase.hpp>
#include <iostream>

using std::cout;
using std::cerr;
using std::endl;

using namespace BLOCXX_NAMESPACE;

class COWExampleImpl : public COWIntrusiveCountableBase
{
public:
	COWExampleImpl(int v)
		: COWIntrusiveCountableBase()
		, m_value(v)
	{
		cout << "COWExampleImpl(int v) CTOR" << endl;
	}

	COWExampleImpl(const COWExampleImpl& arg)
		: COWIntrusiveCountableBase(arg)
		, m_value(arg.m_value)
	{
		cout << "COWExampleImpl(const COWExampleImpl& arg) CTOR" << endl;
	}

	~COWExampleImpl()
	{
		cout << "~COWExampleImpl() DTOR" << endl;
	}

	COWExampleImpl& operator= (const COWExampleImpl& arg)
	{
      cout << "COWExampleImpl operator= COWExampleImpl" << endl;
		m_value = arg.m_value;
		return *this;
	}

	COWExampleImpl* clone() const 
	{ 
		return new COWExampleImpl(*this); 
	}

	int getValue() const 
	{ 
		return m_value; 
	}

	void setValue(int v)
	{
		m_value = v;
	}

private:
	int m_value;
};


class COWExample
{
public:
	COWExample(int v)
		: m_impl(new COWExampleImpl(v))
	{
		cout << "COWExample(int v) CTOR" << endl;
	}

	COWExample(const COWExample& arg)
		: m_impl(arg.m_impl)
	{
		cout << "COWExample(const COWExample& arg) CTOR" << endl;
	}

	~COWExample()
	{
		cout << "~COWExample() DTOR" << endl;
	}

	COWExample& operator= (const COWExample& arg)
	{
		m_impl = arg.m_impl;
		return *this;
	}

	int getValue() const 
	{ 
		return m_impl->getValue();
	}

	void setValue(int v)
	{
		m_impl->setValue(v);
	}

	const void* getImplAddr() const
	{
		return static_cast<const void*>(m_impl.getPtr());
	}

private:
	COWIntrusiveReference<COWExampleImpl> m_impl;
};


int
main(int argc, char** argv)
{
	cout << "Constructing 1st COWExample" << endl;
	COWExample ce1(5);
	cout << "Constructing 2nd COWExample" << endl;
	COWExample ce2(10);	// ce2 has a different COWExampleImpl than ce1

	// Show two COWExample objects with different underlying
	// COWExampleImpl objects.
	// Should have different values on getValue and getImplAddr.
	cout << "ce1.getValue(): " << ce1.getValue() << endl;
	cout << "ce1.getImplAddr(): " << ce1.getImplAddr() << endl;
	cout << "ce2.getValue(): " << ce2.getValue() << endl;
	cout << "ce2.getImplAddr(): " << ce2.getImplAddr() << endl;

	// Show two COWExample objects sharing the same underlying
	// COWExampleImpl object through the assignment operator.
	cout << "Assigning ce2 to ce1" << endl;
	
	ce2 = ce1;		// ce1 and ce2 have the same COWExampleImpl now.

	// These should show the same values on getValue and
	// getImplAddr now.
	cout << "ce1.getValue(): " << ce1.getValue() << endl;
	cout << "ce1.getImplAddr(): " << ce1.getImplAddr() << endl;
	cout << "ce2.getValue(): " << ce2.getValue() << endl;
	cout << "ce2.getImplAddr(): " << ce2.getImplAddr() << endl;

	// Now let's show the copy on write funtionality.
	// This should cause ce1 and ce2 to have different
	// COWExampleImpl objects after the call to ce1.setValue
	cout << "Setting value on ce1 to 123" << endl;
	ce1.setValue(123);
	cout << "ce1.getValue(): " << ce1.getValue() << endl;
	cout << "ce1.getImplAddr(): " << ce1.getImplAddr() << endl;
	cout << "ce2.getValue(): " << ce2.getValue() << endl;
	cout << "ce2.getImplAddr(): " << ce2.getImplAddr() << endl;

	cout << "main() returning" << endl;
	return 0;
}
