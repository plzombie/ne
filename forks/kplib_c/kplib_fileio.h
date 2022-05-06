// Copyright (c) 1998-2008 Ken Silverman
// Copyright (c) 2014 plzombie

#ifndef KPLIB_FILEIO_H
#define KPLIB_FILEIO_H

#include <stdint.h>

extern void kzuninit ();
extern int kzaddstack (const char *filnam);
extern int kzaddstack2 (const char *filnam, int adddirs); // new to kplib.c
extern void kzsetfil (FILE *fil);
extern intptr_t kzopen (const char *filnam);
extern int kzaccess(const char *filnam); // new to kplib.c
extern void kzfindfilestart (const char *st);
extern int kzfindfile (char *filnam);
extern int kzread (void *buffer, int leng);
extern int kzfilelength ();
extern int kzseek (int offset, int whence);
extern int kztell ();
extern int kzgetc ();
extern int kzeof ();
extern void kzclose ();

#endif
