/* Provide prerequisite shuffle support.
Copyright (C) 2022 Free Software Foundation, Inc.
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

#include "makeint.h"

#include "shuffle.h"

#include "filedef.h"
#include "dep.h"

/* Supported shuffle modes.  */
static void random_shuffle_array (void ** a, size_t len);
static void reverse_shuffle_array (void ** a, size_t len);
static void identity_shuffle_array (void ** a, size_t len);

/* The way goals and rules are shuffled during update.  */
enum shuffle_mode
  {
    /* No shuffle data is populated or used.  */
    sm_none,
    /* Random within dependency list.  */
    sm_random,
    /* Inverse order.  */
    sm_reverse,
    /* identity order. Differs from SM_NONE by explicitly populating
       the traversal order.  */
    sm_identity,
  };

/* Shuffle configuration.  */
static struct
  {
    enum shuffle_mode mode;
    unsigned int seed;
    void (*shuffler) (void **a, size_t len);
    char strval[INTSTR_LENGTH];
  } config = { sm_none, 0, NULL, "" };

/* Return string value of --shuffle= option passed.
   If none was passed or --shuffle=none was used function
   returns NULL.  */
const char *
shuffle_get_mode ()
{
  return config.strval[0] ? config.strval : NULL;
}

void
shuffle_set_mode (const char *cmdarg)
{
  /* Parse supported '--shuffle' mode.  */
  if (strcasecmp (cmdarg, "random") == 0)
    {
      config.mode = sm_random;
      config.seed = (unsigned int) (time (NULL) ^ make_pid ());
    }
  else if (strcasecmp (cmdarg, "reverse") == 0)
    config.mode = sm_reverse;
  else if (strcasecmp (cmdarg, "identity") == 0)
    config.mode = sm_identity;
  else if (strcasecmp (cmdarg, "none") == 0)
    config.mode = sm_none;
  /* Assume explicit seed if starts from a digit.  */
  else
    {
      const char *err;
      config.mode = sm_random;
      config.seed = make_toui (cmdarg, &err);

      if (err)
        {
          OS (error, NILF, _("invalid shuffle mode: '%s'"), cmdarg);
          die (MAKE_FAILURE);
        }
    }

  switch (config.mode)
    {
    case sm_random:
      config.shuffler = random_shuffle_array;
      sprintf (config.strval, "%u", config.seed);
      break;
    case sm_reverse:
      config.shuffler = reverse_shuffle_array;
      strcpy (config.strval, "reverse");
      break;
    case sm_identity:
      config.shuffler = identity_shuffle_array;
      strcpy (config.strval, "identity");
      break;
    case sm_none:
      config.strval[0] = '\0';
      break;
    }
}

/* Shuffle array elements using RAND().  */
static void
random_shuffle_array (void **a, size_t len)
{
  size_t i;
  for (i = 0; i < len; i++)
    {
      void *t;

      /* Pick random element and swap. */
      unsigned int j = rand () % len;
      if (i == j)
        continue;

      /* Swap. */
      t = a[i];
      a[i] = a[j];
      a[j] = t;
    }
}

/* Shuffle array elements using reverse order.  */
static void
reverse_shuffle_array (void **a, size_t len)
{
  size_t i;
  for (i = 0; i < len / 2; i++)
    {
      void *t;

      /* Pick mirror and swap. */
      unsigned int j = len - 1 - i;

      /* Swap. */
      t = a[i];
      a[i] = a[j];
      a[j] = t;
    }
}

/* Shuffle array elements using identity order.  */
static void
identity_shuffle_array (void **a UNUSED, size_t len UNUSED)
{
  /* No-op!  */
}

/* Shuffle list of dependencies by populating '->next'
   field in each 'struct dep'.  */
static void
shuffle_deps (struct dep *deps)
{
  size_t ndeps = 0;
  struct dep *dep;
  void **da;
  void **dp;

  for (dep = deps; dep; dep = dep->next)
    ndeps++;

  if (ndeps == 0)
    return;

  /* Allocate array of all deps, store, shuffle, write back.  */
  da = xmalloc (sizeof (struct dep *) * ndeps);

  /* Store locally.  */
  for (dep = deps, dp = da; dep; dep = dep->next, dp++)
    *dp = dep;

  /* Shuffle.  */
  config.shuffler (da, ndeps);

  /* Write back.  */
  for (dep = deps, dp = da; dep; dep = dep->next, dp++)
    dep->shuf = *dp;

  free (da);
}

/* Shuffle 'deps' of each 'file' recursively.  */
static void
shuffle_file_deps_recursive (struct file *f)
{
  struct dep *dep;

  /* Implicit rules do not always provide any depends.  */
  if (!f)
    return;

  /* Avoid repeated shuffles and loops.  */
  if (f->was_shuffled)
    return;
  f->was_shuffled = 1;

  shuffle_deps (f->deps);

  /* Shuffle dependencies. */
  for (dep = f->deps; dep; dep = dep->next)
    shuffle_file_deps_recursive (dep->file);
}

/* Shuffle goal dependencies first, then shuffle dependency list
   of each file reachable from goaldep recursively.  Used by
   --shuffle flag to introduce artificial non-determinism in build
   order.  .*/

void
shuffle_deps_recursive (struct dep *deps)
{
  struct dep *dep;

  /* Exit early if shuffling was not requested.  */
  if (config.mode == sm_none)
    return;

  /* Do not reshuffle targets if Makefile is explicitly marked as
     problematic for parallelism.  */
  if (not_parallel)
    return;

  /* Set specific seed at the top level of recursion.  */
  if (config.mode == sm_random)
    srand (config.seed);

  shuffle_deps (deps);

  /* Shuffle dependencies. */
  for (dep = deps; dep; dep = dep->next)
    shuffle_file_deps_recursive (dep->file);
}
