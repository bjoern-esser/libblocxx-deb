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


#ifndef testSharedLibraryLoader_HPP_
#define testSharedLibraryLoader_HPP_

#include "blocxx/SharedLibrary.hpp"
#include "blocxx/SharedLibraryLoader.hpp"
#include "blocxx/ProviderIFCBaseIFC.hpp"
#include "blocxx/ProviderIFCLoader.hpp"
#include "blocxx/LocalCIMOMHandle.hpp"
#include "UnitTestEnvironment.hpp"
#include "blocxx/CIMInstance.hpp"
#include "blocxx/CIMObjectPath.hpp"
#include "blocxx/CIMValue.hpp"
#include "blocxx/NoSuchProviderException.hpp"
#include "blocxx/CIMClass.hpp"
#include "blocxx/LocalOperationContext.hpp"
#include "blocxx/CerrLogger.hpp"

using namespace blocxx;

static int testFunction( int i )
{
	//return i + 0xABCDEF01;
	return 0;
}

static const char* versionFunction()
{
	return BLOCXX_VERSION;
}

typedef int (*fptype)(int);

class testSharedLibrary: public SharedLibrary
{
	public:
		virtual ~testSharedLibrary(){}

	protected:
		virtual bool doGetFunctionPointer( const String&, void** fp ) const
		{
			*fp = (void*)(&testFunction);
			return true;
		}
};

class TestInstanceProvider : public InstanceProviderIFC
{
public:
	virtual void enumInstances(
			const ProviderEnvironmentIFCRef& env,
			const String& ns,
			const String& className,
			CIMInstanceResultHandlerIFC& result,
			WBEMFlags::ELocalOnlyFlag localOnly,
			WBEMFlags::EDeepFlag deep,
			WBEMFlags::EIncludeQualifiersFlag includeQualifiers,
			WBEMFlags::EIncludeClassOriginFlag includeClassOrigin,
			const StringArray* propertyList,
			const CIMClass& requestedClass,
			const CIMClass& cimClass )
	{
	}
	virtual void enumInstanceNames(const ProviderEnvironmentIFCRef &, const String &, const String &, CIMObjectPathResultHandlerIFC &, const CIMClass &)
	{
	}
	virtual CIMInstance getInstance(
		const ProviderEnvironmentIFCRef& env,
		const String& ns,
		const CIMObjectPath& instanceName,
		WBEMFlags::ELocalOnlyFlag localOnly,
		WBEMFlags::EIncludeQualifiersFlag includeQualifiers,
		WBEMFlags::EIncludeClassOriginFlag includeClassOrigin,
		const StringArray* propertyList,
		const CIMClass& cimClass )
	{
		return CIMInstance(CIMNULL);
	}
	virtual CIMObjectPath createInstance(const ProviderEnvironmentIFCRef &, const String &, const CIMInstance &)
	{
		return CIMObjectPath(CIMNULL);
	}
	virtual void modifyInstance(
		const ProviderEnvironmentIFCRef& env,
		const String& ns,
		const CIMInstance& modifiedInstance,
		const CIMInstance& previousInstance,
		WBEMFlags::EIncludeQualifiersFlag includeQualifiers,
		const StringArray* propertyList,
		const CIMClass& theClass)
	{
	}
	virtual void deleteInstance(const ProviderEnvironmentIFCRef &, const String &, const CIMObjectPath &)
	{
	}
};

class TestMethodProvider : public MethodProviderIFC
{
public:
	virtual CIMValue invokeMethod(const ProviderEnvironmentIFCRef &, const String &, const CIMObjectPath &, const String &, const CIMParamValueArray &, CIMParamValueArray &)
	{
		return CIMValue(CIMNULL);
	}
};

