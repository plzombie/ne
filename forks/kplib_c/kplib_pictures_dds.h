// Copyright (c) 1998-2008 Ken Silverman
// Copyright (c) 2014 plzombie

#ifndef KPLIB_PICTURES_DDS_H
#define KPLIB_PICTURES_DDS_H

#include <stdint.h>

extern int kddsrend (const char *buf, int leng,
	intptr_t frameptr, int bpl, int xdim, int ydim, int xoff, int yoff);

#endif
