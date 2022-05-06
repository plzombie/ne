// Copyright (c) 1998-2008 Ken Silverman
// Copyright (c) 2014 plzombie

#include <string.h>
#include <stdlib.h>

#include "kplib_pictures_jpg.h"

#include "kplib_swapbits.h"
#include "kplib_minmax.h"
#include "kplib_globalshit.h"

#if defined(_MSC_VER) && !defined(__POCC__)
#define inline __inline
#endif

#if defined(BIGENDIAN)
static inline unsigned int bswap (unsigned int a)
{
	return(((a&0xff0000)>>8) + ((a&0xff00)<<8) + (a<<24) + (a>>24));
}
#endif

static int bytesperline, xres, yres, globxoffs, globyoffs;
static intptr_t frameplace;

static const int pow2mask[32] =
{
	0x00000000,0x00000001,0x00000003,0x00000007,
	0x0000000f,0x0000001f,0x0000003f,0x0000007f,
	0x000000ff,0x000001ff,0x000003ff,0x000007ff,
	0x00000fff,0x00001fff,0x00003fff,0x00007fff,
	0x0000ffff,0x0001ffff,0x0003ffff,0x0007ffff,
	0x000fffff,0x001fffff,0x003fffff,0x007fffff,
	0x00ffffff,0x01ffffff,0x03ffffff,0x07ffffff,
	0x0fffffff,0x1fffffff,0x3fffffff,0x7fffffff,
};
static const int pow2long[32] =
{
	0x00000001,0x00000002,0x00000004,0x00000008,
	0x00000010,0x00000020,0x00000040,0x00000080,
	0x00000100,0x00000200,0x00000400,0x00000800,
	0x00001000,0x00002000,0x00004000,0x00008000,
	0x00010000,0x00020000,0x00040000,0x00080000,
	0x00100000,0x00200000,0x00400000,0x00800000,
	0x01000000,0x02000000,0x04000000,0x08000000,
	0x10000000,0x20000000,0x40000000,0x80000000,
};

//============================ KPEGILIB begins ===============================

	//11/01/2000: This code was originally from KPEG.C
	//   All non 32-bit color drawing was removed
	//   "Motion" JPG code was removed
	//   A lot of parameters were added to kpeg() for library usage
static int kpeginited = 0;
static int clipxdim, clipydim;

static int hufmaxatbit[8][20], hufvalatbit[8][20], hufcnt[8];
static unsigned char hufnumatbit[8][20], huftable[8][256];
static int hufquickval[8][1024], hufquickbits[8][1024], hufquickcnt[8];
static int quantab[4][64], dct[12][64], lastdc[4], unzig[64], zigit[64]; //dct:10=MAX (says spec);+2 for hacks
static unsigned char gnumcomponents, dcflagor[64];
static int gcompid[4], gcomphsamp[4], gcompvsamp[4], gcompquantab[4], gcomphsampshift[4], gcompvsampshift[4];
static int lnumcomponents, lcompid[4], lcompdc[4], lcompac[4], lcomphsamp[4], lcompvsamp[4], lcompquantab[4];
static int lcomphvsamp0, lcomphsampshift0, lcompvsampshift0;
static int colclip[1024], colclipup8[1024], colclipup16[1024];

static inline int mulshr24 (int a, int b)
{
	return((int)((((int64_t)a)*((int64_t)b))>>24));
}

static inline int mulshr32 (int a, int b)
{
	return((int)((((int64_t)a)*((int64_t)b))>>32));
}

static int cosqr16[8] =    //cosqr16[i] = ((cos(PI*i/16)*sqrt(2))<<24);
  {23726566,23270667,21920489,19727919,16777216,13181774,9079764,4628823};
static int crmul[4096], cbmul[4096];

