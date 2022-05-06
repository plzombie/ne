/*
	Файл	: ngl_text.h

	Описание: Весь текст

	История	: 04.08.12	Создан

*/

#include "../nyan/nyan_text.h"

#define NGL_VSYNC L"Vsync"

#define NGL_SUPPORTVSYNC L"Support Vsync"
#define NGL_MAXANISOTROPY L"Maximum degree of anisotropy"
#define NGL_DEFAULTANISOTROPY L"Default degree of anisotropy"

#define NGL_SCREENINFO L"%ls: Screen mode %dx%dx%d, depth bits %d, fullscreen %ls"

// Названия функций

#define F_NGLTEXCONVERTTOSUPPORTED L"nglTexConvertToSupported()"
#define F_NGLLOADTEXTURE L"nglLoadTexture()"
#define F_NGLFREETEXTURE L"nglFreeTexture()"

#define F_NGLBATCHINIT L"nglBatchInit()"
#define F_NGLBATCHCLOSE L"nglBatchClose()"

// Сообщения об ошибках

#define ERR_UNSUPPORTEDNGLCOLORFORMAT L"Unsupported nglcolorformat parameter"

#define ERR_CANTCHANGEVSYNC L"Can't change vsync"

#define ERR_CANTCHANGEUPDATEINTERVAL L"Can't change update interval"

#include "../commonsrc/render/ngl_text_common.h"
