// Copyright (c) 1998-2008 Ken Silverman
// Copyright (c) 2014 plzombie

#include "kplib_pictures_pcx.h"

#include "kplib_swapbits.h"
#include "kplib_minmax.h"
#include "kplib_globalshit.h"

//==============================  PCX begins =================================
	//Note: currently only supports 8 and 24 bit PCX
int kpcxrend (const char *buf, int fleng,
	intptr_t daframeplace, int dabytesperline, int daxres, int dayres,
	int daglobxoffs, int daglobyoffs)
{
	int i, j, x, y, nplanes, x0, x1, y0, y1, bpl, xsiz, ysiz;
	intptr_t p;
	unsigned char c, *cptr;

	if (*(int *)buf != LSWAPIB(0x0801050a)) return(-1);
	xsiz = SSWAPIB(*(short *)&buf[ 8])-SSWAPIB(*(short *)&buf[4])+1; if (xsiz <= 0) return(-1);
	ysiz = SSWAPIB(*(short *)&buf[10])-SSWAPIB(*(short *)&buf[6])+1; if (ysiz <= 0) return(-1);
		//buf[3]: bpp/plane:{1,2,4,8}
	nplanes = buf[65]; //nplanes*bpl bytes per scanline; always be decoding break at the end of scan line
	bpl = SSWAPIB(*(short *)&buf[66]); //#bytes per scanline. Must be EVEN. May have unused data.
	if (nplanes == 1)
	{
		//if (buf[fleng-769] != 12) return(-1); //Some PCX are buggy!
		cptr = (unsigned char *)&buf[fleng-768];
		for(i=0;i<256;i++)
		{
			kplib_palcol[i] = (((int)cptr[0])<<16) +
									(((int)cptr[1])<< 8) +
									(((int)cptr[2])    ) + LSWAPIB(0xff000000);
			cptr += 3;
		}
		kplib_coltype = 3; kplib_bitdepth = 8; kplib_paleng = 256; //For PNGOUT
	}
	else if (nplanes == 3)
	{
		kplib_coltype = 2;

			//Make sure background is opaque (since 24-bit PCX renderer doesn't do it)
		x0 = max(daglobxoffs,0); x1 = min(xsiz+daglobxoffs,daxres);
		y0 = max(daglobyoffs,0); y1 = min(ysiz+daglobyoffs,dayres);
		p = y0*dabytesperline + daframeplace+3;
		for(y=y0;y<y1;y++,p+=dabytesperline)
			for(x=x0;x<x1;x++) *(unsigned char *)(((intptr_t)x<<2)+p) = 255;
	}

	x = x0 = daglobxoffs; x1 = xsiz+daglobxoffs;
	y = y0 = daglobyoffs; y1 = ysiz+daglobyoffs;
	cptr = (unsigned char *)&buf[128];
	p = y*dabytesperline+daframeplace;

	if (bpl > xsiz) { daxres = min(daxres,x1); x1 += bpl-xsiz; }

	j = nplanes-1; daxres <<= 2; x0 <<= 2; x1 <<= 2; x <<= 2; x += j;
	if (nplanes == 1) //8-bit PCX
	{
		do
		{
			c = *cptr++; if (c < 192) i = 1; else { i = (c&63); c = *cptr++; }
			j = kplib_palcol[(int)c];
			for(;i;i--)
			{
				if ((unsigned int)y < (unsigned int)dayres)
					if ((unsigned int)x < (unsigned int)daxres) *(int *)(x+p) = j;
				x += 4; if (x >= x1) { x = x0; y++; p += dabytesperline; }
			}
		} while (y < y1);
	}
	else if (nplanes == 3) //24-bit PCX
	{
		do
		{
			c = *cptr++; if (c < 192) i = 1; else { i = (c&63); c = *cptr++; }
			for(;i;i--)
			{
				if ((unsigned int)y < (unsigned int)dayres)
					if ((unsigned int)x < (unsigned int)daxres) *(char *)(x+p) = c;
				x += 4; if (x >= x1) { j--; if (j < 0) { j = 3-1; y++; p += dabytesperline; } x = x0+j; }
			}
		} while (y < y1);
	}

	return(0);
}

//===============================  PCX ends ==================================
