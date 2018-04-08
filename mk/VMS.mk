# GNU -*-Makefile-*- to build GNU make on VMS
#
# VMS overrides for use with Basic.mk.
#
# Copyright (C) 2017-2018 Free Software Foundation, Inc.
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

src = [.src]
glob = [.glob]
SRCDIR = []

OBJEXT = .obj
EXEEXT = .exe

CP = copy
MKDIR = create/dir
RM = delete

e =
s = $e $e
c = ,

defs = HAVE_CONFIG_H

ifeq ($(CC),cc)
defs += VMS unlink=remove allocated_variable_expand_for_file=alloc_var_expand_for_file
else
defs += GCC_IS_NATIVE
ifeq ($(ARCH),VAX)
defs += VAX
endif
endif

extra_CPPFLAGS = /define=($(subst $s,$c,$(patsubst %,"%",$(defs))))

cinclude = /nested=none/include=($(src),$(glob))
ifeq ($(CC),cc)
cprefix = /prefix=(all,except=(glob,globfree))
cwarn = /standard=relaxed/warn=(disable=questcompare)
endif

extra_CFLAGS = $(cinclude)$(cprefix)$(cwarn)

#extra_LDFLAGS = /deb
extra_LDFLAGS =

# If your system needs extra libraries loaded in, define them here.
# System V probably need -lPW for alloca.
# if on vax, uncomment the following line
#LDLIBS = ,c.opt/opt
ifeq ($(CC),cc)
#LDLIBS =,sys$$library:vaxcrtl.olb/lib
else
LDLIBS =,gnu_cc_library:libgcc.olb/lib
endif

# If your system doesn't have alloca, or the one provided is bad,
# uncomment this
#ALLOCA = $(alloca_SOURCES)

prog_SOURCES += $(ALLOCA) $(glob_SOURCES) $(vms_SOURCES)

COMPILE.cmd = $(CC) $(extra_CFLAGS)$(CFLAGS)/obj=$@ $(extra_CPPFLAGS)$(CPPFLAGS) $<

LINK.cmd = $(LD)$(extra_LDFLAGS)$(LDFLAGS)/exe=$@ $(subst $s,$c,$^)$(LDLIBS)

# Don't know how to do this
CHECK.cmd =

define RM.cmd
	-purge [...]
	-$(RM) $(PROG);
	-$(RM) $(src)*.$(OBJEXT);
endef


$(OUTDIR)$(src)config.h: $(SRCDIR)$(src)config.h.W32
	$(CP.cmd)
