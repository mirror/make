# GNU -*-Makefile-*- to build GNU make on MS-DOS with DJGPP
#
# MS-DOS overrides for use with Makebase.mk.
#
# Copyright (C) 2017 Free Software Foundation, Inc.
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

prog_SOURCES += getloadavg.c $(glob_SOURCES)

extra_CPPFLAGS += -I$(SRCDIR)/glob -DINCLUDEDIR=\"c:/djgpp/include\" -DLIBDIR=\"c:/djgpp/lib\"

MKDIR.cmd = command.com /c mkdir $(subst /,\\,$@)
RM.cmd = command.com /c del /F /Q $(subst /,\\,$(OBJECTS) $(PROG))

$(OUTDIR)/config.h: $(SRCDIR)/configh.dos
	command.com /c copy /Y $(subst /,\\,$< $@)
