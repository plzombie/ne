/*
	Файл	: nyan_vis_init.c

	Описание: Визуализациа

	История	: 05.08.12	Создан

*/

#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#include "nyan_vismodes_publicapi.h"
#include "nyan_draw_publicapi.h"
#include "nyan_texformat_publicapi.h"
#include "nyan_log_publicapi.h"
#include "nyan_filesys_publicapi.h"
#include "nyan_filesys_dirpaths_publicapi.h"
#include "nyan_mem_publicapi.h"
#include "nyan_fps_publicapi.h"
#include "nyan_threads_publicapi.h"
#include "nyan_vis_init_publicapi.h"
#include "nyan_vis_fonts_publicapi.h"
#include "nyan_vis_draw_publicapi.h"
#include "nyan_vis_texture_publicapi.h"
#include "nyan_vis_3dmodel_publicapi.h"

#include "nyan_text.h"

#include "nyan_vis_init.h"
#include "nyan_vis_draw.h"
#include "nyan_vis_fonts.h"
#include "nyan_vis_3dmodel.h"
#include "nyan_vis_texture.h"
#include "nyan_fps.h"
#include "nyan_getproc.h"

#include "nyan_apifordlls.h"

#include "nyan_nglapi.h"

#include "../extclib/_wcsicmp.h"

nglSetupDll_type funcptr_nglSetupDll = 0;
nglInit_type funcptr_nglInit = 0;
nglClose_type funcptr_nglClose = 0;
nglFlip_type funcptr_nglFlip = 0;
nglClear_type funcptr_nglClear = 0;
nglGetStatusi_type funcptr_nglGetStatusi = 0;
nglSetStatusi_type funcptr_nglSetStatusi = 0;
nglGetStatusf_type funcptr_nglGetStatusf = 0;
nglSetStatusf_type funcptr_nglSetStatusf = 0;
nglGetStatusw_type funcptr_nglGetStatusw = 0;
nglSetStatusw_type funcptr_nglSetStatusw = 0;
nglGetKey_type funcptr_nglGetKey = 0;
nglSetScreen_type funcptr_nglSetScreen = 0;
nglSetClippingRegion_type funcptr_nglSetClippingRegion = 0;
nglIsTex_type funcptr_nglIsTex = 0;
nglLoadTexture_type funcptr_nglLoadTexture = 0;
nglUpdateTexture_type funcptr_nglUpdateTexture = 0;
nglFreeTexture_type funcptr_nglFreeTexture = 0;
nglFreeAllTextures_type funcptr_nglFreeAllTextures = 0;
nglBatch2dDraw_type funcptr_nglBatch2dDraw = 0;
nglBatch2dAdd_type funcptr_nglBatch2dAdd = 0;
nglBatch3dDrawMesh_type funcptr_nglBatch3dDrawMesh = 0;
nglBatch3dDrawIndexedMesh_type funcptr_nglBatch3dDrawIndexedMesh = 0;
nglBatch2dBegin_type funcptr_nglBatch2dBegin = 0;
nglBatch2dEnd_type funcptr_nglBatch2dEnd = 0;
nglBatch3dSetModelviewMatrix_type funcptr_nglBatch3dSetModelviewMatrix = 0;
nglBatch3dSetAmbientLight_type funcptr_nglBatch3dSetAmbientLight = 0;
nglBatch3dBegin_type funcptr_nglBatch3dBegin = 0;
nglBatch3dEnd_type funcptr_nglBatch3dEnd = 0;
nglReadScreen_type funcptr_nglReadScreen = 0;

#ifndef N_NODYLIB
	#define NGLAPI_IMPORTFUNC(name) funcptr_##name = (name##_type)nGetProcAddress(ngl_dllhandle, #name, "_"#name); \
		if(!funcptr_##name) { \
			nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT8, F_NVATTACHRENDER, ERR_CANTIMPORTFUNC, L###name); \
			return false; \
		}
#else
	#define NGLAPI_IMPORTFUNC(name) funcptr_##name = name;
#endif

#ifndef N_NODYLIB
static void *ngl_dllhandle;
#endif

int nv_isinit = false;
int nv_isrenderattached = false;

