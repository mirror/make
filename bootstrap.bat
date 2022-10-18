@echo off
:: Copyright (C) 2018-2022 Free Software Foundation, Inc.
:: This file is part of GNU Make.
::
:: GNU Make is free software; you can redistribute it and/or modify it under
:: the terms of the GNU General Public License as published by the Free
:: Software Foundation; either version 3 of the License, or (at your option)
:: any later version.
::
:: GNU Make is distributed in the hope that it will be useful, but WITHOUT
:: ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
:: FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for.
:: more details.
::
:: You should have received a copy of the GNU General Public License along
:: with this program.  If not, see <https://www.gnu.org/licenses/>.

setlocal
set "svurl=https://git.savannah.gnu.org/cgit"
set "gnuliburl=%svurl%/gnulib.git/plain"

where curl >nul 2>&1
if ERRORLEVEL 1 (
    echo Cannot find curl: it must be installed for bootstrap
    exit /b 1
)

where sed >nul 2>&1
if ERRORLEVEL 1 (
    echo Cannot find sed: it must be installed for bootstrap
    echo Hint: you can use the sed provided in the Git for Windows install
    exit /b 1
)

if exist lib goto Downloads
mkdir lib
if ERRORLEVEL 1 exit /b 1

:Downloads
echo -- Downloading Gnulib modules
call :Download lib getloadavg.c
call :Download lib intprops.h
call :Download lib intprops-internal.h

echo -- Configuring the workspace
copy /Y gl\lib\*.* lib > nul

:: Create a sed script to convert templates
if exist convert.sed del /Q convert.sed
echo s,@PACKAGE@,make,g > convert.sed
if ERRORLEVEL 1 goto Failed
echo s,@PACKAGE_BUGREPORT@,bug-make@gnu.org,g >> convert.sed
if ERRORLEVEL 1 goto Failed
echo s,@PACKAGE_NAME@,GNU Make,g >> convert.sed
if ERRORLEVEL 1 goto Failed
echo s,@PACKAGE_TARNAME@,make,g >> convert.sed
if ERRORLEVEL 1 goto Failed
echo s,@PACKAGE_URL@,https://www.gnu.org/software/make/,g >> convert.sed
sed -n "s/^AC_INIT(\[GNU.Make\],\[\([0-9.]*\)\].*/s,@PACKAGE_VERSION@,\1,g/p" configure.ac >> convert.sed
if ERRORLEVEL 1 goto Failed
sed -z -e s/\\\n//g -e "s/[ \t][ \t]*/ /g" -e "s, [^ ]*\.h,,g" -e "s,src/,$(src),g" -e "s,lib/,$(lib),g" Makefile.am | sed -n "s/^\([A-Za-z0-9]*\)_SRCS *= *\(.*\)/s,%%\1_SOURCES%%,\2,/p" >> convert.sed
if ERRORLEVEL 1 goto Failed

echo - Creating Basic.mk
sed -f convert.sed Basic.mk.template > Basic.mk
if ERRORLEVEL 1 goto Failed
echo - Creating src\mkconfig.h
sed -f convert.sed src\mkconfig.h.in > src\mkconfig.h
if ERRORLEVEL 1 goto Failed

echo - Creating src\gmk-default.h
echo static const char *const GUILE_module_defn = ^" \ > src\gmk-default.h
sed -e "s/;.*//" -e "/^[ \t]*$/d" -e "s/\"/\\\\\"/g" -e "s/$/ \\\/" src\gmk-default.scm >> src\gmk-default.h
if ERRORLEVEL 1 goto Failed
echo ^";>> src\gmk-default.h

echo.
echo Done.  Run build_w32.bat to build GNU make.
goto :EOF

:Download
if exist "%1\%2" goto :EOF
echo - Downloading %1\%2
curl -sS -o "%1\%2" "%gnuliburl%/%1/%2"
if ERRORLEVEL 1 exit /b 1
goto :EOF

:Failed
echo *** Bootstrap failed.
echo Resolve the issue, or use the configured source in the release tarball
exit /b 1