static void initkpeg ()
{
	int i, x, y;

	x = 0;  //Back & forth diagonal pattern (aligning bytes for best compression)
	for(i=0;i<16;i+=2)
	{
		for(y=8-1;y>=0;y--)
			if ((unsigned)(i-y) < (unsigned)8) unzig[x++] = (y<<3)+i-y;
		for(y=0;y<8;y++)
			if ((unsigned)(i+1-y) < (unsigned)8) unzig[x++] = (y<<3)+i+1-y;
	}
	for(i=64-1;i>=0;i--) zigit[unzig[i]] = i;
	for(i=64-1;i>=0;i--) dcflagor[i] = (unsigned char)(1<<(unzig[i]>>3));

	for(i=0;i<128;i++) colclip[i] = i+128;
	for(i=128;i<512;i++) colclip[i] = 255;
	for(i=512;i<896;i++) colclip[i] = 0;
	for(i=896;i<1024;i++) colclip[i] = i-896;
	for(i=0;i<1024;i++)
	{
		colclipup8[i] = (colclip[i]<<8);
		colclipup16[i] = (colclip[i]<<16)+0xff000000; //Hack: set alphas to 255
	}
#if defined(BIGENDIAN)
	for(i=0;i<1024;i++)
	{
		colclip[i] = bswap(colclip[i]);
		colclipup8[i] = bswap(colclipup8[i]);
		colclipup16[i] = bswap(colclipup16[i]);
	}
#endif

	for(i=0;i<2048;i++)
	{
		crmul[(i<<1)+0] = (i-1024)*1470104; //1.402*1048576
		crmul[(i<<1)+1] = (i-1024)*-748830; //-0.71414*1048576
		cbmul[(i<<1)+0] = (i-1024)*-360857; //-0.34414*1048576
		cbmul[(i<<1)+1] = (i-1024)*1858077; //1.772*1048576
	}

	memset((void *)&dct[10][0],0,64*2*sizeof(dct[0][0]));
}

static void huffgetval (int index, int curbits, int num, int *daval, int *dabits)
{
	int b, v, pow2, *hmax;

	hmax = &hufmaxatbit[index][0];
	pow2 = pow2long[curbits-1];
	if (num&pow2) v = 1; else v = 0;
	for(b=1;b<=16;b++)
	{
		if (v < hmax[b])
		{
			*dabits = b;
			*daval = huftable[index][hufvalatbit[index][b]+v];
			return;
		}
		pow2 >>= 1; v <<= 1;
		if (num&pow2) v++;
	}
	*dabits = 16; *daval = 0;
}

