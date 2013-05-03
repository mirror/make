/* dlfcn.h replacement for MS-Windows build.  */
#ifndef DLFCN_H
#define DLFCN_H

#define RTLD_LAZY   1
#define RTLD_NOW    2
#define RTLD_GLOBAL 4

extern void *dlopen (const char *, int);
extern void *dlsym (void *, const char *);
extern char *dlerror (void);
extern int   dlclose (void *);

#endif	/* DLFCN_H */
