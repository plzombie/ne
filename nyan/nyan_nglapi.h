/*
	Файл	: nyan_nglapi.h

	Описание: API графической библиотеки

	История	: 05.08.12	Создан

*/

#ifndef NYAN_NGLAPI_H
#define NYAN_NGLAPI_H

#include <stdbool.h>
#include <wchar.h>

#include "nyan_decls_publicapi.h"
#include "nyan_draw_publicapi.h"

#include "nyan_engapi.h"

#ifndef N_NODYLIB
	#define NGLAPI_FUNC(ret, name, params) typedef ret (N_APIENTRY *name##_type) params; extern name##_type funcptr_##name;
#else // Версия, не поддерживающая загрузку из dll
	#define NGLAPI_FUNC(ret, name, params) typedef ret (N_APIENTRY *name##_type) params; extern name##_type funcptr_##name; extern ret N_APIENTRY name params;
#endif

NGLAPI_FUNC(bool, nglSetupDll, (engapi_type *engapi))
NGLAPI_FUNC(bool, nglInit, (void))
NGLAPI_FUNC(bool, nglClose, (void))
NGLAPI_FUNC(void, nglFlip, (void))
NGLAPI_FUNC(void, nglClear, (unsigned int mask))
NGLAPI_FUNC(int, nglGetStatusi, (int status))
NGLAPI_FUNC(void, nglSetStatusi, (int status, int param))
NGLAPI_FUNC(float, nglGetStatusf, (int status))
NGLAPI_FUNC(void, nglSetStatusf, (int status, float param))
NGLAPI_FUNC(const wchar_t *, nglGetStatusw, (int status))
NGLAPI_FUNC(void, nglSetStatusw, (int status, const wchar_t *param))
NGLAPI_FUNC(char, nglGetKey, (unsigned char keyid))
NGLAPI_FUNC(void, nglSetScreen, (unsigned int winx, unsigned int winy, unsigned int winbpp, int winmode, int vsync))
NGLAPI_FUNC(void, nglSetClippingRegion, (int sx, int sy, int ex, int ey))
NGLAPI_FUNC(bool, nglIsTex, (unsigned int id))
NGLAPI_FUNC(unsigned int, nglLoadTexture, (unsigned int flags, unsigned int sizex, unsigned int sizey, unsigned int nglcolorformat, unsigned int nglrowalignment, unsigned char *buffer))
NGLAPI_FUNC(bool, nglUpdateTexture, (unsigned int texid, unsigned char *buffer))
NGLAPI_FUNC(bool, nglFreeTexture, (unsigned int id))
NGLAPI_FUNC(bool, nglFreeAllTextures, (void))
NGLAPI_FUNC(bool, nglBatch2dDraw, (void))
NGLAPI_FUNC(bool, nglBatch2dAdd, (int batch2d_type, unsigned int batch_currenttex, unsigned int vertices, nv_2dvertex_type *varray))
NGLAPI_FUNC(bool, nglBatch3dDrawMesh, (unsigned int batch_currenttex, unsigned int vertices, nv_3dvertex_type *varray))
NGLAPI_FUNC(bool, nglBatch3dDrawIndexedMesh, (unsigned int batch_currenttex, unsigned int vertices, nv_3dvertex_type *varray, unsigned int triangles, unsigned int *iarray))
NGLAPI_FUNC(void, nglBatch2dBegin, (void))
NGLAPI_FUNC(void, nglBatch2dEnd, (void))
NGLAPI_FUNC(bool, nglBatch3dSetModelviewMatrix, (double *matrix))
NGLAPI_FUNC(void, nglBatch3dSetAmbientLight, (float r, float g, float b, float a))
NGLAPI_FUNC(void, nglBatch3dBegin, (void))
NGLAPI_FUNC(void, nglBatch3dEnd, (void))
NGLAPI_FUNC(bool, nglReadScreen, (unsigned char *buffer))

#endif
