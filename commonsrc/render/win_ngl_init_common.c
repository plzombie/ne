/*
	Файл	: win_ngl_init_common.c

	Описание: Инициализация графической библиотеки. Платформа windows. Общие между рендерами функции.

	История	: 26.07.17	Создан

*/

#define WINVER 0x0A00
#define _WIN32_WINNT 0x0A00

#include <stdbool.h>
#include <Windows.h>

#include "win_ngl_init_common.h"

#include "../../nyan/nyan_engapi.h"
#include "../../nyan/nyan_text.h"
#include "../../nyan/nyan_vismodes_publicapi.h"
#include "ngl_text_common.h"

extern engapi_type *ngl_ea;
extern unsigned int ngl_win_updateinterval;
extern bool ngl_isinit;

extern unsigned int ngl_winx, ngl_winy;
extern int ngl_winmbl, ngl_winmbr, ngl_winmbm, ngl_winmx, ngl_winmy, ngl_winmwheel;
extern int ngl_winmode;
extern int ngl_win_exitmsg;
extern char ngl_winkeys[256];

extern bool ngl_win_havetouch;
extern int ngl_wintouchcount, ngl_wintouchmaxcount;
extern int ngl_wintouchx[NV_STATUS_WINTOUCHMAX_X-NV_STATUS_WINTOUCH0_X+1];
extern int ngl_wintouchy[NV_STATUS_WINTOUCHMAX_Y-NV_STATUS_WINTOUCH0_Y+1];
extern int ngl_wintouchpressed[NV_STATUS_WINTOUCHMAX_PRESSED-NV_STATUS_WINTOUCH0_PRESSED+1];

extern const wchar_t *ngl_wintitle;
extern wchar_t ngl_wintextinputbuf[256];
extern int ngl_wintextinputbufsize;

extern int ngl_windpix;
extern int ngl_windpiy;

HDC ngl_win_hDC = NULL; // Device Context
HWND ngl_win_hWnd = NULL; // Window handle
static HINSTANCE ngl_win_hInstance = NULL; // Instance приложения

static bool ngl_win8touchsupport = false; // Поддержка сообщений WM_POINTER*, появившихся начиная с восьмёрки
static DWORD ngl_wintouchlookuptable[NV_STATUS_WINTOUCHMAX_X-NV_STATUS_WINTOUCH0_X+1] = {0};

HANDLE ngl_win_hUpdateTimer = NULL;
bool ngl_win_timerneedcancel = false;

// Функции для тача из WinAPI
#ifndef SM_DIGITIZER
#define SM_DIGITIZER 94
#endif // !SM_DIGITIZER

#ifndef NID_READY
#define NID_READY 0x00000080
#endif

#ifndef NID_MULTI_INPUT
#define NID_MULTI_INPUT 0x00000040
#endif

#ifndef WM_TOUCH
#define WM_TOUCH 0x0240
#endif

#ifndef TOUCH_COORD_TO_PIXEL
#define TOUCH_COORD_TO_PIXEL(l) ((l)/100)
#endif

typedef BOOL (WINAPI *RegisterTouchWindow_type)(HWND hWnd, ULONG ulFlags);
typedef BOOL (WINAPI *UnregisterTouchWindow_type)(HWND hWnd);
typedef BOOL (WINAPI *CloseTouchInputHandle_type)(HTOUCHINPUT hTouchInput);
typedef BOOL (WINAPI *GetTouchInputInfo_type)(HTOUCHINPUT hTouchInput, UINT cInputs, PTOUCHINPUT pInputs, int cbSize);

RegisterTouchWindow_type funcptr_RegisterTouchWindow = 0;
UnregisterTouchWindow_type funcptr_UnregisterTouchWindow = 0;
CloseTouchInputHandle_type funcptr_CloseTouchInputHandle = 0;
GetTouchInputInfo_type funcptr_GetTouchInputInfo = 0;

// DPI в Windows 8+
#ifndef WM_DPICHANGED
#define WM_DPICHANGED 0x02E0
#endif

#ifndef WM_GETDPISCALEDSIZE
#define WM_GETDPISCALEDSIZE 0x02E4
#endif

#ifndef DPI_ENUMS_DECLARED
#define DPI_ENUMS_DECLARED

typedef enum _PROCESS_DPI_AWARENESS { 
	PROCESS_DPI_UNAWARE = 0,
	PROCESS_SYSTEM_DPI_AWARE = 1,
	PROCESS_PER_MONITOR_DPI_AWARE = 2
} PROCESS_DPI_AWARENESS;

typedef enum _MONITOR_DPI_TYPE { 
	MDT_EFFECTIVE_DPI = 0,
	MDT_ANGULAR_DPI = 1,
	MDT_RAW_DPI = 2,
	MDT_DEFAULT = MDT_EFFECTIVE_DPI
} MONITOR_DPI_TYPE;
#endif

#ifndef _DPI_AWARENESS_CONTEXTS_
#define _DPI_AWARENESS_CONTEXTS_

DECLARE_HANDLE(DPI_AWARENESS_CONTEXT);

#define DPI_AWARENESS_CONTEXT_UNAWARE              ((DPI_AWARENESS_CONTEXT)-1)
#define DPI_AWARENESS_CONTEXT_SYSTEM_AWARE         ((DPI_AWARENESS_CONTEXT)-2)
#define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE    ((DPI_AWARENESS_CONTEXT)-3)
#define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 ((DPI_AWARENESS_CONTEXT)-4)
#endif

