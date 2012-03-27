PROGS= intRef

noinst_PROGRAMS = $(PROGS)
#check_PROGRAMS = unitMain


intRef_LDADD = \
-L$(top_builddir)/src/blocxx -lblocxx \
$(SSL_LIBS) $(ICONV_LIB) \
$(BINLINKAPPENDFLAGS)

intRef_LDFLAGS = \
$(CXXBINLINKFLAGS) \
-o intRef$(EXEEXT)

intRef_SOURCES = \
COWIntrusiveRefExample.cpp

INCLUDES = \
-I$(top_srcdir)/src 

REPLIBFLAGS=


