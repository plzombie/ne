// Copyright (c) 1998-2008 Ken Silverman
// Copyright (c) 2014 plzombie

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kplib_pictures_png.h"

#include "kplib_swapbits.h"
#include "kplib_minmax.h"
#include "kplib_globalshit.h"
#include "kplib_globalshit_kzfs.h"

#if defined(_MSC_VER) && !defined(__POCC__)
	#define inline __inline
#endif

static int bytesperline, xres, yres, globxoffs, globyoffs;
static intptr_t frameplace;

//Initialized tables (can't be in union)
//jpg:                png:
//   crmul      16384    abstab10    4096
//   cbmul      16384    hxbit        472
//   dct         4608    pow2mask     128*
//   colclip     4096
//   colclipup8  4096
//   colclipup16 4096
//   unzig        256
//   pow2mask     128*
//   dcflagor      64

char *kplib_filterlist = 0;
int kplib_filterlistmal = 0;

//============================ KPNGILIB begins ===============================

//07/31/2000: KPNG.C first ported to C from READPNG.BAS
//10/11/2000: KPNG.C split into 2 files: KPNG.C and PNGINLIB.C
//11/24/2000: Finished adding support for coltypes 4&6
//03/31/2001: Added support for Adam7-type interlaced images
//Currently, there is no support for:
//   * 16-bit color depth
//   * Some useless ancillary chunks, like: gAMA(gamma) & pHYs(aspect ratio)

//	.PNG specific variables:
static int bakr = 0x80, bakg = 0x80, bakb = 0x80; //this used to be public...
static int xm, xmn[4], xr0, xr1, xplc, yplc;
static intptr_t nfplace;
static int /*clen[320], cclen[19], bitpos,*/ filt, xsiz, ysiz;
static int xsizbpl, ixsiz, ixoff, iyoff, ixstp, iystp, intlac, nbpl, trnsrgb;
static unsigned char /*slidebuf[32768],*/ opixbuf0[4], opixbuf1[4];

static int initpass () //Interlaced images have 7 "passes", non-interlaced have 1
{
	int i, j, k;

	do
	{
		i = (intlac<<2);
		ixoff = ((0x04020100>>i)&15);
		iyoff = ((0x00402010>>i)&15);
		if (((ixoff >= xsiz) || (iyoff >= ysiz)) && (intlac >= 2)) { i = -1; intlac--; }
	} while (i < 0);
	j = ((0x33221100>>i)&15); ixstp = (1<<(unsigned int)j);
	k = ((0x33322110>>i)&15); iystp = (1<<(unsigned int)k);

		//xsiz=12      0123456789ab
		//j=3,ixoff=0  0       1       ((12+(1<<3)-1 - 0)>>3) = 2
		//j=3,ixoff=4      2           ((12+(1<<3)-1 - 4)>>3) = 1
		//j=2,ixoff=2    3   4   5     ((12+(1<<2)-1 - 2)>>2) = 3
		//j=1,ixoff=1   6 7 8 9 a b    ((12+(1<<1)-1 - 1)>>1) = 6
	ixsiz = ((xsiz+ixstp-1-ixoff)>>j); //It's confusing! See the above example.
	nbpl = (bytesperline<<k);

		//Initialize this to make filters fast:
	xsizbpl = ((0x04021301>>(kplib_coltype<<2))&15)*ixsiz;
	switch (kplib_bitdepth)
	{
		case 1: xsizbpl = ((xsizbpl+7)>>3); break;
		case 2: xsizbpl = ((xsizbpl+3)>>2); break;
		case 4: xsizbpl = ((xsizbpl+1)>>1); break;
	}

	memset(olinbuf,0,(xsizbpl+1)*sizeof(olinbuf[0]));
	*(int *)&opixbuf0[0] = *(int *)&opixbuf1[0] = 0;
	xplc = xsizbpl; yplc = globyoffs+iyoff; xm = 0; filt = -1;

	i = globxoffs+ixoff; i = (((-(i>=0))|(ixstp-1))&i);
	k = (((-(yplc>=0))|(iystp-1))&yplc);
	nfplace = k*bytesperline + (i<<2) + frameplace;

		//Precalculate x-clipping to screen borders (speeds up putbuf)
		//Equation: (0 <= xr <= ixsiz) && (0 <= xr*ixstp+globxoffs+ixoff <= xres)
	xr0 = max((-globxoffs-ixoff+(1<<j)-1)>>j,0);
	xr1 = min((xres-globxoffs-ixoff+(1<<j)-1)>>j,ixsiz);
	xr0 = ixsiz-xr0;
	xr1 = ixsiz-xr1;

		  if (kplib_coltype == 4) { xr0 = xr0*2;   xr1 = xr1*2;   }
	else if (kplib_coltype == 2) { xr0 = xr0*3-2; xr1 = xr1*3-2; }
	else if (kplib_coltype == 6) { xr0 = xr0*4-2; xr1 = xr1*4-2; }
	else
	{
		switch(kplib_bitdepth)
		{
			case 1: xr0 += ((-ixsiz)&7)+7;
					  xr1 += ((-ixsiz)&7)+7; break;
			case 2: xr0 = ((xr0+((-ixsiz)&3)+3)<<1);
					  xr1 = ((xr1+((-ixsiz)&3)+3)<<1); break;
			case 4: xr0 = ((xr0+((-ixsiz)&1)+1)<<2);
					  xr1 = ((xr1+((-ixsiz)&1)+1)<<2); break;
		}
	}
	ixstp <<= 2;
	return(0);
}

