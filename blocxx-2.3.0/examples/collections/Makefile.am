PROGS = \
mapDemo \
listDemo \
arrayDemo

noinst_PROGRAMS = $(PROGS)

COMMON_LDADD =  \
-L$(top_builddir)/src/blocxx -lblocxx \
$(SSL_LIBS) $(ICONV_LIB) $(PCRE_LIB) \
$(BINLINKAPPENDFLAGS)

COMMON_LDFLAGS = $(CXXBINLINKFLAGS) -o $@

mapDemo_LDADD = $(COMMON_LDADD)
mapDemo_LDFLAGS = $(COMMON_LDFLAGS)
mapDemo_SOURCES = mapDemo.cpp

listDemo_LDADD = $(COMMON_LDADD)
listDemo_LDFLAGS = $(COMMON_LDFLAGS)
listDemo_SOURCES = listDemo.cpp

arrayDemo_LDADD = $(COMMON_LDADD)
arrayDemo_LDFLAGS = $(COMMON_LDFLAGS)
arrayDemo_SOURCES = arrayDemo.cpp

INCLUDES = -I$(top_srcdir)/src 

REPLIBFLAGS=
