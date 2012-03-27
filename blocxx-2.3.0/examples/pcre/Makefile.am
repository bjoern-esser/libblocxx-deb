PROGS = \
execute \
capture \
replace \
grep \
split \
match

noinst_PROGRAMS = $(PROGS)

COMMON_LDADD =  \
-L$(top_builddir)/src/blocxx -lblocxx \
$(SSL_LIBS) $(ICONV_LIB) $(PCRE_LIB)\
$(BINLINKAPPENDFLAGS)

COMMON_LDFLAGS = $(CXXBINLINKFLAGS) -o $@

execute_SOURCES = execute.cpp
execute_LDADD = $(COMMON_LDADD)
execute_LDFLAGS = $(COMMON_LDFLAGS)

capture_SOURCES = capture.cpp
capture_LDADD = $(COMMON_LDADD)
capture_LDFLAGS = $(COMMON_LDFLAGS)

replace_SOURCES = replace.cpp
replace_LDADD = $(COMMON_LDADD)
replace_LDFLAGS = $(COMMON_LDFLAGS)

grep_SOURCES    = grep.cpp
grep_LDADD = $(COMMON_LDADD)
grep_LDFLAGS = $(COMMON_LDFLAGS)

split_SOURCES   = split.cpp
split_LDADD = $(COMMON_LDADD)
split_LDFLAGS = $(COMMON_LDFLAGS)

match_SOURCES   = match.cpp
match_LDADD = $(COMMON_LDADD)
match_LDFLAGS = $(COMMON_LDFLAGS)

INCLUDES = -I$(top_srcdir)/src 

REPLIBFLAGS=
