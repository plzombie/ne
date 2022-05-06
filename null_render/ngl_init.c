/*
	Файл	: ngl_init.c

	Описание: Инициализация графической библиотеки

	История	: 05.10.12	Создан

*/

#include <stdio.h>
#include <stdbool.h>

#include "ngl.h"
#include "ngl_text.h"

#include "../nyan/nyan_texformat_publicapi.h"
#include "../nyan/nyan_vismodes_publicapi.h"

#include "ngl_init.h"
#include "ngl_batch.h"
#include "ngl_texture.h"

engapi_type *ngl_ea = 0;

bool ngl_isinit = false;

#define NGL_DEFAULT_WINX 640
#define NGL_DEFAULT_WINY 480
#define NGL_DEFAULT_WINBPP 32

unsigned int ngl_winx = NGL_DEFAULT_WINX, ngl_winy = NGL_DEFAULT_WINY, ngl_winbpp = NGL_DEFAULT_WINBPP; // Ширина и высота окна, бит на пиксель
int ngl_winmbl = NV_KEYSTATUS_UNTOUCHED, ngl_winmbr = NV_KEYSTATUS_UNTOUCHED, ngl_winmbm = NV_KEYSTATUS_UNTOUCHED, ngl_winmx = 0, ngl_winmy = 0, ngl_winmwheel = 0; // Параметры мыши
int ngl_winmode = NV_MODE_WINDOWED_FIXED, ngl_win_vsync = 0;
int ngl_winclippingregion = false, ngl_winclippingregion_sx = 0, ngl_winclippingregion_sy = 0, ngl_winclippingregion_ex = NGL_DEFAULT_WINX, ngl_winclippingregion_ey = NGL_DEFAULT_WINY;
int ngl_win_exitmsg = false;
unsigned int ngl_win_updateinterval = 0;
char ngl_winkeys[256] = {0};

bool ngl_win_havetouch = false;
int ngl_wintouchcount = 0, ngl_wintouchmaxcount = NV_STATUS_WINTOUCHMAX_X-NV_STATUS_WINTOUCH0_X+1;
int ngl_wintouchx[NV_STATUS_WINTOUCHMAX_X-NV_STATUS_WINTOUCH0_X+1] = {0};
int ngl_wintouchy[NV_STATUS_WINTOUCHMAX_Y-NV_STATUS_WINTOUCH0_Y+1] = {0};
int ngl_wintouchpressed[NV_STATUS_WINTOUCHMAX_PRESSED-NV_STATUS_WINTOUCH0_PRESSED+1] = {0};

const wchar_t *ngl_wintitle = N_NAME; // Имя окна
wchar_t ngl_wintextinputbuf[256] = {0}; // Буфер, куда выводится текст, введённый между кадрами
int ngl_wintextinputbufsize = 0;

int ngl_windpix = 100;
int ngl_windpiy = 100;

float ngl_winbcred = 0.0, ngl_winbcgreen = 0.0, ngl_winbcblue = 0.0;
bool ngl_winbcchanged = false;

int ngl_glrowalignment = 4;

N_API void N_APIENTRY_EXPORT nglSetScreen(unsigned int winx, unsigned int winy, unsigned int winbpp, int winmode, int vsync);
static void nglUpdateClippingRegion(void);
N_API void N_APIENTRY_EXPORT nglSetClippingRegion(int sx, int sy, int ex, int ey);

/*
	Функция	: nglSetupDll

	Описание: Настройка dll графической библиотеки

	История	: 05.10.12	Создан

*/
N_API bool N_APIENTRY_EXPORT nglSetupDll(engapi_type *engapi)
{
	if(engapi) {
		if(engapi->mysize == sizeof(engapi_type)) {
			ngl_ea = engapi;

			return true;
		}
	}

	return false;
}

