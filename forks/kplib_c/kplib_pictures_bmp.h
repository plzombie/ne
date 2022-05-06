// Copyright (c) 1998-2008 Ken Silverman
// Copyright (c) 2014 plzombie

#ifndef KPLIB_PICTURES_BMP_H
#define KPLIB_PICTURES_BMP_H

#include <stdint.h>

extern int kbmprend (const char *buf, int fleng,
	intptr_t daframeplace, int dabytesperline, int daxres, int dayres,
	int daglobxoffs, int daglobyoffs);

#endif
