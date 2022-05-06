// Copyright (c) 1998-2008 Ken Silverman
// Copyright (c) 2014 plzombie

#if !defined(_WIN32) && !defined(__DOS__)
	#define _rotl(value, shift) ( ((shift)==0||(shift)==32)?((unsigned int)(value)):((((unsigned int)(value)) << (shift)) | (((unsigned int)(value)) >> (32-(shift)))) )
	#ifndef BIGENDIAN
		#define _rotr(value, shift) ( ((shift)==0||(shift)==32)?((unsigned int)(value)):((((unsigned int)(value)) >> (shift)) | (((unsigned int)(value)) << (32-(shift)))) )
	#endif
#else
	#include <stdlib.h>
#endif

#include "kplib_pictures_bmp.h"

#include "kplib_swapbits.h"
#include "kplib_globalshit.h"

#ifdef BIGENDIAN
#define ROTR_LE(x,y) _rotl((x),(y))
#else
#define ROTR_LE(x,y) _rotr((x),(y))
#endif

//==============================  BMP begins =================================
//TODO: handle BI_RLE8 and BI_RLE4 (compression types 1&2 respectively)
//                      +===============+
//                      |  0(2): "BM"   |
//+=====================+ 10(4): rastoff+==================+
//|headsiz=12 (OS/2 1.x)| 14(4): headsiz| All new formats: |
//+=====================================+==============================================+
//| 18(2): xsiz                         | 18(4): xsiz                                  |
//| 20(2): ysiz                         | 22(4): ysiz                                  |
//| 22(2): planes (always 1)            | 26(2): planes (always 1)                     |
//| 24(2): cdim (1,4,8,24)              | 28(2): cdim (1,4,8,16,24,32)                 |
//| if (cdim < 16)                      | 30(4): compression (0,1,2,3!?,4)             |
//|    26(rastoff-14-headsiz): pal(bgr) | 34(4): (bitmap data size+3)&3                |
//|                                     | 46(4): N colors (0=2^cdim)                   |
//|                                     | if (cdim < 16)                               |
//|                                     |    14+headsiz(rastoff-14-headsiz): pal(bgr0) |
//+=====================+===============+==============================================+
//                      | rastoff(?): bitmap data |
//                      +=========================+
int kbmprend (const char *buf, int fleng,
	intptr_t daframeplace, int dabytesperline, int daxres, int dayres,
	int daglobxoffs, int daglobyoffs)
{
	int i, j, x, y, x0, x1, y0, y1, rastoff, headsiz, xsiz, ysiz, cdim, comp, cptrinc, *lptr;
	const char *cptr;

	(void)fleng; // Unused parameter

	headsiz = *(int *)&buf[14];
	if (headsiz == LSWAPIB(12)) //OS/2 1.x (old format)
	{
		if (*(short *)(&buf[22]) != SSWAPIB(1)) return(-1);
		xsiz = (int)SSWAPIB(*(unsigned short *)&buf[18]);
		ysiz = (int)SSWAPIB(*(unsigned short *)&buf[20]);
		cdim = (int)SSWAPIB(*(unsigned short *)&buf[24]);
		comp = 0;
	}
	else //All newer formats...
	{
		if (*(short *)(&buf[26]) != SSWAPIB(1)) return(-1);
		xsiz = LSWAPIB(*(int *)&buf[18]);
		ysiz = LSWAPIB(*(int *)&buf[22]);
		cdim = (int)SSWAPIB(*(unsigned short *)&buf[28]);
		comp = LSWAPIB(*(int *)&buf[30]);
	}
	if ((xsiz <= 0) || (!ysiz)) return(-1);
		//cdim must be: (1,4,8,16,24,32)
	if (((unsigned int)(cdim-1) >= (unsigned int)32) || (!((1<<cdim)&0x1010113))) return(-1);
	if ((comp != 0) && (comp != 3)) return(-1);

	rastoff = LSWAPIB(*(int *)&buf[10]);

	if (cdim < 16)
	{
		if (cdim == 2) { kplib_palcol[0] = 0xffffffff; kplib_palcol[1] = LSWAPIB(0xff000000); }
		if (headsiz == LSWAPIB(12)) j = 3; else j = 4;
		for(i=0,cptr=&buf[headsiz+14];cptr<&buf[rastoff];i++,cptr+=j)
			kplib_palcol[i] = ((*(int *)&cptr[0])|LSWAPIB(0xff000000));
		kplib_coltype = 3; kplib_bitdepth = (signed char)cdim; kplib_paleng = i; //For PNGOUT
	}
	else if (!(cdim&15))
	{
		kplib_coltype = 2;
		switch(cdim)
		{
			case 16: kplib_palcol[0] = 10; kplib_palcol[1] = 5; kplib_palcol[2] = 0; kplib_palcol[3] = 5; kplib_palcol[4] = 5; kplib_palcol[5] = 5; break;
			/*32bit bmp's are always abgr (at least, in GIMP)*/// case 32: kplib_palcol[0] = 16; kplib_palcol[1] = 8; kplib_palcol[2] = 0; kplib_palcol[3] = 8; kplib_palcol[4] = 8; kplib_palcol[5] = 8; break;
		}
		if (comp == 3) //BI_BITFIELD (RGB masks)
		{
			for(i=0;i<3;i++)
			{
				j = *(int *)&buf[headsiz+(i<<2)+14];
				for(kplib_palcol[i]=0;kplib_palcol[i]<32;kplib_palcol[i]++)
				{
					if (j&1) break;
					j = (((unsigned int)j)>>1);
				}
				for(kplib_palcol[i+3]=0;kplib_palcol[i+3]<32;kplib_palcol[i+3]++)
				{
					if (!(j&1)) break;
					j = (((unsigned int)j)>>1);
				}
			}
		}
		kplib_palcol[0] = 24-(kplib_palcol[0]+kplib_palcol[3]);
		kplib_palcol[1] = 16-(kplib_palcol[1]+kplib_palcol[4]);
		kplib_palcol[2] =  8-(kplib_palcol[2]+kplib_palcol[5]);
		kplib_palcol[3] = ((-1<<(24-kplib_palcol[3]))&0x00ff0000);
		kplib_palcol[4] = ((-1<<(16-kplib_palcol[4]))&0x0000ff00);
		kplib_palcol[5] = ((-1<<( 8-kplib_palcol[5]))&0x000000ff);
	}

	cptrinc = (((xsiz*cdim+31)>>3)&~3); cptr = &buf[rastoff];
	if (ysiz < 0) { ysiz = -ysiz; } else { cptr = &cptr[(ysiz-1)*cptrinc]; cptrinc = -cptrinc; }

	x0 = daglobxoffs; x1 = xsiz+daglobxoffs;
	y0 = daglobyoffs; y1 = ysiz+daglobyoffs;
	if ((x0 >= daxres) || (x1 <= 0) || (y0 >= dayres) || (y1 <= 0)) return(0);
	if (x0 < 0) x0 = 0;
	if (x1 > daxres) x1 = daxres;
	for(y=y0;y<y1;y++,cptr=&cptr[cptrinc])
	{
		if ((unsigned int)y >= (unsigned int)dayres) continue;
		lptr = (int *)(y*dabytesperline-(daglobyoffs<<2)+daframeplace);
		switch(cdim)
		{
			case  1: for(x=x0;x<x1;x++) lptr[x] = kplib_palcol[(int)((cptr[x>>3]>>((x&7)^7))&1)]; break;
			case  4: for(x=x0;x<x1;x++) lptr[x] = kplib_palcol[(int)((cptr[x>>1]>>(((x&1)^1)<<2))&15)]; break;
			case  8: for(x=x0;x<x1;x++) lptr[x] = kplib_palcol[(int)(cptr[x])]; break;
			case 16: for(x=x0;x<x1;x++)
						{
							i = ((int)(*(short *)&cptr[x<<1]));
							lptr[x] = (_rotl(i,kplib_palcol[0])&kplib_palcol[3]) +
										 (_rotl(i,kplib_palcol[1])&kplib_palcol[4]) +
										 (_rotl(i,kplib_palcol[2])&kplib_palcol[5]) + LSWAPIB(0xff000000);
						} break;
			case 24: for(x=x0;x<x1;x++) lptr[x] = ((*(int *)&cptr[x*3])|LSWAPIB(0xff000000)); break;
			case 32: for(x=x0;x<x1;x++)
						{
							i = (*(int *)&cptr[x<<2]);
							lptr[x] = ROTR_LE(i, 8); // I use rotr, because a will be first byte in little-endian archtectures
							/*Why? Where he read about this? they are always abgr*///lptr[x] = (_rotl(i,kplib_palcol[0])&kplib_palcol[3]) +
										 //(_rotl(i,kplib_palcol[1])&kplib_palcol[4]) +
										 //(_rotl(i,kplib_palcol[2])&kplib_palcol[5]) + LSWAPIB(0xff000000);
						} break;
		}

	}
	return(0);
}
//===============================  BMP ends ==================================
