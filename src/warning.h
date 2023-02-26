/* Control warning output in GNU Make.
Copyright (C) 2023 Free Software Foundation, Inc.
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

/* Types of warnings we can show.  */
enum warning_type
  {
    wt_invalid_var = 0, /* Assign to an invalid variable name.  */
    wt_invalid_ref,     /* Reference an invalid variable name.  */
    wt_undefined_var,   /* Reference an undefined variable name.  */
    wt_max
  };

/* State of a given warning.  */
enum warning_state
  {
    w_unset = 0,
    w_ignore,
    w_warn,
    w_error
  };

/* The default state of warnings.  */
extern enum warning_state default_warnings[wt_max];

/* Current state of warnings.  */
extern enum warning_state warnings[wt_max];

/* Global warning settings.  */
extern enum warning_state warn_global;

/* Get the current state of a given warning.  */
#define warn_get(_w) (warnings[_w] != w_unset ? warnings[_w] \
                      : warn_global != w_unset ? warn_global \
                      : default_warnings[_w])

/* Set the current state of a given warning.  Can't use w_unset here.  */
#define warn_set(_w,_f) do{ warnings[_w] = (_f); } while (0)

/* True if we should check for the warning in the first place.  */
#define warn_check(_w)  (warn_get (_w) > w_ignore)

/* Check if the warning is ignored.  */
#define warn_ignored(_w) (warn_get (_w) == w_ignore)

/* Check if the warning is in "warn" mode.  */
#define warn_warned(_w)  (warn_get (_w) == w_warn)

/* Check if the warning is in "error" mode.  */
#define warn_error(_w)   (warn_get (_w) == w_error)