/*
	Функция	: nglInit

	Описание: Инициализация графического движка

	История	: 05.10.12	Создан

*/
N_API bool N_APIENTRY_EXPORT nglInit(void)
{
	if(ngl_isinit || !ngl_ea) return false;
	
	ngl_ea->nlPrint(F_NGLINIT); ngl_ea->nlAddTab(1);
	
	if(!nglInitWindow()) {
		ngl_ea->nlAddTab(-1); ngl_ea->nlPrint(LOG_FDEBUGFORMAT, F_NGLINIT, N_FALSE);
		return false;
	}
	
	// Определение угла анизотропии
	ngl_ea->nlPrint(LOG_FDEBUGFORMAT3,F_NGLINIT,NGL_MAXANISOTROPY,ngl_tex_maxanis);
	if(ngl_tex_anis > ngl_tex_maxanis) ngl_tex_anis = ngl_tex_maxanis;
	ngl_ea->nlPrint(LOG_FDEBUGFORMAT3,F_NGLINIT,NGL_DEFAULTANISOTROPY,ngl_tex_anis);
	
	ngl_isinit = true;
	
	if(!nglBatchInit()) {
		nglCloseWindow();
		ngl_isinit = false;
		ngl_ea->nlAddTab(-1); ngl_ea->nlPrint(LOG_FDEBUGFORMAT, F_NGLINIT, N_OK);
		return false;
	}
	
	nglUpdateClippingRegion();

	ngl_win_exitmsg = false;

	ngl_textures_sync_mutex = ngl_ea->nCreateMutex();
	if(!ngl_textures_sync_mutex) {
		nglCloseWindow();
		ngl_isinit = false;
		ngl_ea->nlAddTab(-1); ngl_ea->nlPrint(LOG_FDEBUGFORMAT, F_NGLINIT, N_FALSE);
	}
	
	ngl_ea->nlAddTab(-1); ngl_ea->nlPrint(LOG_FDEBUGFORMAT, F_NGLINIT, N_OK);
	
	return true;
}

/*
	Функция : nglSetDefaultParameters

	Описание: Устанавливает параметры по умолчанию для графического движка

	История : 09.04.18	Создан

*/
static void nglSetDefaultParameters(void)
{
	size_t i;

	ngl_winx = NGL_DEFAULT_WINX; ngl_winy = NGL_DEFAULT_WINY; ngl_winbpp = NGL_DEFAULT_WINBPP; // Ширина и высота окна, бит на пиксель
	ngl_winmbl = ngl_winmbr = ngl_winmbm = NV_KEYSTATUS_UNTOUCHED; ngl_winmx = 0; ngl_winmy = 0; ngl_winmwheel = 0; // Параметры мыши
	ngl_winmode = NV_MODE_WINDOWED_FIXED; ngl_win_vsync = 0;
	ngl_winclippingregion = false; ngl_winclippingregion_sx = 0; ngl_winclippingregion_sy = 0; ngl_winclippingregion_ex = NGL_DEFAULT_WINX; ngl_winclippingregion_ey = NGL_DEFAULT_WINY;

	ngl_win_updateinterval = 0;

	for(i = 0; i < 256; i++)
		ngl_winkeys[i] = NV_KEYSTATUS_UNTOUCHED;

	for(i = 0; i < NV_STATUS_WINTOUCHMAX_X-NV_STATUS_WINTOUCH0_X+1; i++)
		ngl_wintouchx[i] = NV_KEYSTATUS_UNTOUCHED;

	for(i = 0; i < NV_STATUS_WINTOUCHMAX_Y-NV_STATUS_WINTOUCH0_Y+1; i++)
		ngl_wintouchy[i] = NV_KEYSTATUS_UNTOUCHED;

	for(i = 0; i < NV_STATUS_WINTOUCHMAX_PRESSED-NV_STATUS_WINTOUCH0_PRESSED+1; i++)
		ngl_wintouchpressed[i] = NV_KEYSTATUS_UNTOUCHED;

	ngl_wintextinputbuf[0] = 0;
	ngl_wintextinputbufsize = 0;

	ngl_winbcred = ngl_winbcgreen = ngl_winbcblue = 0.0f;
	ngl_winbcchanged = false;
}

/*
	Функция	: nglClose

	Описание: Деинициализация графического движка

	История	: 05.08.12	Создан

*/
N_API bool N_APIENTRY_EXPORT nglClose(void)
{
	if(!ngl_isinit) return false;
	
	ngl_ea->nlPrint(F_NGLCLOSE); ngl_ea->nlAddTab(1);
	
	nglBatchClose();

	nglFreeAllTextures();

	ngl_ea->nDestroyMutex(ngl_textures_sync_mutex);
	
	if(!nglCloseWindow())
		ngl_ea->nlPrint(LOG_FDEBUGFORMAT6, F_NGLCLOSE, F_NGLCLOSEWINDOW, ERR_RETURNSFALSE);
		
	nglSetDefaultParameters();
		
	ngl_isinit = false;
	
	ngl_ea->nlAddTab(-1); ngl_ea->nlPrint(LOG_FDEBUGFORMAT, F_NGLCLOSE, N_OK);
	
	return true;
}