/*
	Функция	: nvAttachRender

	Описание: Прикрепляет рендер к движку

	История	: 16.08.12	Создан

*/
N_API bool N_APIENTRY_EXPORT nvAttachRender(const wchar_t *dllname)
{
	if(nv_isinit) { nlPrint(LOG_FDEBUGFORMAT, F_NVATTACHRENDER, N_FALSE); return false; }

#ifdef N_NODYLIB
	nlPrint(F_NVATTACHRENDER); nlAddTab(1);
#else
	if(!dllname) { nlPrint(LOG_FDEBUGFORMAT, F_NVATTACHRENDER, N_FALSE); return false; }

	nlPrint(LOG_FDEBUGFORMAT7, F_NVATTACHRENDER, N_FNAME, dllname); nlAddTab(1);

	if(nv_isrenderattached)
		nFreeLib(ngl_dllhandle);

	nv_isrenderattached = false;

	ngl_dllhandle = nLoadModule(dllname);
	if(!ngl_dllhandle) {
		nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT8, F_NVATTACHRENDER, ERR_CANTLOADDLL, dllname);
		return false;
	}
#endif

	// Импортирую nglSetupDll
	NGLAPI_IMPORTFUNC(nglSetupDll)

	// Импортирую nglInit
	NGLAPI_IMPORTFUNC(nglInit)

	NGLAPI_IMPORTFUNC(nglClose)

	NGLAPI_IMPORTFUNC(nglFlip)

	NGLAPI_IMPORTFUNC(nglClear)

	NGLAPI_IMPORTFUNC(nglGetStatusi)

	NGLAPI_IMPORTFUNC(nglSetStatusi)

	NGLAPI_IMPORTFUNC(nglGetStatusf)

	NGLAPI_IMPORTFUNC(nglSetStatusf)

	NGLAPI_IMPORTFUNC(nglGetStatusw)

	NGLAPI_IMPORTFUNC(nglSetStatusw)

	NGLAPI_IMPORTFUNC(nglGetKey)

	NGLAPI_IMPORTFUNC(nglSetScreen)

	NGLAPI_IMPORTFUNC(nglSetClippingRegion)

	NGLAPI_IMPORTFUNC(nglIsTex)

	NGLAPI_IMPORTFUNC(nglLoadTexture)

	NGLAPI_IMPORTFUNC(nglUpdateTexture)

	NGLAPI_IMPORTFUNC(nglFreeTexture)

	NGLAPI_IMPORTFUNC(nglFreeAllTextures)

	NGLAPI_IMPORTFUNC(nglBatch2dAdd)

	NGLAPI_IMPORTFUNC(nglBatch2dDraw)

	NGLAPI_IMPORTFUNC(nglBatch3dDrawMesh)

	NGLAPI_IMPORTFUNC(nglBatch3dDrawIndexedMesh)

	NGLAPI_IMPORTFUNC(nglBatch2dBegin)

	NGLAPI_IMPORTFUNC(nglBatch2dEnd)

	NGLAPI_IMPORTFUNC(nglBatch3dSetModelviewMatrix)

	NGLAPI_IMPORTFUNC(nglBatch3dSetAmbientLight)

	NGLAPI_IMPORTFUNC(nglBatch3dBegin)

	NGLAPI_IMPORTFUNC(nglBatch3dEnd)

	NGLAPI_IMPORTFUNC(nglReadScreen)

	// Настраиваю dll, передаю функции движка
	if(!funcptr_nglSetupDll(&n_ea)) {
#ifndef N_NODYLIB
		nFreeLib(ngl_dllhandle);
#endif
		nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT8, F_NVATTACHRENDER, ERR_CANTLOADDLL, dllname);
		return false;
	}

	nv_isrenderattached = true;

	nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_NVATTACHRENDER, N_OK);

	return true;
}

/*
	Функция	: nvInit

	Описание: Инициализирует модуль визуализации

	История	: 05.08.12	Создан

*/
int nvInit(void)
{
	if(nv_isinit) return false;

	nlPrint(F_NVINIT); nlAddTab(1);

	nv_textures_sync_mutex = nCreateMutex();
	if(!nv_textures_sync_mutex) {
		nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_NVINIT, N_FALSE);
		return false;
	}

	if(!nv_isrenderattached) nvAttachRender(L"ngl.dll");

	if(!nv_isrenderattached) { nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT2, F_NVINIT, N_FALSE, ERR_UMUSTATTACHRENDERBSTE); return false; }

	if(!funcptr_nglInit()) {
		nDestroyMutex(nv_textures_sync_mutex);
		nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_NVINIT, N_FALSE);
#ifndef N_NODYLIB
		nFreeLib(ngl_dllhandle);
