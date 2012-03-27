# This file is to be included wherever substitutions need to be done on files
# that are not listed in the toplevel configure script.  The reason for using
# this instead of directly substituting using ${TOP_BUILDDIR}/config.status
# --file=<FOO>:<FOO>.in is that a good portion of the substitutions depend on
# the shell expansion of other substitutions.  So... If you were to have an
# entry such as:
#   localstatedir=@localstatedir@
# It would then be expanded as (for example):
#   localstatedir=${prefix}/var
#
# This is undesireable in files that have no definitions of ${prefix}, or files
# that are processed by tools that do not understand shell expansion (like
# config files, test cases, etc.)

# The replacement script will now be generated at the end of the configure
# script.  If it is deleted, the configure script will need to be executed again.

# Note: Additional substitutions beyond those defined by the configure process
# can be included by defining one of two things:
# (1) <NAME>_extra_substitutions
# (2) global_substitutions
#
# Example:
# To add a replacement of 'trash' with 'garbage' in the file 'foo', which is
# created from 'foo.in', define something like:
#   foo_extra_substitions=-e 's~trash~garbage~g'
# or
#   global_subtitutions=-e 's~trash~garbage~g'

% : %.in $(top_builddir)/replacement_script.sed
	rm -f $@ $@.partial
	@echo "Creating $@ from $<";
	sed -f $(top_builddir)/replacement_script.sed < $< > $@.partial
	sed -n $($(*)_extra_substitutions) $(global_substitutions) -e p < $@.partial > $@
	rm -f $@.partial