/*
	Функция	: nglFlip

	Описание: Переключение кадра, получение сообщений от окна

	История	: 01.06.12	Создан

*/
N_API void N_APIENTRY_EXPORT nglFlip(void)
{
	unsigned int i;

	if(!ngl_isinit) return;

	if(ngl_winmbl == NV_KEYSTATUS_RELEASED) ngl_winmbl = NV_KEYSTATUS_UNTOUCHED;
	if(ngl_winmbr == NV_KEYSTATUS_RELEASED) ngl_winmbr = NV_KEYSTATUS_UNTOUCHED;
	if(ngl_winmbm == NV_KEYSTATUS_RELEASED) ngl_winmbm = NV_KEYSTATUS_UNTOUCHED;
	ngl_winmwheel = 0;
	if(ngl_win_havetouch)
		for(i = 0; i < (unsigned int)ngl_wintouchmaxcount; i++)
			if(ngl_wintouchpressed[i] == NV_KEYSTATUS_RELEASED) ngl_wintouchpressed[i] = NV_KEYSTATUS_UNTOUCHED;
	for(i = 0; i < 255; i++)
		if(ngl_winkeys[i] == NV_KEYSTATUS_RELEASED) ngl_winkeys[i] = NV_KEYSTATUS_UNTOUCHED;
	nglUpdateWindow();
	ngl_win_exitmsg = true;

	if(ngl_winbcchanged) {
		// Смена цветов фона
		ngl_winbcchanged = false;
	}
	// Очистка экрана

	nglUpdateClippingRegion();
}

/*
	Функция	: nglClear

	Описание: Очищает буфер цвета или буфер глубины

	История	: 07.02.14	Создан

*/
N_API void N_APIENTRY_EXPORT nglClear(unsigned int mask)
{

	if(!ngl_isinit) return;

	if(ngl_winbcchanged) {
		// Смена цветов фона
		ngl_winbcchanged = false;
	}

	if(mask & NV_CLEAR_COLOR_BUFFER) {
		// Очистка буфера глубины
	}

	if(mask & NV_CLEAR_DEPTH_BUFFER) {
		// Очистка экрана
	}
}

/*
	Функция	: nglGetStatusi

	Описание: Возвращает параметры, связанные с графическим движком

	История	: 01.06.12	Создан

*/
N_API int N_APIENTRY_EXPORT nglGetStatusi(int status)
{
	switch(status) {
		case NV_STATUS_WINMODE:
			return ngl_winmode;
		case NV_STATUS_WINBPP:
			return ngl_winbpp;
		case NV_STATUS_WINX:
			return ngl_winx;
		case NV_STATUS_WINY:
			return ngl_winy;
		case NV_STATUS_WINMBL:
			return ngl_winmbl;
		case NV_STATUS_WINMBR:
			return ngl_winmbr;
		case NV_STATUS_WINMBM:
			return ngl_winmbm;
		case NV_STATUS_WINMX:
			return ngl_winmx;
		case NV_STATUS_WINMY:
			return ngl_winmy;
		case NV_STATUS_WIN_EXITMSG:
			return ngl_win_exitmsg;
		case NV_STATUS_WINVSYNC:
			return ngl_win_vsync;
		case NV_STATUS_SUPPORT_WINVSYNC:
			return ngl_win_swapc_ext;
		case NV_STATUS_WINCLIPPINGREGION:
			return ngl_winclippingregion;
		case NV_STATUS_WINCLIPPINGREGIONSX:
			return ngl_winclippingregion_sx;
		case NV_STATUS_WINCLIPPINGREGIONSY:
			return ngl_winclippingregion_sy;
		case NV_STATUS_WINCLIPPINGREGIONEX:
			return ngl_winclippingregion_ex;
		case NV_STATUS_WINCLIPPINGREGIONEY:
			return ngl_winclippingregion_ey;
		case NV_STATUS_WINUPDATEINTERVAL:
			return ngl_win_updateinterval;
		case NV_STATUS_SUPPORT_WINTOUCH:
			return ngl_win_havetouch;
		case NV_STATUS_WINTOUCHCOUNT:
			return ngl_wintouchcount;
		case NV_STATUS_WINTOUCHMAXCOUNT:
			return ngl_wintouchmaxcount;
		case NV_STATUS_IS_WINDPINONSTANDARD:
			if(ngl_windpix != 100 || ngl_windpiy != 100)
				return 1;
			else
				return 0;
		case NV_STATUS_WINDPIX:
			return ngl_windpix;
		case NV_STATUS_WINDPIY:
			return ngl_windpiy;
		case NV_STATUS_WINMWHEEL:
			return ngl_winmwheel;
		default:
			if(status >= NV_STATUS_WINTOUCH0_X && status <= NV_STATUS_WINTOUCHMAX_X)
				return ngl_wintouchx[status-NV_STATUS_WINTOUCH0_X];

			if(status >= NV_STATUS_WINTOUCH0_Y && status <= NV_STATUS_WINTOUCHMAX_Y)
				return ngl_wintouchy[status-NV_STATUS_WINTOUCH0_Y];

			if(status >= NV_STATUS_WINTOUCH0_PRESSED && status <= NV_STATUS_WINTOUCHMAX_PRESSED)
				return ngl_wintouchpressed[status-NV_STATUS_WINTOUCH0_PRESSED];

			return 0;
	}
}

