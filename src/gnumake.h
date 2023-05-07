/* External interfaces usable by dynamic objects loaded into GNU Make.

Copyright (C) 2013-2023 Free Software Foundation, Inc.
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

#ifndef _GNUMAKE_H_
#define _GNUMAKE_H_

#define GMK_ABI_VERSION 1

/* Specify the location of elements read from makefiles.  */
typedef struct
  {
    const char *filenm;
    unsigned long lineno;
  } gmk_floc;

typedef char *(*gmk_func_ptr)(const char *nm, unsigned int argc, char **argv);

/* When an object is loaded by GNU Make, a setup method will be invoked.
   The name of the method is either derived from the filename of the object,
   or specified explicitly in the makefile.  It has the signature:

     int <setup_fn> (unsigned int abi_version, const gmk_floc *flocp);

   The abi_version will be set to GMK_ABI_VERSION.

   When an object is unloaded by GNU Make, an unload method will be invoked.
   The name of the method is derived from the filename of the object, with
   _gmk_unload appended.  It has the signature:

     void <object>_gmk_unload (void);

  There will only be one unload method invoked regardless of the number of
  setup methods within the object.  */

#ifdef _WIN32
# ifdef GMK_BUILDING_MAKE
#  define GMK_EXPORT  __declspec(dllexport)
# else
#  define GMK_EXPORT  __declspec(dllimport)
# endif
#else
# define GMK_EXPORT
#endif

/* Free memory returned by the gmk_expand() and gmk_free() functions.  */
GMK_EXPORT void gmk_free (char *str);

/* Allocate memory in GNU Make's context.  */
GMK_EXPORT char *gmk_alloc (unsigned int len);

/* Run $(eval ...) on the provided string BUFFER.  */
GMK_EXPORT void gmk_eval (const char *buffer, const gmk_floc *floc);

/* Run GNU Make expansion on the provided string STR.
   Returns an allocated buffer that the caller must free with gmk_free().  */
GMK_EXPORT char *gmk_expand (const char *str);

/* Register a new GNU Make function NAME (maximum of 255 chars long).
   When the function is expanded in the makefile, FUNC will be invoked with
   the appropriate arguments.

   The return value of FUNC must be either NULL, in which case it expands to
   the empty string, or a pointer to the result of the expansion in a string
   created by gmk_alloc().  GNU Make will free the memory when it's done.

   MIN_ARGS is the minimum number of arguments the function requires.
   MAX_ARGS is the maximum number of arguments (or 0 if there's no maximum).
   MIN_ARGS and MAX_ARGS may not exceed 255.

   The FLAGS value may be GMK_FUNC_DEFAULT, or one or more of the following
   flags OR'd together:

     GMK_FUNC_NOEXPAND: the arguments to the function will be not be expanded
                        before FUNC is called.
*/
GMK_EXPORT void gmk_add_function (const char *name, gmk_func_ptr func,
                                  unsigned int min_args, unsigned int max_args,
                                  unsigned int flags);

#define GMK_FUNC_DEFAULT    0x00
#define GMK_FUNC_NOEXPAND   0x01

#endif  /* _GNUMAKE_H_ */
