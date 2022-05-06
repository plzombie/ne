// Copyright (c) 1998-2008 Ken Silverman
// Copyright (c) 2014 plzombie

#ifndef KPLIB_SWAPBITS_H
#define KPLIB_SWAPBITS_H

#if defined(__POWERPC__)
	#define BIGENDIAN 1
#endif

#ifdef BIGENDIAN
	#define LSWAPIB(a) (unsigned int)((((unsigned int)(a)>>8)&0xff00)+(((unsigned int)(a)&0xff00)<<8)+((unsigned int)(a)<<24)+((unsigned int)(a)>>24))
	#define SSWAPIB(a) (unsigned short)(((unsigned short)(a)>>8)+((unsigned short)(a)<<8))
	#define LSWAPIL(a) (a)
	#define SSWAPIL(a) (a)
#else
	#define LSWAPIB(a) (a)
	#define SSWAPIB(a) (a)
	#define LSWAPIL(a) (unsigned int)((((unsigned int)(a)>>8)&0xff00)+(((unsigned int)(a)&0xff00)<<8)+((unsigned int)(a)<<24)+((unsigned int)(a)>>24))
	#define SSWAPIL(a) (unsigned short)(((unsigned short)(a)>>8)+((unsigned short)(a)<<8))
#endif

#endif
