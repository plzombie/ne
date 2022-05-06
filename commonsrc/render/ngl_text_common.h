/*
	Файл	: ngl_text_common.h

	Описание: Весь текст

	История	: 26.07.17	Создан

*/

#ifndef NGL_TEXT_COMMON_H
#define NGL_TEXT_COMMON_H

#define NGL_VSYNC L"Vsync"

#define NGL_SUPPORTTOUCHSCREEN L"Support touch screen"
#define NGL_SUPPORTVSYNC L"Support Vsync"
#define NGL_MAXANISOTROPY L"Maximum degree of anisotropy"
#define NGL_DEFAULTANISOTROPY L"Default degree of anisotropy"

#define NGL_SCREENINFO L"%ls: Screen mode %dx%dx%d, depth bits %d, fullscreen %ls"

// Названия функций

#define F_NGLINIT L"nglInit()"
#define F_NGLCLOSE L"nglClose()"

#define F_NGLINITWINDOW L"nglInitWindow()"
#define F_NGLCLOSEWINDOW L"nglCloseWindow()"
#define F_NGLUPDATEWINDOW L"nglUpdateWindow()"

#define F_NGLCREATEWINDOW L"nglCreateWindow()"
#define F_NGLDESTROYWINDOW L"nglDestroyWindow()"

#define F_NGLTEXCONVERTTOSUPPORTED L"nglTexConvertToSupported()"
#define F_NGLLOADTEXTURE L"nglLoadTexture()"
#define F_NGLFREETEXTURE L"nglFreeTexture()"

#define F_NGLBATCHINIT L"nglBatchInit()"
#define F_NGLBATCHCLOSE L"nglBatchClose()"

#define F_NGLSETSTATUSI L"nglSetStatusi()"

// Сообщения об ошибках

#define ERR_FAILEDTOCREATEWINDOW L"Failed to create window"
#define ERR_FAILEDTODESTROYWINDOW L"Failed to destroy window"

#define ERR_FAILEDTOSETFSMODE L"Failed to set fullscreen mode"

#define ERR_FAILEDTOCREATETIMER L"Failed to create timer"
#define ERR_CANTCHANGEUPDATEINTERVAL L"Can't change update interval"

#define ERR_CANTCHANGEVSYNC L"Can't change vsync"

// Ошибки в винде
#define ERR_FAILEDTOREGISTERWINDOWCLASS L"Failed to register window class"
#define ERR_FAILEDTOUNREGISTERWINDOWCLASS L"Failed to unregister window class"
#define ERR_FAILEDTOGETDC L"Failed to get Device Context"
#define ERR_FAILEDTORELEASEDC L"Failed to release Device Context"

#endif