#endif
		nv_isrenderattached = false;
		return false;
	}

	nv_isinit = true;

	nlPrint(NV_SCREENINFO, F_NVINIT, nvGetStatusi(NV_STATUS_WINX), nvGetStatusi(NV_STATUS_WINY), nvGetStatusi(NV_STATUS_WINBPP), (nvGetStatusi(NV_STATUS_WINMODE) == NV_MODE_FULLSCREEN)?N_YES:N_NO, nvGetStatusi(NV_STATUS_WINDPIX), nvGetStatusi(NV_STATUS_WINDPIY));

	nvDrawInit();

	fps_deltaclocks = 1;
	fps_lastclocks = nClock();
	if(fps_lastclocks == -1)
		fps_lastclocks = 0;
	afps_deltaclocks = 1;
	afps_lastnulldeltaclockslen = 1;
	afps_lastnulldeltas = 0;
	afps_nulldeltas = 0;

	nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_NVINIT, N_OK);

	return true;
}

/*
	Функция	: nvClose

	Описание: Деинициализирует модуль визуализации

	История	: 05.08.12	Создан

*/
int nvClose(void)
{
	if(!nv_isinit) return false;

	nlPrint(F_NVCLOSE); nlAddTab(1);

	nvDrawClose();

	nvDestroyAllFonts();

	nvDestroyAll3dModels();

	nvDestroyAllTextures();

	nDestroyMutex(nv_textures_sync_mutex);
	nv_textures_sync_mutex = 0;

	if(nv_fontvertexbufsize) {
		nFreeMemory(nv_fontvertexbuf);
		nv_fontvertexbuf = 0;
		nv_fontvertexbufsize = 0;
	}

	if(!funcptr_nglClose())
		nlPrint(LOG_FDEBUGFORMAT6, F_NVCLOSE, L"nglClose()", ERR_RETURNSFALSE);

#ifndef N_NODYLIB
	if(nv_isrenderattached) {
		nv_isrenderattached = false;
		nFreeLib(ngl_dllhandle);
	}
#endif

	nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_NVCLOSE, N_OK);

	nv_isinit = false;

	return true;
}

/*
	Функция	: nvFlip

	Описание: Переключение кадра, получение сообщений от окна

	История	: 12.08.12	Создан

*/
void nvFlip(void)
{
	int64_t curclocks;

	if(!nv_isinit) return;

	if(nv_draw_state == NV_DRAW_STATE_2D) nvEnd2d();
	if(nv_draw_state == NV_DRAW_STATE_3D) nvEnd3d();

	curclocks = nClock();
	if(curclocks == -1) {
		fps_deltaclocks = 0;
	} else {
		fps_deltaclocks = curclocks-fps_lastclocks;
		fps_lastclocks = curclocks;
	}

	// Подсчитываем количество кадров длинной в 0 тиков в afps_nulldeltas
	// Как только появился кадр с fps_deltaclocks != 0, записываем его длину в afps_lastnulldeltaclockslen
	// А количество нулевых кадров afps_nulldeltas в afps_lastnulldeltas
	// Считаем, что fps_deltaclocks - это длина afps_nulldeltas + последнего кадра
	// В следующий раз, когда попадается кадры длины 0, считаем его усреднённую дину (afps_deltaclocks) как длину предыдущих нулевых кадров
	if(fps_deltaclocks == 0) {
		afps_nulldeltas++;
		afps_deltaclocks = (double)afps_lastnulldeltaclockslen / (double)(afps_lastnulldeltas + 1);
	} else {
		if (afps_nulldeltas) {
			afps_lastnulldeltas = afps_nulldeltas;
			afps_lastnulldeltaclockslen = fps_deltaclocks;
			afps_nulldeltas = 0;
			afps_deltaclocks = (double)afps_lastnulldeltaclockslen / (double)(afps_lastnulldeltas + 1);
		} else
			afps_deltaclocks = (double)fps_deltaclocks;
	}

	funcptr_nglFlip();
}

/*
	Функция	: nvClear

	Описание: Очищает буфер цвета или буфер глубины

	История	: 05.02.14	Создан

*/
N_API void N_APIENTRY_EXPORT nvClear(unsigned int mask)
{
	if(!nv_isinit) return;

	funcptr_nglClear(mask);
}

/*
	Функция	: nvGetStatusi

	Описание: Возвращает параметры, связанные с графическим движком

	История	: 12.08.12	Создан

*/
N_API int N_APIENTRY_EXPORT nvGetStatusi(int status)
{
	if(!nv_isrenderattached) { nlPrint(LOG_FDEBUGFORMAT2, F_NVGETSTATUSI, N_FALSE, ERR_UMUSTATTACHRENDERBCRP); return 0; }

	return funcptr_nglGetStatusi(status);
}

