// Copyright (c) 1998-2008 Ken Silverman
// Copyright (c) 2014 plzombie

#include "kplib_globalshit_kzfs.h"

#include "kplib_swapbits.h"
#include "kplib_minmax.h"

#if defined(_MSC_VER) && !defined(__POCC__)
	#define inline __inline
#endif

#if 0
static inline int testflag (int c) { return(0); }

static inline void cpuid (int a, int *s) {}

	//Bit numbers of return value:
	//0:FPU, 4:RDTSC, 15:CMOV, 22:MMX+, 23:MMX, 25:SSE, 26:SSE2, 30:3DNow!+, 31:3DNow!
static int getcputype ()
{
	int i, cpb[4], cpid[4];
	if (!testflag(0x200000)) return(0);
	cpuid(0,cpid); if (!cpid[0]) return(0);
	cpuid(1,cpb); i = (cpb[3]&~((1<<22)|(1<<30)|(1<<31)));
	cpuid(0x80000000,cpb);
	if (((unsigned int)cpb[0]) > 0x80000000)
	{
		cpuid(0x80000001,cpb);
		i |= (cpb[3]&(1<<31));
		if (!((cpid[1]^0x68747541)|(cpid[3]^0x69746e65)|(cpid[2]^0x444d4163))) //AuthenticAMD
			i |= (cpb[3]&((1<<22)|(1<<30)));
	}
	if (i&(1<<25)) i |= (1<<22); //SSE implies MMX+ support
	return(i);
}
#else
static int getcputype() { return 0; }
#endif

//-----------------------------------

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

//-----------------------------------

	//Hack for peekbits,getbits,suckbits (to prevent lots of duplicate code)
	//   0: PNG: do 12-byte chunk_header removal hack
	// !=0: ZIP: use 64K buffer (olinbuf)
int zipfilmode;

kzfilestate kzfs;

int hxbit[59][2], ibuf0[288], nbuf0[32], ibuf1[32], nbuf1[32];
unsigned char pnginited = 0, olinbuf[65536];
int gotcmov = -2, abstab10[1024];
const unsigned char *filptr;
int clen[320], cclen[19], bitpos;
int ccind[19] = {16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15};
unsigned char slidebuf[32768];

//-----------------------------------

static unsigned char fakebuf[8];
unsigned char *nfilptr;
static int nbitpos;
void suckbitsnextblock ()
{
	int n;

	if (!zipfilmode)
	{
		if (!nfilptr)
		{     //|===|===|crc|lng|typ|===|===|
				//        \  fakebuf: /
				//          |===|===|
				//----x     O---x     O--------
			nbitpos = LSWAPIL(*(int *)&filptr[8]);
			nfilptr = (unsigned char *)&filptr[nbitpos+12];
			*(int *)&fakebuf[0] = *(int *)&filptr[0]; //Copy last dword of IDAT chunk
			if (*(int *)&filptr[12] == LSWAPIB(0x54414449)) //Copy 1st dword of next IDAT chunk
				*(int *)&fakebuf[4] = *(int *)&filptr[16];
			filptr = &fakebuf[4]; bitpos -= 32;
		}
		else
		{
			filptr = nfilptr; nfilptr = 0;
			bitpos -= ((nbitpos-4)<<3);
		}
		//if (n_from_suckbits < 4) will it crash?
	}
	else
	{
			//NOTE: should only read bytes inside compsize, not 64K!!! :/
		*(int *)&olinbuf[0] = *(int *)&olinbuf[sizeof(olinbuf)-4];
		n = min((unsigned)(kzfs.compleng-kzfs.comptell),sizeof(olinbuf)-4);
		fread(&olinbuf[4],n,1,kzfs.fil);
		kzfs.comptell += n;
		bitpos -= ((sizeof(olinbuf)-4)<<3);
	}
}

/*static inline*/ int peekbits (int n) { return((LSWAPIB(*(int *)&filptr[bitpos>>3])>>(bitpos&7))&pow2mask[n]); }
/*static inline*/ void suckbits (int n) { bitpos += n; if (bitpos >= 0) suckbitsnextblock(); }
/*static inline*/ int getbits (int n) { int i = peekbits(n); suckbits(n); return(i); }

//-----------------------------------

//	Variables to speed up dynamic Huffman decoding:
int qhufval0[1<<LOGQHUFSIZ0], qhufval1[1<<LOGQHUFSIZ1];
unsigned char qhufbit0[1<<LOGQHUFSIZ0], qhufbit1[1<<LOGQHUFSIZ1];

