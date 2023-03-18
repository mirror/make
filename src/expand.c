/* Variable expansion functions for GNU Make.
Copyright (C) 1988-2023 Free Software Foundation, Inc.
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

#include <assert.h>

#include "commands.h"
#include "debug.h"
#include "filedef.h"
#include "job.h"
#include "variable.h"
#include "rule.h"
#include "warning.h"

/* Initially, any errors reported when expanding strings will be reported
   against the file where the error appears.  */
const floc **expanding_var = &reading_file;

/* The next two describe the variable output buffer.
   This buffer is used to hold the variable-expansion of a line of the
   makefile.  It is made bigger with realloc whenever it is too small.
   variable_buffer_length is the size currently allocated.
   variable_buffer is the address of the buffer.

   For efficiency, it's guaranteed that the buffer will always have
   VARIABLE_BUFFER_ZONE extra bytes allocated.  This allows you to add a few
   extra chars without having to call a function.  Note you should never use
   these bytes unless you're _sure_ you have room (you know when the buffer
   length was last checked.  */

#define VARIABLE_BUFFER_ZONE    5

static size_t variable_buffer_length;
char *variable_buffer;

/* Append LENGTH chars of STRING at PTR which must point into variable_buffer.
   The buffer will always be kept nul-terminated.
   The updated pointer into the buffer is returned as the value.  Thus, the
   value returned by each call to variable_buffer_output should be the first
   argument to the following call.  */

char *
variable_buffer_output (char *ptr, const char *string, size_t length)
{
  size_t newlen = length + (ptr - variable_buffer);

  assert (ptr >= variable_buffer);
  assert (ptr < variable_buffer + variable_buffer_length);

  if (newlen + VARIABLE_BUFFER_ZONE + 1 > variable_buffer_length)
    {
      size_t offset = ptr - variable_buffer;
      variable_buffer_length = (newlen + 100 > 2 * variable_buffer_length
                                ? newlen + 100
                                : 2 * variable_buffer_length);
      variable_buffer = xrealloc (variable_buffer, variable_buffer_length + 1);
      ptr = variable_buffer + offset;
    }

  ptr = mempcpy (ptr, string, length);
  *ptr = '\0';
  return ptr;
}

/* Return a pointer to the beginning of the variable buffer.
   This is called from main() and it should never be null afterward.  */

char *
initialize_variable_output ()
{
  /* If we don't have a variable output buffer yet, get one.  */

  if (!variable_buffer)
    {
      variable_buffer_length = 200;
      variable_buffer = xmalloc (variable_buffer_length);
    }

  variable_buffer[0] = '\0';

  return variable_buffer;
}

/* Install a new variable_buffer context, returning the current one for
   safe-keeping.  */

void
install_variable_buffer (char **bufp, size_t *lenp)
{
  *bufp = variable_buffer;
  *lenp = variable_buffer_length;

  variable_buffer = NULL;
  initialize_variable_output ();
}

/* Free the current variable_buffer and restore a previously-saved one.
 */

void
restore_variable_buffer (char *buf, size_t len)
{
  free (variable_buffer);

  variable_buffer = buf;
  variable_buffer_length = len;
}

/* Restore a previously-saved variable_buffer context, and return the
   current one.
 */

char *
swap_variable_buffer (char *buf, size_t len)
{
  char *p = variable_buffer;

  variable_buffer = buf;
  variable_buffer_length = len;

  return p;
}


/* Recursively expand V.  The returned string is malloc'd.  */

static char *allocated_variable_append (const struct variable *v);

