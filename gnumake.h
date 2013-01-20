/* External interfaces usable by dynamic objects loaded into GNU Make.
Copyright (C) 2013 Free Software Foundation, Inc.
This file is part of GNU Make.

GNU Make is free software; you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation; either version 3 of the License, or (at your option) any later
version.

GNU Make is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program.  If not, see <http://www.gnu.org/licenses/>.  */

#ifndef _GNUMAKE_H_
#define _GNUMAKE_H_

/* Specify the location of elements read from makefiles.  */
typedef struct
  {
    const char *filenm;
    unsigned long lineno;
  } gmk_floc;

#endif  /* _GNUMAKE_H_ */