static inline int bitrev (int b, int c)
{
	int i, j;
	for(i=1,j=0,c=(1<<c);i<c;i+=i) { j += j; if (b&i) j++; }
	return(j);
}

int hufgetsym (int *hitab, int *hbmax)
{
	int v, n;

	v = n = 0;
	do { v = (v<<1)+getbits(1)+hbmax[n]-hbmax[n+1]; n++; } while (v >= 0);
	return(hitab[hbmax[n]+v]);
}

//	This did not result in a speed-up on P4-3.6Ghz (02/22/2005)
//static int hufgetsym_skipb (int *hitab, int *hbmax, int n, int addit)
//{
//   int v;
//
//   v = bitrev(getbits(n),n)+addit;
//   do { v = (v<<1)+getbits(1)+hbmax[n]-hbmax[n+1]; n++; } while (v >= 0);
//   return(hitab[hbmax[n]+v]);
//}

void qhufgencode (int *hitab, int *hbmax, int *qhval, unsigned char *qhbit, int numbits)
{
	int i, j, k, n, r;

		//r is the bit reverse of i. Ex: if: i = 1011100111, r = 1110011101
	i = r = 0;
	for(n=1;n<=numbits;n++)
		for(k=hbmax[n-1];k<hbmax[n];k++)
			for(j=i+pow2mask[numbits-n];i<=j;i++)
			{
				r = bitrev(i,numbits);
				qhval[r] = hitab[k];
				qhbit[r] = (char)n;
			}
	for(j=pow2mask[numbits];i<=j;i++)
	{
		r = bitrev(i,numbits);

		//k = 0;
		//for(n=0;n<numbits;n++)
		//   k = (k<<1) + ((r>>n)&1) + hbmax[n]-hbmax[n+1];
		//
		//n = numbits;
		//k = hbmax[n]-r;
		//
		//j = peekbits(LOGQHUFSIZ); i = qhufval[j]; j = qhufbit[j];
		//
		//i = j = 0;
		//do
		//{
		//   i = (i<<1)+getbits(1)+nbuf0[j]-nbuf0[j+1]; j++;
		//} while (i >= 0);
		//i = ibuf0[nbuf0[j]+i];
		//qhval[r] = k;

		qhbit[r] = 0; //n-32;
	}

	//   //hufgetsym_skipb related code:
	//for(k=n=0;n<numbits;n++) k = (k<<1)+hbmax[n]-hbmax[n+1];
	//return(k);
}

	//inbuf[inum] : Bit length of each symbol
	//inum        : Number of indices
	//hitab[inum] : Indices from size-ordered list to original symbol
	//hbmax[0-31] : Highest index (+1) of n-bit symbol
void hufgencode (int *inbuf, int inum, int *hitab, int *hbmax)
{
	int i, tbuf[31];

	for(i=30;i;i--) tbuf[i] = 0;
	for(i=inum-1;i>=0;i--) tbuf[inbuf[i]]++;
	tbuf[0] = hbmax[0] = 0; //Hack to remove symbols of length 0?
	for(i=0;i<31;i++) hbmax[i+1] = hbmax[i]+tbuf[i];
	for(i=0;i<inum;i++) if (inbuf[i]) hitab[hbmax[inbuf[i]]++] = i;
}

//-----------------------------------

void initpngtables()
{
	int i, j, k;

		//hxbit[0-58][0-1] is a combination of 4 different tables:
		//   1st parameter: [0-29] are distances, [30-58] are lengths
		//   2nd parameter: [0]: extra bits, [1]: base number

	j = 1; k = 0;
	for(i=0;i<30;i++)
	{
		hxbit[i][1] = j; j += (1<<k);
		hxbit[i][0] = k; k += ((i&1) && (i >= 2));
	}
	j = 3; k = 0;
	for(i=257;i<285;i++)
	{
		hxbit[i+30-257][1] = j; j += (1<<k);
		hxbit[i+30-257][0] = k; k += ((!(i&3)) && (i >= 264));
	}
	hxbit[285+30-257][1] = 258; hxbit[285+30-257][0] = 0;

	k = getcputype();
	if (k&(1<<15))
	{
		gotcmov = 4;
		for(i=0;i<512;i++) abstab10[512+i] = abstab10[512-i] = i;
	}
}