static void invdct8x8 (int *dc, unsigned char dcflag)
{
	#define SQRT2 23726566   //(sqrt(2))<<24
	#define C182 31000253    //(cos(PI/8)*2)<<24
	#define C18S22 43840978  //(cos(PI/8)*sqrt(2)*2)<<24
	#define C38S22 18159528  //(cos(PI*3/8)*sqrt(2)*2)<<24
	int *edc, t0, t1, t2, t3, t4, t5, t6, t7;

	edc = dc+64;
	do
	{
		if (dcflag&1) //pow2char[z])
		{
			t3 = dc[2] + dc[6];
			t2 = (mulshr32(dc[2]-dc[6],SQRT2<<6)<<2) - t3;
			t4 = dc[0] + dc[4]; t5 = dc[0] - dc[4];
			t0 = t4+t3; t3 = t4-t3; t1 = t5+t2; t2 = t5-t2;
			t4 = (mulshr32(dc[5]-dc[3]+dc[1]-dc[7],C182<<6)<<2);
			t7 = dc[1] + dc[7] + dc[5] + dc[3];
			t6 = (mulshr32(dc[3]-dc[5],C18S22<<5)<<3) + t4 - t7;
			t5 = (mulshr32(dc[1]+dc[7]-dc[5]-dc[3],SQRT2<<6)<<2) - t6;
			t4 = (mulshr32(dc[1]-dc[7],C38S22<<6)<<2) - t4 + t5;
			dc[0] = t0+t7; dc[7] = t0-t7; dc[1] = t1+t6; dc[6] = t1-t6;
			dc[2] = t2+t5; dc[5] = t2-t5; dc[4] = t3+t4; dc[3] = t3-t4;
		}
		dc += 8; dcflag >>= 1;
	} while (dc < edc);
	dc -= 32; edc -= 24;
	do
	{
		t3 = dc[2*8-32] + dc[6*8-32];
		t2 = (mulshr32(dc[2*8-32]-dc[6*8-32],SQRT2<<6)<<2) - t3;
		t4 = dc[0*8-32] + dc[4*8-32]; t5 = dc[0*8-32] - dc[4*8-32];
		t0 = t4+t3; t3 = t4-t3; t1 = t5+t2; t2 = t5-t2;
		t4 = (mulshr32(dc[5*8-32]-dc[3*8-32]+dc[1*8-32]-dc[7*8-32],C182<<6)<<2);
		t7 = dc[1*8-32] + dc[7*8-32] + dc[5*8-32] + dc[3*8-32];
		t6 = (mulshr32(dc[3*8-32]-dc[5*8-32],C18S22<<5)<<3) + t4 - t7;
		t5 = (mulshr32(dc[1*8-32]+dc[7*8-32]-dc[5*8-32]-dc[3*8-32],SQRT2<<6)<<2) - t6;
		t4 = (mulshr32(dc[1*8-32]-dc[7*8-32],C38S22<<6)<<2) - t4 + t5;
		dc[0*8-32] = t0+t7; dc[7*8-32] = t0-t7; dc[1*8-32] = t1+t6; dc[6*8-32] = t1-t6;
		dc[2*8-32] = t2+t5; dc[5*8-32] = t2-t5; dc[4*8-32] = t3+t4; dc[3*8-32] = t3-t4;
		dc++;
	} while (dc < edc);
}

static void yrbrend (int x, int y, int *ldct)
{
	int i, j, ox, oy, xx, yy, xxx, yyy, xxxend, yyyend, yv, cr, cb, *odc, *dc, *dc2;
	intptr_t p, pp;

	odc = ldct; dc2 = &ldct[10<<6];
	for(yy=0;yy<(lcompvsamp[0]<<3);yy+=8)
	{
		oy = y+yy+globyoffs; if ((unsigned)oy >= (unsigned)clipydim) { odc += (lcomphsamp[0]<<6); continue; }
		pp = oy*bytesperline + ((x+globxoffs)<<2) + frameplace;
		for(xx=0;xx<(lcomphsamp[0]<<3);xx+=8,odc+=64)
		{
			ox = x+xx+globxoffs; if ((unsigned)ox >= (unsigned)clipxdim) continue;
			p = pp+((intptr_t)xx<<2);
			dc = odc;
			if (lnumcomponents > 1) dc2 = &ldct[(lcomphvsamp0<<6)+((yy>>lcompvsampshift0)<<3)+(xx>>lcomphsampshift0)];
			xxxend = min(clipxdim-ox,8);
			yyyend = min(clipydim-oy,8);
			if ((lcomphsamp[0] == 1) && (xxxend == 8))
			{
				for(yyy=0;yyy<yyyend;yyy++)
				{
					for(xxx=0;xxx<8;xxx++)
					{
						yv = dc[xxx];
						cr = (dc2[xxx+64]>>(20-1))&~1;
						cb = (dc2[xxx   ]>>(20-1))&~1;
						((int *)p)[xxx] = colclipup16[(unsigned)(yv+crmul[cr+2048]               )>>22]+
												  colclipup8[(unsigned)(yv+crmul[cr+2049]+cbmul[cb+2048])>>22]+
													  colclip[(unsigned)(yv+cbmul[cb+2049]               )>>22];
					}
					p += bytesperline;
					dc += 8;
					if (!((yyy+1)&(lcompvsamp[0]-1))) dc2 += 8;
				}
			}
			else if ((lcomphsamp[0] == 2) && (xxxend == 8))
			{
				for(yyy=0;yyy<yyyend;yyy++)
				{
					for(xxx=0;xxx<8;xxx+=2)
					{
						yv = dc[xxx];
						cr = (dc2[(xxx>>1)+64]>>(20-1))&~1;
						cb = (dc2[(xxx>>1)   ]>>(20-1))&~1;
						i = crmul[cr+2049]+cbmul[cb+2048];
						cr = crmul[cr+2048];
						cb = cbmul[cb+2049];
						((int *)p)[xxx] = colclipup16[(unsigned)(yv+cr)>>22]+
												  colclipup8[(unsigned)(yv+ i)>>22]+
													  colclip[(unsigned)(yv+cb)>>22];
						yv = dc[xxx+1];
						((int *)p)[xxx+1] = colclipup16[(unsigned)(yv+cr)>>22]+
													 colclipup8[(unsigned)(yv+ i)>>22]+
														 colclip[(unsigned)(yv+cb)>>22];
					}
					p += bytesperline;
					dc += 8;
					if (!((yyy+1)&(lcompvsamp[0]-1))) dc2 += 8;
				}
			}
			else
			{
				for(yyy=0;yyy<yyyend;yyy++)
				{
					i = 0; j = 1;
					for(xxx=0;xxx<xxxend;xxx++)
					{
						yv = dc[xxx];
						j--;
						if (!j)
						{
							j = lcomphsamp[0];
							cr = (dc2[i+64]>>(20-1))&~1;
							cb = (dc2[i   ]>>(20-1))&~1;
							i++;
						}
						((int *)p)[xxx] = colclipup16[(unsigned)(yv+crmul[cr+2048]               )>>22]+
												  colclipup8[(unsigned)(yv+crmul[cr+2049]+cbmul[cb+2048])>>22]+
													  colclip[(unsigned)(yv+cbmul[cb+2049]               )>>22];
					}
					p += bytesperline;
					dc += 8;
					if (!((yyy+1)&(lcompvsamp[0]-1))) dc2 += 8;
				}
			}
		}
	}
}
void (*kplib_yrbrend_func)(int,int,int *) = yrbrend;