#ifndef BLOCXX_DISABLE_ASSOCIATION_TRAVERSAL
class TestAssociatorProvider : public AssociatorProviderIFC
{
public:
	virtual void associatorNames(
		const ProviderEnvironmentIFCRef &,
		CIMObjectPathResultHandlerIFC &,
		const String &,
		const CIMObjectPath &,
		const String &,
		const String &,
		const String &,
		const String &)
	{
	}
	virtual void references(
		const ProviderEnvironmentIFCRef& env,
		CIMInstanceResultHandlerIFC& result,
		const String& ns,
		const CIMObjectPath& objectName,
		const String& resultClass,
		const String& role,
		WBEMFlags::EIncludeQualifiersFlag includeQualifiers,
		WBEMFlags::EIncludeClassOriginFlag includeClassOrigin,
		const StringArray* propertyList)
	{
	}
	virtual void referenceNames(
		const ProviderEnvironmentIFCRef &,
		CIMObjectPathResultHandlerIFC &,
		const String &,
		const CIMObjectPath &,
		const String &,
		const String &)
	{
	}
	virtual void associators(
		const ProviderEnvironmentIFCRef& env,
		CIMInstanceResultHandlerIFC& result,
		const String& ns,
		const CIMObjectPath& objectName,
		const String& assocClass,
		const String& resultClass,
		const String& role,
		const String& resultRole,
		WBEMFlags::EIncludeQualifiersFlag includeQualifiers,
		WBEMFlags::EIncludeClassOriginFlag includeClassOrigin,
		const StringArray* propertyList)
	{
	}
};
#endif

class TestIndicationProvider : public IndicationProviderIFC
{
public:
	virtual void deActivateFilter(const ProviderEnvironmentIFCRef &, const WQLSelectStatement &, const String &, const String&, const StringArray& )
	{
	}
	virtual void activateFilter(const ProviderEnvironmentIFCRef &, const WQLSelectStatement &, const String &, const String&, const StringArray& )
	{
	}
	virtual void authorizeFilter(const ProviderEnvironmentIFCRef &, const WQLSelectStatement &, const String &, const String&, const StringArray&, const String &)
	{
	}
	virtual int mustPoll(const ProviderEnvironmentIFCRef &, const WQLSelectStatement &, const String &, const String&, const StringArray&)
	{
		return 0;
	}
};

