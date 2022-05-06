/*
	Файл	: win_ngl_init.c

	Описание: Инициализация графической библиотеки.

	История	: 05.10.12	Создан

*/

#include <stdio.h>
#include <stdbool.h>

#ifndef UNICODE
	#define UNICODE
#endif
#include <windows.h>

#include "ngl.h"
#include "ngl_text.h"

#include "softgl/softgl.h"

#include "../nyan/nyan_vismodes_publicapi.h"

#include "ngl_init.h"
#include "ngl_batch.h"
#include "win_ngl_init.h"

bool ngl_win_swapc_ext = false; // Поддержка вертикальной синхронизации

static unsigned char *ngl_outputbuf;

static LRESULT (__stdcall WndProc)(HWND w_hWnd, UINT message, WPARAM wParam, LPARAM lParam);

/*
	Функция	: nglInitWindow

	Описание: Создаёт окно

	История	: 5.10.12	Создан

*/
bool nglInitWindow(void)
{
	int window_pos_x = 0, window_pos_y = 0;
	DWORD window_style, window_exstyle; // Стили окна
	RECT window_rect; // Прямоугольник, описывающий окно

	nglSetDpiAware(window_pos_x, window_pos_y);
	
	if(ngl_winmode == NV_MODE_FULLSCREEN) {
		DEVMODEW dm;

		memset(&dm, 0, sizeof(DEVMODEW));
		dm.dmSize = sizeof(DEVMODEW);
		dm.dmPelsWidth = ngl_winx;
		dm.dmPelsHeight = ngl_winy;
		dm.dmBitsPerPel = ngl_winbpp;
		dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL;

		if(ChangeDisplaySettingsW(&dm, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL) {
			dm.dmBitsPerPel = 32;
			if(ChangeDisplaySettingsW(&dm, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL) {
				ngl_ea->nlPrint(LOG_FDEBUGFORMAT, F_NGLINITWINDOW, ERR_FAILEDTOSETFSMODE);
				ngl_winmode = NV_MODE_WINDOWED_FIXED;
				window_pos_x = window_pos_y = CW_USEDEFAULT;
			} else { // В полноэкранном режиме не учитываем DPI
				ngl_windpix = 100;
				ngl_windpiy = 100;
				ngl_winbpp = 32;
			}
		} else { // В полноэкранном режиме не учитываем DPI
			ngl_windpix = 100;
			ngl_windpiy = 100;
		}
	} else
		window_pos_x = window_pos_y = CW_USEDEFAULT;

	nglGetWindowFlags(ngl_winmode, &window_style, &window_exstyle);

	nglAdjustWindowRect(&window_rect, ngl_winx, ngl_winy, ngl_windpix, ngl_windpiy, ngl_winmode);

	if(!nglCreateWindow((wchar_t *)ngl_wintitle, window_style, window_exstyle, L"NGLWindowSoftwareGL", WndProc, window_pos_x, window_pos_y, window_rect.right-window_rect.left, window_rect.bottom-window_rect.top)) {
		ngl_ea->nlPrint(LOG_FDEBUGFORMAT, F_NGLINITWINDOW, ERR_FAILEDTOCREATEWINDOW);
		goto error;
	}
	
	// Если окно не полноэкранное, проверяем, имеет ли оно то же dpi,
	// что и в точке (window_pos_x, window_pos_y),
	// т.к. мы не можем получить dpi окна до его создания.
	if(ngl_winmode != NV_MODE_FULLSCREEN) {
		int newdpix, newdpiy;
		
		nglGetDpiForWindow(&newdpix, &newdpiy, ngl_win_hWnd, window_pos_x, window_pos_y);
		
		// Если dpi изменилось, то растягиваем окно
		if(ngl_windpix != newdpix || ngl_windpiy != newdpiy) {
			ngl_windpix = newdpix;
			ngl_windpiy = newdpiy;
			
			nglAdjustWindowRect(&window_rect, ngl_winx, ngl_winy, ngl_windpix, ngl_windpiy, ngl_winmode);
			SetWindowPos(ngl_win_hWnd, HWND_TOP, 0, 0, window_rect.right-window_rect.left, window_rect.bottom-window_rect.top, SWP_NOMOVE | SWP_NOZORDER);
		}
	}

	ShowWindow(ngl_win_hWnd, SW_SHOWDEFAULT); // Показать окно
	SetForegroundWindow(ngl_win_hWnd); // Вынести окно на передний план

	// Поддержка вертикальной синхронизации
	ngl_win_swapc_ext = false;
	
	// Установка таймера, который будет использоваться для обновления окна
	nglInitUpdateInterval();

	// Инициализация тачскрина
	nglInitTouchInterface();

	ngl_outputbuf = ngl_ea->nAllocMemory((ngl_windpix*ngl_winx/100)*(ngl_windpiy*ngl_winy/100)*4);

	return true;
	
error: // Аварийный выход в процессе создания окна и контекста OpenGL	
	if(ngl_win_hDC || ngl_win_hWnd)
		nglDestroyWindow(L"NGLWindowSoftwareGL");
	
	if(ngl_winmode == NV_MODE_FULLSCREEN)
			ChangeDisplaySettings(NULL, 0); // Возвращение на рабочий стол
	
	return false;
}

/*
	Функция	: nglCloseWindow

	Описание: Уничтожает окно

	История	: 5.10.12	Создан

*/
bool nglCloseWindow(void)
{
	nglCloseTouchInterface();

	if(ngl_winmode == NV_MODE_FULLSCREEN)
		ChangeDisplaySettings(NULL, 0); // Возвращение на рабочий стол

	if(ngl_win_hDC || ngl_win_hWnd)
		nglDestroyWindow(L"NGLWindowSoftwareGL");

	nglCloseUpdateTimer();

	ngl_ea->nFreeMemory(ngl_outputbuf);
	ngl_outputbuf = 0;

	return true;
}

/*
	Функция	: nglUpdateWindow

	Описание: Обновление окна

	История	: 5.10.12	Создан

*/
void nglUpdateWindow(void)
{
	unsigned int sx, sy, scolortype; unsigned char *surface;

	if(sglGetSurface(&sx, &sy, &scolortype, &surface)) {
		BITMAPINFO bmi;
		unsigned int i;

		for(i = 0; i < sx*sy; i++) {
			ngl_outputbuf[i*4] = surface[i*4+2];
			ngl_outputbuf[i*4+1] = surface[i*4+1];
			ngl_outputbuf[i*4+2] = surface[i*4];
			ngl_outputbuf[i*4+3] = surface[i*4+3];
		}

		// Отрисовка изображения. Комментарии взяты из https://habrahabr.ru/post/165279/
		memset(&(bmi.bmiHeader), 0, sizeof(BITMAPINFOHEADER));
		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);// размер структуры
		bmi.bmiHeader.biWidth = sx;// ширина картинки
		bmi.bmiHeader.biHeight = -(int)sy;// высота картинки, минус нужен чтобы изображение не было перевернутым
		bmi.bmiHeader.biPlanes = 1;// количество слоев - всегда 1
		bmi.bmiHeader.biBitCount = 32;// кол-во бит на пиксель
		bmi.bmiHeader.biCompression = BI_RGB;// формат
		bmi.bmiHeader.biSizeImage = sx*sy*4;// размер картинки

		StretchDIBits(ngl_win_hDC,
					  0, 0, ngl_windpix*ngl_winx/100, ngl_windpiy*ngl_winy/100, // прямоугольник куда выводить
					  0, 0, sx, sy, // прямоугольник откуда выводить
					  ngl_outputbuf, // указатель на массив пикселей
					  &bmi, // параметры
					  DIB_RGB_COLORS, // формат вывода
					  SRCCOPY); // режим вывода

		//glRasterPos2i(0, 0);
		//glPixelZoom(1, -1);  
		//glDrawPixels(sx, sy, GL_RGBA, GL_UNSIGNED_BYTE, surface);
		//unsigned int i;
		//for(i = 0; i < sy; i++) {
		//	memcpy(ngl_outputbuf+(sy-i-1)*4*sx, surface+i*4*sx, sx*4);
		//}
		//glDrawPixels(sx, sy, GL_RGBA, GL_UNSIGNED_BYTE, ngl_outputbuf);
	}

	if(ngl_win_updateinterval) {
		if(WaitForSingleObject(ngl_win_hUpdateTimer, INFINITE) == WAIT_OBJECT_0) {
			if(ngl_win_timerneedcancel == true) {
				ngl_win_timerneedcancel = false;
				
				ngl_win_updateinterval = 0;

				CancelWaitableTimer(ngl_win_hUpdateTimer);
			}
		}
	}

	nglProcessWindowMessages();
}

/*
	Функция	: nglChangeVSYNC

	Описание: Меняет вертикальную синхронизацию

	История	: 5.10.12	Создан

*/
void nglChangeVSYNC(int vsync)
{
	(void)vsync; // Неиспользуемая переменная

	ngl_ea->nlPrint(LOG_FDEBUGFORMAT, F_NGLSETSTATUSI, ERR_CANTCHANGEVSYNC);
	ngl_win_vsync = 0;
}

/*
	Функция	: WndProc

	Описание: Обработка событий окна

	История	: 01.06.12	Создан

*/
static LRESULT (__stdcall WndProc)(HWND w_hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	bool need_DefWindowProc = false;
	LRESULT result = 0;

	need_DefWindowProc = nglProcessWindow(w_hWnd, message, wParam, lParam, &result);

	switch(message) {
		case WM_SIZE:
			if(ngl_isinit) {
				ngl_ea->nFreeMemory(ngl_outputbuf);
				ngl_outputbuf = ngl_ea->nAllocMemory((ngl_windpix*ngl_winx/100)*(ngl_windpiy*ngl_winy/100)*4);

				sglDestroySurface();
				sglCreateSurface(ngl_windpix*ngl_winx/100, ngl_windpiy*ngl_winy/100, SGL_COLORFORMAT_R8G8B8A8);
				sglClearSurface(ngl_winbcred, ngl_winbcgreen, ngl_winbcblue, 1.0);
			}
			
			break;
	}

	if(need_DefWindowProc)
		return DefWindowProc(w_hWnd, message, wParam, lParam);
	else
		return result;
}
