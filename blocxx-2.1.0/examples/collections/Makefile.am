PROGS=mapDemo listDemo arrayDemo

noinst_PROGRAMS = $(PROGS)

mapDemo_LDADD = \
-L$(top_builddir)/src/blocxx -lblocxx \
$(SSL_LIBS) $(ICONV_LIB) \
$(BINLINKAPPENDFLAGS)

mapDemo_LDFLAGS = \
$(CXXBINLINKFLAGS) \
-o mapDemo$(EXEEXT)

mapDemo_SOURCES = \
mapDemo.cpp



listDemo_LDADD = \
-L$(top_builddir)/src/blocxx -lblocxx \
$(SSL_LIBS) $(ICONV_LIB) \
$(BINLINKAPPENDFLAGS)

listDemo_LDFLAGS = \
$(CXXBINLINKFLAGS) \
-o listDemo$(EXEEXT)

listDemo_SOURCES = \
listDemo.cpp




arrayDemo_LDADD = \
-L$(top_builddir)/src/blocxx -lblocxx \
$(SSL_LIBS) $(ICONV_LIB) \
$(BINLINKAPPENDFLAGS)

arrayDemo_LDFLAGS = \
$(CXXBINLINKFLAGS) \
-o arrayDemo$(EXEEXT)

arrayDemo_SOURCES = \
arrayDemo.cpp




INCLUDES = \
-I$(top_srcdir)/src 

REPLIBFLAGS=