char *
recursively_expand_for_file (struct variable *v, struct file *file)
{
  char *value;
  const floc *this_var;
  const floc **saved_varp;
  struct variable_set_list *savev = 0;
  int set_reading = 0;

  /* If we're expanding to put into the environment of a shell function then
     ignore any recursion issues: for backward-compatibility we will use
     the value of the environment variable we were started with.  */
  if (v->expanding && env_recursion)
    {
      size_t nl = strlen (v->name);
      char **ep;
      DB (DB_VERBOSE,
          (_("%s:%lu: not recursively expanding %s to export to shell function\n"),
           v->fileinfo.filenm, v->fileinfo.lineno, v->name));

      /* We could create a hash for the original environment for speed, but a
         reasonably written makefile shouldn't hit this situation...  */
      for (ep = environ; *ep != 0; ++ep)
        if ((*ep)[nl] == '=' && strncmp (*ep, v->name, nl) == 0)
          return xstrdup ((*ep) + nl + 1);

      /* If there's nothing in the parent environment, use the empty string.
         This isn't quite correct since the variable should not exist at all,
         but getting that to work would be involved. */
      return xstrdup ("");
    }

  /* Don't install a new location if this location is empty.
     This can happen for command-line variables, builtin variables, etc.  */
  saved_varp = expanding_var;
  if (v->fileinfo.filenm)
    {
      this_var = &v->fileinfo;
      expanding_var = &this_var;
    }

  /* If we have no other file-reading context, use the variable's context. */
  if (!reading_file)
    {
      set_reading = 1;
      reading_file = &v->fileinfo;
    }

  if (v->expanding)
    {
      if (!v->exp_count)
        /* Expanding V causes infinite recursion.  Lose.  */
        OS (fatal, *expanding_var,
            _("Recursive variable '%s' references itself (eventually)"),
            v->name);
      --v->exp_count;
    }

  if (file)
    install_file_context (file, &savev, NULL);

  v->expanding = 1;
  if (v->append)
    value = allocated_variable_append (v);
  else
    value = allocated_expand_string (v->value);
  v->expanding = 0;

  if (set_reading)
    reading_file = 0;

  if (file)
    restore_file_context (savev, NULL);

  expanding_var = saved_varp;

  return value;
}

/* Expand a simple reference to variable NAME, which is LENGTH chars long.
   The result is written to PTR which must point into the variable_buffer.
   Returns a pointer to the new end of the variable_buffer.  */

char *
expand_variable_output (char *ptr, const char *name, size_t length)
{
  struct variable *v;
  unsigned int recursive;
  char *value;

  v = lookup_variable (name, length);

  if (!v)
    warn_undefined (name, length);

  /* If there's no variable by that name or it has no value, stop now.  */
  if (!v || (v->value[0] == '\0' && !v->append))
    return ptr;

  /* Remember this since expansion could change it.  */
  recursive = v->recursive;

  value = recursive ? recursively_expand (v) : v->value;

  ptr = variable_buffer_output (ptr, value, strlen (value));

  if (recursive)
    free (value);

  return ptr;
}

/* Expand a simple reference to variable NAME, which is LENGTH chars long.
   The result is written to BUF which must point into the variable_buffer.
   If BUF is NULL, start at the beginning of the current variable_buffer.
   Returns BUF, or the beginning of the buffer if BUF is NULL.  */

char *
expand_variable_buf (char *buf, const char *name, size_t length)
{
  if (!buf)
    buf = initialize_variable_output ();

  expand_variable_output (buf, name, length);

  return buf;
}

/* Expand a simple reference to variable NAME, which is LENGTH chars long.
   Returns an allocated buffer containing the value.  */

char *
allocated_expand_variable (const char *name, size_t length)
{
  char *obuf;
  size_t olen;

  install_variable_buffer (&obuf, &olen);

  expand_variable_output (variable_buffer, name, length);

  return swap_variable_buffer (obuf, olen);
}

/* Expand a simple reference to variable NAME, which is LENGTH chars long.
   Error messages refer to the file and line where FILE's commands were found.
   Expansion uses FILE's variable set list.
   Returns an allocated buffer containing the value.  */

char *
allocated_expand_variable_for_file (const char *name, size_t length, struct file *file)
{
  char *result;
  struct variable_set_list *savev;
  const floc *savef;

  if (!file)
    return allocated_expand_variable (name, length);

  install_file_context (file, &savev, &savef);

  result = allocated_expand_variable (name, length);

  restore_file_context (savev, savef);

  return result;
}

/* Scan STRING for variable references and expansion-function calls.  Only
   LENGTH bytes of STRING are actually scanned.
   If LENGTH is SIZE_MAX, scan until a null byte is found.

   Write the results to BUF, which must point into variable_buffer.  If
   BUF is NULL, start at the beginning of the current variable_buffer.

   Return a pointer to BUF, or to the beginning of the new buffer if BUF is
   NULL.
 */
