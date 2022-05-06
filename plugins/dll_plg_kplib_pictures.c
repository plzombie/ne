/*
	Файл	: dll_plg_kplib_pictures.c

	Описание: Плагин для загрузки изображений через kplib.c

	История	: 05.08.14	Создан

*/

#include <stdbool.h>
#include <stdlib.h>
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

#include "../forks/kplib_c/kplib_pictures.h"

#include "dll_plg_kplib_pictures.h"

engapi_type *ea = 0;

bool N_APIENTRY kplib_pictures_plgSupportExt(const wchar_t *fname, const wchar_t *fext);
bool N_APIENTRY kplib_pictures_plgLoad(const wchar_t *fname, nv_texture_type *tex);

nv_tex_plugin_type plg = {sizeof(nv_tex_plugin_type), &kplib_pictures_plgSupportExt, &kplib_pictures_plgLoad};

plg_api_type plg_api = {sizeof(plg_api_type), N_PLG_TYPE_TEXLOADER, L"Image files via Ken\'s Picture LIBrary (*.png, *.jpg, *.gif, *.cel, *.bmp, *.pcx, *.dds, *.tga)", &plg};

/*
	Функция	: plgGetApi

	Описание: Возвращает API плагина

	История	: 05.08.14	Создан

*/
N_API plg_api_type * N_APIENTRY plgGetApi(void)
{
	if(!ea) return 0;

	return &plg_api;
}

/*
	Функция	: plgSetupDll

	Описание: Настройка dll плагина

	История	: 05.08.14	Создан

*/
N_API bool N_APIENTRY plgSetupDll(engapi_type *engapi)
{
	if(engapi) {
		if(engapi->mysize == sizeof(engapi_type)) {
			ea = engapi;

			ea->nlPrint(L"%ls", L"this software using kplib.c:\n\n"
				L"KPLIB.C: Ken\'s Picture LIBrary written by Ken Silverman\n"
				L"Copyright (c) 1998-2008 Ken Silverman\n"
				L"Ken Silverman\'s official web site: http://advsys.net/ken\n\n"
				L"I offer this code to the community for free use - all I ask is that my name be\n"
				L"included in the credits.\n\n"
				L"-Ken S.\n");

			return true;
		}
	}

	return false;
}

/*
	Функция	: kplib_pictures_plgSupportExt

	Описание: Проверяет поддержку типа файла

	История	: 05.08.14	Создан

*/
bool N_APIENTRY kplib_pictures_plgSupportExt(const wchar_t *fname, const wchar_t *fext)
{
	(void)fname; // Неиспользуемая переменная

	if(_wcsicmp(fext,L"png") == 0) return true;
	if(_wcsicmp(fext,L"jpg") == 0) return true;
	if(_wcsicmp(fext,L"jpeg") == 0) return true;
	if(_wcsicmp(fext,L"gif") == 0) return true;
	if(_wcsicmp(fext,L"cel") == 0) return true;
	if(_wcsicmp(fext,L"pic") == 0) return true;
	if(_wcsicmp(fext,L"bmp") == 0) return true;
	if(_wcsicmp(fext,L"pcx") == 0) return true;
	if(_wcsicmp(fext,L"dds") == 0) return true;
	if(_wcsicmp(fext,L"tga") == 0) return true;
	if(_wcsicmp(fext,L"tpic") == 0) return true;
	return false;
}

#include <math.h>

/*
	Функция	: kplib_pictures_plgLoad

	Описание: Загружает изображение

	История	: 05.08.14	Создан

*/
bool N_APIENTRY kplib_pictures_plgLoad(const wchar_t *fname, nv_texture_type *tex)
{
	unsigned int f, length, i;
	unsigned char *buf, *p, *p2;
	int err;

	if(!ea) return false;

	ea->nlPrint(L"%ls %ls %ls", L"kplib_pictures_plgLoad()", L"fname", fname); ea->nlAddTab(1);


	f = ea->nFileOpen(fname);

	if(f == 0) {
		ea->nlAddTab(-1); ea->nlPrint(L"%ls: %ls", L"kplib_pictures_plgLoad()", L"File not founded");
		return false;
	}


	// Чтение файла в память
	length = (unsigned int)ea->nFileLength(f);

	buf = ea->nAllocMemory(length);

	if(!buf) {
		ea->nlAddTab(-1); ea->nlPrint(L"%ls: %ls", L"kplib_pictures_plgLoad()", L"false");
		ea->nFileClose(f);
		return false;
	}

	ea->nFileRead(f, buf, length);

	ea->nFileClose(f);


	// Чтение изображения
	if(kpgetdim((const char *)buf, length, (int *)(&tex->sizex), (int *)(&tex->sizey)) == KPLIB_NONE) {
		ea->nlAddTab(-1); ea->nlPrint(L"%ls: %ls", L"kplib_pictures_plgLoad()", L"Can't get image dimension");
		ea->nFreeMemory(buf);
		return false;
	}

	if(tex->sizex == 0 || tex->sizey == 0) {
		ea->nlAddTab(-1); ea->nlPrint(L"%ls: %ls", L"kplib_pictures_plgLoad()", L"false");
		ea->nFreeMemory(buf);
		return false;
	}

	tex->buffer = ea->nAllocMemory(tex->sizex*tex->sizey*4);

	if(!tex->buffer) {
		ea->nlAddTab(-1); ea->nlPrint(L"%ls: %ls", L"kplib_pictures_plgLoad()", L"false");
		ea->nFreeMemory(buf);
		return false;
	}

	err = kprender((const char *)buf, length, (intptr_t)(tex->buffer), tex->sizex*4, tex->sizex, tex->sizey, 0, 0);

	if(err < 0) {
#if 0
		ea->nlAddTab(-1); ea->nlPrint(L"%ls: %ls %d", L"kplib_pictures_plgLoad()", L"false, error code:", err);
		ea->nFreeMemory(buf);
		ea->nFreeMemory(tex->buffer);
		return false;
#else
		ea->nlPrint(L"%ls: %ls %d", L"kplib_pictures_plgLoad()", L"kprender returned", err);
		for(i = 0; i < tex->sizey; i++) {
			unsigned int j;
			for(j = 0; j < tex->sizex; j++) {
				tex->buffer[(i*tex->sizex+j)*4] = 192;
				tex->buffer[(i*tex->sizex+j)*4+1] = 128;
				tex->buffer[(i*tex->sizex+j)*4+2] = 255;
				tex->buffer[(i*tex->sizex+j)*4+3] = 128+(int)(16*cos(sqrt(i*i+j*j)));
			}
		}
#endif
	}

	p = ea->nReallocMemory(buf, tex->sizex*4);
	if(p) {
		buf = p;
		p = tex->buffer;
		p2 = &tex->buffer[tex->sizex*4*(tex->sizey-1)];
		for(i = 0;i < tex->sizey/2;i++) {
			memcpy(buf, p, tex->sizex*4);
			memcpy(p, p2, tex->sizex*4);
			memcpy(p2, buf, tex->sizex*4);
			p += tex->sizex*4;
			p2 -= tex->sizex*4;
		}
	} else
		ea->nlPrint(L"%ls: %ls", L"kplib_pictures_plgLoad()", L"Warning: can't flip image");

	ea->nFreeMemory(buf);

	tex->nglcolorformat = NGL_COLORFORMAT_B8G8R8A8;
	tex->nglrowalignment = 4;

	ea->nlAddTab(-1); ea->nlPrint(L"%ls: %ls", L"kplib_pictures_plgLoad()", L"ok");

	return true;
}

