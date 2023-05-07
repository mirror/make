/* Loading dynamic objects for GNU Make.
Copyright (C) 2012-2023 Free Software Foundation, Inc.
This file is part of GNU Make.

GNU Make is free software; you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation; either version 3 of the License, or (at your option) any later
version.

GNU Make is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program.  If not, see <https://www.gnu.org/licenses/>.  */

#include "makeint.h"

#if MAKE_LOAD

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <errno.h>

#include "debug.h"
#include "filedef.h"
#include "variable.h"

/* Tru64 V4.0 does not have this flag */
#ifndef RTLD_GLOBAL
# define RTLD_GLOBAL 0
#endif

#define GMK_SETUP       "_gmk_setup"
#define GMK_UNLOAD      "_gmk_unload"

typedef int (*setup_func_t)(unsigned int abi, const floc *flocp);
typedef void (*unload_func_t)(void);

struct load_list
  {
    struct load_list *next;
    const char *name;
    void *dlp;
    unload_func_t unload;
  };

static struct load_list *loaded_syms = NULL;

static setup_func_t
load_object (const floc *flocp, int noerror, const char *ldname,
             const char *setupnm)
{
  static void *global_dl = NULL;
  char *buf;
  const char *fp;
  char *endp;
  void *dlp;
  struct load_list *new;
  setup_func_t symp;

  if (! global_dl)
    {
      global_dl = dlopen (NULL, RTLD_NOW|RTLD_GLOBAL);
      if (! global_dl)
        {
          const char *err = dlerror ();
          OS (fatal, flocp, _("failed to open global symbol table: %s"), err);
        }
    }

  /* Find the prefix of the ldname.  */
  fp = strrchr (ldname, '/');
#ifdef HAVE_DOS_PATHS
  if (fp)
    {
      const char *fp2 = strchr (fp, '\\');

      if (fp2 > fp)
        fp = fp2;
    }
  else
    fp = strrchr (ldname, '\\');
  /* The (improbable) case of d:foo.  */
  if (fp && *fp && fp[1] == ':')
    fp++;
#endif
  if (!fp)
    fp = ldname;
  else
    ++fp;

  endp = buf = alloca (strlen (fp) + CSTRLEN (GMK_UNLOAD) + 1);
  while (isalnum ((unsigned char) *fp) || *fp == '_')
    *(endp++) = *(fp++);

  /* If we didn't find a symbol name yet, construct it from the prefix.  */
  if (! setupnm)
    {
      memcpy (endp, GMK_SETUP, CSTRLEN (GMK_SETUP) + 1);
      setupnm = buf;
    }

  DB (DB_VERBOSE, (_("Loading symbol %s from %s\n"), setupnm, ldname));

  symp = (setup_func_t) dlsym (global_dl, setupnm);
  if (symp)
    return symp;

  /* If the path has no "/", try the current directory first.  */
  dlp = NULL;
  if (! strchr (ldname, '/')
#ifdef HAVE_DOS_PATHS
      && ! strchr (ldname, '\\')
#endif
      )
    dlp = dlopen (concat (2, "./", ldname), RTLD_LAZY|RTLD_GLOBAL);

  /* If we haven't opened it yet, try the default search path.  */
  if (! dlp)
    dlp = dlopen (ldname, RTLD_LAZY|RTLD_GLOBAL);

  /* Still no?  Then fail.  */
  if (! dlp)
    {
      const char *err = dlerror ();
      if (noerror)
        DB (DB_BASIC, ("%s\n", err));
      else
        OS (error, flocp, "%s", err);
      return NULL;
    }

  DB (DB_VERBOSE, (_("Loaded shared object %s\n"), ldname));

  /* Assert that the GPL license symbol is defined.  */
  symp = (setup_func_t) dlsym (dlp, "plugin_is_GPL_compatible");
  if (! symp)
    OS (fatal, flocp,
        _("loaded object %s is not declared to be GPL compatible"), ldname);

  symp = (setup_func_t) dlsym (dlp, setupnm);
  if (! symp)
    {
      const char *err = dlerror ();
      OSSS (fatal, flocp, _("failed to load symbol %s from %s: %s"),
            setupnm, ldname, err);
    }

  new = xcalloc (sizeof (struct load_list));
  new->next = loaded_syms;
  loaded_syms = new;
  new->name = ldname;
  new->dlp = dlp;

  /* Compute the name of the unload function and look it up.  */
  memcpy (endp, GMK_UNLOAD, CSTRLEN (GMK_UNLOAD) + 1);

  new->unload = (unload_func_t) dlsym (dlp, buf);
  if (new->unload)
    DB (DB_VERBOSE, (_("Detected symbol %s in %s\n"), buf, ldname));

  return symp;
}

