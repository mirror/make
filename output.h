/* Output to stdout / stderr for GNU make
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

struct output
  {
    int out;
    int err;
    unsigned int syncout:1;     /* True if we want to synchronize output.  */
 };

extern struct output *output_context;

#define OUTPUT_SET(_new)    do{ if ((_new)->syncout) output_context = (_new); }while(0)
#define OUTPUT_UNSET()      do{ output_context = NULL; }while(0)

void output_init (struct output *out, unsigned int syncout);
void output_close (struct output *out);

void output_start (void);
void outputs (int is_err, const char *msg);

#ifdef OUTPUT_SYNC
void output_dump (struct output *out);
#endif