class testProviderMux: public ProviderIFCBaseIFC
{
	public:
		testProviderMux( const char* const name )
			: ProviderIFCBaseIFC(), m_name(name) {}
		virtual ~testProviderMux() {}
		virtual void doInit(const ProviderEnvironmentIFCRef&,
			InstanceProviderInfoArray& ia,
			SecondaryInstanceProviderInfoArray& si,
#ifndef BLOCXX_DISABLE_ASSOCIATION_TRAVERSAL
			AssociatorProviderInfoArray& aa,
#endif
			MethodProviderInfoArray& ma,
			IndicationProviderInfoArray& inda)
		{
			if (m_name == "lib1")
			{
				// instance provider registration
				{
					InstanceProviderInfo ipi;
					ipi.setProviderName("TestInstanceProvider");
					ipi.addInstrumentedClass("SelfReg");
					ia.push_back(ipi);
				}
				{
					InstanceProviderInfo ipi;
					ipi.setProviderName("TestInstanceProvider");
					StringArray namespaces;
					namespaces.push_back("root");
					namespaces.push_back("root/good");
					InstanceProviderInfo::ClassInfo ci("SelfRegTwoNamespaces", namespaces);
					ipi.addInstrumentedClass(ci);
					ia.push_back(ipi);
				}
				// method provider registration
				{
					MethodProviderInfo mpi;
					mpi.setProviderName("TestMethodProvider");
					mpi.addInstrumentedClass("SelfReg");
					ma.push_back(mpi);
				}
				{
					MethodProviderInfo mpi;
					mpi.setProviderName("TestMethodProvider");
					StringArray namespaces;
					namespaces.push_back("root");
					namespaces.push_back("root/good");
					StringArray methods;
					MethodProviderInfo::ClassInfo ci("SelfRegTwoNamespaces", namespaces, methods);
					mpi.addInstrumentedClass(ci);
					ma.push_back(mpi);
				}
				{
					MethodProviderInfo mpi;
					mpi.setProviderName("TestMethodProvider");
					StringArray namespaces;
					StringArray methods;
					methods.push_back("TestMethod");
					MethodProviderInfo::ClassInfo ci("SelfRegOneMethod", namespaces, methods);
					mpi.addInstrumentedClass(ci);
					ma.push_back(mpi);
				}
				{
					MethodProviderInfo mpi;
					mpi.setProviderName("TestMethodProvider");
					StringArray namespaces;
					namespaces.push_back("root");
					StringArray methods;
					methods.push_back("TestMethod");
					MethodProviderInfo::ClassInfo ci("SelfRegOneNamespaceOneMethod", namespaces, methods);
					mpi.addInstrumentedClass(ci);
					ma.push_back(mpi);
				}
#ifndef BLOCXX_DISABLE_ASSOCIATION_TRAVERSAL
				// associator provider registration
				{
					AssociatorProviderInfo api;
					api.setProviderName("TestAssociatorProvider");
					api.addInstrumentedClass("SelfReg");
					aa.push_back(api);
				}
				{
					AssociatorProviderInfo api;
					api.setProviderName("TestAssociatorProvider");
					StringArray namespaces;
					namespaces.push_back("root");
					namespaces.push_back("root/good");
					AssociatorProviderInfo::ClassInfo ci("SelfRegTwoNamespaces", namespaces);
					api.addInstrumentedClass(ci);
					aa.push_back(api);
				}
#endif
				// indication provider registration
				{
					IndicationProviderInfo indi;
					indi.setProviderName("TestIndicationProvider");
					indi.addInstrumentedClass("SelfReg");
					inda.push_back(indi);
				}
				{
					IndicationProviderInfo indi;
					indi.setProviderName("TestIndicationProvider");
					StringArray namespaces;
					namespaces.push_back("root");
					namespaces.push_back("root/good");
					IndicationProviderInfo::ClassInfo ci("SelfRegTwoNamespaces", namespaces);
					indi.addInstrumentedClass(ci);
					inda.push_back(indi);
				}
				{
					IndicationProviderInfo indi;
					indi.setProviderName("TestIndicationProvider1");
					indi.addInstrumentedClass("SelfReg2");
					inda.push_back(indi);
				}
				{
					IndicationProviderInfo indi;
					indi.setProviderName("TestIndicationProvider2");
					indi.addInstrumentedClass("SelfReg2");
					inda.push_back(indi);
				}
				{
					// life-cycle type indication
					IndicationProviderInfo indi;
					indi.setProviderName("TestIndicationProvider");
					StringArray namespaces;
					StringArray classes;
					classes.push_back("TestClass1");
					IndicationProviderInfo::ClassInfo ci1("CIM_InstCreation", namespaces, classes);
					indi.addInstrumentedClass(ci1);
					IndicationProviderInfo::ClassInfo ci2("CIM_InstModification", namespaces, classes);
					indi.addInstrumentedClass(ci2);
					IndicationProviderInfo::ClassInfo ci3("CIM_InstDeletion", namespaces, classes);
					indi.addInstrumentedClass(ci3);
					inda.push_back(indi);
				}
				{
					// life-cycle type indication
					IndicationProviderInfo indi;
					indi.setProviderName("TestIndicationProvider2");
					StringArray namespaces;
					StringArray classes;
					classes.push_back("TestClass2");
					classes.push_back("TestClass3");
					IndicationProviderInfo::ClassInfo ci1("CIM_InstCreation", namespaces, classes);
					indi.addInstrumentedClass(ci1);
					IndicationProviderInfo::ClassInfo ci2("CIM_InstModification", namespaces, classes);
					indi.addInstrumentedClass(ci2);
					IndicationProviderInfo::ClassInfo ci3("CIM_InstDeletion", namespaces, classes);
					indi.addInstrumentedClass(ci3);
					inda.push_back(indi);
				}

			}
		}

		virtual InstanceProviderIFCRef doGetInstanceProvider(
			const ProviderEnvironmentIFCRef&, const char* provIdString)
		{
			if (String(provIdString) == "TestInstanceProvider")
			{
				return InstanceProviderIFCRef(new TestInstanceProvider);
			}
			BLOCXX_THROW(NoSuchProviderException, provIdString);
		}