int
load_file (const floc *flocp, struct file *file, int noerror)
{
  const char *ldname = file->name;
  char *buf;
  char *setupnm = NULL;
  const char *fp;
  int r;
  setup_func_t symp;

  /* Break the input into an object file name and a symbol name.  If no symbol
     name was provided, compute one from the object file name.  */
  fp = strchr (ldname, '(');
  if (fp)
    {
      const char *ep;

      /* There's an open paren, so see if there's a close paren: if so use
         that as the symbol name.  We can't have whitespace: it would have
         been chopped up before this function is called.  */
      ep = strchr (fp+1, ')');
      if (ep && ep[1] == '\0')
        {
          size_t l = fp - ldname;

          ++fp;
          if (fp == ep)
            OS (fatal, flocp, _("empty symbol name for load: %s"), ldname);

          /* Make a copy of the ldname part.  */
          buf = alloca (strlen (ldname) + 1);
          memcpy (buf, ldname, l);
          buf[l] = '\0';
          ldname = buf;

          /* Make a copy of the symbol name part.  */
          setupnm = buf + l + 1;
          memcpy (setupnm, fp, ep - fp);
          setupnm[ep - fp] = '\0';
        }
    }

  /* Make sure this name is in the string cache.  */
  ldname = file->name = strcache_add (ldname);

  /* If this object has been loaded, we're done: return -1 to ensure make does
     not rebuild again.  If a rebuild is allowed it was set up when this
     object was initially loaded.  */
  file = lookup_file (ldname);
  if (file && file->loaded)
    return -1;

  /* Load it!  */
  symp = load_object (flocp, noerror, ldname, setupnm);
  if (! symp)
    return 0;

  /* Invoke the setup function.  */
  {
    unsigned int abi = GMK_ABI_VERSION;
    r = (*symp) (abi, flocp);
  }

  /* If the load didn't fail, add the file to the .LOADED variable.  */
  if (r)
    do_variable_definition(flocp, ".LOADED", ldname, o_file, f_append_value, 0);

  return r;
}

int
unload_file (const char *name)
{
  struct load_list **dp = &loaded_syms;

  /* Unload and remove the entry for this file.  */
  while (*dp != NULL)
    {
      struct load_list *d = *dp;

      if (streq (d->name, name))
        {
          int rc;

          DB (DB_VERBOSE, (_("Unloading shared object %s\n"), name));

          if (d->unload)
            (*d->unload) ();

          rc = dlclose (d->dlp);
          if (rc)
            perror_with_name ("dlclose: ", d->name);

          *dp = d->next;
          free (d);
          return rc;
        }

      dp = &d->next;
    }

  return 0;
}

void
unload_all ()
{
  while (loaded_syms)
    {
      struct load_list *d = loaded_syms;
      loaded_syms = loaded_syms->next;

      if (d->unload)
        (*d->unload) ();

      if (dlclose (d->dlp))
        perror_with_name ("dlclose: ", d->name);

      free (d);
    }
}

#else

int
load_file (const floc *flocp, struct file *file UNUSED, int noerror)
{
  if (! noerror)
    O (fatal, flocp,
       _("'load' is not supported on this platform"));

  return 0;
}

int
unload_file (const char *name UNUSED)
{
  O (fatal, NILF, "INTERNAL: cannot unload when load is not supported");
}

void
unload_all ()
{
}

#endif  /* MAKE_LOAD */