// Тач в Windows 8+
#ifndef WM_POINTERUPDATE
#define WM_POINTERUPDATE 0x0245
#define WM_POINTERDOWN 0x0246
#define WM_POINTERUP 0x0247
#define WM_POINTERENTER 0x0249
#define WM_POINTERLEAVE 0x024A
#define WM_POINTERCAPTURECHANGED 0x024C
#define GET_POINTERID_WPARAM(wParam) (LOWORD(wParam))
#endif

// Shcore.dll
typedef HRESULT (WINAPI *SetProcessDpiAwareness_type)(PROCESS_DPI_AWARENESS value);
typedef HRESULT (WINAPI *GetDpiForMonitor_type)(HMONITOR hmonitor, MONITOR_DPI_TYPE dpiType, UINT *dpiX, UINT *dpiY);

SetProcessDpiAwareness_type funcptr_SetProcessDpiAwareness = 0;
GetDpiForMonitor_type funcptr_GetDpiForMonitor = 0;

// User32.dll
typedef BOOL (WINAPI *SetProcessDPIAware_type)(void);
typedef HMONITOR(WINAPI *MonitorFromWindow_type)(HWND hwnd, DWORD dwFlags);
typedef HMONITOR (WINAPI *MonitorFromPoint_type)(POINT pt, DWORD dwFlags);
typedef BOOL (WINAPI *EnableNonClientDpiScaling_type)(HWND hwnd);
typedef BOOL (WINAPI *AdjustWindowRectExForDpi_type)(LPRECT lpRect, DWORD dwStyle, BOOL bMenu, DWORD dwExStyle, UINT dpi);
typedef BOOL (WINAPI *IsMouseInPointerEnabled_type)(void);
typedef DPI_AWARENESS_CONTEXT (WINAPI *SetProcessDpiAwarenessContext_type)(DPI_AWARENESS_CONTEXT dpiContext);
typedef UINT (WINAPI *GetDpiForWindow_type)(HWND hwnd);

SetProcessDPIAware_type funcptr_SetProcessDPIAware = 0;
MonitorFromPoint_type funcptr_MonitorFromPoint = 0;
MonitorFromWindow_type funcptr_MonitorFromWindow = 0;
EnableNonClientDpiScaling_type funcptr_EnableNonClientDpiScaling = 0;
AdjustWindowRectExForDpi_type funcptr_AdjustWindowRectExForDpi = 0;
IsMouseInPointerEnabled_type funcptr_IsMouseInPointerEnabled = 0;
SetProcessDpiAwarenessContext_type funcptr_SetProcessDpiAwarenessContext = 0;
GetDpiForWindow_type funcptr_GetDpiForWindow = 0;

bool ngl_need_enablenonclientdpiscaling = true;

/*
	Функция	: nglSetProcessDPIAware

	Описание: Делает окно Dpi Aware

	История	: 01.08.17	Создан

*/
bool nglMarkProcessDPIAware(void)
{
	ngl_need_enablenonclientdpiscaling = true;
	
	if(funcptr_SetProcessDpiAwarenessContext) {
		if(funcptr_SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)) {
			ngl_need_enablenonclientdpiscaling = false;
			return true;
		}
	}

	if(funcptr_SetProcessDpiAwareness) {
		if(funcptr_SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE) == S_OK)
			return true;
		else
			return false;
	}

	if(funcptr_SetProcessDPIAware) {
		if(funcptr_SetProcessDPIAware())
			return true;
		else
			return false;
	}

	return true;
}

/*
	Функция	: nglGetDpiForWindow

	Описание: Возвращает dpi для окна.
		Если hwnd == NULL, то возвращает dpi для монитора в точке

	История	: 31.07.17	Создан
		11.03.18	Выделен из nglSetDpiAware
		12.03.18	Переименован из nglGetDpiForMonitor в nglGetDpiForWindow

*/
void nglGetDpiForWindow(int *windpix, int *windpiy, HWND hwnd, int fallback_x, int fallback_y)
{
	if (funcptr_GetDpiForWindow && hwnd) {
		UINT dpiX;
		
		//wprintf(L"path0\n");
		dpiX = funcptr_GetDpiForWindow(hwnd);

		*windpix = 100 * dpiX / 96;
		*windpiy = 100 * dpiX / 96;
	} else if(funcptr_GetDpiForMonitor && funcptr_MonitorFromPoint && funcptr_MonitorFromWindow) {
		HMONITOR hMonitor;
		POINT pt;
		UINT dpiX, dpiY;

		if(hwnd) {
			//wprintf(L"path1\n");
			hMonitor = funcptr_MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
		} else {
			//wprintf(L"path2\n");
			pt.x = fallback_x;
			pt.y = fallback_y;

			hMonitor = funcptr_MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
		}

		if(funcptr_GetDpiForMonitor(hMonitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY) != S_OK) {
			return;
		}

		*windpix = 100*dpiX/96;
		*windpiy = 100*dpiY/96;
	} else {
		HDC hdc = GetDC(NULL);

		if(hdc) {
			*windpix = 100*GetDeviceCaps(hdc, LOGPIXELSX)/96;
			*windpiy = 100*GetDeviceCaps(hdc, LOGPIXELSY)/96;

			if(!(*windpix))
				*windpix = 100;

			if(!(*windpiy))
				*windpiy = 100;

			ReleaseDC(NULL, hdc);
		}
	}
}

