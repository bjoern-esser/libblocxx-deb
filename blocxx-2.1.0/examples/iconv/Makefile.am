if HAVE_ICONV_SUPPORT

PROGS = iconv_test1

noinst_PROGRAMS = $(PROGS)


iconv_test1_LDADD = \
-L$(top_builddir)/src/blocxx -lblocxx \
$(SSL_LIBS) $(ICONV_LIB) \
$(BINLINKAPPENDFLAGS)

iconv_test1_LDFLAGS = \
$(CXXBINLINKFLAGS) \
-o iconv_test1$(EXEEXT)

iconv_test1_SOURCES = \
iconv_test1.cpp

INCLUDES = \
-I$(top_srcdir)/src 

REPLIBFLAGS=

endif