static int Paeth (int a, int b, int c)
{
	int pa, pb, pc;

	pa = b-c; pb = a-c; pc = labs(pa+pb); pa = labs(pa); pb = labs(pb);
	if ((pa <= pb) && (pa <= pc)) return(a);
	if (pb <= pc) return(b); else return(c);
}

static inline int Paeth686 (int a, int b, int c)
{
	return(Paeth(a,b,c));
}

static inline void rgbhlineasm (int x, int xr1, intptr_t p, int ixstp)
{
	int i;
	if (!trnsrgb)
	{
		for(;x>xr1;p+=ixstp,x-=3) *(int *)p = (*(int *)&olinbuf[x])|LSWAPIB(0xff000000);
		return;
	}
	for(;x>xr1;p+=ixstp,x-=3)
	{
		i = (*(int *)&olinbuf[x])|LSWAPIB(0xff000000);
		if (i == trnsrgb) i &= LSWAPIB(0xffffff);
		*(int *)p = i;
	}
}

static inline void pal8hlineasm (int x, int xr1, intptr_t p, int ixstp)
{
	for(;x>xr1;p+=ixstp,x--) *(int *)p = kplib_palcol[olinbuf[x]];
}

	//Autodetect filter
	//    /f0: 0000000...
	//    /f1: 1111111...
	//    /f2: 2222222...
	//    /f3: 1333333...
	//    /f3: 3333333...
	//    /f4: 4444444...
	//    /f5: 0142321...
static void putbuf (const unsigned char *buf, int leng)
{
	int i, x;
	intptr_t p;

	if (filt < 0)
	{
		if (leng <= 0) return;
		filt = buf[0];
		if ((kplib_filterlist) && ((unsigned)yplc < (unsigned)kplib_filterlistmal)) kplib_filterlist[yplc] = filt;
		if (filt == gotcmov) filt = 5;
		i = 1;
	} else i = 0;

	while (i < leng)
	{
		x = i+xplc; if (x > leng) x = leng;
		switch (filt)
		{
			case 0:
				while (i < x) { olinbuf[xplc] = buf[i]; xplc--; i++; }
				break;
			case 1:
				while (i < x)
				{
					olinbuf[xplc] = (unsigned char)(opixbuf1[xm] += buf[i]);
					xm = xmn[xm]; xplc--; i++;
				}
				break;
			case 2:
				while (i < x) { olinbuf[xplc] += (unsigned char)buf[i]; xplc--; i++; }
				break;
			case 3:
				while (i < x)
				{
					opixbuf1[xm] = olinbuf[xplc] = (unsigned char)(((opixbuf1[xm]+olinbuf[xplc])>>1)+buf[i]);
					xm = xmn[xm]; xplc--; i++;
				}
				break;
			case 4:
				while (i < x)
				{
					opixbuf1[xm] = (char)(Paeth(opixbuf1[xm],olinbuf[xplc],opixbuf0[xm])+buf[i]);
					opixbuf0[xm] = olinbuf[xplc];
					olinbuf[xplc] = opixbuf1[xm];
					xm = xmn[xm]; xplc--; i++;
				}
				break;
			case 5: //Special hack for Paeth686 (Doesn't have to be case 5)
				while (i < x)
				{
					opixbuf1[xm] = (char)(Paeth686(opixbuf1[xm],olinbuf[xplc],opixbuf0[xm])+buf[i]);
					opixbuf0[xm] = olinbuf[xplc];
					olinbuf[xplc] = opixbuf1[xm];
					xm = xmn[xm]; xplc--; i++;
				}
				break;
		}

		if (xplc > 0) return;

			//Draw line!
		if ((unsigned int)yplc < (unsigned int)yres)
		{
			x = xr0; p = nfplace;
			switch (kplib_coltype)
			{
				case 2: rgbhlineasm(x,xr1,p,ixstp); break;
				case 4:
					for(;x>xr1;p+=ixstp,x-=2)
						*(int *)p = (kplib_palcol[olinbuf[x]]&LSWAPIB(0xffffff))|LSWAPIL((int)olinbuf[x-1]);
					break;
				case 6:
					for(;x>xr1;p+=ixstp,x-=4)
					{
						*(char *)(p  ) = olinbuf[x  ]; //B
						*(char *)(p+1) = olinbuf[x+1]; //G
						*(char *)(p+2) = olinbuf[x+2]; //R
						*(char *)(p+3) = olinbuf[x-1]; //A
					}
					break;
				default:
					switch(kplib_bitdepth)
					{
						case 1: for(;x>xr1;p+=ixstp,x-- ) *(int *)p = kplib_palcol[olinbuf[x>>3]>>(x&7)]; break;
						case 2: for(;x>xr1;p+=ixstp,x-=2) *(int *)p = kplib_palcol[olinbuf[x>>3]>>(x&6)]; break;
						case 4: for(;x>xr1;p+=ixstp,x-=4) *(int *)p = kplib_palcol[olinbuf[x>>3]>>(x&4)]; break;
						case 8: pal8hlineasm(x,xr1,p,ixstp); break; //for(;x>xr1;p+=ixstp,x-- ) *(int *)p = kplib_palcol[olinbuf[x]]; break;
					}
					break;
			}
			nfplace += nbpl;
		}

		*(int *)&opixbuf0[0] = *(int *)&opixbuf1[0] = 0;
		xplc = xsizbpl; yplc += iystp;
		if ((intlac) && (yplc >= globyoffs+ysiz)) { intlac--; initpass(); }
		if (i < leng)
		{
			filt = buf[i++];
			if ((kplib_filterlist) && ((unsigned)yplc < (unsigned)kplib_filterlistmal)) kplib_filterlist[yplc] = filt;
			if (filt == gotcmov) filt = 5;
		} else filt = -1;
	}
}