		virtual MethodProviderIFCRef doGetMethodProvider(
			const ProviderEnvironmentIFCRef&, const char* provIdString)
		{
			if (String(provIdString) == "TestMethodProvider")
			{
				return MethodProviderIFCRef(new TestMethodProvider);
			}
			BLOCXX_THROW(NoSuchProviderException, provIdString);
		}

#ifndef BLOCXX_DISABLE_ASSOCIATION_TRAVERSAL
		virtual AssociatorProviderIFCRef doGetAssociatorProvider(
			const ProviderEnvironmentIFCRef&, const char* provIdString)
		{
			if (String(provIdString) == "TestAssociatorProvider")
			{
				return AssociatorProviderIFCRef(new TestAssociatorProvider);
			}
			BLOCXX_THROW(NoSuchProviderException, provIdString);
		}
#endif

		virtual IndicationProviderIFCRef doGetIndicationProvider(
			const ProviderEnvironmentIFCRef&, const char* provIdString)
		{
			if (String(provIdString) == "TestIndicationProvider"
				|| String(provIdString) == "TestIndicationProvider1"
				|| String(provIdString) == "TestIndicationProvider2")
			{
				return IndicationProviderIFCRef(new TestIndicationProvider);
			}
			BLOCXX_THROW(NoSuchProviderException, provIdString);
		}

		virtual const char* getName() const { return m_name.c_str(); }

		virtual PolledProviderIFCRefArray doGetPolledProviders(
			const ProviderEnvironmentIFCRef&)
		{
			return PolledProviderIFCRefArray();
		}

		virtual IndicationExportProviderIFCRefArray doGetIndicationExportProviders(
			const ProviderEnvironmentIFCRef&)
		{
			return IndicationExportProviderIFCRefArray();
		}

		virtual void doUnloadProviders(const ProviderEnvironmentIFCRef&)
		{
		}

private:
	String m_name;

};

static ProviderIFCBaseIFC* testCreateProviderMux()
{
	return new testProviderMux( "lib1" );
}

static ProviderIFCBaseIFC* testCreateProviderMux2()
{
	return new testProviderMux( "lib2" );
}

static ProviderIFCBaseIFC* testCreateProviderMux3()
{
	return new testProviderMux( "lib3" );
}

class testMuxSharedLibrary: public SharedLibrary
{
	public:
		virtual ~testMuxSharedLibrary(){}

	protected:
		virtual bool doGetFunctionPointer( const String& name, void**fp ) const
		{
			if (name == "getOWVersion")
				*fp = (void*)(&versionFunction);
			else
				*fp = (void*)(&testCreateProviderMux);
			return true;
		}
};
			
class testMuxSharedLibrary2: public SharedLibrary
{
	public:
		virtual ~testMuxSharedLibrary2(){}

	protected:
		virtual bool doGetFunctionPointer( const String& name, void**fp ) const
		{
			if (name == "getOWVersion")
				*fp = (void*)(&versionFunction);
			else
				*fp = (void*)(&testCreateProviderMux2);
			return true;
		}
};
			
class testMuxSharedLibrary3: public SharedLibrary
{
	public:
		virtual ~testMuxSharedLibrary3(){}

	protected:
		virtual bool doGetFunctionPointer( const String& name, void**fp ) const
		{
			if (name == "getOWVersion")
				*fp = (void*)(&versionFunction);
			else
				*fp = (void*)(&testCreateProviderMux3);
			return true;
		}
};
			
class testSharedLibraryLoader: public SharedLibraryLoader
{
	public:
		virtual ~testSharedLibraryLoader(){}

		virtual SharedLibraryRef loadSharedLibrary( const String& name ) const
		{
			if ( name == "lib1" )
				return SharedLibraryRef( new testMuxSharedLibrary );
			else if ( name == "lib2" )
				return SharedLibraryRef( new testMuxSharedLibrary2 );
			else if ( name == "lib3" )
				return SharedLibraryRef( new testMuxSharedLibrary3 );
			else
				return SharedLibraryRef( new testSharedLibrary );
		}
};

