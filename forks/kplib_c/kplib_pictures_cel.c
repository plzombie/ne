// Copyright (c) 1998-2008 Ken Silverman
// Copyright (c) 2014 plzombie

#include "kplib_pictures_cel.h"

#include "kplib_swapbits.h"
#include "kplib_globalshit.h"

//==============================  CEL begins =================================

	//   //old .CEL format:
	//short id = 0x9119, xdim, ydim, xoff, yoff, id = 0x0008;
	//int imagebytes, filler[4];
	//char pal6bit[256][3], image[ydim][xdim];
int kcelrend (const char *buf, int fleng,
	intptr_t daframeplace, int dabytesperline, int daxres, int dayres,
	int daglobxoffs, int daglobyoffs)
{
	int i, x, y, x0, x1, y0, y1, xsiz, ysiz;
	const char *cptr;

	(void)fleng; // Unused parameter

	kplib_coltype = 3; kplib_bitdepth = 8; kplib_paleng = 256; //For PNGOUT

	xsiz = (int)SSWAPIB(*(unsigned short *)&buf[2]); if (xsiz <= 0) return(-1);
	ysiz = (int)SSWAPIB(*(unsigned short *)&buf[4]); if (ysiz <= 0) return(-1);

	cptr = &buf[32];
	for(i=0;i<256;i++)
	{
		kplib_palcol[i] = (((int)cptr[0])<<18) +
								(((int)cptr[1])<<10) +
								(((int)cptr[2])<< 2) + LSWAPIB(0xff000000);
		cptr += 3;
	}

	x0 = daglobxoffs; x1 = xsiz+daglobxoffs;
	y0 = daglobyoffs; y1 = ysiz+daglobyoffs;
	for(y=y0;y<y1;y++)
		for(x=x0;x<x1;x++)
		{
			if (((unsigned int)x < (unsigned int)daxres) && ((unsigned int)y < (unsigned int)dayres))
				*(int *)(y*dabytesperline+x*4+daframeplace) = *(kplib_palcol+cptr[0]); // kplib_palcol[cptr[0]] - я не знаю, могут ли быть элементы cptr отрицательными, а такой вариант генерирует варнинги (что меня бесит); поэтому закомментил и переписал
			cptr++;
		}
	return(0);
}

//===============================  CEL ends ==================================
