SUBDIRS = src test examples doc

m4dir = $(datadir)/aclocal
m4_DATA = blocxx.m4

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = blocxx.pc

EXTRA_DIST = \
LICENSE \
blocxx_ar.sh \
Makefile-shared-lib.inc \
Makefile-shared-lib-inst.inc \
Makefile-replacement.inc \
fake_library_creator.sh \
create_replacement_script.sh \
$(m4_DATA) \
$(pkgconfig_DATA)

BUILT_SOURCES = blocxx.pc

DISTCLEANFILES = \
Doxyfile \
_configs.sed \
replacement_script.sed \
blocxx/BLOCXX_config.h

SUFFIXES = .sh 


.sh: Makefile
	@RM@ -f $@ $@-t
	@SED@ \
		-e 's!@''srcdir''@!$(srcdir)!g' \
	$< > $@-t
	@MV@	$@-t $@

docs: Doxyfile
	$(DOXYGEN) Doxyfile

rpm: dist
	mkdir -p rpmbuild/{SOURCES,SPECS,SRPMS,RPMS,BUILD}
	rpmbuild --define="_topdir `pwd`/rpmbuild" -ta $(distdir).tar.gz

install-data-local:
	$(INSTALL_HEADER) blocxx/BLOCXX_config.h "$(DESTDIR)$(pkgincludedir)/BLOCXX_config.h"

uninstall-local:
	rm -f "$(DESTDIR)$(pkgincludedir)/BLOCXX_config.h"

# Since the replacement script is listed in BUILT_SOURCES, if we include this,
# the replacement sed script will be generated before anything else (even
# subdirectories).
include ${top_srcdir}/Makefile-replacement.inc
