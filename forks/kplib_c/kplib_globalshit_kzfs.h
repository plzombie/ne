// Copyright (c) 1998-2008 Ken Silverman
// Copyright (c) 2014 plzombie

#ifndef KPLIB_GLOBALSHIT_KZFS_H
#define KPLIB_GLOBALSHIT_KZFS_H

#include <stdio.h>

extern int zipfilmode;

typedef struct
{
	FILE *fil;   //0:no file open, !=0:open file (either stand-alone or zip)
	int comptyp; //0:raw data (can be ZIP or stand-alone), 8:PKZIP LZ77 *flate
	int seek0;   //0:stand-alone file, !=0: start of zip compressed stream data
	int compleng;//Global variable for compression FIFO
	int comptell;//Global variable for compression FIFO
	int leng;    //Uncompressed file size (bytes)
	int pos;     //Current uncompressed relative file position (0<=pos<=leng)
	int endpos;  //Temp global variable for kzread
	int jmpplc;  //Store place where decompression paused
	int i;       //For stand-alone/ZIP comptyp#0, this is like "uncomptell"
					  //For ZIP comptyp#8&btype==0 "<64K store", this saves i state
	int bfinal;  //LZ77 decompression state (for later calls)
} kzfilestate;

extern kzfilestate kzfs;

extern int hxbit[59][2], ibuf0[288], nbuf0[32], ibuf1[32], nbuf1[32];
extern unsigned char pnginited, olinbuf[65536];
extern int gotcmov, abstab10[1024];
extern const unsigned char *filptr;
extern int clen[320], cclen[19], bitpos;
extern int ccind[19];
extern unsigned char slidebuf[32768];

extern unsigned char *nfilptr;
extern void suckbitsnextblock ();
extern int peekbits (int n);
extern void suckbits (int n);
extern int getbits (int n);

//	Variables to speed up dynamic Huffman decoding:
#define LOGQHUFSIZ0 9
#define LOGQHUFSIZ1 6
extern int qhufval0[1<<LOGQHUFSIZ0], qhufval1[1<<LOGQHUFSIZ1];
extern unsigned char qhufbit0[1<<LOGQHUFSIZ0], qhufbit1[1<<LOGQHUFSIZ1];

extern int hufgetsym (int *hitab, int *hbmax);
extern void qhufgencode (int *hitab, int *hbmax, int *qhval, unsigned char *qhbit, int numbits);
extern void hufgencode (int *inbuf, int inum, int *hitab, int *hbmax);

extern void initpngtables();

#endif