/*
	Функция	: nvSetStatusi

	Описание: Устанавливает параметры, связанные с графическим движком

	История	: 12.08.12	Создан

*/
N_API void N_APIENTRY_EXPORT nvSetStatusi(int status, int param)
{
	if(!nv_isrenderattached) { nlPrint(LOG_FDEBUGFORMAT2, F_NVSETSTATUSI, N_FALSE, ERR_UMUSTATTACHRENDERBCRP); return; }

	funcptr_nglSetStatusi(status, param);
}

/*
	Функция	: nvGetStatusf

	Описание: Возвращает параметры, связанные с графическим движком

	История	: 23.08.12	Создан

*/
N_API float N_APIENTRY_EXPORT nvGetStatusf(int status)
{
	if(!nv_isrenderattached) { nlPrint(LOG_FDEBUGFORMAT2, F_NVGETSTATUSF, N_FALSE, ERR_UMUSTATTACHRENDERBCRP); return 0; }

	return funcptr_nglGetStatusf(status);
}

/*
	Функция	: nvSetStatusf

	Описание: Устанавливает параметры, связанные с графическим движком

	История	: 23.08.12	Создан

*/
N_API void N_APIENTRY_EXPORT nvSetStatusf(int status, float param)
{
	if(!nv_isrenderattached) { nlPrint(LOG_FDEBUGFORMAT2, F_NVSETSTATUSF, N_FALSE, ERR_UMUSTATTACHRENDERBCRP); return; }

	funcptr_nglSetStatusf(status, param);
}

/*
	Функция	: nvGetStatusw

	Описание: Возвращает параметры, связанные с графическим движком

	История	: 23.08.12	Создан

*/
N_API const wchar_t * N_APIENTRY_EXPORT nvGetStatusw(int status)
{
	if(!nv_isrenderattached) { nlPrint(LOG_FDEBUGFORMAT2, F_NVGETSTATUSW, N_FALSE, ERR_UMUSTATTACHRENDERBCRP); return 0; }

	return funcptr_nglGetStatusw(status);
}

/*
	Функция	: nvSetStatusw

	Описание: Устанавливает параметры, связанные с графическим движком

	История	: 23.08.12	Создан

*/
N_API void N_APIENTRY_EXPORT nvSetStatusw(int status, const wchar_t *param)
{
	if(!nv_isrenderattached) { nlPrint(LOG_FDEBUGFORMAT2, F_NVSETSTATUSW, N_FALSE, ERR_UMUSTATTACHRENDERBCRP); return; }

	funcptr_nglSetStatusw(status, param);
}

/*
	Функция	: nvGetKey

	Описание: Возвращает состояние кнопки

	История	: 12.08.12	Создан

*/
N_API char N_APIENTRY_EXPORT nvGetKey(unsigned char keyid)
{
	if(!nv_isrenderattached) { nlPrint(LOG_FDEBUGFORMAT2, F_NVGETKEY, N_FALSE, ERR_UMUSTATTACHRENDERBCRP); return 0; }

	return funcptr_nglGetKey(keyid);
}

/*
	Функция	: nvSetScreen

	Описание: Устанавливает параметры экрана

	История	: 12.08.12	Создан

*/
N_API void N_APIENTRY_EXPORT nvSetScreen(int winx, int winy, int winbpp, int winmode, int vsync)
{
	if(!nv_isrenderattached) { nlPrint(LOG_FDEBUGFORMAT2, F_NVSETSCREEN, N_FALSE, ERR_UMUSTATTACHRENDERBCRP); return; }

	funcptr_nglSetScreen(winx, winy, winbpp, winmode, vsync);
}

/*
Функция	: nvSetClippingRegion

Описание: Устанавливает область отрисовки

История	: 21.10.16	Создан

*/
N_API void N_APIENTRY_EXPORT nvSetClippingRegion(int sx, int sy, int ex, int ey)
{
	if(!nv_isrenderattached) { nlPrint(LOG_FDEBUGFORMAT2, F_NVSETCLIPPINGREGION, N_FALSE, ERR_UMUSTATTACHRENDERBCRP); return; }

	funcptr_nglSetClippingRegion(sx, sy, ex, ey);
}

