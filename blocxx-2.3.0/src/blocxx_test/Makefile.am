SUBDIRS = \
CppUnit

include $(top_srcdir)/Makefile-shared-lib.inc
all-local : all-blocxx-shared-libs
clean-local : clean-blocxx-shared-libs

include $(top_srcdir)/Makefile-shared-lib-inst.inc
install-exec-local : install-blocxx-shared-libs
uninstall-local : uninstall-blocxx-shared-libs

BLOCXX_SHARED_LIBS = libblocxx_test.$(LIB_EXT)
SHAREDLIBADD = $(DL_LIB) $(SSL_LIBS) $(REGEX_LIB) $(PCRE_LIB) $(ICONV_LIB) -L$(top_builddir)/src/blocxx -lblocxx
libblocxx_test_a_LIBADD = $(top_builddir)/src/blocxx_test/CppUnit/libCppUnit.a

# increment this number every time a release breaks binary compatibility
LIBRARY_BINARY_VERSION = $(BLOCXX_LIBRARY_VERSION)
MAKEDYLIB=1

# We need to install the real library, not a fake version.
NO_FAKE_LIB_INSTALL=1

blocxxstatlib_LIBRARIES = libblocxx_test.a

thelibdir = $(libdir)
blocxxstatlibdir = $(thelibdir)

# Shouldn't depend on anything not in common. DON'T ADD ANYTHING TO THIS LIST!
INCLUDES = -I$(top_srcdir)/src -I$(top_builddir)/src

pkglibdir = $(libdir)/blocxx
pkgincludedir = $(includedir)/blocxx_test

libblocxx_test_a_SOURCES = \
TextUtils.cpp \
FileUtils.cpp \
TestFileSystem.cpp \
FedPipe.cpp \
ZeroFileSystem.cpp \
CannedFileSystemNormalFile.cpp \
CannedFileSystemEntryIFC.cpp \
CannedFileSystem.cpp \
CannedFileSystemImpl.cpp \
RerootFileSystemImpl.cpp \
FileSystemMockObjectScope.cpp \
UnmockedFunctions.cpp

pkginclude_HEADERS = \
TestHackery.hpp \
LogUtils.hpp \
TextUtils.hpp \
FileUtils.hpp \
TestFileSystem.hpp \
FedPipe.hpp \
ZeroFileSystem.hpp \
CannedFileSystemNormalFile.hpp \
CannedFileSystemEntryIFC.hpp \
CannedFileSystem.hpp \
CannedFileSystemImpl.hpp \
RerootFileSystemImpl.hpp \
FileSystemMockObjectScope.hpp \
UnmockedFunctions.hpp
