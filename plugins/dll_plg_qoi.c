/*
	Файл	: dll_plg_qoi.c

	Описание: Плагин для загрузки Quite OK изображений

	История	: 11.01.23	Создан

*/

#include <stdbool.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <wchar.h>

#if defined(N_POSIX)
	#define N_API __attribute__ ((visibility("default")))
	#define N_APIENTRY
	#define _wcsicmp wcscasecmp
#else
	#define N_API __declspec(dllexport)
	#define N_APIENTRY __cdecl
#endif

#include "../nyan/nyan_engapi.h"
#include "../nyan/nyan_texformat_publicapi.h"
#include "../nyan/nyan_plgtypes_publicapi.h"

engapi_type *ea = 0;

#define QOI_IMPLEMENTATION
#define QOI_NO_STDIO
#define QOI_MALLOC(size) (ea->nAllocMemory(size))
#define QOI_FREE(ptr) (ea->nFreeMemory(ptr))
#include "../forks/qoi/qoi.h"

#include "dll_plg_qoi.h"

bool N_APIENTRY qoi_plgSupportExt(const wchar_t *fname, const wchar_t *fext);
bool N_APIENTRY qoi_plgLoad(const wchar_t *fname, nv_texture_type *tex);

nv_tex_plugin_type plg = {sizeof(nv_tex_plugin_type), &qoi_plgSupportExt, &qoi_plgLoad};

plg_api_type plg_api = {sizeof(plg_api_type), N_PLG_TYPE_TEXLOADER, L"Quite OK images (*.qoi)", &plg};

/*
	Функция	: plgGetApi

	Описание: Возвращает API плагина

	История	: 11.01.23	Создан

*/
N_API plg_api_type * N_APIENTRY plgGetApi(void)
{
	if(!ea) return 0;

	return &plg_api;
}

/*
	Функция	: plgSetupDll

	Описание: Настройка dll плагина

	История	: 11.01.23	Создан

*/
N_API bool N_APIENTRY plgSetupDll(engapi_type *engapi)
{
	if(engapi) {
		if(engapi->mysize == sizeof(engapi_type)) {
			ea = engapi;

			ea->nlPrint(L"This software using QOI:\n"
					L"MIT License\n"
					L"\n"
					L"Copyright (c) 2022 Dominic Szablewski\n"
					L"\n"
					L"Permission is hereby granted, free of charge, to any person obtaining a copy\n"
					L"of this software and associated documentation files (the \"Software\"), to deal\n"
					L"in the Software without restriction, including without limitation the rights\n"
					L"to use, copy, modify, merge, publish, distribute, sublicense, and/or sell\n"
					L"copies of the Software, and to permit persons to whom the Software is\n"
					L"furnished to do so, subject to the following conditions:\n"
					L"\n"
					L"The above copyright notice and this permission notice shall be included in all\n"
					L"copies or substantial portions of the Software.\n"
					L"\n"
					L"THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\n"
					L"IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\n"
					L"FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE\n"
					L"AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\n"
					L"LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,\n"
					L"OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE\n"
					L"SOFTWARE.");

			return true;
		}
	}

	return false;
}

/*
	Функция	: qoi_plgSupportExt

	Описание: Проверяет поддержку типа файла

	История	: 11.01.23	Создан

*/
bool N_APIENTRY qoi_plgSupportExt(const wchar_t *fname, const wchar_t *fext)
{
	(void)fname; // Неиспользуемая переменная

	if(_wcsicmp(fext,L"qoi") == 0) return true;
	return false;
}

/*
	Функция	: qoi_plgLoad

	Описание: Загружает qoi файл

	История	: 11.01.23	Создан

*/
bool N_APIENTRY qoi_plgLoad(const wchar_t *fname, nv_texture_type *tex)
{
	unsigned int f;
	long long filesize;
	void *filedata;
	size_t i, linesize;
	unsigned char *data, *p_in, *p_out;
	qoi_desc desc;
	
	if(!ea) return false;

	ea->nlPrint(L"%ls %ls %ls", L"qoi_plgLoad()", L"fname", fname); ea->nlAddTab(1);

	f = ea->nFileOpen(fname);

	if(f == 0) {
		ea->nlAddTab(-1); ea->nlPrint(L"%ls: %ls", L"qoi_plgLoad()", L"File not found");
		return false;
	}
	
	filesize = ea->nFileLength(f);
	if(filesize > INT_MAX) {
		ea->nlAddTab(-1); ea->nlPrint(L"%ls: %ls", L"qoi_plgLoad()", L"File too large");
		ea->nFileClose(f);
		return false;
	}
	
	filedata = ea->nAllocMemory((size_t)filesize);
	if(!filedata) {
		ea->nlAddTab(-1); ea->nlPrint(L"%ls: %ls", L"qoi_plgLoad()", L"Can't allocate memory");
		ea->nFileClose(f);
		return false;
	}
	
	ea->nFileRead(f,filedata,filesize);
	ea->nFileClose(f);
	data = qoi_decode(filedata, (int)filesize, &desc, 0);
	ea->nFreeMemory(filedata);
	
	if(!data) {
		ea->nlAddTab(-1); ea->nlPrint(L"%ls: %ls", L"qoi_plgLoad()", L"Can't decode file");
		return false;
	}
		
	if(desc.channels == 3)
		tex->nglcolorformat = NGL_COLORFORMAT_R8G8B8;
	else if(desc.channels == 4)
		tex->nglcolorformat = NGL_COLORFORMAT_R8G8B8A8;
	else {
		ea->nlAddTab(-1); ea->nlPrint(L"%ls: %ls", L"qoi_plgLoad()", L"Inknown file type");
		ea->nFreeMemory(data);
		tex->buffer = 0;
		return false;
	}
	tex->sizex = desc.width;
	tex->sizey = desc.height;
	if((tex->sizex*3)%4 == 0) tex->nglrowalignment = 4; else tex->nglrowalignment = 1;
	
	linesize = (size_t)desc.width*(size_t)desc.channels;
	tex->buffer = ea->nAllocMemory((size_t)desc.height*linesize);
	if(!tex->buffer) {
		ea->nlAddTab(-1); ea->nlPrint(L"%ls: %ls", L"qoi_plgLoad()", L"Can't allocate memory");
		ea->nFreeMemory(data);
		return false;
	}
	
	p_in = data+(desc.height-1)*linesize;
	p_out = tex->buffer;
	for(i = 0; i < desc.height; i++) {
		memcpy(p_out, p_in, linesize);
		p_out += linesize;
		p_in -= linesize;
	}
	
	ea->nFreeMemory(data);
	
	ea->nlAddTab(-1); ea->nlPrint(L"%ls: %ls", L"qoi_plgLoad()", L"ok");
	
	return true;
}
