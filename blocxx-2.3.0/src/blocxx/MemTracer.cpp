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

#define BLOCXX_MEM_TRACER_CPP_INCLUDE_GUARD_
#include "blocxx/BLOCXX_config.h"
#ifdef BLOCXX_DEBUG_MEMORY
#include "blocxx/MemTracer.hpp"
#include "blocxx/Mutex.hpp"
#include <map>
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <errno.h>

#ifdef BLOCXX_HAVE_UNISTD_H
#include <unistd.h>
#endif

#define BLOCXX_MEM_SIG 0xaaaaaaaa
#define BLOCXX_FREE_MEM_SIG 0xbbbbbbbb

namespace BLOCXX_NAMESPACE
{

// need to use our own allocator to avoid a deadlock with the standard allocator locking with a non-recursive mutex.
template <typename T>
class MemTracerAllocator
{
public:
	typedef std::size_t size_type;
	typedef std::ptrdiff_t difference_type;
	typedef T value_type;
	typedef value_type* pointer;
	typedef const value_type* const_pointer;
	typedef value_type& reference;
	typedef const value_type& const_reference;
	template <class U> struct rebind
	{
		typedef MemTracerAllocator<U> other;
	};

	MemTracerAllocator() throw() {}
	template <class U> MemTracerAllocator(const MemTracerAllocator<U>& other) throw() {}
	pointer address(reference r) const { return &r; }
	const_pointer address(const_reference r) const { return &r; }
	pointer allocate(size_type n, const_pointer hint = 0)
	{
		return static_cast<pointer>(::malloc(n));
	}
	void deallocate(pointer p, size_type n)
	{
		::free(p);
	}
	size_type max_size() const throw() { return static_cast<size_type>(-1); }
	void construct(pointer p, const_reference val) { new(p) value_type(val); }
	void destroy(pointer p) { p->~value_type(); }

};



static const char* const noFile = "<no file>";
class MemTracer
{
public:
	class Entry
	{
	public:
		Entry (char const * file, int line, size_t sz)
		: m_file(file), m_line(line), m_size(sz), m_isDeleted(false) {}
		Entry()
		: m_file(NULL), m_line(-1), m_size(0), m_isDeleted(false) {}
		char const* getFile() const { return m_file; }
		int getLine() const { return m_line; }
		size_t getSize() const { return m_size; }
		void setDeleted() { m_isDeleted = true; }
		bool isDeleted() { return m_isDeleted; }
	private:
		char const* m_file;
		int m_line;
		size_t m_size;
		bool m_isDeleted;
	};
private:
	class Lock
	{
	public:
		Lock(MemTracer & tracer) : m_tracer(tracer) { m_tracer.lock (); }
		~Lock()
		{
			try
			{
				m_tracer.unlock ();
			}
			catch (...)
			{
				// don't let exceptions escape
			}
		}
	private:
		MemTracer& m_tracer;
	};
	typedef MemTracerAllocator<std::pair<void* const, Entry> > alloc_t;
	typedef std::map<void*, Entry, std::less<void*>, alloc_t > map_t;
	typedef map_t::iterator iterator;
	friend class Lock;
public:
	MemTracer();
	~MemTracer();
	void add(void* p, char const* file, int line, size_t sz);
	void* remove(void * p);
	void dump();
	Entry getEntry(void* idx);
	void printEntry(void* p);
	void checkMap();
private:
	void lock() { m_lockCount++; }
	void unlock() { m_lockCount--; }
private:
	map_t m_map;
	int m_lockCount;
};

static Mutex* memguard = NULL;
static MemTracer* MemoryTracer = 0;
static bool _shuttingDown = false;
static bool noFree = false;
static bool aggressive = false;
static bool disabled = false;
//////////////////////////////////////////////////////////////////////////////
void
myAtExitFunction()
{
	_shuttingDown = true;
	if (MemoryTracer != 0)
	{
		fprintf(stderr, "*******************************************************************************\n");
		fprintf(stderr, "*   D U M P I N G   M E M O R Y\n");
		fprintf(stderr, "*******************************************************************************\n");
		MemoryTracer->dump();
		fprintf(stderr, "-------------------------------------------------------------------------------\n");
		fprintf(stderr, "-   D O N E   D U M P I N G   M E M O R Y\n");
		fprintf(stderr, "-------------------------------------------------------------------------------\n");
	}
	else
	{
		fprintf(stderr, BLOCXX_PACKAGE_NAME": MemoryTracer object does not exist\n");
	}
}
static bool owInternal = false;
static bool initialized = false;
void
processEnv()
{
	if (!initialized)
	{
		if (getenv("BLOCXX_MEM_DISABLE") && getenv("BLOCXX_MEM_DISABLE")[0] == '1')
		{
			disabled = true;
		}
		if (getenv("BLOCXX_MEM_NOFREE") && getenv("BLOCXX_MEM_NOFREE")[0] == '1')
		{
			noFree = true;
		}
		if (getenv("BLOCXX_MEM_AGGRESSIVE") && getenv("BLOCXX_MEM_AGGRESSIVE")[0] == '1')
		{
			aggressive = true;
			fprintf(stderr, "MemTracer running in aggressive mode.\n");
		}
		initialized = true;
	}
}
//////////////////////////////////////////////////////////////////////////////
void
allocMemTracer()
{
	owInternal = true;
	processEnv();
	if (!disabled)
	{
		if (memguard == 0)
		{
			memguard = new Mutex;
			memguard->acquire();
		}
		if (MemoryTracer == 0)
		{
			atexit(myAtExitFunction);
			MemoryTracer = new MemTracer;
		}
	}
	owInternal = false;
}
//////////////////////////////////////////////////////////////////////////////
void DumpMemory()
{
	if (MemoryTracer != 0)
	{
		MemoryTracer->dump();
	}
	else
	{
		fprintf(stderr, BLOCXX_PACKAGE_NAME": MemoryTracer object does not exist\n");
	}
}
//////////////////////////////////////////////////////////////////////////////
MemTracer::MemTracer() : m_lockCount (0)
{
}
//////////////////////////////////////////////////////////////////////////////
MemTracer::~MemTracer()
{
	try
	{
		dump();
	}
	catch (...)
	{
		// don't let exceptions escape
	}
}
//static int delCount = 0;
//////////////////////////////////////////////////////////////////////////////
static void*
checkSigs(void* p, size_t sz)
{
	assert(sz);
	assert(p);
	unsigned long* plong = (unsigned long*)((char*)p - 4);
	if (*plong != BLOCXX_MEM_SIG)
	{
		fprintf(stderr, "UNDERRUN: Beginning boundary problem.  "
			"Sig is %x\n", (unsigned int)*plong);
		MemoryTracer->printEntry(p);
		assert(0);
	}
	plong = (unsigned long*)((char*)p + sz);
	if (*plong != BLOCXX_MEM_SIG)
	{
		fprintf(stderr, "OVERRUN: Ending boundary problem.  "
			"Sig is %x\n", (unsigned int)*plong);
		MemoryTracer->printEntry(p);
		fflush(stderr);
		assert(0);
	}
	return (void*)((char*)p - 4);
}
//////////////////////////////////////////////////////////////////////////////
static void*
checkAndSwitchSigs(void* p, size_t sz)
{
	assert(sz);
	assert(p);
	unsigned long* plong = (unsigned long*)((char*)p - 4);
	if (*plong != BLOCXX_MEM_SIG)
	{
		fprintf(stderr, "UNDERRUN: Beginning boundary problem.  "
			"Sig is %x\n", (unsigned int)*plong);
		assert(0);
	}
	*plong = BLOCXX_FREE_MEM_SIG;
	plong = (unsigned long*)((char*)p + sz);
	if (*plong != BLOCXX_MEM_SIG)
	{
		fprintf(stderr, "OVERRUN: Ending boundary problem.  "
			"Sig is %x\n", (unsigned int)*plong);
		assert(0);
	}
	*plong = BLOCXX_FREE_MEM_SIG;
	return (void*)((char*)p - 4);
}
//////////////////////////////////////////////////////////////////////////////
void
MemTracer::checkMap()
{
	for (iterator it = m_map.begin(); it != m_map.end(); ++it)
	{
		if (!it->second.isDeleted())
		{
			checkSigs(it->first, it->second.getSize());
		}
	}
}
//////////////////////////////////////////////////////////////////////////////
void
MemTracer::add(void* p, char const* file, int line, size_t sz)
{
	const char* pfile = noFile;
	if (file)
	{
		pfile = strdup(file);
	}
	m_map[p] = Entry(pfile, line, sz);
}
//////////////////////////////////////////////////////////////////////////////
void*
MemTracer::remove(void* p)
{
	iterator it = m_map.find(p);
	if (it != m_map.end())
	{
		if (noFree)
		{
			if (it->second.isDeleted())
			{
				fprintf(stderr, "DOUBLE DELETE (NOFREE): Attempting to double "
					"delete memory at: %p\n", p);
				assert(0);
			}
		}
		void* ptrToFree = checkAndSwitchSigs(p, it->second.getSize());
		void* pfile = (void*) it->second.getFile();
		if (noFree)
		{
			it->second.setDeleted();
		}
		else
		{
			m_map.erase(it);
			if (pfile != noFile)
			{
				free(pfile);
			}
		}
		return ptrToFree;
	}
	fprintf(stderr, "Attempting to delete memory not in map: %p\n", p);
	if (!noFree)
	{
		fprintf(stderr, "Trying to check beginning signature...\n");
		unsigned long* plong = (unsigned long*)((char*)p - 4);
		if (*plong == BLOCXX_MEM_SIG)
		{
			fprintf(stderr, "MemTracer is broken\n");
			assert(0);
		}
		if (*plong == BLOCXX_FREE_MEM_SIG)
		{
			fprintf(stderr, "DOUBLE DELETE: This memory was previously freed by MemTracer, "
				"probably double delete\n");
			assert(0);
		}
		fprintf(stderr, "No signature detected.\n");
	}
	fprintf(stderr, "UNKNOWN ADDRESS\n");
	assert(0);
	return p;
}
//////////////////////////////////////////////////////////////////////////////
void
MemTracer::printEntry(void* p)
{
	Entry entry = getEntry(p);
	fprintf(stderr, "\tFILE: %s", entry.getFile());
	fprintf(stderr, "\tLINE: %d", entry.getLine());
	fprintf(stderr, "\tSIZE: %lu", static_cast<unsigned long>(entry.getSize()));
	fprintf(stderr, "\tADDR: %p\n", p);
}
//////////////////////////////////////////////////////////////////////////////
MemTracer::Entry
MemTracer::getEntry(void* idx)
{
	memguard->acquire();
	iterator it = m_map.find(idx);
	MemTracer::Entry rval;
	if (it != m_map.end())
	{
		rval = it->second;
	}
	memguard->release();
	return rval;
}
//////////////////////////////////////////////////////////////////////////////
void
MemTracer::dump()
{
	memguard->acquire();
	if (m_map.size() != 0)
	{
		fprintf(stderr, "**** %lu MEMORY LEAK(S) DETECTED\n", static_cast<unsigned long>(m_map.size()));
		size_t total = 0;
		for (iterator it = m_map.begin(); it != m_map.end (); ++it)
		{
			if (!it->second.isDeleted())
			{
				fprintf(stderr, "\tFILE: %s", it->second.getFile());
				fprintf(stderr, "\tLINE: %d", it->second.getLine());
				fprintf(stderr, "\tSIZE: %lu", static_cast<unsigned long>(it->second.getSize()));
				fprintf(stderr, "\tADDR: %p\n", it->first);
				total += it->second.getSize();
			}
		}
		fprintf(stderr, "***** END MEMORY LEAKS - TOTAL MEMORY LEAKED = %lu\n", static_cast<unsigned long>(total));
	}
	memguard->release();
}
//////////////////////////////////////////////////////////////////////////////
static void
writeSigs(void *& p, size_t size)
{
	unsigned long* plong = (unsigned long*)p;
	*plong = BLOCXX_MEM_SIG;
	plong = (unsigned long*)((char*)p + size + 4);
	*plong = BLOCXX_MEM_SIG;
	p = (void*)((char*)p + 4);
}
static int internalNewCount = 0;
//////////////////////////////////////////////////////////////////////////////
static void*
doNew(size_t size, char const* file, int line)
{
	if (memguard)
	{
		memguard->acquire();
	}
	if (owInternal || disabled)
	{
		++internalNewCount;
		if (internalNewCount > 2 && !disabled)
		{
			fprintf(stderr, "INTERNAL NEW called more than twice!  "
				"Possible bug in MemTracer.\n");
			assert(0);
		}
		void* rval =  malloc(size);
		if (memguard)
		{
			memguard->release();
		}
		--internalNewCount;
		return rval;
	}
	allocMemTracer();
	if (disabled)
	{
		return malloc(size);
	}
	if (aggressive)
	{
		MemoryTracer->checkMap();
	}
	void* p = malloc(size + 8);
   if (!p)
	{
		memguard->release();
		perror("malloc failed.");
		exit(errno);
	}
	writeSigs(p, size);
	owInternal = true;
	assert (MemoryTracer);
	MemoryTracer->add(p, file, line, size);
	owInternal = false;
	memguard->release();
	return p;
}
//////////////////////////////////////////////////////////////////////////////
static void
doDelete(void* p)
{
	if (p)
	{
		if (memguard)
		{
			memguard->acquire();
		}
		if (owInternal || disabled)
		{
			if (!disabled)
			{
				fprintf(stderr, "INTERNAL DELETE: %p\n", p);
			}
			free(p);
			if (memguard)
			{
				memguard->release();
			}
			return;
		}
		if (aggressive)
		{
			MemoryTracer->checkMap();
		}
		owInternal = true;
		if (MemoryTracer != 0)
		{
			p = MemoryTracer->remove((void*)((char*)p));
		}
		else
		{
			printf("** MemTracer can't remove delete from map: ADDR: %p\n", p);
		}
		if (!noFree)
		{
			free(p);
		}
		owInternal = false;
		memguard->release();
	}
	if (_shuttingDown)
	{
		memguard->release();
		fprintf(stderr, "delete called\n");
	}
}

} // end namespace BLOCXX_NAMESPACE

//////////////////////////////////////////////////////////////////////////////
void*
operator new[](std::size_t size, char const* file, int line) throw(std::bad_alloc)
{
	return BLOCXX_NAMESPACE::doNew(size, file, line);
}
//////////////////////////////////////////////////////////////////////////////
void*
operator new(std::size_t size, char const* file, int line) throw(std::bad_alloc)
{
	return BLOCXX_NAMESPACE::doNew(size, file, line);
}
//////////////////////////////////////////////////////////////////////////////
void*
operator new[](std::size_t size) throw(std::bad_alloc)
{
	return BLOCXX_NAMESPACE::doNew(size, NULL, 0);
}
//////////////////////////////////////////////////////////////////////////////
void*
operator new(std::size_t size) throw(std::bad_alloc)
{
	return BLOCXX_NAMESPACE::doNew(size, NULL, 0);
}
void
operator delete(void* p)
{
	BLOCXX_NAMESPACE::doDelete(p);
}
void
operator delete[](void* p)
{
	BLOCXX_NAMESPACE::doDelete(p);
}


#endif	// BLOCXX_DEBUG_MEMORY


