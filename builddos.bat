@echo off
rem Copyright (C) 1998-2017 Free Software Foundation, Inc.
rem This file is part of GNU Make.
rem
rem GNU Make is free software; you can redistribute it and/or modify it under
rem the terms of the GNU General Public License as published by the Free
rem Software Foundation; either version 3 of the License, or (at your option)
rem any later version.
rem
rem GNU Make is distributed in the hope that it will be useful, but WITHOUT
rem ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
rem FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for.
rem more details.
rem
rem You should have received a copy of the GNU General Public License along
rem with this program.  If not, see <http://www.gnu.org/licenses/>.

echo Building Make for MSDOS with DJGPP

rem The SmallEnv trick protects against too small environment block,
rem in which case the values will be truncated and the whole thing
rem goes awry.  COMMAND.COM will say "Out of environment space", but
rem many people don't care, so we force them to care by refusing to go.

rem Where is the srcdir?
set XSRC=.
if not "%XSRC%"=="." goto SmallEnv
if "%1%"=="" goto SrcDone
set XSRC=%1
if not "%XSRC%"=="%1" goto SmallEnv

:SrcDone

copy /Y %XSRC%/configh.dos ./config.h

if not exist glob mkdir glob

rem Echo ON so they will see what is going on.
@echo on
gcc -c -I%XSRC% -I%XSRC%/glob -DHAVE_CONFIG_H -O2 -g %XSRC%/commands.c -o commands.o
gcc -c -I%XSRC% -I%XSRC%/glob -DHAVE_CONFIG_H -O2 -g %XSRC%/output.c -o output.o
gcc -c -I%XSRC% -I%XSRC%/glob -DHAVE_CONFIG_H -O2 -g %XSRC%/job.c -o job.o
gcc -c -I%XSRC% -I%XSRC%/glob -DHAVE_CONFIG_H -O2 -g %XSRC%/dir.c -o dir.o
gcc -c -I%XSRC% -I%XSRC%/glob -DHAVE_CONFIG_H -O2 -g %XSRC%/file.c -o file.o
gcc -c -I%XSRC% -I%XSRC%/glob -DHAVE_CONFIG_H -O2 -g %XSRC%/misc.c -o misc.o
gcc -c -I%XSRC% -I%XSRC%/glob -DHAVE_CONFIG_H -O2 -g %XSRC%/main.c -o main.o
gcc -c -I%XSRC% -I%XSRC%/glob -DHAVE_CONFIG_H -DINCLUDEDIR=\"c:/djgpp/include\" -O2 -g %XSRC%/read.c -o read.o
gcc -c -I%XSRC% -I%XSRC%/glob -DHAVE_CONFIG_H -DLIBDIR=\"c:/djgpp/lib\" -O2 -g %XSRC%/remake.c -o remake.o
gcc -c -I%XSRC% -I%XSRC%/glob -DHAVE_CONFIG_H -O2 -g %XSRC%/rule.c -o rule.o
gcc -c -I%XSRC% -I%XSRC%/glob -DHAVE_CONFIG_H -O2 -g %XSRC%/implicit.c -o implicit.o
gcc -c -I%XSRC% -I%XSRC%/glob -DHAVE_CONFIG_H -O2 -g %XSRC%/default.c -o default.o
gcc -c -I%XSRC% -I%XSRC%/glob -DHAVE_CONFIG_H -O2 -g %XSRC%/variable.c -o variable.o
gcc -c -I%XSRC% -I%XSRC%/glob -DHAVE_CONFIG_H -O2 -g %XSRC%/expand.c -o expand.o
gcc -c -I%XSRC% -I%XSRC%/glob -DHAVE_CONFIG_H -O2 -g %XSRC%/function.c -o function.o
gcc -c -I%XSRC% -I%XSRC%/glob -DHAVE_CONFIG_H -O2 -g %XSRC%/vpath.c -o vpath.o
gcc -c -I%XSRC% -I%XSRC%/glob -DHAVE_CONFIG_H -O2 -g %XSRC%/hash.c -o hash.o
gcc -c -I%XSRC% -I%XSRC%/glob -DHAVE_CONFIG_H -O2 -g %XSRC%/strcache.c -o strcache.o
gcc -c -I%XSRC% -I%XSRC%/glob -DHAVE_CONFIG_H -O2 -g %XSRC%/version.c -o version.o
gcc -c -I%XSRC% -I%XSRC%/glob -DHAVE_CONFIG_H -O2 -g %XSRC%/ar.c -o ar.o
gcc -c -I%XSRC% -I%XSRC%/glob -DHAVE_CONFIG_H -O2 -g %XSRC%/arscan.c -o arscan.o
gcc -c -I%XSRC% -I%XSRC%/glob -DHAVE_CONFIG_H -O2 -g %XSRC%/signame.c -o signame.o
gcc -c -I%XSRC% -I%XSRC%/glob -DHAVE_CONFIG_H -O2 -g %XSRC%/remote-stub.c -o remote-stub.o
gcc -c -I%XSRC% -I%XSRC%/glob -DHAVE_CONFIG_H -O2 -g %XSRC%/getopt.c -o getopt.o
gcc -c -I%XSRC% -I%XSRC%/glob -DHAVE_CONFIG_H -O2 -g %XSRC%/getopt1.c -o getopt1.o
gcc -c -I%XSRC% -I%XSRC%/glob -DHAVE_CONFIG_H -O2 -g %XSRC%/glob/glob.c -o glob/glob.o
gcc -c -I%XSRC% -I%XSRC%/glob -DHAVE_CONFIG_H -O2 -g %XSRC%/glob/fnmatch.c -o glob/fnmatch.o
@echo off
echo commands.o > respf.$$$
for %%f in (job output dir file misc main read remake rule implicit default variable) do echo %%f.o >> respf.$$$
for %%f in (expand function vpath hash strcache version ar arscan signame remote-stub getopt getopt1) do echo %%f.o >> respf.$$$
for %%f in (glob/glob glob/fnmatch) do echo %%f.o >> respf.$$$
rem gcc  -c -I%XSRC% -I%XSRC%/glob -DHAVE_CONFIG_H -O2 -g %XSRC%/guile.c -o guile.o
rem echo guile.o >> respf.$$$
@echo Linking...
@echo on
gcc -o make.exe @respf.$$$
@echo off
if not exist make.exe echo Make.exe build failed...
if exist make.exe echo make.exe is now built!
if exist make.exe del respf.$$$
if exist make.exe copy /Y Basic.mk Makefile
goto End

:SmallEnv
echo Your environment is too small.  Please enlarge it and run me again.

:End
set XRSC=
@echo on
