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
:: with this program.  If not, see <http://www.gnu.org/licenses/>.

setlocal
set "svurl=https://git.savannah.gnu.org/cgit"
set "gnuliburl=%svurl%/gnulib.git/plain"

call :Download lib getloadavg.c
call :Download lib intprops.h
goto :Done

:Download
echo Downloading %1\%2
curl -sS -o %1\%2 "%gnuliburl%/%1/%2"
if ERRORLEVEL 1 exit /b 1
goto :EOF

:Done
echo Done.  Run build_w32.bat to build GNU make.
goto :EOF
