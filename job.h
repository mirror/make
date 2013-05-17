/* Definitions for managing subprocesses in GNU Make.
Copyright (C) 1992-2013 Free Software Foundation, Inc.
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

#ifndef SEEN_JOB_H
#define SEEN_JOB_H

#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#else
# include <sys/file.h>
#endif

/* How to set close-on-exec for a file descriptor.  */

#if !defined F_SETFD
# ifdef WINDOWS32
#  define CLOSE_ON_EXEC(_d)  process_noinherit(_d)
# else
#  define CLOSE_ON_EXEC(_d)
# endif
#else
# ifndef FD_CLOEXEC
#  define FD_CLOEXEC 1
# endif
# define CLOSE_ON_EXEC(_d) (void) fcntl ((_d), F_SETFD, FD_CLOEXEC)
#endif

#ifdef OUTPUT_SYNC
# ifdef WINDOWS32
/* For emulations in w32/compat/posixfcn.c.  */
#  define F_GETFD 1
#  define F_SETLKW 2
/* Implementation note: None of the values of l_type below can be zero
   -- they are compared with a static instance of the struct, so zero
   means unknown/invalid, see w32/compat/posixfcn.c. */
#  define F_WRLCK 1
#  define F_UNLCK 2

struct flock
  {
    short l_type;
    short l_whence;
    off_t l_start;
    off_t l_len;
    pid_t l_pid;
  };

/* This type is actually a HANDLE, but we want to avoid including
   windows.h as much as possible.  */
typedef intptr_t sync_handle_t;

/* Public functions emulated/provided in posixfcn.c.  */
int fcntl (intptr_t fd, int cmd, ...);
intptr_t create_mutex (void);
int same_stream (FILE *f1, FILE *f2);

#  define RECORD_SYNC_MUTEX(m) record_sync_mutex(m)
void record_sync_mutex (const char *str);
void prepare_mutex_handle_string (intptr_t hdl);

# else  /* !WINDOWS32 */

typedef int sync_handle_t;      /* file descriptor */

#  define RECORD_SYNC_MUTEX(m) (void)(m)

# endif
#endif  /* OUTPUT_SYNC */

/* Structure describing a running or dead child process.  */

struct child
  {
    struct child *next;         /* Link in the chain.  */

    struct file *file;          /* File being remade.  */

    char **environment;         /* Environment for commands.  */
    char *sh_batch_file;        /* Script file for shell commands */
    char **command_lines;       /* Array of variable-expanded cmd lines.  */
    char *command_ptr;          /* Ptr into command_lines[command_line].  */

#ifdef VMS
    char *comname;              /* Temporary command file name */
    int efn;                    /* Completion event flag number */
    int cstatus;                /* Completion status */
#endif

    unsigned int command_line;  /* Index into command_lines.  */
    int          outfd;         /* File descriptor for saving stdout */
    int          errfd;         /* File descriptor for saving stderr */
    pid_t        pid;           /* Child process's ID number.  */
    unsigned int remote:1;      /* Nonzero if executing remotely.  */
    unsigned int noerror:1;     /* Nonzero if commands contained a '-'.  */
    unsigned int good_stdin:1;  /* Nonzero if this child has a good stdin.  */
    unsigned int deleted:1;     /* Nonzero if targets have been deleted.  */
    unsigned int dontcare:1;    /* Saved dontcare flag.  */
  };

extern struct child *children;

int is_bourne_compatible_shell(const char *path);
void new_job (struct file *file);
void reap_children (int block, int err);
void start_waiting_jobs (void);

char **construct_command_argv (char *line, char **restp, struct file *file,
                               int cmd_flags, char** batch_file);
#ifdef VMS
int child_execute_job (char *argv, struct child *child);
#elif defined(__EMX__)
int child_execute_job (int stdin_fd, int stdout_fd, char **argv, char **envp);
#else
void child_execute_job (int stdin_fd, int stdout_fd, char **argv, char **envp);
#endif
#ifdef _AMIGA
void exec_command (char **argv);
#elif defined(__EMX__)
int exec_command (char **argv, char **envp);
#else
void exec_command (char **argv, char **envp);
#endif

extern unsigned int job_slots_used;

void block_sigs (void);
#ifdef POSIX
void unblock_sigs (void);
#else
#ifdef  HAVE_SIGSETMASK
extern int fatal_signal_mask;
#define unblock_sigs()  sigsetmask (0)
#else
#define unblock_sigs()
#endif
#endif

extern unsigned int jobserver_tokens;

#endif /* SEEN_JOB_H */
