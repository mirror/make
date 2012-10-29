/* Loading dynamic objects for GNU Make.
Copyright (C) 2012 Free Software Foundation, Inc.
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

#include "make.h"

#if MAKE_LOAD

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <errno.h>

#define SYMBOL_EXTENSION        "_gmake_setup"

static void *global_dl = NULL;

#include "debug.h"
#include "filedef.h"
#include "variable.h"

static int
init_symbol (const struct floc *flocp, const char *ldname, load_func_t symp)
{
  int r;
  const char *p;
  int nmlen = strlen (ldname);
  char *loaded = allocated_variable_expand("$(.LOADED)");

  /* If it's already been loaded don't do it again.  */
  p = strstr (loaded, ldname);
  r = p && (p==loaded || p[-1]==' ') && (p[nmlen]=='\0' || p[nmlen]==' ');
  free (loaded);
  if (r)
    return 1;

  /* Now invoke the symbol.  */
  r = (*symp) (flocp);

  /* If it succeeded, add the symbol to the loaded variable.  */
  if (r > 0)
    do_variable_definition (flocp, ".LOADED", ldname, o_default, f_append, 0);

  return r;
}

int
load_file (const struct floc *flocp, const char *ldname, int noerror)
{
  load_func_t symp;
  const char *fp;
  char *symname = NULL;
  char *new = alloca (strlen (ldname) + CSTRLEN (SYMBOL_EXTENSION) + 1);

  if (! global_dl)
    {
      global_dl = dlopen (NULL, RTLD_NOW|RTLD_GLOBAL);
      if (! global_dl)
        fatal (flocp, _("Failed to open global symbol table: %s"), dlerror());
    }

  /* If a symbol name was provided, use it.  */
  fp = strchr (ldname, '(');
  if (fp)
    {
      const char *ep;

      /* If there's an open paren, see if there's a close paren: if so use
         that as the symbol name.  We can't have whitespace: it would have
         been chopped up before this function is called.  */
      ep = strchr (fp+1, ')');
      if (ep && ep[1] == '\0')
        {
          int l = fp - ldname;;

          ++fp;
          if (fp == ep)
            fatal (flocp, _("Empty symbol name for load: %s"), ldname);

          /* Make a copy of the ldname part.  */
          memcpy (new, ldname, l);
          new[l] = '\0';
          ldname = new;

          /* Make a copy of the symbol name part.  */
          symname = new + l + 1;
          memcpy (symname, fp, ep - fp);
          symname[ep - fp] = '\0';
        }
    }

  /* If we didn't find a symbol name yet, construct it from the ldname.  */
  if (! symname)
    {
      char *p = new;

      fp = strrchr (ldname, '/');
      if (!fp)
        fp = ldname;
      else
        ++fp;
      while (isalnum (*fp) || *fp == '_')
        *(p++) = *(fp++);
      strcpy (p, SYMBOL_EXTENSION);
      symname = new;
    }

  DB (DB_VERBOSE, (_("Loading symbol %s from %s\n"), symname, ldname));

  /* See if it's already defined.  */
  symp = (load_func_t) dlsym (global_dl, symname);
  if (! symp) {
    void *dlp = dlopen (ldname, RTLD_LAZY|RTLD_GLOBAL);
    if (! dlp)
      {
        if (noerror)
          DB (DB_BASIC, ("%s", dlerror()));
        else
          error (flocp, "%s", dlerror());
        return 0;
      }

    symp = dlsym (dlp, symname);
    if (! symp)
      fatal (flocp, _("Failed to load symbol %s from %s: %s"),
             symname, ldname, dlerror());
  }

  /* Invoke the symbol to initialize the loaded object.  */
  return init_symbol(flocp, ldname, symp);
}

#else

int
load_file (const struct floc *flocp, const char *ldname, int noerror)
{
  if (! noerror)
    fatal (flocp, _("The 'load' operation is not supported on this platform."));

  return 0;
}

#endif  /* MAKE_LOAD */
