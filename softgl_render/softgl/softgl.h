
#if defined(_MSC_VER) && !defined(__POCC__)
	#ifndef true
		#define true 1
	#endif
	#ifndef false
		#define false 0
	#endif
	#ifndef bool
		#define bool char
	#endif
#else
	#include <stdbool.h>
#endif

#ifdef SGL_EXPORT
	#ifdef SGL_WINDOWS
		#define SGL_API __declspec(dllexport)
	#else
		#define SGL_API
	#endif
#elif defined(SGL_STATIC)
	#define SGL_API
#else
	#ifdef SGL_WINDOWS
		#define SGL_API __declspec(dllimport)
	#elif defined(SGL_CAUSEWAY)
		#define SGL_API extern
	#else
		#define SGL_API
	#endif
#endif

#define SGL_APIENTRY __cdecl

#ifdef SGL_CAUSEWAY
	#ifdef SGL_EXPORT
		#define SGL_APIENTRY_EXPORT __export N_APIENTRY
	#else
		#define SGL_APIENTRY_EXPORT SGL_APIENTRY
	#endif
#else
	#define SGL_APIENTRY_EXPORT SGL_APIENTRY
#endif

// Возвращает цвет вершины
#define SGL_COLOR(R,G,B,A) ((unsigned int)(((int)(A) << 24) | ((int)(B) << 16) | ((int)(G) << 8) | (int)(R)))

// Тип, описывающий вершину(2D)
typedef struct {
	float tx;
	float ty;
	unsigned int colorRGBA;
	float x;
	float y;
	float z;
} sgl_2dvertex_type;

// Константы, определяющие что рисовать
#define SGL_DRAW_MIN 1
#define SGL_DRAWPOINT 1 
#define SGL_DRAWLINE 2 
#define SGL_DRAWTRIANGLE 3
#define SGL_DRAW_MAX 3

// Форматы пикселей
#define SGL_COLORFORMAT_R8G8B8		0x0001 // Поддерживаются текстуры
#define SGL_COLORFORMAT_R8G8B8A8	0x0002 // Поддерживаются текстуры и поверхности
#define SGL_COLORFORMAT_B8G8R8		0x0003
#define SGL_COLORFORMAT_B8G8R8A8	0x0004
#define SGL_COLORFORMAT_A8B8G8R8	0x0005
#define SGL_COLORFORMAT_L8		0x0006
#define SGL_COLORFORMAT_L8A8		0x0007
#define SGL_COLORFORMAT_X1R5G5B5	0x0008
#define SGL_COLORFORMAT_R5G6B5		0x0009

SGL_API bool SGL_APIENTRY_EXPORT sglCreateSurface(unsigned int sx, unsigned int sy, unsigned int scolortype);
SGL_API bool SGL_APIENTRY_EXPORT sglGetSurface(unsigned int *sx, unsigned int *sy, unsigned int *scolortype, unsigned char **surface);
SGL_API bool SGL_APIENTRY_EXPORT sglDestroySurface(void);
SGL_API bool SGL_APIENTRY_EXPORT sglFlipBuffers(void);
SGL_API bool SGL_APIENTRY_EXPORT sglClearSurface(float cr, float cg, float cb, float ca);
SGL_API void SGL_APIENTRY_EXPORT sglSetClippingRegion(unsigned int sx, unsigned int sy, unsigned int ex, unsigned int ey);
SGL_API bool SGL_APIENTRY_EXPORT sglDraw2d(int batch2d_type, unsigned int batch_currenttex, unsigned int vertices, sgl_2dvertex_type *varray);
SGL_API unsigned int SGL_APIENTRY_EXPORT sglCreateTexture(unsigned int sx, unsigned int sy, unsigned int scolortype, unsigned int rowalignment, unsigned char *buffer);
SGL_API bool SGL_APIENTRY_EXPORT sglDestroyTexture(unsigned int texid);
SGL_API void SGL_APIENTRY_EXPORT sglDestroyAllTextures(void);
SGL_API bool SGL_APIENTRY_EXPORT sglIsTexture(unsigned int texid);
