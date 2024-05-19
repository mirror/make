/* Control warning output in GNU Make.
Copyright (C) 2023-2024 Free Software Foundation, Inc.
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
#include "warning.h"
#include "variable.h"

/* Current action for each warning.  */
enum warning_action warnings[wt_max];

/* The default behavior of warnings.  */
static struct warning_data warn_default;

/* Warning settings from the .WARNING variable.  */
static struct warning_data warn_variable;

/* Warning settings from the command line.  */
static struct warning_data warn_flag;

static const char *w_action_map[w_error+1] = {NULL, "ignore", "warn", "error"};
static const char *w_name_map[wt_max] = {
                                          "circular-dep",
                                          "invalid-ref",
                                          "invalid-var",
                                          "undefined-var"
                                        };

#define encode_warn_action(_b,_s) \
    variable_buffer_output (_b, w_action_map[_s], strlen (w_action_map[_s]))
#define encode_warn_name(_b,_t)   \
    variable_buffer_output (_b, w_name_map[_t], strlen (w_name_map[_t]))

static void set_warnings ()
{
  /* Called whenever any warnings could change; resets the current actions.  */
  for (enum warning_type wt = 0; wt < wt_max; ++wt)
    warnings[wt] =
        warn_flag.actions[wt]     != w_unset ? warn_flag.actions[wt]
      : warn_flag.global          != w_unset ? warn_flag.global
      : warn_variable.actions[wt] != w_unset ? warn_variable.actions[wt]
      : warn_variable.global      != w_unset ? warn_variable.global
      : warn_default.actions[wt];
}

void
warn_init ()
{
  memset (&warn_default, '\0', sizeof (warn_default));
  memset (&warn_variable, '\0', sizeof (warn_variable));
  memset (&warn_flag, '\0', sizeof (warn_flag));

  /* All warnings must have a default.  */
  warn_default.global = w_warn;
  warn_default.actions[wt_circular_dep] = w_warn;
  warn_default.actions[wt_invalid_ref] = w_warn;
  warn_default.actions[wt_invalid_var] = w_warn;
  warn_default.actions[wt_undefined_var] = w_ignore;

  set_warnings ();
}

static void
init_data (struct warning_data *data)
{
  data->global = w_unset;
  for (enum warning_type wt = 0; wt < wt_max; ++wt)
    data->actions[wt] = w_unset;
}

static enum warning_action
decode_warn_action (const char *action, size_t length)
{
  for (enum warning_action st = w_ignore; st <= w_error; ++st)
    {
      size_t len = strlen (w_action_map[st]);
      if (length == len && strncasecmp (action, w_action_map[st], length) == 0)
        return st;
    }

  return w_unset;
}

static enum warning_type
decode_warn_name (const char *name, size_t length)
{
  for (enum warning_type wt = 0; wt < wt_max; ++wt)
    {
      size_t len = strlen (w_name_map[wt]);
      if (length == len && strncasecmp (name, w_name_map[wt], length) == 0)
        return wt;
    }

  return wt_max;
}

void
decode_warn_actions (const char *value, const floc *flocp)
{
  struct warning_data *data = &warn_flag;

  NEXT_TOKEN (value);

  if (flocp)
    {
      data = &warn_variable;
      /* When a variable is set to empty, reset everything.  */
      if (*value == '\0')
        init_data (data);
    }

  while (*value != '\0')
    {
      enum warning_action action;

      /* Find the end of the next warning definition.  */
      const char *ep = value;
      while (! STOP_SET (*ep, MAP_BLANK|MAP_COMMA|MAP_NUL))
        ++ep;

      /* If the value is just an action set it globally.  */
      action = decode_warn_action (value, ep - value);
      if (action != w_unset)
        data->global = action;
      else
        {
          enum warning_type type;
          const char *cp = memchr (value, ':', ep - value);
          int wl, al;

          if (!cp)
            cp = ep;
          wl = (int)(cp - value);
          type = decode_warn_name (value, wl);
          if (cp == ep)
            action = w_warn;
          else
            {
              /* There's a warning action: decode it.  */
              ++cp;
              al = (int)(ep - cp);
              action = decode_warn_action (cp, al);
            }

          if (type == wt_max)
            {
              if (!flocp)
                ONS (fatal, NILF, _("unknown warning '%.*s'"), wl, value);
              ONS (error, flocp,
                   _("unknown warning '%.*s': ignored"), wl, value);
            }
          else if (action == w_unset)
            {
              if (!flocp)
                ONS (fatal, NILF,
                     _("unknown warning action '%.*s'"), al, cp);
              ONS (error, flocp,
                   _("unknown warning action '%.*s': ignored"), al, cp);
            }
          else
            data->actions[type] = action;
        }

      value = ep;
      while (STOP_SET (*value, MAP_BLANK|MAP_COMMA))
        ++value;
    }

  set_warnings ();
}

char *
encode_warn_flag (char *fp)
{
  enum warning_type wt;
  char sp = '=';

  /* See if any warning options are set.  */
  for (wt = 0; wt < wt_max; ++wt)
    if (warn_flag.actions[wt] != w_unset)
      break;
  if (wt == wt_max && warn_flag.global == w_unset)
    return fp;

  /* Something is set so construct a --warn option.  */
  fp = variable_buffer_output (fp, STRING_SIZE_TUPLE (" --warn"));

  /* If only a global action set to warn, we're done.  */
  if (wt == wt_max && warn_flag.global == w_warn)
    return fp;

  /* If a global action is set, add it.  */
  if (warn_flag.global > w_unset)
    {
      fp = variable_buffer_output (fp, &sp, 1);
      sp = ',';
      fp = encode_warn_action (fp, warn_flag.global);
    }

  /* Add any specific actions.  */
  if (wt != wt_max)
    for (wt = 0; wt < wt_max; ++wt)
      {
        enum warning_action act = warn_flag.actions[wt];
        if (act > w_unset)
          {
            fp = variable_buffer_output (fp, &sp, 1);
            sp = ',';
            fp = encode_warn_name (fp, wt);
            if (act != w_warn)
              fp = encode_warn_action (variable_buffer_output (fp, ":", 1), act);
          }
      }

  return fp;
}

void
warn_get_vardata (struct warning_data *data)
{
  memcpy (data, &warn_variable, sizeof (warn_variable));
}

void
warn_set_vardata (const struct warning_data *data)
{
  memcpy (&warn_variable, data, sizeof (warn_variable));
  set_warnings ();
}
