# This is a -*-Makefile-*-, or close enough
#
# Copyright (C) 1997-2017 Free Software Foundation, Inc.
# This file is part of GNU Make.
#
# GNU Make is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation; either version 3 of the License, or (at your option) any later
# version.
#
# GNU Make is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program.  If not, see <http://www.gnu.org/licenses/>.

AUTOMAKE_OPTIONS = dist-bzip2 silent-rules std-options subdir-objects
ACLOCAL_AMFLAGS = -I config

MAKE_HOST =	@MAKE_HOST@

SUBDIRS =	glob config po doc

bin_PROGRAMS =	make
include_HEADERS = gnumake.h

man_MANS =	make.1

make_SRCS =	ar.c arscan.c commands.c commands.h debug.h default.c dep.h \
		dir.c expand.c file.c filedef.h function.c getopt.c getopt.h \
		getopt1.c gettext.h guile.c hash.c hash.h implicit.c job.c \
		job.h load.c loadapi.c main.c makeint.h misc.c os.h output.c \
		output.h read.c remake.c rule.c rule.h signame.c strcache.c \
		variable.c variable.h version.c vpath.c

glob_SRCS =	glob/fnmatch.c glob/fnmatch.h glob/glob.c glob/glob.h

w32_SRCS =	w32/pathstuff.c w32/w32os.c w32/compat/dirent.c \
		w32/compat/posixfcn.c w32/include/dirent.h w32/include/dlfcn.h \
		w32/include/pathstuff.h w32/include/sub_proc.h \
		w32/include/w32err.h w32/subproc/misc.c w32/subproc/proc.h \
		w32/subproc/sub_proc.c w32/subproc/w32err.c

vms_SRCS =	vms_exit.c vms_export_symbol.c vms_progname.c vmsdir.h \
		vmsfunctions.c vmsify.c vmsjobs.c

amiga_SRCS =	amiga.c amiga.h

make_SOURCES =	$(make_SRCS)
EXTRA_make_SOURCES = $(amiga_SRCS) $(vms_SRCS)

make_LDADD =	$(GUILE_LIBS) $(LIBOBJS) $(ALLOCA) $(GLOBLIB) \
		@GETLOADAVG_LIBS@ @LIBINTL@

localedir =	$(datadir)/locale

AM_CPPFLAGS =	-DLIBDIR=\"$(libdir)\" -DINCLUDEDIR=\"$(includedir)\" \
		-DLOCALEDIR=\"$(localedir)\" $(GLOBINC)

AM_CFLAGS =	$(GUILE_CFLAGS)

if WINDOWSENV
  make_SOURCES += $(w32_SRCS)
  AM_CPPFLAGS  += -I $(top_srcdir)/w32/include
else
  make_SOURCES += posixos.c
endif

if USE_CUSTOMS
  make_SOURCES += remote-cstms.c
else
  make_SOURCES += remote-stub.c
endif

# Extra stuff to include in the distribution.

mk_FILES =	Basic.mk mk/Amiga.mk mk/Windows32.mk mk/msdosdjgpp.mk

# test/scripts are added via dist-hook below.
test_FILES =	tests/run_make_tests tests/run_make_tests.bat \
		tests/run_make_tests.pl tests/test_driver.pl \
		tests/config-flags.pm.in tests/config_flags_pm.com \
		tests/mkshadow tests/jhelp.pl tests/guile.supp tests/README

EXTRA_DIST =	ChangeLog README build.sh.in $(man_MANS) \
		README.customs README.OS2 \
		README.Amiga SCOPTIONS config.ami \
		README.DOS builddos.bat configh.dos \
		README.W32 config.h.W32 build_w32.bat \
		README.VMS makefile.vms makefile.com config.h-vms \
		vms_export_symbol_test.com \
		gmk-default.scm gmk-default.h \
		$(mk_FILES) $(test_FILES)


# This is built during configure, but behind configure's back

DISTCLEANFILES = build.sh

# --------------- Local INSTALL Section

# If necessary, change the gid of the app and turn on the setgid flag.
#

# Whether or not make needs to be installed setgid.
# The value should be either 'true' or 'false'.
# On some systems, the getloadavg function (used to implement the '-l'
# switch) will not work unless make is installed setgid kmem.
#
inst_setgid = @NEED_SETGID@

