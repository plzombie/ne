// Copyright (c) 1998-2008 Ken Silverman
// Copyright (c) 2014 plzombie

#include <string.h>

#include "kplib_pictures_gif.h"

#include "kplib_swapbits.h"
#include "kplib_minmax.h"
#include "kplib_globalshit.h"

//================================ GIF begins ================================

static unsigned char suffix[4100], filbuffer[768], tempstack[4096];
static int prefix[4100];

int kgifrend (const char *kfilebuf, int kfilelength,
	intptr_t daframeplace, int dabytesperline, int daxres, int dayres,
	int daglobxoffs, int daglobyoffs)
{
	int i, x, y, xsiz, ysiz, yinc, xend, xspan, yspan, currstr, numbitgoal;
	int lzcols, dat, blocklen, bitcnt, xoff, yoff, transcol, backcol, *lptr;
	intptr_t p;
	char numbits, startnumbits, chunkind, ilacefirst;
	const unsigned char *ptr, *cptr;

	(void)kfilelength; // Unused parameter

	kplib_coltype = 3; kplib_bitdepth = 8; //For PNGOUT

	if ((kfilebuf[0] != 'G') || (kfilebuf[1] != 'I') || (kfilebuf[2] != 'F')) return(-1);
	kplib_paleng = (1<<((kfilebuf[10]&7)+1));
	ptr = (unsigned char *)&kfilebuf[13];
	if (kfilebuf[10]&128) { cptr = ptr; ptr += kplib_paleng*3; }
	transcol = -1;
	while ((chunkind = *ptr++) == '!')
	{      //! 0xf9 leng flags ?? ?? transcol
		if (ptr[0] == 0xf9) { if (ptr[2]&1) transcol = (int)(((unsigned char)ptr[5])); }
		ptr++;
		do { i = *ptr++; ptr += i; } while (i);
	}
	if (chunkind != ',') return(-1);

	xoff = SSWAPIB(*(unsigned short *)&ptr[0]);
	yoff = SSWAPIB(*(unsigned short *)&ptr[2]);
	xspan = SSWAPIB(*(unsigned short *)&ptr[4]);
	yspan = SSWAPIB(*(unsigned short *)&ptr[6]); ptr += 9;
	if (ptr[-1]&64) { yinc = 8; ilacefirst = 1; }
				  else { yinc = 1; ilacefirst = 0; }
	if (ptr[-1]&128)
	{
		kplib_paleng = (1<<((ptr[-1]&7)+1));
		cptr = ptr; ptr += kplib_paleng*3;
	}

	for(i=0;i<kplib_paleng;i++)
		kplib_palcol[i] = LSWAPIB((((int)cptr[i*3])<<16) + (((int)cptr[i*3+1])<<8) + ((int)cptr[i*3+2]) + 0xff000000);
	for(;i<256;i++) kplib_palcol[i] = LSWAPIB(0xff000000);
	if (transcol >= 0) kplib_palcol[transcol] &= LSWAPIB(~0xff000000);

		//Handle GIF files with different logical&image sizes or non-0 offsets (added 05/15/2004)
	xsiz = SSWAPIB(*(unsigned short *)&kfilebuf[6]);
	ysiz = SSWAPIB(*(unsigned short *)&kfilebuf[8]);
	if ((xoff != 0) || (yoff != 0) || (xsiz != xspan) || (ysiz != yspan))
	{
		int xx[4], yy[4];
		if (kfilebuf[10]&128) backcol = kplib_palcol[(unsigned char)kfilebuf[11]]; else backcol = 0;

			//Fill border to backcol
		xx[0] = max(daglobxoffs           ,     0); yy[0] = max(daglobyoffs           ,     0);
		xx[1] = min(daglobxoffs+xoff      ,daxres); yy[1] = min(daglobyoffs+yoff      ,dayres);
		xx[2] = max(daglobxoffs+xoff+xspan,     0); yy[2] = min(daglobyoffs+yoff+yspan,dayres);
		xx[3] = min(daglobxoffs+xsiz      ,daxres); yy[3] = min(daglobyoffs+ysiz      ,dayres);

		lptr = (int *)(yy[0]*dabytesperline+daframeplace);
		for(y=yy[0];y<yy[1];y++,lptr=(int *)(((intptr_t)lptr)+dabytesperline))
			for(x=xx[0];x<xx[3];x++) lptr[x] = backcol;
		for(;y<yy[2];y++,lptr=(int *)(((intptr_t)lptr)+dabytesperline))
		{  for(x=xx[0];x<xx[1];x++) lptr[x] = backcol;
			for(x=xx[2];x<xx[3];x++) lptr[x] = backcol;
		}
		for(;y<yy[3];y++,lptr=(int *)(((intptr_t)lptr)+dabytesperline))
			for(x=xx[0];x<xx[3];x++) lptr[x] = backcol;

		daglobxoffs += xoff; //Offset bitmap image by extra amount
		daglobyoffs += yoff;
	}

	xspan += daglobxoffs;
	yspan += daglobyoffs;  //UGLY HACK
	y = daglobyoffs;
	if ((unsigned int)y < (unsigned int)dayres)
		{ p = y*dabytesperline+daframeplace; x = daglobxoffs; xend = xspan; }
	else
		{ x = daglobxoffs+0x80000000; xend = xspan+0x80000000; }

	lzcols = (1<<(*ptr)); startnumbits = (char)((*ptr)+1); ptr++;
	for(i=lzcols-1;i>=0;i--) { suffix[i] = (char)(prefix[i] = i); }
	currstr = lzcols+2; numbits = startnumbits; numbitgoal = (lzcols<<1);
	blocklen = *ptr++;
	memcpy(filbuffer,ptr,blocklen); ptr += blocklen;
	bitcnt = 0;
	while (1)
	{
		dat = (LSWAPIB(*(int *)&filbuffer[bitcnt>>3])>>(bitcnt&7)) & (numbitgoal-1);
		bitcnt += numbits;
		if ((bitcnt>>3) > blocklen-3)
		{
			*(short *)filbuffer = *(short *)&filbuffer[bitcnt>>3];
			i = blocklen-(bitcnt>>3);
			blocklen = (int)*ptr++;
			memcpy(&filbuffer[i],ptr,blocklen); ptr += blocklen;
			bitcnt &= 7; blocklen += i;
		}
		if (dat == lzcols)
		{
			currstr = lzcols+2; numbits = startnumbits; numbitgoal = (lzcols<<1);
			continue;
		}
		if ((currstr == numbitgoal) && (numbits < 12))
			{ numbits++; numbitgoal <<= 1; }

		prefix[currstr] = dat;
		for(i=0;dat>=lzcols;dat=prefix[dat]) tempstack[i++] = suffix[dat];
		tempstack[i] = (char)prefix[dat];
		suffix[currstr-1] = suffix[currstr] = (char)dat;

		for(;i>=0;i--)
		{
			if ((unsigned int)x < (unsigned int)daxres)
				*(int *)(p+((intptr_t)x<<2)) = kplib_palcol[(int)tempstack[i]];
			x++;
			if (x == xend)
			{
				y += yinc;
				if (y >= yspan)
					switch(yinc)
					{
						case 8: if (!ilacefirst) { y = daglobyoffs+2; yinc = 4; break; }
								  ilacefirst = 0; y = daglobyoffs+4; yinc = 8; break;
						case 4: y = daglobyoffs+1; yinc = 2; break;
						case 2: case 1: return(0);
					}
				if ((unsigned int)y < (unsigned int)dayres)
					{ p = y*dabytesperline+daframeplace; x = daglobxoffs; xend = xspan; }
				else
					{ x = daglobxoffs+0x80000000; xend = xspan+0x80000000; }
			}
		}
		currstr++;
	}
}

//===============================  GIF ends ==================================
