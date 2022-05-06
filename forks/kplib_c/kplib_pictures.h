// Copyright (c) 1998-2008 Ken Silverman
// Copyright (c) 2014 plzombie

#ifndef KPLIB_PICTURES_H
#define KPLIB_PICTURES_H

#include <stdint.h>

enum //kpgetdim() return values:
{
	KPLIB_NONE=0,
	KPLIB_PNG, KPLIB_JPG, KPLIB_GIF, KPLIB_CEL,
	KPLIB_BMP, KPLIB_PCX, KPLIB_DDS, KPLIB_TGA,
};

extern int kpgetdim (const char *buf, int leng, int *xsiz, int *ysiz);
extern int kprender (const char *buf, int leng, intptr_t frameptr, int bpl,
					int xdim, int ydim, int xoff, int yoff);

#endif
