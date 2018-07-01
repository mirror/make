# GNU -*-Makefile-*- to build GNU make on MS-DOS with DJGPP
#
# MS-DOS overrides for use with Basic.mk.
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

OBJEXT = o
EXEEXT = .exe

CC = gcc

prog_SOURCES += $(loadavg_SOURCES) $(glob_SOURCES)

BUILT_SOURCES += $(lib)fnmatch.h $(lib)glob.h

INCLUDEDIR = c:/djgpp/include
LIBDIR = c:/djgpp/lib
LOCALEDIR = c:/djgpp/share

MKDIR = command.com /c mkdir
MKDIR.cmd = $(MKDIR) $(subst /,\\,$@)

RM = command.com /c del /F /Q
RM.cmd = $(RM) $(subst /,\\,$(OBJECTS) $(PROG))

CP = command.com /c copy /Y
CP.cmd = $(CP) $(subst /,\\,$< $@)

$(OUTDIR)src/config.h: $(SRCDIR)/src/configh.dos
	$(CP.cmd)