/*
	Функция	: nglSetDpiAware

	Описание: Делает окно Dpi Aware, получает начальные параметры

	История	: 31.07.17	Создан

*/
void nglSetDpiAware(int window_pos_x, int window_pos_y)
{
	HMODULE hmodule_shcore, hmodule_user32;	

	ngl_windpix = 100;
	ngl_windpiy = 100;

	hmodule_shcore = LoadLibraryW(L"Shcore.dll");

	if(hmodule_shcore) {
		funcptr_SetProcessDpiAwareness = (SetProcessDpiAwareness_type)GetProcAddress(hmodule_shcore, "SetProcessDpiAwareness");
		funcptr_GetDpiForMonitor = (GetDpiForMonitor_type)GetProcAddress(hmodule_shcore, "GetDpiForMonitor");

		if(funcptr_SetProcessDpiAwareness == NULL && funcptr_GetDpiForMonitor == NULL) {
			FreeLibrary(hmodule_shcore);
		}
	}

	hmodule_user32 = GetModuleHandleW(L"User32.dll");

	if(hmodule_user32) {
		funcptr_SetProcessDPIAware = (SetProcessDPIAware_type)GetProcAddress(hmodule_user32, "SetProcessDPIAware");
		funcptr_MonitorFromPoint = (MonitorFromPoint_type)GetProcAddress(hmodule_user32, "MonitorFromPoint");
		funcptr_MonitorFromWindow = (MonitorFromWindow_type)GetProcAddress(hmodule_user32, "MonitorFromWindow");
		funcptr_EnableNonClientDpiScaling = (EnableNonClientDpiScaling_type)GetProcAddress(hmodule_user32, "EnableNonClientDpiScaling");
		funcptr_AdjustWindowRectExForDpi = (AdjustWindowRectExForDpi_type)GetProcAddress(hmodule_user32, "AdjustWindowRectExForDpi");
		funcptr_SetProcessDpiAwarenessContext = (SetProcessDpiAwarenessContext_type)GetProcAddress(hmodule_user32, "SetProcessDpiAwarenessContext");
		funcptr_GetDpiForWindow = (GetDpiForWindow_type)GetProcAddress(hmodule_user32, "GetDpiForWindow");
	}

	if(nglMarkProcessDPIAware()) {
		nglGetDpiForWindow(&ngl_windpix, &ngl_windpiy, NULL, window_pos_x, window_pos_y);
	} else {
		ngl_windpix = ngl_windpiy = 100;
		//wprintf(L"Can\'t set Dpi awareness");
	}

	//wprintf(L"Dpi scale is %d %d\n", ngl_windpix, ngl_windpiy);
}

/*
	Функция	: nglInitUpdateInterval

	Описание: Начальная установка таймера, который будет использоваться для обновления окна

	История	: 27.07.17	Создан

*/
void nglInitUpdateInterval(void)
{
	ngl_win_hUpdateTimer = CreateWaitableTimerW(NULL, FALSE, NULL);
	ngl_win_timerneedcancel = false;
	if(ngl_win_updateinterval != 0 && ngl_win_hUpdateTimer != NULL) {
		LARGE_INTEGER duetime;

		duetime.QuadPart = -10000 * (LONGLONG)ngl_win_updateinterval;

		SetWaitableTimer(ngl_win_hUpdateTimer, &duetime, ngl_win_updateinterval, NULL, NULL, 0);
	}
}

/*
	Функция	: nglChangeUpdateInterval

	Описание: Меняет интервал обновления экрана

	История	: 23.03.17	Создан

*/
void nglChangeUpdateInterval(unsigned int updateinterval)
{
	if(ngl_win_updateinterval == updateinterval) return;

	if(ngl_isinit) {
		if(ngl_win_hUpdateTimer != NULL) {
			if(updateinterval == 0)
				ngl_win_timerneedcancel = true;
			else {
				LARGE_INTEGER duetime;
				
				ngl_win_updateinterval = updateinterval;

				duetime.QuadPart = -10000 * (LONGLONG)ngl_win_updateinterval;

				SetWaitableTimer(ngl_win_hUpdateTimer, &duetime, ngl_win_updateinterval, NULL, NULL, 0);
			}
		} else {
			ngl_ea->nlPrint(LOG_FDEBUGFORMAT, F_NGLSETSTATUSI, ERR_CANTCHANGEUPDATEINTERVAL); ngl_win_updateinterval = 0;
		}
	} else {
		ngl_win_updateinterval = updateinterval;
	}
}

/*
	Функция	: nglGetWindowFlags

	Описание: Установка флагов окна в зависимости от режима экрана
		window_style - переменная, куда будет записан стиль окна, window_exstyle - расширенный стиль окна

	История	: 27.08.17	Создан

*/
void nglGetWindowFlags(int winmode, DWORD *window_style, DWORD *window_exstyle)
{
	if(winmode == NV_MODE_FULLSCREEN) {
		*window_exstyle = WS_EX_APPWINDOW;
		*window_style = WS_POPUP | WS_MAXIMIZE;
	} else if(winmode == NV_MODE_WINDOWED) {
		*window_exstyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		*window_style = WS_OVERLAPPEDWINDOW;
	} else {
		*window_exstyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		*window_style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
	}
}

/*
	Функция	: nglAdjustWindowRect

	Описание: Установка прямоугольника окна в зависимости от dpi, режима и разрешения экрана

	История	: 04.09.17	Создан
		12.03.18 Переименован из nglGetWindowRect в nglAdjustWindowRect

*/
void nglAdjustWindowRect(RECT *window_rect, unsigned int winx, unsigned int winy, unsigned int dpix, unsigned int dpiy, int winmode)
{
	DWORD window_style, window_exstyle;

	window_rect->left = 0;
	window_rect->top = 0;
	window_rect->right = dpix*winx/100;
	window_rect->bottom = dpiy*winy/100;

	nglGetWindowFlags(winmode, &window_style, &window_exstyle);

	if(funcptr_AdjustWindowRectExForDpi) {
		funcptr_AdjustWindowRectExForDpi(window_rect, window_style, FALSE, window_exstyle, 96*dpix/100);
	} else {
		AdjustWindowRectEx(window_rect, window_style, FALSE, window_exstyle);
	}
}

