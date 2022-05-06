/*
	Файл	: win_ngl_init.c

	Описание: Инициализация графической библиотеки. Платформа windows

	История	: 05.08.12	Создан

*/

#define WINVER 0x0A00
#define _WIN32_WINNT 0x0A00
#define NTDDI_VERSION 0x06030000

#include <stdio.h>
#include <stdbool.h>

#include "../extclib/mbstowcsl.h"

#ifndef UNICODE
	#define UNICODE
#endif
#include <windows.h>
#include <GL/glu.h>
#include "../forks/gl/wglext.h"

#include "ngl.h"
#include "ngl_text.h"

#include "../nyan/nyan_vismodes_publicapi.h"

#include "ngl_init.h"
#include "ngl_batch.h"
#include "win_ngl_init.h"

// WGL_EXT_swap_control
static PFNWGLSWAPINTERVALEXTPROC funcptr_wglSwapIntervalEXT;

// WGL_ARB_extensions_string
static PFNWGLGETEXTENSIONSSTRINGARBPROC funcptr_wglGetExtensionsStringARB;

static HGLRC ngl_win_hRC = NULL; // OpenGL Rendering Context

static HGLRC *ngl_win_separate_hRCs = NULL; // Separate Thread Rendering Contexts
static unsigned int ngl_win_nof_separate_hRCs = 0; // Количество отдельных контекстов отрисовки
static unsigned int ngl_win_nof_used_separate_hRCs = 0; // Количество отдельных __используемых__ контекстов отрисовки
static uintptr_t ngl_win_separate_hRCs_synch_mutex = 0;

static bool ngl_win_getextstrarb_ext = false; // WGL_ARB_extensions_string
bool ngl_win_swapc_ext = false; // WGL_EXT_swap_control

static LRESULT (__stdcall WndProc)(HWND w_hWnd, UINT message, WPARAM wParam, LPARAM lParam);

/*
	Функция : nglIsMainRContext

	Описание: Возвращает true, если текущий контекст - главный (контекст рендеринга).

	История	: 14.11.17	Создан

*/
bool nglIsMainRContext(void)
{
	if(wglGetCurrentContext() == ngl_win_hRC)
		return true;
	else
		return false;
}

/*
	Функция : nglMakeCurrentRContextNull

	Описание: Сбрасывает текущий используемый контекст.

	История	: 07.12.17	Создан

*/
void N_APIENTRY nglMakeCurrentRContextNull(void *param)
{
	(void)param; // Unused

	if(wglGetCurrentContext())
		wglMakeCurrent(NULL, NULL);
}

/*
	Функция : nglAddSeparateRContextIfNeeded

	Описание: Если для потока не выбран контекст, создаёт отдельный констекст OpenGL, добавляет его в массив.
		В данной реализации контексты уже созданы и добавлены, нужно только увеличить счётчик ngl_win_nof_used_separate_hRCs.

	История	: 14.11.17	Создан

*/
bool nglAddSeparateRContextIfNeeded(void)
{
	if(wglGetCurrentContext() != NULL)
		return true;

	ngl_ea->nLockMutex(ngl_win_separate_hRCs_synch_mutex);
		if(ngl_win_nof_used_separate_hRCs < ngl_win_nof_separate_hRCs) {
			wglMakeCurrent(ngl_win_hDC, ngl_win_separate_hRCs[ngl_win_nof_used_separate_hRCs]);
			ngl_win_nof_used_separate_hRCs++;
		} else {
			ngl_ea->nUnlockMutex(ngl_win_separate_hRCs_synch_mutex);
			ngl_ea->nlPrint(LOG_FDEBUGFORMAT, F_NGLADDSEPARATERCONTEXTIFNEEDED, ERR_FAILEDTOCREATESEPARATERC);
			return false;
		}
	ngl_ea->nUnlockMutex(ngl_win_separate_hRCs_synch_mutex);

	return true;

}

