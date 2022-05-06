// Copyright (c) 1998-2008 Ken Silverman
// Copyright (c) 2014 plzombie

#include <stdlib.h>

#include "kplib_fileio.h"
#include "kplib_pictures.h"
#include "kplib_pictures_misc.h"

//===================== HANDY PICTURE function begins ========================

void kpzload (const char *filnam, intptr_t *pic, int *bpl, int *xsiz, int *ysiz)
{
	char *buf;
	int leng;

	(*pic) = 0;
	if (!kzopen(filnam)) return;
	leng = kzfilelength();
	buf = (char *)malloc(leng); if (!buf) { kzclose(); return; }
	kzread(buf,leng);
	kzclose();

	kpgetdim(buf,leng,xsiz,ysiz);
	(*bpl) = ((*xsiz)<<2);
	(*pic) = (intptr_t)malloc((*ysiz)*(*bpl)); if (!(*pic)) { free(buf); return; }
	if (kprender(buf,leng,*pic,*bpl,*xsiz,*ysiz,0,0) < 0) { free(buf); free((void *)*pic); (*pic) = 0; return; }
	free(buf);
}
//====================== HANDY PICTURE function ends =========================