int kpngrend (const char *kfilebuf, int kfilength,
	intptr_t daframeplace, int dabytesperline, int daxres, int dayres,
	int daglobxoffs, int daglobyoffs)
{
	int i, j, k, bfinal, btype, hlit, hdist, leng;
	int slidew, slider;
	//int qhuf0v, qhuf1v;

	(void)kfilength; // Unused parameter

	if (!pnginited) { pnginited = 1; initpngtables(); }

	if ((*(int *)&kfilebuf[0] != LSWAPIB(0x474e5089)) || (*(int *)&kfilebuf[4] != LSWAPIB(0x0a1a0a0d)))
		return(-1); //"Invalid PNG file signature"
	filptr = (unsigned char *)&kfilebuf[8];

	trnsrgb = 0;

	while (1)
	{
		leng = LSWAPIL(*(int *)&filptr[0]); i = *(int *)&filptr[4];
		filptr = &filptr[8];

		if (i == LSWAPIB(0x52444849)) //IHDR (must be first)
		{
			xsiz = LSWAPIL(*(int *)&filptr[0]); if (xsiz <= 0) return(-1);
			ysiz = LSWAPIL(*(int *)&filptr[4]); if (ysiz <= 0) return(-1);
			kplib_bitdepth = filptr[8]; if (!((1<<kplib_bitdepth)&0x116)) return(-1); //"Bit depth not supported"
			kplib_coltype = filptr[9]; if (!((1<<kplib_coltype)&0x5d)) return(-1); //"Color type not supported"
			if (filptr[10]) return(-1); //"Only *flate is supported"
			if (filptr[11]) return(-1); //"Filter not supported"
			if (filptr[12] >= 2) return(-1); //"Unsupported interlace type"
			intlac = filptr[12]*7; //0=no interlace/1=Adam7 interlace

				//Save code by making grayscale look like a palette color scheme
			if ((!kplib_coltype) || (kplib_coltype == 4))
			{
				j = 0xff000000; k = (255 / ((1<<kplib_bitdepth)-1))*0x10101;
				kplib_paleng = (1<<kplib_bitdepth);
				for(i=0;i<kplib_paleng;i++,j+=k) kplib_palcol[i] = LSWAPIB(j);
			}
		}
		else if (i == LSWAPIB(0x45544c50)) //PLTE (must be before IDAT)
		{
			kplib_paleng = leng/3;
			for(i=kplib_paleng-1;i>=0;i--) kplib_palcol[i] = LSWAPIB((LSWAPIL(*(int *)&filptr[i*3])>>8)|0xff000000);
		}
		else if (i == LSWAPIB(0x44474b62)) //bKGD (must be after PLTE and before IDAT)
		{
			switch(kplib_coltype)
			{
				case 0: case 4:
					kplib_bakcol = (((int)filptr[0]<<8)+(int)filptr[1])*255/((1<<kplib_bitdepth)-1);
					kplib_bakcol = kplib_bakcol*0x10101+0xff000000; break;
				case 2: case 6:
					if (kplib_bitdepth == 8)
						{ kplib_bakcol = (((int)filptr[1])<<16)+(((int)filptr[3])<<8)+((int)filptr[5])+0xff000000; }
					else
					{
						for(i=0,kplib_bakcol=0xff000000;i<3;i++)
							kplib_bakcol += ((((((int)filptr[i<<1])<<8)+((int)filptr[(i<<1)+1]))/257)<<(16-(i<<3)));
					}
					break;
				case 3:
					kplib_bakcol = kplib_palcol[filptr[0]]; break;
			}
			bakr = ((kplib_bakcol>>16)&255);
			bakg = ((kplib_bakcol>>8)&255);
			bakb = (kplib_bakcol&255);
			kplib_bakcol = LSWAPIB(kplib_bakcol);
		}
		else if (i == LSWAPIB(0x534e5274)) //tRNS (must be after PLTE and before IDAT)
		{
			switch(kplib_coltype)
			{
				case 0:
					if (kplib_bitdepth <= 8)
						kplib_palcol[(int)filptr[1]] &= LSWAPIB(0xffffff);
					//else {} // /c0 /d16 not yet supported
					break;
				case 2:
					if (kplib_bitdepth == 8)
						{ trnsrgb = LSWAPIB((((int)filptr[1])<<16)+(((int)filptr[3])<<8)+((int)filptr[5])+0xff000000); }
					//else {} //WARNING: PNG docs say: MUST compare all 48 bits :(
					break;
				case 3:
					for(i=min(leng,kplib_paleng)-1;i>=0;i--)
						kplib_palcol[i] &= LSWAPIB((((int)filptr[i])<<24)|0xffffff);
					break;
				default:;
			}
		}
		else if (i == LSWAPIB(0x54414449)) { break; }  //IDAT

		filptr = &filptr[leng+4]; //crc = LSWAPIL(*(int *)&filptr[-4]);
	}

		//Initialize this for the getbits() function
	zipfilmode = 0;
	filptr = &filptr[leng-4]; bitpos = -((leng-4)<<3); nfilptr = 0;
	//if (leng < 4) will it crash?

	frameplace = daframeplace;
	bytesperline = dabytesperline;
	xres = daxres;
	yres = dayres;
	globxoffs = daglobxoffs;
	globyoffs = daglobyoffs;

	switch (kplib_coltype)
	{
		case 4: xmn[0] = 1; xmn[1] = 0; break;
		case 2: xmn[0] = 1; xmn[1] = 2; xmn[2] = 0; break;
		case 6: xmn[0] = 1; xmn[1] = 2; xmn[2] = 3; xmn[3] = 0; break;
		default: xmn[0] = 0; break;
	}
	switch (kplib_bitdepth)
	{
		case 1: for(i=2;i<256;i++) kplib_palcol[i] = kplib_palcol[i&1]; break;
		case 2: for(i=4;i<256;i++) kplib_palcol[i] = kplib_palcol[i&3]; break;
		case 4: for(i=16;i<256;i++) kplib_palcol[i] = kplib_palcol[i&15]; break;
	}

		//coltype: bitdepth:  format:
		//  0     1,2,4,8,16  I
		//  2           8,16  RGB
		//  3     1,2,4,8     P
		//  4           8,16  IA
		//  6           8,16  RGBA
	xsizbpl = ((0x04021301>>(kplib_coltype<<2))&15)*xsiz;
	switch (kplib_bitdepth)
	{
		case 1: xsizbpl = ((xsizbpl+7)>>3); break;
		case 2: xsizbpl = ((xsizbpl+3)>>2); break;
		case 4: xsizbpl = ((xsizbpl+1)>>1); break;
	}
		//Tests to see if xsiz > allocated space in olinbuf
		//Note: xsizbpl gets re-written inside initpass()
	if ((xsizbpl+1)*sizeof(olinbuf[0]) > sizeof(olinbuf)) return(-1);

	initpass();

	slidew = 0; slider = 16384;
	kplib_zlibcompflags = getbits(16); //Actually 2 fields: 8:compmethflags, 8:addflagscheck
	do
	{
		bfinal = getbits(1); btype = getbits(2);
		if (btype == 0)
		{
			  //Raw (uncompressed)
			suckbits((-bitpos)&7);  //Synchronize to start of next byte
			i = getbits(16); if ((getbits(16)^i) != 0xffff) return(-1);
			for(;i;i--)
			{
				if (slidew >= slider)
				{
					putbuf(&slidebuf[(slider-16384)&32767],16384); slider += 16384;
					if ((yplc >= yres) && (intlac < 2)) goto kpngrend_goodret;
				}
				slidebuf[(slidew++)&32767] = (char)getbits(8);
			}
			continue;
		}
		if (btype == 3) continue;

		if (btype == 1) //Fixed Huffman
		{
			hlit = 288; hdist = 32; i = 0;
			for(;i<144;i++) clen[i] = 8; //Fixed bit sizes (literals)
			for(;i<256;i++) clen[i] = 9; //Fixed bit sizes (literals)
			for(;i<280;i++) clen[i] = 7; //Fixed bit sizes (EOI,lengths)
			for(;i<288;i++) clen[i] = 8; //Fixed bit sizes (lengths)
			for(;i<320;i++) clen[i] = 5; //Fixed bit sizes (distances)
		}
		else  //Dynamic Huffman
		{
			kplib_numhufblocks++;
			hlit = getbits(5)+257; hdist = getbits(5)+1; j = getbits(4)+4;
			for(i=0;i<j;i++) cclen[ccind[i]] = getbits(3);
			for(;i<19;i++) cclen[ccind[i]] = 0;
			hufgencode(cclen,19,ibuf0,nbuf0);

			j = 0; k = hlit+hdist;
			while (j < k)
			{
				i = hufgetsym(ibuf0,nbuf0);
				if (i < 16) { clen[j++] = i; continue; }
				if (i == 16)
					{ for(i=getbits(2)+3;i;i--) { clen[j] = clen[j-1]; j++; } }
				else
				{
					if (i == 17) i = getbits(3)+3; else i = getbits(7)+11;
					for(;i;i--) clen[j++] = 0;
				}
			}
		}

		hufgencode(clen,hlit,ibuf0,nbuf0);
		//qhuf0v = //hufgetsym_skipb related code
		qhufgencode(ibuf0,nbuf0,qhufval0,qhufbit0,LOGQHUFSIZ0);

		hufgencode(&clen[hlit],hdist,ibuf1,nbuf1);
		//qhuf1v = //hufgetsym_skipb related code
		qhufgencode(ibuf1,nbuf1,qhufval1,qhufbit1,LOGQHUFSIZ1);

		while (1)
		{
			if (slidew >= slider)
			{
				putbuf(&slidebuf[(slider-16384)&32767],16384); slider += 16384;
				if ((yplc >= yres) && (intlac < 2)) goto kpngrend_goodret;
			}

			k = peekbits(LOGQHUFSIZ0);
			if (qhufbit0[k]) { i = qhufval0[k]; suckbits((int)qhufbit0[k]); } else i = hufgetsym(ibuf0,nbuf0);
			//else i = hufgetsym_skipb(ibuf0,nbuf0,LOGQHUFSIZ0,qhuf0v); //hufgetsym_skipb related code

			if (i < 256) { slidebuf[(slidew++)&32767] = (char)i; continue; }
			if (i == 256) break;
			i = getbits(hxbit[i+30-257][0]) + hxbit[i+30-257][1];

			k = peekbits(LOGQHUFSIZ1);
			if (qhufbit1[k]) { j = qhufval1[k]; suckbits((int)qhufbit1[k]); } else j = hufgetsym(ibuf1,nbuf1);
			//else j = hufgetsym_skipb(ibuf1,nbuf1,LOGQHUFSIZ1,qhuf1v); //hufgetsym_skipb related code

			j = getbits(hxbit[j][0]) + hxbit[j][1];
			i += slidew; do { slidebuf[slidew&32767] = slidebuf[(slidew-j)&32767]; slidew++; } while (slidew < i);
		}
	} while (!bfinal);

	slider -= 16384;
	if (!((slider^slidew)&32768))
		putbuf(&slidebuf[slider&32767],slidew-slider);
	else
	{
		putbuf(&slidebuf[slider&32767],(-slider)&32767);
		putbuf(slidebuf,slidew&32767);
	}

kpngrend_goodret:;
	if (kplib_coltype == 4) kplib_paleng = 0; //For /c4, kplib_palcol/kplib_paleng used as LUT for "*0x10101": alpha is invalid!
	return(0);
}

//============================= KPNGILIB ends ================================