char *
expand_string_buf (char *buf, const char *string, size_t length)
{
  struct variable *v;
  const char *p, *p1;
  char *save;
  char *o;
  size_t line_offset;

  if (!buf)
    buf = initialize_variable_output ();
  o = buf;
  line_offset = buf - variable_buffer;

  if (length == 0)
    return variable_buffer;

  /* We need a copy of STRING: due to eval, it's possible that it will get
     freed as we process it (it might be the value of a variable that's reset
     for example).  Also having a nil-terminated string is handy.  */
  save = length == SIZE_MAX ? xstrdup (string) : xstrndup (string, length);
  p = save;

  while (1)
    {
      /* Copy all following uninteresting chars all at once to the
         variable output buffer, and skip them.  Uninteresting chars end
         at the next $ or the end of the input.  */

      p1 = strchr (p, '$');

      o = variable_buffer_output (o, p, p1 != 0 ? (size_t) (p1 - p) : strlen (p) + 1);

      if (p1 == 0)
        break;
      p = p1 + 1;

      /* Dispatch on the char that follows the $.  */

      switch (*p)
        {
        case '$':
        case '\0':
          /* $$ or $ at the end of the string means output one $ to the
             variable output buffer.  */
          o = variable_buffer_output (o, p1, 1);
          break;

        case '(':
        case '{':
          /* $(...) or ${...} is the general case of substitution.  */
          {
            char openparen = *p;
            char closeparen = (openparen == '(') ? ')' : '}';
            const char *begp;
            const char *beg = p + 1;
            char *op;
            char *abeg = NULL;
            const char *end, *colon;

            op = o;
            begp = p;
            if (handle_function (&op, &begp))
              {
                o = op;
                p = begp;
                break;
              }

            /* Is there a variable reference inside the parens or braces?
               If so, expand it before expanding the entire reference.  */

            end = strchr (beg, closeparen);
            if (end == 0)
              /* Unterminated variable reference.  */
              O (fatal, *expanding_var, _("unterminated variable reference"));
            p1 = lindex (beg, end, '$');
            if (p1 != 0)
              {
                /* BEG now points past the opening paren or brace.
                   Count parens or braces until it is matched.  */
                int count = 0;
                for (p = beg; *p != '\0'; ++p)
                  {
                    if (*p == openparen)
                      ++count;
                    else if (*p == closeparen && --count < 0)
                      break;
                  }
                /* If COUNT is >= 0, there were unmatched opening parens
                   or braces, so we go to the simple case of a variable name
                   such as '$($(a)'.  */
                if (count < 0)
                  {
                    abeg = expand_argument (beg, p); /* Expand the name.  */
                    beg = abeg;
                    end = strchr (beg, '\0');
                  }
              }
            else
              /* Advance P to the end of this reference.  After we are
                 finished expanding this one, P will be incremented to
                 continue the scan.  */
              p = end;

            /* This is not a reference to a built-in function and
               any variable references inside are now expanded.
               Is the resultant text a substitution reference?  */

            colon = lindex (beg, end, ':');
            if (colon)
              {
                /* This looks like a substitution reference: $(FOO:A=B).  */
                const char *subst_beg = colon + 1;
                const char *subst_end = lindex (subst_beg, end, '=');
                if (subst_end == 0)
                  /* There is no = in sight.  Punt on the substitution
                     reference and treat this as a variable name containing
                     a colon, in the code below.  */
                  colon = 0;
                else
                  {
                    const char *replace_beg = subst_end + 1;
                    const char *replace_end = end;

                    /* Extract the variable name before the colon
                       and look up that variable.  */
                    v = lookup_variable (beg, colon - beg);
                    if (v == 0)
                      warn_undefined (beg, colon - beg);

                    /* If the variable is not empty, perform the
                       substitution.  */
                    if (v != 0 && *v->value != '\0')
                      {
                        char *pattern, *replace, *ppercent, *rpercent;
                        char *value = (v->recursive
                                       ? recursively_expand (v)
                                       : v->value);

                        /* Copy the pattern and the replacement.  Add in an
                           extra % at the beginning to use in case there
                           isn't one in the pattern.  */
                        pattern = alloca (subst_end - subst_beg + 2);
                        *(pattern++) = '%';
                        memcpy (pattern, subst_beg, subst_end - subst_beg);
                        pattern[subst_end - subst_beg] = '\0';

                        replace = alloca (replace_end - replace_beg + 2);
                        *(replace++) = '%';
                        memcpy (replace, replace_beg,
                               replace_end - replace_beg);
                        replace[replace_end - replace_beg] = '\0';

                        /* Look for %.  Set the percent pointers properly
                           based on whether we find one or not.  */
                        ppercent = find_percent (pattern);
                        if (ppercent)
                          {
                            ++ppercent;
                            rpercent = find_percent (replace);
                            if (rpercent)
                              ++rpercent;
                          }
                        else
                          {
                            ppercent = pattern;
                            rpercent = replace;
                            --pattern;
                            --replace;
                          }

                        o = patsubst_expand_pat (o, value, pattern, replace,
                                                 ppercent, rpercent);

                        if (v->recursive)
                          free (value);
                      }
                  }
              }

            if (colon == 0)
              /* This is an ordinary variable reference.
                 Look up the value of the variable.  */
                o = expand_variable_output (o, beg, end - beg);

            free (abeg);
          }
          break;

        default:
          if (ISSPACE (p[-1]))
            break;

          /* A $ followed by a random char is a variable reference:
             $a is equivalent to $(a).  */
          o = expand_variable_output (o, p, 1);

          break;
        }

      if (*p == '\0')
        break;

      ++p;
    }

  free (save);

  return (variable_buffer + line_offset);
}


