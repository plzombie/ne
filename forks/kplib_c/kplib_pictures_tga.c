// Copyright (c) 1998-2008 Ken Silverman
// Copyright (c) 2014 plzombie

#include "kplib_pictures_tga.h"

#include "kplib_swapbits.h"
#include "kplib_globalshit.h"

//=============================  TARGA begins ================================

int ktgarend (const char *header, int fleng,
	intptr_t daframeplace, int dabytesperline, int daxres, int dayres,
	int daglobxoffs, int daglobyoffs)
{
	int i, x, y, pi, xi, yi, x0, x1, y0, y1, xsiz, ysiz, rlestat, colbyte, pixbyte;
	intptr_t p;
	const unsigned char *fptr, *cptr, *nptr;

		//Ugly and unreliable identification for .TGA!
	if ((fleng < 19) || (header[1]&0xfe)) return(-1);
	if ((header[2] >= 12) || (!((1<<header[2])&0xe0e))) return(-1);
	if ((header[16]&7) || (header[16] == 0) || (header[16] > 32)) return(-1);
	if (header[17]&0xc0) return(-1);

	fptr = (unsigned char *)&header[header[0]+18];
	xsiz = (int)SSWAPIB(*(unsigned short *)&header[12]); if (xsiz <= 0) return(-1);
	ysiz = (int)SSWAPIB(*(unsigned short *)&header[14]); if (ysiz <= 0) return(-1);
	colbyte = ((((int)header[16])+7)>>3);

	if (header[1] == 1)
	{
		pixbyte = ((((int)header[7])+7)>>3);
		cptr = &fptr[-SSWAPIB(*(unsigned short *)&header[3])*pixbyte];
		fptr += SSWAPIB(*(unsigned short *)&header[5])*pixbyte;
	} else pixbyte = colbyte;

	switch(pixbyte) //For PNGOUT
	{
		case 1: kplib_coltype = 0; kplib_bitdepth = 8; kplib_palcol[0] = LSWAPIB(0xff000000);
				  for(i=1;i<256;i++) kplib_palcol[i] = kplib_palcol[i-1]+LSWAPIB(0x10101); break;
		case 2: case 3: kplib_coltype = 2; break;
		case 4: kplib_coltype = 6; break;
	}

	if (!(header[17]&16)) { x0 = 0;      x1 = xsiz; xi = 1; }
						  else { x0 = xsiz-1; x1 = -1;   xi =-1; }
	if (header[17]&32) { y0 = 0;      y1 = ysiz; yi = 1; pi = dabytesperline; }
					  else { y0 = ysiz-1; y1 = -1;   yi =-1; pi =-dabytesperline; }
	x0 += daglobxoffs; y0 += daglobyoffs;
	x1 += daglobxoffs; y1 += daglobyoffs;
	if (header[2] < 8) rlestat = -2; else rlestat = -1;

	p = y0*dabytesperline+daframeplace;
	for(y=y0;y!=y1;y+=yi,p+=pi)
		for(x=x0;x!=x1;x+=xi)
		{
			if (rlestat < 128)
			{
				if ((rlestat&127) == 127) { rlestat = (int)fptr[0]; fptr++; }
				if (header[1] == 1)
				{
					if (colbyte == 1) i = fptr[0];
									 else i = (int)SSWAPIB(*(unsigned short *)&fptr[0]);
					nptr = &cptr[i*pixbyte];
				} else nptr = fptr;

				switch(pixbyte)
				{
					case 1: i = kplib_palcol[(int)nptr[0]]; break;
					case 2: i = (int)SSWAPIB(*(unsigned short *)&nptr[0]);
						i = LSWAPIB(((i&0x7c00)<<9) + ((i&0x03e0)<<6) + ((i&0x001f)<<3) + 0xff000000);
						break;
					case 3: i = (*(int *)&nptr[0]) | LSWAPIB(0xff000000); break;
					case 4: i = (*(int *)&nptr[0]); break;
				}
				fptr += colbyte;
			}
			if (rlestat >= 0) rlestat--;

			if (((unsigned int)x < (unsigned int)daxres) && ((unsigned int)y < (unsigned int)dayres))
				*(int *)(x*4+p) = i;
		}
	return(0);
}

//==============================  TARGA ends =================================