/*
	Функция	: nglSetStatusi

	Описание: Устанавливает параметры, связанные с графическим движком

	История	: 09.07.12	Создан

*/
N_API void N_APIENTRY_EXPORT nglSetStatusi(int status, int param)
{
	switch(status) {
		case NV_STATUS_WINMODE:
			if(!ngl_isinit)
				ngl_winmode = param;
			else
				nglSetScreen(ngl_winx, ngl_winy, ngl_winbpp, param, ngl_win_vsync);
			return;
		case NV_STATUS_WINBPP:
			if(!ngl_isinit)
				ngl_winbpp = param;
			else
				nglSetScreen(ngl_winx, ngl_winy, param, ngl_winmode, ngl_win_vsync);
			return;
		case NV_STATUS_WINX:
			if(!ngl_isinit)
				ngl_winx = param;
			else
				nglSetScreen(param, ngl_winy, ngl_winbpp, ngl_winmode, ngl_win_vsync);
			return;
		case NV_STATUS_WINY:
			if(!ngl_isinit)
				ngl_winy = param;
			else
				nglSetScreen(ngl_winx, param, ngl_winbpp, ngl_winmode, ngl_win_vsync);
			return;
		case NV_STATUS_WINVSYNC:
			nglChangeVSYNC(param);
			return;
		case NV_STATUS_WINCLIPPINGREGION:
			if(ngl_winclippingregion != param) {
				ngl_winclippingregion = param;
				nglUpdateClippingRegion();
			}
			return;
		case NV_STATUS_WINCLIPPINGREGIONSX:
			if(ngl_winclippingregion_sx != param)
				nglSetClippingRegion(param, ngl_winclippingregion_sy, ngl_winclippingregion_ex, ngl_winclippingregion_ey);
			return;
		case NV_STATUS_WINCLIPPINGREGIONSY:
			if(ngl_winclippingregion_sy != param)
				nglSetClippingRegion(ngl_winclippingregion_sx, param, ngl_winclippingregion_ex, ngl_winclippingregion_ey);
			return;
		case NV_STATUS_WINCLIPPINGREGIONEX:
			if(ngl_winclippingregion_ex != param)
				nglSetClippingRegion(ngl_winclippingregion_sx, ngl_winclippingregion_sy, param, ngl_winclippingregion_ey);
			return;
		case NV_STATUS_WINCLIPPINGREGIONEY:
			if(ngl_winclippingregion_ey != param)
				nglSetClippingRegion(ngl_winclippingregion_sx, ngl_winclippingregion_sy, ngl_winclippingregion_ex, param);
			return;
		case NV_STATUS_WINUPDATEINTERVAL:
			nglChangeUpdateInterval(param);
			return;
		default:
			return;
	}
}

/*
	Функция	: nglGetStatusf

	Описание: Возвращает параметры, связанные с графическим движком

	История	: 23.08.12	Создан

*/
N_API float N_APIENTRY_EXPORT nglGetStatusf(int status)
{
	switch(status) {
		case NV_STATUS_WINBCRED:
			return ngl_winbcred;
		case NV_STATUS_WINBCGREEN:
			return ngl_winbcgreen;
		case NV_STATUS_WINBCBLUE:
			return ngl_winbcblue;
		default:
			return 0;
	}
}

/*
	Функция	: nglSetStatusf

	Описание: Устанавливает параметры, связанные с графическим движком

	История	: 23.08.12	Создан

*/
N_API void N_APIENTRY_EXPORT nglSetStatusf(int status, float param)
{
	switch(status) {
		case NV_STATUS_WINBCRED:
			ngl_winbcred = param;
			ngl_winbcchanged = true;
			return;
		case NV_STATUS_WINBCGREEN:
			ngl_winbcgreen = param;
			ngl_winbcchanged = true;
			return;
		case NV_STATUS_WINBCBLUE:
			ngl_winbcblue = param;
			ngl_winbcchanged = true;
			return;
		default:
			return;
	}
}