/*
	Функция	: nvMakeScreenshot

	Описание: Устанавливает параметры экрана

	История	: 12.08.12	Создан

*/
N_API void N_APIENTRY_EXPORT nvMakeScreenshot(const wchar_t *filename, unsigned int relpath)
{
	unsigned char *scrbuf;
	unsigned int f = 0;
	unsigned char *p, *p2;
	wchar_t *absfilename = 0;
	int sizex, sizey;
	int mode, offset, i;
	wchar_t *filext;

	if(!nv_isinit) return;

	nlPrint(LOG_FDEBUGFORMAT7, F_NVMAKESCREENSHOT, N_FNAME, filename); nlAddTab(1);

	filext = wcsrchr(filename,L'.')+1;

	if(_wcsicmp(filext,L"bmp") == 0) {
		mode = 1; // Сохранить скриншот в bmp файл
		offset = 54;
	/*} else if(_wcsicmp(filext,L"nek0") == 0) {
		mode = 2; // Сохранить скриншот в bmp файл
		offset = 20;*/
	} else {
		mode = 0; // Сохранить скриншот в tga файл
		offset = 18;
	}

	sizex = funcptr_nglGetStatusi(NV_STATUS_WINX);
	sizey = funcptr_nglGetStatusi(NV_STATUS_WINY);

	scrbuf = nAllocMemory(sizex*sizey*4+offset);
	if(!scrbuf) {
		nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_NVMAKESCREENSHOT, N_FALSE);
		return;
	}

	switch(mode) {
		case 0: // TGA
			scrbuf[0] = 0;
			scrbuf[1] = 0;
			scrbuf[2] = 2;
			scrbuf[3] = 0;
			scrbuf[4] = 0;
			scrbuf[5] = 0;
			scrbuf[6] = 0;
			scrbuf[7] = 0;
			*(short *)(scrbuf+8) = 0;
			*(short *)(scrbuf+10) = 0;
			*(short *)(scrbuf+12) = sizex;
			*(short *)(scrbuf+14) = sizey;
			scrbuf[16] = 32;
			scrbuf[17] = 0;
			break;
		case 1: // BMP
			scrbuf[0] = 'B'; //bfType
			scrbuf[1] = 'M';
			*((int *)(scrbuf+2)) = sizex*sizey*4+offset; //bfSize
			scrbuf[6] = 0;//bfReserved1
			scrbuf[7] = 0;
			scrbuf[8] = 0;//bfReserved2
			scrbuf[9] = 0;
			*((int *)(scrbuf+10)) = 54;//bfOffBits
			*((int *)(scrbuf+14)) = 40;//biSize
			*((int *)(scrbuf+18)) = sizex;//biWidth
			*((int *)(scrbuf+22)) = sizey;//biHeight
			*((short *)(scrbuf+26)) = 1;//biPlanes
			*((short *)(scrbuf+28)) = 32;//biBitCount
			*((int *)(scrbuf+30)) = 0;//biCompression
			*((int *)(scrbuf+34)) = 0;//biSizeImage
			*((int *)(scrbuf+38)) = 0;//biXPelsPerMeter
			*((int *)(scrbuf+42)) = 0;//biYPelsPerMeter
			*((int *)(scrbuf+46)) = 0;//biClrUsed
			*((int *)(scrbuf+50)) = 0;//biClrImportant
			break;
		/*case 2:
			scrbuf[0] = 'N';
			scrbuf[1] = 'E';
			scrbuf[2] = 'K';
			scrbuf[3] = '0';
			*((int *)(scrbuf+4)) = sizex;
			*((int *)(scrbuf+8)) = sizey;
			*((int *)(scrbuf+12)) = NGL_COLORFORMAT_R8G8B8A8;
			*((int *)(scrbuf+16)) = 4;*/
	}

	funcptr_nglReadScreen(scrbuf+offset);

	if(mode < 2) {
		p = scrbuf+offset; p2 = scrbuf+offset+2;
		for(i = 0; i<sizex*sizey; i++) { // RGBA в BGRA
			unsigned char b;

			b = *p;
			*p = *p2;
			*p2 = b;
			p += 4;
			p2 += 4;
		}
	}

	p = scrbuf+offset+3;
	for(i = 0; i<sizex*sizey; i++) {
		*p = 255; p += 4;
	}

	absfilename = nFileGetAbsoluteFilename(filename, relpath);
	if(!absfilename) goto EXIT;

	if(!nFileCreate(absfilename, true, NF_PATH_CURRENTDIR)) goto EXIT;

	f = nFileOpen(absfilename);

	if(!f) goto EXIT;

	nlPrint(L"SS SIZE %ld", nFileWrite(f, scrbuf, sizex*sizey*4+offset));

	nFileClose(f);

EXIT:
	if(absfilename) nFreeMemory(absfilename);

	nFreeMemory(scrbuf);

	nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_NVMAKESCREENSHOT, f?N_OK:false);
}