/* Expand an argument for an expansion function.
   The text starting at STR and ending at END is variable-expanded
   into a null-terminated string that is returned as the value.
   This is done without clobbering 'variable_buffer' or the current
   variable-expansion that is in progress.  */

char *
expand_argument (const char *str, const char *end)
{
  char *tmp, *alloc = NULL;
  char *r;

  if (str == end)
    return xstrdup ("");

  if (!end || *end == '\0')
    return allocated_expand_string (str);

  if (end - str + 1 > 1000)
    tmp = alloc = xmalloc (end - str + 1);
  else
    tmp = alloca (end - str + 1);

  memcpy (tmp, str, end - str);
  tmp[end - str] = '\0';

  r = allocated_expand_string (tmp);

  free (alloc);

  return r;
}


/* Expand STRING for FILE, into the current variable_buffer.
   Error messages refer to the file and line where FILE's commands were found.
   Expansion uses FILE's variable set list.  */

char *
expand_string_for_file (const char *string, struct file *file)
{
  char *result;
  struct variable_set_list *savev;
  const floc *savef;

  if (!file)
    return expand_string (string);

  install_file_context (file, &savev, &savef);

  result = expand_string (string);

  restore_file_context (savev, savef);

  return result;
}

/* Like expand_string_for_file, but the returned string is malloc'd.  */

char *
allocated_expand_string_for_file (const char *string, struct file *file)
{
  char *obuf;
  size_t olen;

  install_variable_buffer (&obuf, &olen);

  expand_string_for_file (string, file);

  return swap_variable_buffer (obuf, olen);
}

/* Like allocated_expand_string, but for += target-specific variables.
   First recursively construct the variable value from its appended parts in
   any upper variable sets.  Then expand the resulting value.  */

static char *
variable_append (const char *name, size_t length,
                 const struct variable_set_list *set, int local)
{
  const struct variable *v;
  char *buf = 0;
  int nextlocal;

  /* If there's nothing left to check, return the empty buffer.  */
  if (!set)
    return initialize_variable_output ();

  /* If this set is local and the next is not a parent, then next is local.  */
  nextlocal = local && set->next_is_parent == 0;

  /* Try to find the variable in this variable set.  */
  v = lookup_variable_in_set (name, length, set->set);

  /* If there isn't one, or this one is private, try the set above us.  */
  if (!v || (!local && v->private_var))
    return variable_append (name, length, set->next, nextlocal);

  /* If this variable type is append, first get any upper values.
     If not, initialize the buffer.  */
  if (v->append)
    buf = variable_append (name, length, set->next, nextlocal);
  else
    buf = initialize_variable_output ();

  /* Append this value to the buffer, and return it.
     If we already have a value, first add a space.  */
  if (buf > variable_buffer)
    buf = variable_buffer_output (buf, " ", 1);

  /* Either expand it or copy it, depending.  */
  if (! v->recursive)
    return variable_buffer_output (buf, v->value, strlen (v->value));

  buf = expand_string_buf (buf, v->value, strlen (v->value));
  return (buf + strlen (buf));
}


static char *
allocated_variable_append (const struct variable *v)
{
  /* Construct the appended variable value.  */
  char *obuf;
  size_t olen;

  install_variable_buffer (&obuf, &olen);

  variable_append (v->name, strlen (v->name), current_variable_set_list, 1);

  return swap_variable_buffer (obuf, olen);
}