/*
	Функция	: nglChangeWindowMode

	Описание: Устанавливает режим окна (полноэкранный, оконный...)
		Разрешение экрана меняется в nglChangeWindowSize

	История	: 09.03.18	Создан

*/
void nglChangeWindowMode(unsigned int winx, unsigned int winy, int winmode)
{
	DWORD window_style, window_exstyle;
	RECT window_rect;
	UINT swp_flags = 0;
	unsigned int oldwinx, oldwiny;
	
	//wprintf(L"nglChangeWindowMode(%d, %d, %d) (old %d, %d, %d)\n", winx, winy, winmode, ngl_winx, ngl_winy, ngl_winmode);
	
	if(!ngl_isinit) return;

	//wprintf(L"nglChangeWindowMode 1\n");

	oldwinx = ngl_winx;
	oldwiny = ngl_winy;

	// Меняем стили окна
	if(winmode != ngl_winmode) {
		nglGetWindowFlags(winmode, &window_style, &window_exstyle);

		if (!SetWindowLongPtr(ngl_win_hWnd, GWL_STYLE, window_style)) {
			nglGetWindowFlags(ngl_winmode, &window_style, &window_exstyle);
			SetWindowLongPtr(ngl_win_hWnd, GWL_STYLE, window_style);
			nglAdjustWindowRect(&window_rect, oldwinx, oldwiny, ngl_windpix, ngl_windpiy, ngl_winmode);
			SetWindowPos(ngl_win_hWnd, HWND_TOP, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOZORDER);
			return;
		}

		//wprintf(L"nglChangeWindowMode 1.1\n");

		if (!SetWindowLongPtr(ngl_win_hWnd, GWL_EXSTYLE, window_exstyle)) {
			nglGetWindowFlags(ngl_winmode, &window_style, &window_exstyle);
			SetWindowLongPtr(ngl_win_hWnd, GWL_STYLE, window_style);
			SetWindowLongPtr(ngl_win_hWnd, GWL_EXSTYLE, window_exstyle);
			nglAdjustWindowRect(&window_rect, oldwinx, oldwiny, ngl_windpix, ngl_windpiy, ngl_winmode);
			SetWindowPos(ngl_win_hWnd, HWND_TOP, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOZORDER);
			return;
		}
		swp_flags |= SWP_FRAMECHANGED | SWP_SHOWWINDOW;

		//wprintf(L"nglChangeWindowMode 1.2\n");

		// Обновляем здесь позицию окна с флагами SWP_FRAMECHANGED | SWP_SHOWWINDOW, чтобы в WM_DPICHANGED подавались корректные данные
		nglAdjustWindowRect(&window_rect, winx, winy, ngl_windpix, ngl_windpiy, winmode);

		if (winmode == NV_MODE_FULLSCREEN)
			SetWindowPos(ngl_win_hWnd, HWND_TOP, 0, 0, window_rect.right - window_rect.left, window_rect.bottom - window_rect.top, swp_flags);
		else
			SetWindowPos(ngl_win_hWnd, HWND_TOP, 0, 0, window_rect.right - window_rect.left, window_rect.bottom - window_rect.top, swp_flags | SWP_NOMOVE | SWP_NOZORDER);

		//wprintf(L"nglChangeWindowMode 1.3\n");
	}

	//wprintf(L"nglChangeWindowMode 2\n");

	// Меняем разрешение экрана
	if(winmode == NV_MODE_FULLSCREEN) {
		if((ngl_winmode != winmode) || (oldwinx != winx) || (oldwiny != winy)) {
			DEVMODEW dm;

			memset(&dm, 0, sizeof(DEVMODEW));
			dm.dmSize = sizeof(DEVMODEW);
			dm.dmPelsWidth = winx;
			dm.dmPelsHeight = winy;
			dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;

			if (ChangeDisplaySettingsW(&dm, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL) {
				if(ngl_winmode != winmode) {
					//wprintf(L"nglChangeWindowMode 2 return 1\n");
					nglAdjustWindowRect(&window_rect, oldwinx, oldwiny, ngl_windpix, ngl_windpiy, ngl_winmode);
					nglGetWindowFlags(ngl_winmode, &window_style, &window_exstyle);
					SetWindowLongPtr(ngl_win_hWnd, GWL_STYLE, window_style);
					SetWindowLongPtr(ngl_win_hWnd, GWL_EXSTYLE, window_exstyle);
					SetWindowPos(ngl_win_hWnd, HWND_TOP, 0, 0, window_rect.right-window_rect.left, window_rect.bottom- window_rect.top, SWP_FRAMECHANGED | SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOZORDER);
				}
				//wprintf(L"nglChangeWindowMode 2 return 2\n");
				return;
			}
			
			// В полноэкранном режиме не учитываем DPI
			ngl_windpix = 100;
			ngl_windpiy = 100;
		}
	} else if(ngl_winmode == NV_MODE_FULLSCREEN) { // winmode != NV_MODE_FULLSCREEN, а ngl_winmode == NV_MODE_FULLSCREEN
		ChangeDisplaySettings(NULL, 0); // Возвращение на рабочий стол
		GetWindowRect(ngl_win_hWnd, &window_rect);
		nglGetDpiForWindow(&ngl_windpix, &ngl_windpiy, ngl_win_hWnd, window_rect.left, window_rect.top);

		// Обрабатываем WM_DPICHANGED, возникающий после ChangeDisplaySettings
		nglProcessWindowMessages();
	}
	
	//wprintf(L"nglChangeWindowMode 3\n");

	// Изменяем размер окна. В том числе тогда, когда размер уже был задан при изменении стилей.
	nglAdjustWindowRect(&window_rect, winx, winy, ngl_windpix, ngl_windpiy, winmode);
	
	if(winmode == NV_MODE_FULLSCREEN)
		SetWindowPos(ngl_win_hWnd, HWND_TOP, 0, 0, window_rect.right-window_rect.left, window_rect.bottom-window_rect.top, swp_flags);
	else
		SetWindowPos(ngl_win_hWnd, HWND_TOP, 0, 0, window_rect.right-window_rect.left, window_rect.bottom-window_rect.top, swp_flags | SWP_NOMOVE | SWP_NOZORDER);
	
	ngl_winmode = winmode;

	//wprintf(L"nglChangeWindowMode 4\n");

	// Установка происходит при обработке события WM_SIZE
	//ngl_winx = winx;
	//ngl_winy = winy;
}

/*
	Функция	: nglChangeWindowTitle

	Описание: Устанавливает заголовок окна

	История	: 20.06.18	Создан

*/
void nglChangeWindowTitle(const wchar_t *title)
{
	if(SetWindowTextW(ngl_win_hWnd, title))
		ngl_wintitle = title;

	return;
}

/*
	Функция	: nglCloseUpdateTimer

	Описание: Уничтожение таймера, использовавшегося для обновления окна

	История	: 27.07.17	Создан

*/
void nglCloseUpdateTimer(void)
{
	if(ngl_win_hUpdateTimer != NULL) {
		CloseHandle(ngl_win_hUpdateTimer);
		ngl_win_hUpdateTimer = NULL;
	}
}

/*
	Функция	: nglInitTouchInterface

	Описание: Создание интерфейса доступа к тачскрину

	История	: 28.07.17	Создан

*/
void nglInitTouchInterface(void)
{
	int sm_digitizer_metric;

	ngl_win_havetouch = false;

	sm_digitizer_metric = GetSystemMetrics(SM_DIGITIZER);

	if(sm_digitizer_metric & (NID_READY | NID_MULTI_INPUT)) {
		HMODULE hmodule;

		hmodule = GetModuleHandleW(L"User32.dll");

		if(hmodule) {
			// Windows 8+ API
			funcptr_IsMouseInPointerEnabled = (IsMouseInPointerEnabled_type)GetProcAddress(hmodule, "IsMouseInPointerEnabled");
			if(funcptr_IsMouseInPointerEnabled) {
				ngl_win8touchsupport = true;
				ngl_win_havetouch = true;
			}

			// Windows 7 API
			funcptr_RegisterTouchWindow = (RegisterTouchWindow_type)GetProcAddress(hmodule, "RegisterTouchWindow");
			funcptr_UnregisterTouchWindow = (UnregisterTouchWindow_type)GetProcAddress(hmodule, "UnregisterTouchWindow");
			funcptr_CloseTouchInputHandle = (CloseTouchInputHandle_type)GetProcAddress(hmodule, "CloseTouchInputHandle");
			funcptr_GetTouchInputInfo = (GetTouchInputInfo_type)GetProcAddress(hmodule, "GetTouchInputInfo");

			if(funcptr_RegisterTouchWindow != NULL && funcptr_UnregisterTouchWindow != NULL && funcptr_CloseTouchInputHandle != NULL && funcptr_GetTouchInputInfo != NULL)
				ngl_win_havetouch = true;
		}
	}

	if(ngl_win_havetouch && !ngl_win8touchsupport) {
		funcptr_RegisterTouchWindow(ngl_win_hWnd, 0);
	}

	ngl_ea->nlPrint(LOG_FDEBUGFORMAT2, F_NGLINITWINDOW, NGL_SUPPORTTOUCHSCREEN, ngl_win_havetouch?N_YES:N_NO);
}

/*
	Функция	: nglGetTouchId

	Описание: Возвращает индекс касания в движке по уникальному идентификатору системы

	История	: 03.10.17	Создан

*/
unsigned int nglGetTouchId(DWORD uid)
{
	int j; // Номер тача inputs[i] в списке движка

	for(j = 0; j < ngl_wintouchcount; j++) {
		if(uid == ngl_wintouchlookuptable[j])
			break;
	}

	if(j == ngl_wintouchcount) {
		if(ngl_wintouchcount < ngl_wintouchmaxcount) {
			ngl_wintouchcount++;
			ngl_wintouchlookuptable[j] = uid;
			ngl_wintouchpressed[j] = NV_KEYSTATUS_UNTOUCHED;
		} else { // Закончились номера под тачи
			// Ищем свободный номер
			for(j = 0; j < ngl_wintouchcount; j++) {
				if(ngl_wintouchpressed[j] == NV_KEYSTATUS_UNTOUCHED) {
					ngl_wintouchlookuptable[j] = uid;
					ngl_wintouchpressed[j] = NV_KEYSTATUS_UNTOUCHED;
					break;
				}
			}
		}
	}

	return j;
}

/*
	Функция	: nglFreeTouchId

	Описание: Освобождает индекс касания движка

	История	: 03.10.17	Создан

*/
void nglFreeTouchId(unsigned int id)
{
	if(id < (unsigned int)ngl_wintouchcount) {
		if(id != (unsigned int)ngl_wintouchcount-1) {
			ngl_wintouchlookuptable[id] = ngl_wintouchlookuptable[ngl_wintouchcount-1];
			ngl_wintouchpressed[id] = ngl_wintouchpressed[ngl_wintouchcount-1];
			ngl_wintouchx[id] = ngl_wintouchx[ngl_wintouchcount-1];
			ngl_wintouchy[id] = ngl_wintouchy[ngl_wintouchcount-1];
		}
		ngl_wintouchcount--;
	}
}

/*
	Функция	: nglSetTouchCoord

	Описание: Устанавливает позицию касания

	История	: 03.10.17	Создан

*/
void nglSetTouchCoord(HWND w_hWnd, unsigned int id, int scrx, int scry)
{
	if(id < (unsigned int)ngl_wintouchcount) {
		POINT point;

		point.x = scrx;
		point.y = scry;

		ScreenToClient(w_hWnd, &point);

		ngl_wintouchx[id] = 100*point.x/ngl_windpix;
		ngl_wintouchy[id] = 100*point.y/ngl_windpiy;
	}
}

/*
	Функция	: nglProcessWM_TOUCH

	Описание: Обрабатывает касание. Возвращает true, если следует вызвать DefWindowProc

	История	: 31.07.17	Создан

*/
bool nglProcessWM_TOUCH(HWND w_hWnd, WPARAM wParam, LPARAM lParam)
{
	unsigned int nof_inputs;
	TOUCHINPUT *inputs;
	bool need_call_DefWindowProc = true;

	if(!ngl_win_havetouch) return true;

	nof_inputs = LOWORD(wParam);

	if(nof_inputs > (unsigned int)ngl_wintouchmaxcount)
		nof_inputs = ngl_wintouchmaxcount;

	inputs = ngl_ea->nAllocMemory(nof_inputs*sizeof(TOUCHINPUT));
	if(!inputs) return true;

	if(funcptr_GetTouchInputInfo((HTOUCHINPUT)lParam, nof_inputs, inputs, sizeof(TOUCHINPUT))) {
		unsigned int i;

		for(i = 0; i < nof_inputs; i++) {
			int j; // Номер тача inputs[i] в списке движка

			// Поиск номера j тача inputs[i] в списке движка
			j = nglGetTouchId(inputs[i].dwID);

			if(j == ngl_wintouchcount)
				continue;

			// Изменение состояния тача
			nglSetTouchCoord(w_hWnd, j, TOUCH_COORD_TO_PIXEL(inputs[i].x), TOUCH_COORD_TO_PIXEL(inputs[i].y));

			if(inputs[i].dwFlags & TOUCHEVENTF_DOWN) {
				ngl_wintouchpressed[j] = NV_KEYSTATUS_PRESSED;
			} else if(inputs[i].dwFlags & TOUCHEVENTF_UP) {
				ngl_wintouchpressed[j] = NV_KEYSTATUS_RELEASED;
			}
		}

		funcptr_CloseTouchInputHandle((HTOUCHINPUT)lParam);

		need_call_DefWindowProc = false;
	}

	ngl_ea->nFreeMemory(inputs);

	return need_call_DefWindowProc;
}

/*
	Функция	: nglCloseTouchInterface

	Описание: Уничтожение интерфейса доступа к тачскрину

	История	: 28.07.17	Создан

*/
void nglCloseTouchInterface(void)
{
	if(ngl_win_havetouch) {
		if(!ngl_win8touchsupport)
			funcptr_UnregisterTouchWindow(ngl_win_hWnd);
		ngl_wintouchcount = 0;
	}
}

/*
	Функция	: nglCreateWindow

	Описание: Создаёт окно window_name и класс window_class. Инициализирует ngl_win_hInstance и ngl_win_hDC.

	История	: 06.04.18	Создан

*/
bool nglCreateWindow(wchar_t * window_name, DWORD window_style, DWORD window_exstyle, wchar_t *window_class, WNDPROC window_proc, int window_pos_x, int window_pos_y, int window_width, int window_height)
{
	WNDCLASSEXW wndclassex;

	ngl_win_hInstance = GetModuleHandleW(NULL);

	memset(&wndclassex, 0, sizeof(WNDCLASSEXW));
	wndclassex.cbSize = sizeof(WNDCLASSEXW); // Размер структуры WNDCLASSEX
	wndclassex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC; // Стили класса (первые два - перерисовка окна при изменении его размеров, последний - собственный Device Context у окна)
	wndclassex.lpfnWndProc = window_proc; // Оконная процедура
	wndclassex.cbClsExtra = 0; // Количество дополнительных байт которые необходимо выделить после структуры класса окна
	wndclassex.cbWndExtra = 0; // Количество дополнительных байт которые необходимо выделить после инстанса окна (что бы это не значило)
	wndclassex.hInstance = ngl_win_hInstance;
	wndclassex.hIcon = LoadIcon(NULL, IDI_APPLICATION); // Иконка окна
	wndclassex.hCursor = LoadCursor(NULL, IDC_ARROW); // Иконка курсора
	wndclassex.hbrBackground = NULL; // Фон (отсутствует)
	wndclassex.lpszMenuName = NULL; // Меню (отсутствует)
	wndclassex.lpszClassName = window_class; // Имя класса
	wndclassex.hIconSm = NULL; // Мелкая иконка такая же, как и крупная

	if(!RegisterClassExW(&wndclassex)) {
		ngl_ea->nlPrint(LOG_FDEBUGFORMAT, F_NGLCREATEWINDOW, ERR_FAILEDTOREGISTERWINDOWCLASS);
		ngl_win_hInstance = NULL;
		return false;
	}

	ngl_win_hWnd = CreateWindowExW(
		window_exstyle, window_class, window_name, window_style | /*WS_CLIPSIBLINGS | */WS_CLIPCHILDREN,
		window_pos_x, window_pos_y, window_width, window_height,
		NULL/*hWndParent*/, NULL/*hMenu*/, ngl_win_hInstance, NULL/*lpParam*/);

	if(!ngl_win_hWnd) {
		ngl_ea->nlPrint(LOG_FDEBUGFORMAT, F_NGLCREATEWINDOW, ERR_FAILEDTOCREATEWINDOW);
		goto FAILED;
	}

	ngl_win_hDC = GetDC(ngl_win_hWnd);
	if(!ngl_win_hDC) {
		ngl_ea->nlPrint(LOG_FDEBUGFORMAT, F_NGLCREATEWINDOW, ERR_FAILEDTOGETDC);
		goto FAILED;
	}

	return true;

FAILED:
	if(ngl_win_hWnd && ngl_win_hDC) {
		ReleaseDC(ngl_win_hWnd, ngl_win_hDC);
		ngl_win_hDC = NULL;
	}

	if(ngl_win_hWnd) {
		DestroyWindow(ngl_win_hWnd);
		ngl_win_hWnd = NULL;
	}

	UnregisterClassW(window_class, ngl_win_hInstance);

	ngl_win_hInstance = NULL;

	return false;
}

/*
	Функция	: nglDestroyWindow

	Описание: Уничтожает созданное окно.
		window_class - класс уничтожаемого окна

	История	: 06.04.18	Создан

*/
bool nglDestroyWindow(wchar_t *window_class)
{
	bool result = true;

	if(ngl_win_hWnd && ngl_win_hDC) {
		if(!ReleaseDC(ngl_win_hWnd, ngl_win_hDC)) {
			ngl_ea->nlPrint(LOG_FDEBUGFORMAT, F_NGLDESTROYWINDOW, ERR_FAILEDTORELEASEDC);
			result = false;
		}
		ngl_win_hDC = NULL;
	}

	if(ngl_win_hWnd) {
		if(!DestroyWindow(ngl_win_hWnd)) {
			ngl_ea->nlPrint(LOG_FDEBUGFORMAT, F_NGLDESTROYWINDOW, ERR_FAILEDTODESTROYWINDOW);
			result = false;
		}
		ngl_win_hWnd = NULL;
	}

	if(ngl_win_hInstance) {
		if(!UnregisterClassW(window_class, ngl_win_hInstance)) {
			ngl_ea->nlPrint(LOG_FDEBUGFORMAT, F_NGLDESTROYWINDOW, ERR_FAILEDTOUNREGISTERWINDOWCLASS);
			result = false;
		}

		ngl_win_hInstance = NULL;
	} else
		result = false;

	return result;
}

/*
	Функция	: nglProcessWindowMessages

	Описание: Получает сообщения окна.

	История	: 06.04.18	Создан

*/
void nglProcessWindowMessages(void)
{
	MSG msg;

	while(PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
		if(msg.message == WM_QUIT)
			ngl_win_exitmsg = true;
		TranslateMessage(&msg); // Преобразует сообщения виртуальной клавиатуры в сообщения содержащие символы
		DispatchMessageW(&msg); // Отправляет сообщение оконной процедуре
	}
}

/*
	Функция	: nglProcessWindow

	Описание: Обрабатывает окно. Возвращает true, если надо вызвать DefWindowProc после

	История	: 29.07.17	Создан
	
*/
bool nglProcessWindow(HWND w_hWnd, UINT message, WPARAM wParam, LPARAM lParam, LRESULT *result)
{
	LPRECT new_rect;

	*result = 0;

	switch(message) {
		case WM_ERASEBKGND:
			*result = 1;
			return false;
		case WM_NCCREATE:
			if(funcptr_EnableNonClientDpiScaling) {
				if(ngl_need_enablenonclientdpiscaling)
					funcptr_EnableNonClientDpiScaling(w_hWnd);
			}
			return true;
		case WM_ACTIVATE:
			return false;
		case WM_CREATE:
			return false;
		case WM_CLOSE:
			PostQuitMessage(0);
			return false;
		case WM_CHAR:
			if(ngl_wintextinputbufsize < 255) {
				*(ngl_wintextinputbuf+ngl_wintextinputbufsize) = (wchar_t)wParam;
				ngl_wintextinputbufsize++;
			}
			return false;
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			if(wParam < 256)
				ngl_winkeys[wParam] = NV_KEYSTATUS_PRESSED;
			if (wParam == VK_SHIFT) {
				if((GetKeyState(VK_LSHIFT) & 0x8000) != 0)
					ngl_winkeys[VK_LSHIFT] = NV_KEYSTATUS_PRESSED;

				if((GetKeyState(VK_RSHIFT) & 0x8000) != 0)
					ngl_winkeys[VK_RSHIFT] = NV_KEYSTATUS_PRESSED;
			}
			if (wParam == VK_CONTROL) {
				if ((GetKeyState(VK_LCONTROL) & 0x8000) != 0)
					ngl_winkeys[VK_LCONTROL] = NV_KEYSTATUS_PRESSED;

				if ((GetKeyState(VK_RCONTROL) & 0x8000) != 0)
					ngl_winkeys[VK_RCONTROL] = NV_KEYSTATUS_PRESSED;
			}
			if (wParam == VK_MENU) {
				if ((GetKeyState(VK_LMENU) & 0x8000) != 0)
					ngl_winkeys[VK_LMENU] = NV_KEYSTATUS_PRESSED;

				if ((GetKeyState(VK_RMENU) & 0x8000) != 0)
					ngl_winkeys[VK_RMENU] = NV_KEYSTATUS_PRESSED;
			}
			return false;
		case WM_KEYUP:
		case WM_SYSKEYUP:
			if(wParam < 256)
				ngl_winkeys[wParam] = NV_KEYSTATUS_RELEASED;
			if (wParam == VK_SHIFT) {
				if((GetKeyState(VK_LSHIFT) & 0x8000) == 0 && ngl_winkeys[VK_LSHIFT] == NV_KEYSTATUS_PRESSED)
					ngl_winkeys[VK_LSHIFT] = NV_KEYSTATUS_RELEASED;

				if((GetKeyState(VK_RSHIFT) & 0x8000) == 0 && ngl_winkeys[VK_RSHIFT] == NV_KEYSTATUS_PRESSED)
					ngl_winkeys[VK_RSHIFT] = NV_KEYSTATUS_RELEASED;
			}
			if (wParam == VK_CONTROL) {
				if ((GetKeyState(VK_LCONTROL) & 0x8000) == 0 && ngl_winkeys[VK_LCONTROL] == NV_KEYSTATUS_PRESSED)
					ngl_winkeys[VK_LCONTROL] = NV_KEYSTATUS_RELEASED;

				if ((GetKeyState(VK_RCONTROL) & 0x8000) == 0 && ngl_winkeys[VK_RCONTROL] == NV_KEYSTATUS_PRESSED)
					ngl_winkeys[VK_RCONTROL] = NV_KEYSTATUS_RELEASED;
			}
			if (wParam == VK_MENU) {
				if ((GetKeyState(VK_LMENU) & 0x8000) == 0 && ngl_winkeys[VK_LMENU] == NV_KEYSTATUS_PRESSED)
					ngl_winkeys[VK_LMENU] = NV_KEYSTATUS_RELEASED;

				if ((GetKeyState(VK_RMENU) & 0x8000) == 0 && ngl_winkeys[VK_RMENU] == NV_KEYSTATUS_PRESSED)
					ngl_winkeys[VK_RMENU] = NV_KEYSTATUS_RELEASED;
			}
			return false;
		case WM_LBUTTONDOWN:
			ngl_winmbl = NV_KEYSTATUS_PRESSED;
			return false;
		case WM_LBUTTONUP:
			ngl_winmbl = NV_KEYSTATUS_RELEASED;
			return false;
		case WM_RBUTTONDOWN:
			ngl_winmbr = NV_KEYSTATUS_PRESSED;
			return false;
		case WM_RBUTTONUP:
			ngl_winmbr = NV_KEYSTATUS_RELEASED;
			return false;
		case WM_MBUTTONDOWN:
			ngl_winmbm = NV_KEYSTATUS_PRESSED;
			return false;
		case WM_MBUTTONUP:
			ngl_winmbm = NV_KEYSTATUS_RELEASED;
			return false;
		case WM_MOUSEMOVE:
			ngl_winmx = 100*LOWORD(lParam)/ngl_windpix;
			ngl_winmy = 100*HIWORD(lParam)/ngl_windpiy;
			return false;
		case WM_MOUSEWHEEL:
			ngl_winmwheel += GET_WHEEL_DELTA_WPARAM(wParam)/WHEEL_DELTA;
			return false;
		case WM_SIZE:
			ngl_winx = 100*LOWORD(lParam)/ngl_windpix;
			ngl_winy = 100*HIWORD(lParam)/ngl_windpiy;

			//wprintf(L"WM_SIZE: dpi %dx%d size %dx%d %d, lw %d hw %d\n", ngl_windpix, ngl_windpiy, ngl_winx, ngl_winy, ngl_winmode, (int)(LOWORD(lParam)), (int)(HIWORD(lParam)));

			return false;
		case WM_TOUCH:
			return nglProcessWM_TOUCH(w_hWnd, wParam, lParam);
		case WM_DPICHANGED:
			if(ngl_winmode != NV_MODE_FULLSCREEN) {
				ngl_windpix = 100*LOWORD(wParam)/96;
				ngl_windpiy = 100*HIWORD(wParam)/96;
			}

			new_rect = (LPRECT)lParam;

			/*wprintf(L"WM_DPICHANGED: %dx%d (%dx%d %d) systrect %d %d\n",
				ngl_windpix, ngl_windpiy, ngl_winx, ngl_winy, ngl_winmode, 
				new_rect->right-new_rect->left, new_rect->bottom-new_rect->top);*/

			SetWindowPos(w_hWnd, HWND_TOP, new_rect->left, new_rect->top, new_rect->right - new_rect->left, new_rect->bottom - new_rect->top, SWP_NOZORDER | SWP_NOACTIVATE);

			return false;
		case WM_POINTERUPDATE:
		case WM_POINTERDOWN:
		case WM_POINTERUP:
		case WM_POINTERENTER:
		case WM_POINTERLEAVE:
		case WM_POINTERCAPTURECHANGED:
			if(ngl_win8touchsupport) {
				DWORD uid;
				unsigned int id;

				uid = GET_POINTERID_WPARAM(wParam);

				id = nglGetTouchId(uid);

				if(id != (unsigned int)ngl_wintouchcount) {
					switch(message) {
						case WM_POINTERDOWN:
							ngl_wintouchpressed[id] = NV_KEYSTATUS_PRESSED;
							nglSetTouchCoord(w_hWnd, id, LOWORD(lParam), HIWORD(lParam));
							break;
						case WM_POINTERUP:
							ngl_wintouchpressed[id] = NV_KEYSTATUS_RELEASED;
							nglSetTouchCoord(w_hWnd, id, LOWORD(lParam), HIWORD(lParam));
							break;
						case WM_POINTERUPDATE:
							nglSetTouchCoord(w_hWnd, id, LOWORD(lParam), HIWORD(lParam));
							break;
						case WM_POINTERENTER:
						case WM_POINTERLEAVE:
							ngl_wintouchpressed[id] = NV_KEYSTATUS_UNTOUCHED;
							nglSetTouchCoord(w_hWnd, id, LOWORD(lParam), HIWORD(lParam));
							break;
						case WM_POINTERCAPTURECHANGED:
							nglFreeTouchId(id);
							break;
					}
				}
			}
			return true;
		default:
			return true;
	}
}