int kpegrend (const char *kfilebuf, int kfilength,
	intptr_t daframeplace, int dabytesperline, int daxres, int dayres,
	int daglobxoffs, int daglobyoffs)
{
	int i, j, v, leng, xdim, ydim, index, prec, restartcnt, restartinterval;
	int x, y, z, xx, yy, zz, *dc, num, curbits, c, daval, dabits, *hqval, *hqbits, hqcnt, *quanptr;
	int passcnt = 0, ghsampmax, gvsampmax, glhsampmax, glvsampmax, glhstep, glvstep;
	int eobrun, Ss, Se, Ah, Al, Alut[2], dctx[12], dcty[12], ldctx[12], ldcty[12], lshx[4], lshy[4];
	short *dctbuf = 0, *dctptr[12], *ldctptr[12], *dcs;
	unsigned char ch, marker, dcflag;
	const unsigned char *kfileptr;

	if (!kpeginited) { kpeginited = 1; initkpeg(); }

	kfileptr = (unsigned char *)kfilebuf;

	if (*(unsigned short *)kfileptr == SSWAPIB(0xd8ff)) kfileptr += 2;
	else return(-1); //"%s is not a JPEG file\n",filename

	restartinterval = 0;
	for(i=0;i<4;i++) lastdc[i] = 0;
	for(i=0;i<8;i++) hufcnt[i] = 0;

	kplib_coltype = 0; kplib_bitdepth = 8; //For PNGOUT
	do
	{
		ch = *kfileptr++; if (ch != 255) continue;
		do { marker = *kfileptr++; } while (marker == 255);
		if (marker != 0xd9) //Don't read past end of buffer
		{
			leng = ((int)kfileptr[0]<<8)+(int)kfileptr[1]-2;
			kfileptr += 2;
		}
		//printf("fileoffs=%08x, marker=%02x,leng=%d",((int)kfileptr)-((int)kfilebuf)-2,marker,leng);
		switch(marker)
		{
			case 0xc0: case 0xc1: case 0xc2:
					//processit!
				kfileptr++; //numbits = *kfileptr++;

				ydim = SSWAPIL(*(unsigned short *)&kfileptr[0]);
				xdim = SSWAPIL(*(unsigned short *)&kfileptr[2]);
				//printf("%s: %ld / %ld = %ld\n",filename,xdim*ydim*3,kfilength,(xdim*ydim*3)/kfilength);

				frameplace = daframeplace;
				bytesperline = dabytesperline;
				xres = daxres;
				yres = dayres;
				globxoffs = daglobxoffs;
				globyoffs = daglobyoffs;

				gnumcomponents = kfileptr[4];
				kfileptr += 5;
				ghsampmax = gvsampmax = glhsampmax = glvsampmax = 0;
				for(z=0;z<gnumcomponents;z++)
				{
					gcompid[z] = kfileptr[0];
					gcomphsamp[z] = (kfileptr[1]>>4);
					gcompvsamp[z] = (kfileptr[1]&15);
					gcompquantab[z] = kfileptr[2];
					for(i=0;i<8;i++) if (gcomphsamp[z] == pow2long[i]) { gcomphsampshift[z] = i; break; }
					for(i=0;i<8;i++) if (gcompvsamp[z] == pow2long[i]) { gcompvsampshift[z] = i; break; }
					if (gcomphsamp[z] > ghsampmax) { ghsampmax = gcomphsamp[z]; glhsampmax = gcomphsampshift[z]; }
					if (gcompvsamp[z] > gvsampmax) { gvsampmax = gcompvsamp[z]; glvsampmax = gcompvsampshift[z]; }
					kfileptr += 3;
				}

				break;
			case 0xc4:  //Huffman table
				do
				{
					ch = *kfileptr++; leng--;
					if (ch >= 16) { index = ch-12; }
								else { index = ch; }
					memcpy((void *)&hufnumatbit[index][1],(void *)kfileptr,16); kfileptr += 16;
					leng -= 16;

					v = 0; hufcnt[index] = 0;
					hufquickcnt[index] = 0;
					for(i=1;i<=16;i++)
					{
						hufmaxatbit[index][i] = v+hufnumatbit[index][i];
						hufvalatbit[index][i] = hufcnt[index]-v;
						memcpy((void *)&huftable[index][hufcnt[index]],(void *)kfileptr,(int)hufnumatbit[index][i]);
						if (i <= 10)
							for(c=0;c<hufnumatbit[index][i];c++)
								for(j=(1<<(10-i));j>0;j--)
								{
									hufquickval[index][hufquickcnt[index]] = huftable[index][hufcnt[index]+c];
									hufquickbits[index][hufquickcnt[index]] = i;
									hufquickcnt[index]++;
								}
						kfileptr += hufnumatbit[index][i];
						leng -= hufnumatbit[index][i];
						hufcnt[index] += hufnumatbit[index][i];
						v = ((v+hufnumatbit[index][i])<<1);
					}

				} while (leng > 0);
				break;
			case 0xdb:
				do
				{
					ch = *kfileptr++; leng--;
					index = (ch&15);
					prec = (ch>>4);
					for(z=0;z<64;z++)
					{
						v = (int)(*kfileptr++);
						if (prec) v = (v<<8)+((int)(*kfileptr++));
						v <<= 19;
						if (unzig[z]&7 ) v = mulshr24(v,cosqr16[unzig[z]&7 ]);
						if (unzig[z]>>3) v = mulshr24(v,cosqr16[unzig[z]>>3]);
						quantab[index][unzig[z]] = v;
					}
					leng -= 64;
					if (prec) leng -= 64;
				} while (leng > 0);
				break;
			case 0xdd:
				restartinterval = SSWAPIL(*(unsigned short *)&kfileptr[0]);
				kfileptr += leng;
				break;
			case 0xda:
				if ((xdim <= 0) || (ydim <= 0)) { if (dctbuf) free(dctbuf); return(-1); }

				lnumcomponents = (int)(*kfileptr++); if (!lnumcomponents) { if (dctbuf) free(dctbuf); return(-1); }
				if (lnumcomponents > 1) kplib_coltype = 2;
				for(z=0;z<lnumcomponents;z++)
				{
					lcompid[z] = kfileptr[0];
					lcompdc[z] = (kfileptr[1]>>4);
					lcompac[z] = (kfileptr[1]&15);
					kfileptr += 2;
				}

				Ss = kfileptr[0];
				Se = kfileptr[1];
				Ah = (kfileptr[2]>>4);
				Al = (kfileptr[2]&15);
				kfileptr += 3;
				//printf("passcnt=%d, Ss=%d, Se=%d, Ah=%d, Al=%d\n",passcnt,Ss,Se,Ah,Al);

				if ((!passcnt) && ((Ss) || (Se != 63) || (Ah) || (Al)))
				{
					for(z=zz=0;z<gnumcomponents;z++)
					{
						dctx[z] = ((xdim+(ghsampmax<<3)-1)>>(glhsampmax+3)) << gcomphsampshift[z];
						dcty[z] = ((ydim+(gvsampmax<<3)-1)>>(glvsampmax+3)) << gcompvsampshift[z];
						zz += dctx[z]*dcty[z];
					}
					z = zz*64*sizeof(short);
					dctbuf = (short *)malloc(z); if (!dctbuf) return(-1);
					memset(dctbuf,0,z);
					for(z=zz=0;z<gnumcomponents;z++) { dctptr[z] = &dctbuf[zz*64]; zz += dctx[z]*dcty[z]; }
				}

				glhstep = glvstep = 0x7fffffff;
				for(z=0;z<lnumcomponents;z++)
					for(zz=0;zz<gnumcomponents;zz++)
						if (lcompid[z] == gcompid[zz])
						{
							ldctptr[z] = dctptr[zz];
							ldctx[z] = dctx[zz];
							ldcty[z] = dcty[zz];
							lcomphsamp[z] = gcomphsamp[zz];
							lcompvsamp[z] = gcompvsamp[zz];
							lcompquantab[z] = gcompquantab[zz];
							if (!z)
							{
								lcomphsampshift0 = gcomphsampshift[zz];
								lcompvsampshift0 = gcompvsampshift[zz];
							}
							lshx[z] = glhsampmax-gcomphsampshift[zz]+3;
							lshy[z] = glvsampmax-gcompvsampshift[zz]+3;
							if (gcomphsampshift[zz] < glhstep) glhstep = gcomphsampshift[zz];
							if (gcompvsampshift[zz] < glvstep) glvstep = gcompvsampshift[zz];
						}
				glhstep = (ghsampmax>>glhstep); lcomphsamp[0] = min(lcomphsamp[0],glhstep); glhstep <<= 3;
				glvstep = (gvsampmax>>glvstep); lcompvsamp[0] = min(lcompvsamp[0],glvstep); glvstep <<= 3;
				lcomphvsamp0 = lcomphsamp[0]*lcompvsamp[0];

				clipxdim = min(xdim+globxoffs,xres);
				clipydim = min(ydim+globyoffs,yres);

				if ((max(globxoffs,0) >= xres) || (min(globxoffs+xdim,xres) <= 0) ||
					 (max(globyoffs,0) >= yres) || (min(globyoffs+ydim,yres) <= 0))
					{ if (dctbuf) free(dctbuf); return(0); }

				Alut[0] = (1<<Al); Alut[1] = -Alut[0];

				restartcnt = restartinterval; eobrun = 0; marker = 0xd0;
				num = 0; curbits = 0;
				for(y=0;y<ydim;y+=glvstep)
					for(x=0;x<xdim;x+=glhstep)
					{
						if (kfileptr-4-(unsigned char *)kfilebuf >= kfilength) goto kpegrend_break2; //rest of file is missing!

						if (!dctbuf) dc = dct[0];
						for(c=0;c<lnumcomponents;c++)
						{
							hqval = &hufquickval[lcompac[c]+4][0];
							hqbits = &hufquickbits[lcompac[c]+4][0];
							hqcnt = hufquickcnt[lcompac[c]+4];
							if (!dctbuf) quanptr = &quantab[lcompquantab[c]][0];
							for(yy=0;yy<(lcompvsamp[c]<<3);yy+=8)
								for(xx=0;xx<(lcomphsamp[c]<<3);xx+=8)
								{  //NOTE: Might help to split this code into firstime vs. refinement (!Ah vs. Ah!=0)

									if (dctbuf) dcs = &ldctptr[c][(((y+yy)>>lshy[c])*ldctx[c] + ((x+xx)>>lshx[c]))<<6];

										//Get DC
									if (!Ss)
									{
										while (curbits < 16) //Getbits
										{
											ch = *kfileptr++; if (ch == 255) kfileptr++;
											num = (num<<8)+((int)ch); curbits += 8;
										}

										if (!Ah)
										{
											i = ((num>>(curbits-10))&1023);
											if (i < hufquickcnt[lcompdc[c]])
												  { daval = hufquickval[lcompdc[c]][i]; curbits -= hufquickbits[lcompdc[c]][i]; }
											else { huffgetval(lcompdc[c],curbits,num,&daval,&dabits); curbits -= dabits; }

											if (daval)
											{
												while (curbits < daval) //Getbits
												{
													ch = *kfileptr++; if (ch == 255) kfileptr++;
													num = (num<<8)+((int)ch); curbits += 8;
												}

												curbits -= daval; v = ((unsigned)num >> curbits) & pow2mask[daval];
												if (v <= pow2mask[daval-1]) v -= pow2mask[daval];
												lastdc[c] += v;
											}
											if (!dctbuf) dc[0] = lastdc[c]; else dcs[0] = (short)(lastdc[c]<<Al);
										}
										else if (num&(pow2long[--curbits])) dcs[0] |= ((short)Alut[0]);
									}

										//Get AC
									if (!dctbuf) memset((void *)&dc[1],0,63*4);
									z = max(Ss,1); dcflag = 1;
									if (eobrun <= 0)
									{
										for(;z<=Se;z++)
										{
											while (curbits < 16) //Getbits
											{
												ch = *kfileptr++; if (ch == 255) kfileptr++;
												num = (num<<8)+((int)ch); curbits += 8;
											}
											i = ((num>>(curbits-10))&1023);
											if (i < hqcnt)
												  { daval = hqval[i]; curbits -= hqbits[i]; }
											else { huffgetval(lcompac[c]+4,curbits,num,&daval,&dabits); curbits -= dabits; }

											zz = (daval>>4); daval &= 15;
											if (daval)
											{
												if (Ah)
												{
													if (curbits < 8) //Getbits
													{
														ch = *kfileptr++; if (ch == 255) kfileptr++;
														num = (num<<8)+((int)ch); curbits += 8;
													}
													if (num&(pow2long[--curbits])) daval = Alut[0]; else daval = Alut[1];
												}
											}
											else if (zz < 15)
											{
												eobrun = pow2long[zz];
												if (zz)
												{
													while (curbits < zz) //Getbits
													{
														ch = *kfileptr++; if (ch == 255) kfileptr++;
														num = (num<<8)+((int)ch); curbits += 8;
													}
													curbits -= zz; eobrun += ((unsigned)num >> curbits) & pow2mask[zz];
												}
												if (!Ah) eobrun--;
												break;
											}
											if (Ah)
											{
												do
												{
													if (dcs[z])
													{
														if (curbits < 8) //Getbits
														{
															ch = *kfileptr++; if (ch == 255) kfileptr++;
															num = (num<<8)+((int)ch); curbits += 8;
														}
														if (num&(pow2long[--curbits])) dcs[z] += (short)Alut[dcs[z] < 0];
												  } else if (--zz < 0) break;
												  z++;
												} while (z <= Se);
												if (daval) dcs[z] = (short)daval;
											}
											else
											{
												z += zz; if (z > Se) break;

												while (curbits < daval) //Getbits
												{
													ch = *kfileptr++; if (ch == 255) kfileptr++;
													num = (num<<8)+((int)ch); curbits += 8;
												}
												curbits -= daval; v = ((unsigned)num >> curbits) & pow2mask[daval];
												if (v <= pow2mask[daval-1]) v -= pow2mask[daval];
												dcflag |= dcflagor[z];
												if (!dctbuf) dc[unzig[z]] = v; else dcs[z] = (short)(v<<Al);
											}
										}
									} else if (!Ah) eobrun--;
									if ((Ah) && (eobrun > 0))
									{
										eobrun--;
										for(;z<=Se;z++)
										{
											if (!dcs[z]) continue;
											if (curbits < 8) //Getbits
											{
												ch = *kfileptr++; if (ch == 255) kfileptr++;
												num = (num<<8)+((int)ch); curbits += 8;
											}
											if (num&(pow2long[--curbits])) dcs[z] += ((short)Alut[dcs[z] < 0]);
										}
									}

									if (!dctbuf)
									{
										for(z=64-1;z>=0;z--) dc[z] *= quanptr[z];
										invdct8x8(dc,dcflag); dc += 64;
									}
								}
							}

						if (!dctbuf) kplib_yrbrend_func(x,y,&dct[0][0]);

						restartcnt--;
						if (!restartcnt)
						{
							kfileptr += 1-(curbits>>3); curbits = 0;
							if ((kfileptr[-2] != 255) || (kfileptr[-1] != marker)) kfileptr--;
							marker++; if (marker >= 0xd8) marker = 0xd0;
							restartcnt = restartinterval;
							for(i=0;i<4;i++) lastdc[i] = 0;
							eobrun = 0;
						}
					}
kpegrend_break2:;
				if (!dctbuf) return(0);
				passcnt++; kfileptr -= ((curbits>>3)+1); break;
			case 0xd9: break;
			default: kfileptr += leng; break;
		}
	} while (kfileptr-(unsigned char *)kfilebuf < kfilength);

	if (!dctbuf) return(0);

	lnumcomponents = gnumcomponents;
	for(i=0;i<gnumcomponents;i++)
	{
		lcomphsamp[i] = gcomphsamp[i]; gcomphsamp[i] <<= 3;
		lcompvsamp[i] = gcompvsamp[i]; gcompvsamp[i] <<= 3;
		lshx[i] = glhsampmax-gcomphsampshift[i]+3;
		lshy[i] = glvsampmax-gcompvsampshift[i]+3;
	}
	lcomphsampshift0 = gcomphsampshift[0];
	lcompvsampshift0 = gcompvsampshift[0];
	lcomphvsamp0 = (lcomphsamp[0]<<lcompvsampshift0);
	for(y=0;y<ydim;y+=gcompvsamp[0])
		for(x=0;x<xdim;x+=gcomphsamp[0])
		{
			dc = dct[0];
			for(c=0;c<gnumcomponents;c++)
				for(yy=0;yy<gcompvsamp[c];yy+=8)
					for(xx=0;xx<gcomphsamp[c];xx+=8,dc+=64)
					{
						dcs = &dctptr[c][(((y+yy)>>lshy[c])*dctx[c] + ((x+xx)>>lshx[c]))<<6];
						quanptr = &quantab[gcompquantab[c]][0];
						for(z=0;z<64;z++) dc[z] = ((int)dcs[zigit[z]])*quanptr[z];
						invdct8x8(dc,0xff);
					}
			kplib_yrbrend_func(x,y,&dct[0][0]);
		}

	free(dctbuf); return(0);
}

//==============================  KPEGILIB ends ==============================
