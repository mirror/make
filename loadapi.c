/* API for GNU Make dynamic objects.
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

#include "gnumake.h"

#include "makeint.h"

#include "filedef.h"
#include "variable.h"

/* Evaluate a buffer as make syntax.
   Ideally eval_buffer() will take const char *, but not yet.  */
void
gmk_eval (const char *buffer, const gmk_floc *floc)
{
  char *s = xstrdup (buffer);
  eval_buffer (s, floc);
  free (s);
}

/* Expand a string and return an allocated buffer.
   Caller must free() this buffer.  */
char *
gmk_expand (const char *ref)
{
  return allocated_variable_expand (ref);
}

/* Register a function to be called from makefiles.  */
void
gmk_add_function (const char *name,
                  char *(*func)(const char *nm, int argc, char **argv),
                  int min, int max, int expand)
{
  define_new_function (reading_file, name, min, max, expand, func);
}