/*
	Функция	: nglInitWindow

	Описание: Создаёт окно

	История	: 12.08.12	Создан

*/
bool nglInitWindow(void)
{
	wchar_t *tempt = 0;
	int window_pos_x = 0, window_pos_y = 0;
	DWORD window_style, window_exstyle; // Стили окна
	RECT window_rect; // Прямоугольник, описывающий окно
	PIXELFORMATDESCRIPTOR pfd; // Формат пикселей окна, аргумент функции ChoosePixelFormat
	int pixelformat; // id формата, выбранного функцией ChoosePixelFormat

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

	if(!nglCreateWindow((wchar_t *)ngl_wintitle, window_style, window_exstyle, L"NGLWindowOpenGL", WndProc, window_pos_x, window_pos_y, window_rect.right-window_rect.left, window_rect.bottom-window_rect.top)) {
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

#if 0
	ngl_ea->nlPrint(L"===Listing all awailable pixel formats");

	pixelformat = 1;

	while (1) {
		if(DescribePixelFormat(ngl_win_hDC, pixelformat, sizeof(PIXELFORMATDESCRIPTOR), &pfd) == 0)
			break;

		if((pfd.dwFlags & (PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER)) && (pfd.iPixelType == PFD_TYPE_RGBA)) {
			ngl_ea->nlPrint(L"   PixelFormat %d dwFlags %d cColorBits %d cDepthBits %d cStencilBits %d PFD_GENERIC_ACCELERATED %ls PFD_GENERIC_FORMAT %ls PFD_SUPPORT_COMPOSITION %ls",
				pixelformat, pfd.dwFlags, pfd.cColorBits, pfd.cDepthBits, pfd.cStencilBits,
				(pfd.dwFlags & PFD_GENERIC_ACCELERATED)?N_YES:N_NO,
				(pfd.dwFlags & PFD_GENERIC_FORMAT)?N_YES:N_NO,
				(pfd.dwFlags & PFD_SUPPORT_COMPOSITION)?N_YES:N_NO);
		}

		pixelformat++;
	}

	ngl_ea->nlPrint(L"===End of listing");
#endif

	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER/* | PFD_SUPPORT_COMPOSITION*/;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = ngl_winbpp;
	pfd.cDepthBits = 32;
	pfd.cAccumBits = pfd.cStencilBits = 0; // Нет буфера аккумулятора и стенсила
	pfd.iLayerType = PFD_MAIN_PLANE; // msdn на странице PIXELFORMATDESCRIPTOR говорит, что больше не используется. Но в ChoosePixelFormat оно есть

	pixelformat = ChoosePixelFormat(ngl_win_hDC, &pfd);
	if(!pixelformat) {
		ngl_ea->nlPrint(LOG_FDEBUGFORMAT, F_NGLINITWINDOW, ERR_FAILEDTOCHOOSEPIXELFORMAT);
		goto error;
	}

	if(!SetPixelFormat(ngl_win_hDC, pixelformat, &pfd)) {
		ngl_ea->nlPrint(LOG_FDEBUGFORMAT, F_NGLINITWINDOW, ERR_FAILEDTOSETPIXELFORMAT);
		goto error;
	}

	DescribePixelFormat(ngl_win_hDC, pixelformat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
	ngl_winbpp = pfd.cColorBits;
	//ngl_ea->nlPrint(NGL_SCREENINFO, F_NGLINITWINDOW, ngl_winx, ngl_winy, ngl_winbpp, pfd.cDepthBits, (ngl_winmode == NV_MODE_FULLSCREEN)?N_YES:N_NO);
	//ngl_ea->nlPrint(L"cStencilBits %d", pfd.cStencilBits);
	//ngl_ea->nlPrint(L"PFD_GENERIC_ACCELERATED %d", pfd.dwFlags & PFD_GENERIC_ACCELERATED);
	//ngl_ea->nlPrint(L"PFD_GENERIC_FORMAT %d", pfd.dwFlags & PFD_GENERIC_FORMAT);
	//ngl_ea->nlPrint(L"PFD_SUPPORT_COMPOSITION %d", pfd.dwFlags & PFD_SUPPORT_COMPOSITION);
	//ngl_ea->nlPrint(L"PixelFormat %d", pixelformat);

	// Создание основного Rendering Context
	ngl_win_hRC = wglCreateContext(ngl_win_hDC);
	if(!ngl_win_hRC) {
		ngl_ea->nlPrint(LOG_FDEBUGFORMAT, F_NGLINITWINDOW, ERR_FAILEDTOCREATERC);
		goto error;
	}

	// Создания мьютекса для синхронизации добавления отдельных контекстов OpenGL
	ngl_win_separate_hRCs_synch_mutex = ngl_ea->nCreateMutex();
	if(!ngl_win_separate_hRCs_synch_mutex) {
		ngl_ea->nlPrint(LOG_FDEBUGFORMAT, F_NGLINITWINDOW, ERR_FAILEDTOCREATESEPARATERC);
		goto error;
	}

	// Добавление отдельных контекстов OpenGL
	ngl_win_nof_separate_hRCs = ngl_ea->nGetMaxThreadsForTasks();
	if(ngl_win_nof_separate_hRCs) {
		unsigned int i;

		ngl_win_separate_hRCs = ngl_ea->nAllocMemory(ngl_win_nof_separate_hRCs*sizeof(HGLRC));
		if(!ngl_win_separate_hRCs) {
			ngl_ea->nlPrint(LOG_FDEBUGFORMAT, F_NGLINITWINDOW, ERR_FAILEDTOCREATESEPARATERC);
			goto error;
		}

		memset(ngl_win_separate_hRCs, 0, ngl_win_nof_separate_hRCs*sizeof(HGLRC));

		for(i = 0; i < ngl_win_nof_separate_hRCs; i++) {
			ngl_win_separate_hRCs[i] = wglCreateContext(ngl_win_hDC);
			if(!(ngl_win_separate_hRCs[i])){
				ngl_ea->nlPrint(LOG_FDEBUGFORMAT, F_NGLINITWINDOW, ERR_FAILEDTOCREATESEPARATERC);
				goto error;
			}

			wglShareLists(ngl_win_hRC, ngl_win_separate_hRCs[i]);
		}
	}

	// Активируем Rendering Context уже после того, как создаём отдельные Rendering Context для каждого потока
	if(!wglMakeCurrent(ngl_win_hDC, ngl_win_hRC)) {
		ngl_ea->nlPrint(LOG_FDEBUGFORMAT, F_NGLINITWINDOW, ERR_FAILEDTOMAKECURRENTRC);
		goto error;
	}

	ShowWindow(ngl_win_hWnd, SW_SHOWDEFAULT); // Показать окно
	SetForegroundWindow(ngl_win_hWnd); // Вынести окно на передний план

	// Получение списка расширений wgl, если возможно
	funcptr_wglGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)wglGetProcAddress("wglGetExtensionsStringARB");
	if(funcptr_wglGetExtensionsStringARB) {
		ngl_win_getextstrarb_ext = true;
		tempt = ngl_ea->nAllocMemory(sizeof(wchar_t)*(strlen((char*)funcptr_wglGetExtensionsStringARB(ngl_win_hDC))+1));
		if(tempt) {
			mbstowcsl(tempt,(char*)funcptr_wglGetExtensionsStringARB(ngl_win_hDC),strlen((char*)funcptr_wglGetExtensionsStringARB(ngl_win_hDC))+1);
			ngl_ea->nlPrint(LOG_FDEBUGFORMAT2,F_NGLINITWINDOW,L"wglGetExtensionsStringARB",tempt);
			ngl_ea->nFreeMemory(tempt);
		}
	} else
		ngl_win_getextstrarb_ext = false;
	ngl_ea->nlPrint(LOG_FDEBUGFORMAT2,F_NGLINITWINDOW,NGL_SUPPORTWGLGETEXTSTR,ngl_win_getextstrarb_ext?N_YES:N_NO);

	// Поддержка вертикальной синхронизации
	ngl_win_swapc_ext = false;
	if(strstr((char*)glGetString(GL_EXTENSIONS),"WGL_EXT_swap_control"))
		ngl_win_swapc_ext = true;
	if(ngl_win_getextstrarb_ext)
		if(strstr((char*)funcptr_wglGetExtensionsStringARB(ngl_win_hDC),"WGL_EXT_swap_control"))
			ngl_win_swapc_ext = true;
	ngl_ea->nlPrint(LOG_FDEBUGFORMAT2,F_NGLINITWINDOW,NGL_SUPPORTVSYNC,ngl_win_swapc_ext?N_YES:N_NO);

	// Установка вертикальной синхронизации
	if(ngl_win_swapc_ext) {
		funcptr_wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
	    if(!funcptr_wglSwapIntervalEXT(ngl_win_vsync)) {
	        ngl_ea->nlPrint(LOG_FDEBUGFORMAT,F_NGLINITWINDOW,ERR_CANTCHANGEVSYNC); ngl_win_vsync = 0; ngl_win_swapc_ext = false; }
	} else
	    ngl_win_vsync = 0;
	ngl_ea->nlPrint(LOG_FDEBUGFORMAT2,F_NGLINITWINDOW,NGL_VSYNC,ngl_win_vsync?N_YES:N_NO);

	// Установка таймера, который будет использоваться для обновления окна
	nglInitUpdateInterval();

	// Инициализация тачскрина
	nglInitTouchInterface();

	return true;

error: // Аварийный выход в процессе создания окна и контекста OpenGL

	if(ngl_win_separate_hRCs_synch_mutex)
		ngl_ea->nDestroyMutex(ngl_win_separate_hRCs_synch_mutex);

	if(ngl_win_nof_separate_hRCs) {
		unsigned int i;

		for(i = 0; i < ngl_win_nof_separate_hRCs; i++) {
			if(ngl_win_separate_hRCs[i])
				wglDeleteContext(ngl_win_separate_hRCs[i]);
		}

		ngl_ea->nFreeMemory(ngl_win_separate_hRCs);
		ngl_win_separate_hRCs = 0;
	}

	if(ngl_win_hRC)  {
		wglDeleteContext(ngl_win_hRC);
		ngl_win_hRC = NULL;
	}

	if(ngl_win_hDC || ngl_win_hWnd)
		nglDestroyWindow(L"NGLWindowOpenGL");

	if(ngl_winmode == NV_MODE_FULLSCREEN)
			ChangeDisplaySettings(NULL, 0); // Возвращение на рабочий стол

	return false;
}

/*
	Функция	: nglCloseWindow

	Описание: Уничтожает окно

	История	: 12.08.12	Создан

*/
bool nglCloseWindow(void)
{
	unsigned int i;

	nglCloseTouchInterface();

	if(ngl_winmode == NV_MODE_FULLSCREEN)
		ChangeDisplaySettings(NULL, 0); // Возвращение на рабочий стол

	if(!wglMakeCurrent(NULL, NULL))
		ngl_ea->nlPrint(LOG_FDEBUGFORMAT, F_NGLCLOSEWINDOW, ERR_FAILEDTOMAKECURRENTRC);

	if(!wglDeleteContext(ngl_win_hRC)) {
		ngl_ea->nlPrint(LOG_FDEBUGFORMAT, F_NGLCLOSEWINDOW, ERR_FAILEDTODELETERC);
	}

	if(ngl_win_nof_separate_hRCs) {
		for(i = 0; i < ngl_win_nof_separate_hRCs; i++) {
			if(!wglDeleteContext(ngl_win_separate_hRCs[i]))
				ngl_ea->nlPrint(LOG_FDEBUGFORMAT, F_NGLCLOSEWINDOW, ERR_FAILEDTODELETESEPARATERC);
		}

		ngl_win_nof_separate_hRCs = 0;
	}

	ngl_win_nof_used_separate_hRCs = 0;

	if(ngl_win_separate_hRCs) {
		ngl_ea->nFreeMemory(ngl_win_separate_hRCs);
		ngl_win_separate_hRCs = 0;
	}

	ngl_ea->nDestroyMutex(ngl_win_separate_hRCs_synch_mutex);

	if(ngl_win_hDC || ngl_win_hWnd)
		nglDestroyWindow(L"NGLWindowOpenGL");

	nglCloseUpdateTimer();

	return true;
}

/*
	Функция	: nglUpdateWindow

	Описание: Обновление окна

	История	: 12.08.12	Создан

*/
void nglUpdateWindow(void)
{
	if(!wglSwapLayerBuffers(ngl_win_hDC, WGL_SWAP_MAIN_PLANE))
		SwapBuffers(ngl_win_hDC);

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

	История	: 18.07.12	Создан

*/
void nglChangeVSYNC(int vsync)
{
	if(ngl_isinit) {
		if(ngl_win_swapc_ext) {
			if(vsync == ngl_win_vsync)
				return;

			if(!funcptr_wglSwapIntervalEXT(vsync)) {
				ngl_ea->nlPrint(LOG_FDEBUGFORMAT,F_NGLSETSTATUSI,ERR_CANTCHANGEVSYNC);
				ngl_win_vsync = 0;
			} else
				ngl_win_vsync = vsync;
		}
	} else
		ngl_win_vsync = vsync;
}

/*
	Функция	: nglGetProcAddress

	Описание: Возращает процедуру по имени

	История	: 14.08.12	Создан

*/
ngl_func_type nglGetProcAddress(const char * name)
{
    return (ngl_func_type)wglGetProcAddress(name);
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
				WORD /*real_x,*/ real_y;
				GLsizei virtua_x, virtua_y;
				GLint start_y;

				//real_x = LOWORD(lParam);
				real_y = HIWORD(lParam);

				virtua_x = ngl_windpix*ngl_winx/100;
				virtua_y = ngl_windpiy*ngl_winy/100;

				if(virtua_y < real_y)
					start_y = real_y-virtua_y;
				else
					start_y = 0;

				//wprintf(L"real %d %d win %d %d start_y %d\n", (int)real_x, (int)real_y, (int)virtua_x, (int)virtua_y, (int)start_y);

				glViewport(0, start_y, virtua_x, virtua_y);
				if(ngl_batch_state == NGL_BATCH_STATE_3D) {
					glMatrixMode(GL_PROJECTION);
					glLoadIdentity();
					gluPerspective(45.0, (ngl_winx != 0 && ngl_winy != 0)?((GLdouble)ngl_winx/(GLdouble)ngl_winy):(1.0), 2.0, 4000.0);
					glMatrixMode(GL_MODELVIEW);
					glLoadIdentity();
				} else if(ngl_batch_state == NGL_BATCH_STATE_2D) {
					glMatrixMode(GL_PROJECTION);
					glLoadIdentity();
					glOrtho(0.0,(ngl_winx)?ngl_winx:1,(ngl_winy)?ngl_winy:1,0.0,-1.0,1.0);
					glMatrixMode(GL_MODELVIEW);
					glLoadIdentity();
				}

				nglCatchOpenGLError(0);
			}

			break;
	}

	if(need_DefWindowProc)
		return DefWindowProc(w_hWnd, message, wParam, lParam);
	else
		return result;
}

