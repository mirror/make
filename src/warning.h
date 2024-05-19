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

/* Types of warnings we can show.
   These can be rearranged but the first value must be 0.  */
enum warning_type
  {
    wt_circular_dep = 0, /* A target depends on itself.  */
    wt_invalid_ref,      /* Reference an invalid variable name.  */
    wt_invalid_var,      /* Assign to an invalid variable name.  */
    wt_undefined_var,    /* Reference an undefined variable name.  */
    wt_max
  };

/* Action taken for a given warning.  Unset must be 0.  */
enum warning_action
  {
    w_unset = 0,
    w_ignore,
    w_warn,
    w_error
  };

struct warning_data
  {
    enum warning_action global;          /* Global setting.  */
    enum warning_action actions[wt_max]; /* Action for each warning type.  */
  };

/* Actions taken for each warning.  */
extern enum warning_action warnings[wt_max];

/* Get the current action for a given warning.  */
#define warn_get(_w)     (warnings[_w])

/* Set the current actin for a given warning.  Can't use w_unset here.
   This should only be used for temporary resetting of warnings.  */
#define warn_set(_w,_f)  do{ warnings[_w] = (_f); }while(0)

/* True if we should check for the warning in the first place.  */
#define warn_check(_w)   (warn_get (_w) > w_ignore)

/* Check if the warning is ignored.  */
#define warn_ignored(_w) (warn_get (_w) == w_ignore)

/* Check if the warning is in "warn" mode.  */
#define warn_warned(_w)  (warn_get (_w) == w_warn)

/* Check if the warning is in "error" mode.  */
#define warn_error(_w)   (warn_get (_w) == w_error)

void warn_init (void);
void decode_warn_actions (const char *value, const floc *flocp);
char *encode_warn_flag (char *fp);

void warn_get_vardata (struct warning_data *data);
void warn_set_vardata (const struct warning_data *data);

#define warning(_t,_f,_m)                                   \
    do{                                                     \
        if (warn_check (_t))                                \
          {                                                 \
            char *_a = xstrdup (_m);                        \
            if (warn_error (_t))                            \
              fatal (_f, strlen (_a), "%s", _a);            \
            error (_f, strlen (_a), _("warning: %s"), _a);  \
            free (_a);                                      \
          }                                                 \
    }while(0)