namespace
{
	class testProviderEnvironment : public ProviderEnvironmentIFC
	{
	public:

		testProviderEnvironment(const LocalCIMOMHandle& ch, LoggerRef l)
		: m_context(), m_ch(new LocalCIMOMHandle(ch)), m_logger(l)
		{}

		testProviderEnvironment()
		: m_context(), m_ch(new LocalCIMOMHandle(CIMOMEnvironmentRef(),RepositoryIFCRef(),m_context)), m_logger(new CerrLogger)
		{}

		virtual CIMOMHandleIFCRef getCIMOMHandle() const
		{
			return m_ch;
		}
		
		virtual CIMOMHandleIFCRef getRepositoryCIMOMHandle() const
		{
			return m_ch;
		}
		
		virtual RepositoryIFCRef getRepository() const
		{
			return RepositoryIFCRef();
		}

		virtual RepositoryIFCRef getAuthorizingRepository() const
		{
			return RepositoryIFCRef();
		}

		virtual String getConfigItem(const String &, const String&) const
		{
			return "";
		}
		virtual StringArray getMultiConfigItem(const String &itemName, 
			const StringArray& defRetVal, const char* tokenizeSeparator = 0) const
		{
			return StringArray();
		}

		virtual String getUserName() const
		{
			return "";
		}

		virtual OperationContext& getOperationContext()
		{
			return m_context;
		}

		virtual ProviderEnvironmentIFCRef clone() const
		{
			return ProviderEnvironmentIFCRef(new testProviderEnvironment());
		}
	private:
		LocalOperationContext m_context;
		CIMOMHandleIFCRef m_ch;
		LoggerRef m_logger;
	};

	ProviderEnvironmentIFCRef createProvEnvRef(const LocalCIMOMHandle& ch)
	{
		LoggerRef log(new CerrLogger);
		log->setLogLevel(E_DEBUG_LEVEL);
		return ProviderEnvironmentIFCRef(new testProviderEnvironment(ch,
			log));
	}
}



class testMuxLoader: public ProviderIFCLoaderBase
{
	public:
		testMuxLoader( SharedLibraryLoaderRef sll ) :
			ProviderIFCLoaderBase(sll, g_testEnvironment)
			{}
		virtual ~testMuxLoader(){}
		virtual void loadIFCs(
				Array<ProviderIFCBaseIFCRef>& out) const
		{
			ProviderIFCBaseIFCRef rval;
			rval = createProviderIFCFromLib( "lib1" );
			if ( rval  )
			{
				out.push_back( rval );
			}

			rval = createProviderIFCFromLib( "lib2" );
			if ( rval )
			{
				out.push_back( rval );
			}

			rval = createProviderIFCFromLib( "lib3" );
			if ( rval )
			{
				out.push_back( rval );
			}

		}
};

class testMuxLoaderBad: public ProviderIFCLoaderBase
{
	public:
		testMuxLoaderBad( SharedLibraryLoaderRef sll ) :
			ProviderIFCLoaderBase(sll, g_testEnvironment)
			{}
		virtual ~testMuxLoaderBad(){}
		virtual void loadIFCs(
				Array<ProviderIFCBaseIFCRef>& out) const
		{
			ProviderIFCBaseIFCRef rval;
			rval = createProviderIFCFromLib( "libbad" );
			if ( rval )
			{
				out.push_back( rval );
			}

			rval = createProviderIFCFromLib( "libbad" );
			if ( rval )
			{
				out.push_back( rval );
			}

			rval = createProviderIFCFromLib( "libbad" );
			if ( rval )
			{
				out.push_back( rval );
			}

		}
};

static SharedLibraryLoaderRef testCreateSharedLibraryLoader();

static ProviderIFCLoaderRef testCreateMuxLoader()
{
	return ProviderIFCLoaderRef( new testMuxLoader(
				testCreateSharedLibraryLoader() ) );
}

static SharedLibraryLoaderRef testCreateSharedLibraryLoader()
{
	return SharedLibraryLoaderRef( new testSharedLibraryLoader );
}

#endif
