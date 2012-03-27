noinst_PROGRAMS = sharedLib

sharedLib_SOURCES = \
dummyLib.hpp \
dummyLib.cpp \
sharedLibs.cpp

sharedLib_LDADD = \
-L$(top_builddir)/src/blocxx -lblocxx \
$(SSL_LIBS) $(ICONV_LIB) $(PCRE_LIB) \
$(BINLINKAPPENDFLAGS)

sharedLib_LDFLAGS = \
$(CXXBINLINKFLAGS) \
-o sharedLib$(EXEEXT)

INCLUDES = \
-I$(top_srcdir)/src 

EXTRA_DIST = \
derivedDummyLib.hpp \
derivedDummyLib.cpp \
SharedLibMakefile \
readme