# Install make setgid to this group so it can get the load average.
#
inst_group = @KMEM_GROUP@

install-exec-local:
	@if $(inst_setgid); then \
	   app=$(DESTDIR)$(bindir)/`echo $(bin_PROGRAMS)|sed '$(transform)'`; \
	   if chgrp $(inst_group) $$app && chmod g+s $$app; then \
	     echo "chgrp $(inst_group) $$app && chmod g+s $$app"; \
	   else \
	     echo "$$app needs to be owned by group $(inst_group) and setgid;"; \
	     echo "otherwise the '-l' option will probably not work."; \
	     echo "You may need special privileges to complete the installation"; \
	     echo "of $$app."; \
	   fi; \
	 else true; fi

# --------------- Generate the Guile default module content

guile.$(OBJEXT): gmk-default.h
gmk-default.h: $(srcdir)/gmk-default.scm
	(echo 'static const char *const GUILE_module_defn = " '\\ \
	  && sed -e 's/;.*//' -e '/^[ \t]*$$/d' -e 's/"/\\"/g' -e 's/$$/ \\/' \
		 $(srcdir)/gmk-default.scm \
	  && echo '";') > $@

# --------------- Local DIST Section

# Install the mk and tests subdirectories
#
dist-hook:
	(cd $(srcdir); \
	 sub=`find tests/scripts -follow \( -name .git -o -name .deps -o -name work -o -name .gitignore -o -name \*.orig -o -name \*.rej -o -name \*~ -o -name \*.out -o -name Makefile \) -prune -o -type f -print`; \
	 tar chf - $$sub) \
	| (cd $(distdir); tar xfBp -)


# --------------- Local CHECK Section

check-local: check-regression check-loadavg
	@banner=" Regression PASSED: GNU Make $(VERSION) ($(MAKE_HOST)) built with $(CC) "; \
	dashes=`echo "$$banner" | sed s/./=/g`; \
	echo; \
	echo "$$dashes"; \
	echo "$$banner"; \
	echo "$$dashes"; \
	echo

.PHONY: check-loadavg check-regression

check-loadavg: loadavg$(EXEEXT)
	@echo The system uptime program believes the load average to be:
	-uptime
	@echo The GNU load average checking code thinks:
	-./loadavg$(EXEEXT)

# The loadavg function is invoked during "make check" to test getloadavg.
check_PROGRAMS = loadavg
nodist_loadavg_SOURCES = getloadavg.c
loadavg_CPPFLAGS = -DTEST
loadavg_LDADD = @GETLOADAVG_LIBS@

# > check-regression
#
# Look for the make test suite, and run it if found and we can find perl.
# If we're building outside the tree, we use symlinks to make a local copy of
# the test suite.  Unfortunately the test suite itself isn't localizable yet.
#
MAKETESTFLAGS =

check-regression: tests/config-flags.pm
	@if test -f '$(srcdir)/tests/run_make_tests'; then \
	  ulimit -n 128; \
	  if $(PERL) -v >/dev/null 2>&1; then \
	    case `cd '$(srcdir)'; pwd` in `pwd`) : ;; \
	      *) test -d tests || mkdir tests; \
		 rm -f srctests; \
		 if ln -s '$(srcdir)/tests' srctests; then \
		   for f in run_make_tests run_make_tests.pl test_driver.pl scripts jhelp.pl; do \
		     rm -f tests/$$f; ln -s ../srctests/$$f tests; \
		   done; fi ;; \
	    esac; \
	    echo "cd tests && $(PERL) ./run_make_tests.pl -srcdir $(abs_srcdir) -make ../make$(EXEEXT) $(MAKETESTFLAGS)"; \
	    cd tests && $(PERL) ./run_make_tests.pl -srcdir '$(abs_srcdir)' -make '../make$(EXEEXT)' $(MAKETESTFLAGS); \
	  else \
	    echo "Can't find a working Perl ($(PERL)); the test suite requires Perl."; \
	  fi; \
	else \
	  echo "Can't find the GNU Make test suite ($(srcdir)/tests)."; \
	fi


# --------------- Maintainer's Section

# Tell automake that I haven't forgotten about this file and it will be
# created before we build a distribution (see maintMakefile in the Git
# distribution).

README:

@MAINT_MAKEFILE@
