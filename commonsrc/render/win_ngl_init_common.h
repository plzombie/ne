/*
	Файл	: win_ngl_init_common.h

	Описание: Заголовок для win_ngl_init_common.c

	История	: 26.07.17	Создан

*/

#ifndef WIN_NGL_INIT_COMMON_H
#define WIN_NGL_INIT_COMMON_H

#include <Windows.h>

extern HDC ngl_win_hDC;
extern HWND ngl_win_hWnd;

extern HANDLE ngl_win_hUpdateTimer;
extern bool ngl_win_timerneedcancel;

extern void nglSetDpiAware(int window_pos_x, int window_pos_y);
extern void nglGetDpiForWindow(int *windpix, int *windpiy, HWND hwnd, int fallback_x, int fallback_y);
extern void nglInitUpdateInterval(void);
extern void nglChangeUpdateInterval(unsigned int updateinterval);
extern void nglCloseUpdateTimer(void);
extern void nglInitTouchInterface(void);
extern void nglCloseTouchInterface(void);
extern bool nglCreateWindow(wchar_t * window_name, DWORD window_style, DWORD window_exstyle, wchar_t *window_class, WNDPROC window_proc, int window_pos_x, int window_pos_y, int window_width, int window_height);
extern bool nglDestroyWindow(wchar_t *window_class);
extern void nglProcessWindowMessages(void);
extern bool nglProcessWindow(HWND w_hWnd, UINT message, WPARAM wParam, LPARAM lParam, LRESULT *result);
extern void nglGetWindowFlags(int winmode, DWORD *dwStyle, DWORD *dwExStyle);
extern void nglAdjustWindowRect(RECT *WindowRect, unsigned int winx, unsigned int winy, unsigned int dpix, unsigned int dpiy, int winmode);

#endif