/*
	Функция	: nglGetStatusw

	Описание: Возвращает параметры, связанные с графическим движком

	История	: 23.08.12	Создан

*/
N_API const wchar_t * N_APIENTRY_EXPORT nglGetStatusw(int status)
{
	switch(status) {
		case NV_STATUS_WINTITLE:
			return ngl_wintitle;
		case NV_STATUS_WINTEXTINPUTBUF:
			return ngl_wintextinputbuf;
		default:
			return 0;
	}
}

/*
	Функция	: nglSetStatusw

	Описание: Устанавливает параметры, связанные с графическим движком

	История	: 23.08.12	Создан

*/
N_API void N_APIENTRY_EXPORT nglSetStatusw(int status, const wchar_t *param)
{
	switch(status) {
		case NV_STATUS_WINTITLE:
			if(ngl_isinit)
				nglChangeWindowTitle(param);
			else
				ngl_wintitle = param;
			return;
		default:
			return;
	}
}

/*
	Функция	: nglGetKey

	Описание: Возвращает состояние кнопки

	История	: 01.06.12	Создан
		  19.07.12	Изменена. Теперь передаётся не указатель на массив кнопок,
				а состояние отдельной кнопки.

*/
N_API char N_APIENTRY_EXPORT nglGetKey(unsigned char keyid)
{
	return ngl_winkeys[keyid];
}

/*
	Функция	: nglSetScreen

	Описание: Устанавливает параметры экрана

	История	: 01.06.12	Создан

*/
N_API void N_APIENTRY_EXPORT nglSetScreen(unsigned int winx, unsigned int winy, unsigned int winbpp, int winmode, int vsync)
{
	if(ngl_isinit) {
		if(ngl_win_vsync != vsync)
			nglChangeVSYNC(vsync);

		nglChangeWindowMode(winx, winy, winmode);
	} else {
		ngl_winx = winx; ngl_winy = winy; ngl_winbpp = winbpp; ngl_winmode = winmode; ngl_win_vsync = vsync;
	}
}

/*
	Функция	: nglSetClippingRegion

	Описание: Устанавливает область отрисовки

	История	: 21.10.16	Создан

*/
N_API void N_APIENTRY_EXPORT nglSetClippingRegion(int sx, int sy, int ex, int ey)
{
	ngl_winclippingregion_sx = sx;
	ngl_winclippingregion_sy = sy;
	ngl_winclippingregion_ex = ex;
	ngl_winclippingregion_ey = ey;

	nglUpdateClippingRegion();
}

/*
	Функция	: nglUpdateClippingRegion

	Описание: Обновляет область отрисовки область отрисовки

	История	: 21.10.16	Создан

*/
static void nglUpdateClippingRegion(void)
{
	if(ngl_winclippingregion) {
		if(ngl_winclippingregion_sx < 0) ngl_winclippingregion_sx = 0;
		if(ngl_winclippingregion_sx >(int)ngl_winx) ngl_winclippingregion_sx = (int)ngl_winx;

		if(ngl_winclippingregion_sy < 0) ngl_winclippingregion_sy = 0;
		if(ngl_winclippingregion_sy >(int)ngl_winy) ngl_winclippingregion_sy = (int)ngl_winy;

		if(ngl_winclippingregion_ex < ngl_winclippingregion_sx) ngl_winclippingregion_ex = ngl_winclippingregion_sx;
		if(ngl_winclippingregion_ex >(int)ngl_winx) ngl_winclippingregion_ex = (int)ngl_winx;

		if(ngl_winclippingregion_ey < ngl_winclippingregion_sy) ngl_winclippingregion_ey = ngl_winclippingregion_sy;
		if(ngl_winclippingregion_ey >(int)ngl_winy) ngl_winclippingregion_ey = (int)ngl_winy;

		if(ngl_isinit) {
			if(ngl_batch_state == NGL_BATCH_STATE_2D)
				nglBatch2dDraw();

			// Код обновления региона отрисовки графического API
		}
	} else {
		ngl_winclippingregion_sx = 0;
		ngl_winclippingregion_sy = 0;
		ngl_winclippingregion_ex = (int)ngl_winx;
		ngl_winclippingregion_ey = (int)ngl_winy;

		if(ngl_isinit) {
			if(ngl_batch_state == NGL_BATCH_STATE_2D)
				nglBatch2dDraw();

			// Код обновления региона отрисовки графического API
		}
	}
}

#if defined(N_CAUSEWAY) && !defined(N_NODYLIB)
int main(int reason)
{
	int result;

	if (!reason) {

		/*
		** DLL initialisation.
		*/
		wprintf(L"DLL startup...\n");

		/* return zero to let the load continue */
		result=0;

	} else {

		/*
		** DLL clean up.
		*/
		wprintf(L"DLL shutdown...\n");

		result = 1;
	}

	return(result);
}
#endif
