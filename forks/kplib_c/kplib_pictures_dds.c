// Copyright (c) 1998-2008 Ken Silverman
// Copyright (c) 2014-2015 plzombie

/*
	Time; Person; Changes
	
	5:18 02.01.2015; plzombie; Add support for X8R8G8B8, A8B8G8R8, X8B8G8R8
*/

#include <string.h>

#include "kplib_pictures_dds.h"

#include "kplib_swapbits.h"
#include "kplib_minmax.h"

//==============================  DDS begins =================================

	// Note:currently supports: DXT1, DXT2, DXT3, DXT4, DXT5, A8R8G8B8, X8R8G8B8, A8B8G8R8, X8B8G8R8
int kddsrend (const char *buf, int leng,
	intptr_t frameptr, int bpl, int xdim, int ydim, int xoff, int yoff)
{
	int x, y, z, xx, yy, xsiz, ysiz, dxt, al[2], ai, j, k, v, c0, c1, stride;
	intptr_t p;
	unsigned int lut[256], r[4], g[4], b[4], a[8], rr, gg, bb;
	unsigned char *uptr, *wptr;

	(void)leng; // Unused parameter

	xsiz = LSWAPIB(*(int *)&buf[16]);
	ysiz = LSWAPIB(*(int *)&buf[12]);
	if ((*(int *)&buf[80])&LSWAPIB(64)) // A8R8G8B8, X8R8G8B8, A8B8G8R8, X8B8G8R8
	{
		int is_x8 = 0, is_bgr;
		
		if ((*(int *)&buf[88]) != LSWAPIB(32)) return(-1);
		if ((*(int *)&buf[96]) != LSWAPIB(0x0000ff00)) return(-1);
		if (((*(int *)&buf[92]) == LSWAPIB(0x00ff0000)) && ((*(int *)&buf[100]) == LSWAPIB(0x000000ff))) // ..R8G8B8
			is_bgr = 0;
		else if (((*(int *)&buf[92]) == LSWAPIB(0x000000ff)) && ((*(int *)&buf[100]) == LSWAPIB(0x00ff0000))) // ..B8G8R8
			is_bgr = 1;
		else
			return(-1);
		if ((*(unsigned int *)&buf[104]) != LSWAPIB(0xff000000)) {
			if((*(int *)&buf[104]) == LSWAPIB(0x00000000)) // X8......
				is_x8 = 1;
			else
				return(-1);
		}
		buf += 128;

		p = yoff*bpl + (xoff<<2) + frameptr; xx = (xsiz<<2);
		if (xoff < 0) { p -= ((intptr_t)xoff<<2); buf -= (xoff<<2); xsiz += xoff; }
		xsiz = (min(xsiz,xdim-xoff)<<2); ysiz = min(ysiz,ydim);
		for(y=0;y<ysiz;y++,p+=bpl,buf+=xx)
		{
			if ((unsigned int)(y+yoff) >= (unsigned int)ydim) continue;
			memcpy((void *)p,(void *)buf,xsiz);
			if(is_x8)
				for(j = 3; j < xsiz; j+=4) {
					((unsigned char *)p)[j] = 255;
				}
			if(is_bgr)
				for(j = 0; j < xsiz; j+=4) {
					unsigned char temp;
					
					temp = ((unsigned char *)p)[j];
					((unsigned char *)p)[j] = ((unsigned char *)p)[j+2];
					((unsigned char *)p)[j+2] = temp;
				}
		}
		return(0);
	}
	if (!((*(int *)&buf[80])&LSWAPIB(4))) return(-1); //FOURCC invalid
	dxt = buf[87]-'0';
	if ((buf[84] != 'D') || (buf[85] != 'X') || (buf[86] != 'T') || (dxt < 1) || (dxt > 5)) return(-1);
	buf += 128;

	if (!(dxt&1))
	{
		for(z=256-1;z>0;z--) lut[z] = (255<<16)/z;
		lut[0] = (1<<16);
	}
	if (dxt == 1) stride = (xsiz<<1); else stride = (xsiz<<2);

	for(y=0;y<ysiz;y+=4,buf+=stride)
		for(x=0;x<xsiz;x+=4)
		{
			if (dxt == 1) uptr = (unsigned char *)(((intptr_t)buf)+((intptr_t)x<<1));
						else uptr = (unsigned char *)(((intptr_t)buf)+((intptr_t)x<<2)+8);
			c0 = SSWAPIB(*(unsigned short *)&uptr[0]);
			r[0] = ((c0>>8)&0xf8); g[0] = ((c0>>3)&0xfc); b[0] = ((c0<<3)&0xfc); a[0] = 255;
			c1 = SSWAPIB(*(unsigned short *)&uptr[2]);
			r[1] = ((c1>>8)&0xf8); g[1] = ((c1>>3)&0xfc); b[1] = ((c1<<3)&0xfc); a[1] = 255;
			if ((c0 > c1) || (dxt != 1))
			{
				r[2] = (((r[0]*2 + r[1] + 1)*(65536/3))>>16);
				g[2] = (((g[0]*2 + g[1] + 1)*(65536/3))>>16);
				b[2] = (((b[0]*2 + b[1] + 1)*(65536/3))>>16); a[2] = 255;
				r[3] = (((r[0] + r[1]*2 + 1)*(65536/3))>>16);
				g[3] = (((g[0] + g[1]*2 + 1)*(65536/3))>>16);
				b[3] = (((b[0] + b[1]*2 + 1)*(65536/3))>>16); a[3] = 255;
			}
			else
			{
				r[2] = (r[0] + r[1])>>1;
				g[2] = (g[0] + g[1])>>1;
				b[2] = (b[0] + b[1])>>1; a[2] = 255;
				r[3] = g[3] = b[3] = a[3] = 0; //Transparent
			}
			v = LSWAPIB(*(int *)&uptr[4]);
			if (dxt >= 4)
			{
				a[0] = uptr[-8]; a[1] = uptr[-7]; k = a[1]-a[0];
				if (k < 0)
				{
					z = a[0]*6 + a[1] + 3;
					for(j=2;j<8;j++) { a[j] = ((z*(65536/7))>>16); z += k; }
				}
				else
				{
					z = a[0]*4 + a[1] + 2;
					for(j=2;j<6;j++) { a[j] = ((z*(65536/5))>>16); z += k; }
					a[6] = 0; a[7] = 255;
				}
				al[0] = LSWAPIB(*(int *)&uptr[-6]);
				al[1] = LSWAPIB(*(int *)&uptr[-3]);
			}
			wptr = (unsigned char *)((y+yoff)*bpl + ((x+xoff)<<2) + frameptr);
			ai = 0;
			for(yy=0;yy<4;yy++,wptr+=bpl)
			{
				if ((unsigned int)(y+yy+yoff) >= (unsigned int)ydim) { ai += 4; continue; }
				for(xx=0;xx<4;xx++,ai++)
				{
					if ((unsigned int)(x+xx+xoff) >= (unsigned int)xdim) continue;

					j = ((v>>(ai<<1))&3);
					switch(dxt)
					{
						case 1: z = a[j]; break;
						case 2: case 3: z = (( uptr[(ai>>1)-8] >> ((xx&1)<<2) )&15)*17; break;
						case 4: case 5: z = a[( al[yy>>1] >> ((ai&7)*3) )&7]; break;
					}
					rr = r[j]; gg = g[j]; bb = b[j];
					if (!(dxt&1))
					{
						bb = min((bb*lut[z])>>16,255);
						gg = min((gg*lut[z])>>16,255);
						rr = min((rr*lut[z])>>16,255);
					}
					wptr[(xx<<2)+0] = (unsigned char)bb;
					wptr[(xx<<2)+1] = (unsigned char)gg;
					wptr[(xx<<2)+2] = (unsigned char)rr;
					wptr[(xx<<2)+3] = (unsigned char)z;
				}
			}
		}
	return(0);
}

//===============================  DDS ends ==================================
