/*
	Файл	: dll_plg_stb_image.c

	Описание: Плагин для загрузки изображений через stb_image.h
		Ссылка: https://github.com/nothings/stb/

	История	: 23.12.16	Создан

*/

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
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

#ifdef __WATCOMC__
	#define STBI_NO_SIMD
#endif
#define STB_IMAGE_IMPLEMENTATION
#include "../forks/stb/stb_image.h"

#include "dll_plg_stb_image.h"

engapi_type *ea = 0;

bool N_APIENTRY stb_image_plgSupportExt(const wchar_t *fname, const wchar_t *fext);
bool N_APIENTRY stb_image_plgLoad(const wchar_t *fname, nv_texture_type *tex);

nv_tex_plugin_type plg = {sizeof(nv_tex_plugin_type), &stb_image_plgSupportExt, &stb_image_plgLoad};

plg_api_type plg_api = {sizeof(plg_api_type), N_PLG_TYPE_TEXLOADER, L"Image files via stb_image (*.jpeg, *.png, *.tga, *.bmp, *.psd, *.gif, *.hdr, *.pic, *.pnm)", &plg};

/*
	Функция	: plgGetApi

	Описание: Возвращает API плагина

	История	: 23.12.16	Создан

*/
N_API plg_api_type * N_APIENTRY plgGetApi(void)
{
	if(!ea) return 0;

	return &plg_api;
}

/*
	Функция	: plgSetupDll

	Описание: Настройка dll плагина

	История	: 23.12.16	Создан

*/
N_API bool N_APIENTRY plgSetupDll(engapi_type *engapi)
{
	if(engapi) {
		if(engapi->mysize == sizeof(engapi_type)) {
			ea = engapi;

			ea->nlPrint(L"%ls", L"stb_image - public domain image loader - http://nothings.org/stb_image.h");

			return true;
		}
	}
	
	return false;
}

/*
	Функция	: stb_image_plgSupportExt

	Описание: Проверяет поддержку типа файла

	История	: 23.12.16	Создан

*/
bool N_APIENTRY stb_image_plgSupportExt(const wchar_t *fname, const wchar_t *fext)
{
	(void)fname; // Неиспользуемая переменная

#ifndef STBI_NO_JPEG
	if(_wcsicmp(fext,L"jpg") == 0) return true; // stb> baseline & progressive (12 bpc/arithmetic not supported, same as stock IJG lib)
	if(_wcsicmp(fext,L"jpeg") == 0) return true; // алиас jpg
#endif
#ifndef STBI_NO_PNG
	if(_wcsicmp(fext,L"png") == 0) return true; // stb> 1/2/4/8-bit-per-channel (16 bpc not supported)
#endif
#ifndef STBI_NO_TGA
	if(_wcsicmp(fext,L"tga") == 0) return true; // stb> not sure what subset, if a subset
	if(_wcsicmp(fext,L"tpic") == 0) return true; // алиас tga
#endif
#ifndef STBI_NO_BMP
	if(_wcsicmp(fext,L"bmp") == 0) return true; // stb> non-1bpp, non-RLE
#endif
#ifndef STBI_NO_PSD
	if(_wcsicmp(fext,L"psd") == 0) return true; // stb> composited view only, no extra channels, 8/16 bit-per-channel
#endif
#ifndef STBI_NO_GIF
	if(_wcsicmp(fext,L"gif") == 0) return true; // stb> *comp always reports as 4-channel
#endif
#ifndef STBI_NO_HDR
	if(_wcsicmp(fext,L"hdr") == 0) return true; // stb> radiance rgbE format
#endif
#ifndef STBI_NO_PIC
	if(_wcsicmp(fext,L"pic") == 0) return true; // stb> Softimage PIC
#endif
#ifndef STBI_NO_PNM
	if(_wcsicmp(fext,L"pnm") == 0) return true; // stb> PPM and PGM binary only
	if(_wcsicmp(fext,L"ppm") == 0) return true; // алиас pnm
	if(_wcsicmp(fext,L"pgm") == 0) return true; // алиас pnm
#endif
	return false;
}

/*
	Функция	: stb_image_plgLoad

	Описание: Загружает изображение

	История	: 23.12.16	Создан

*/
bool N_APIENTRY stb_image_plgLoad(const wchar_t *fname, nv_texture_type *tex)
{
	unsigned int f;
	unsigned char *buf, *p_in, *p_out;
	int length, i;
	long long length_ll;

	int sizex, sizey, noc;
	unsigned char *data;

	if(!ea) return false;

	ea->nlPrint(L"%ls %ls %ls", L"stb_image_plgLoad()", L"fname", fname); ea->nlAddTab(1);

	f = ea->nFileOpen(fname);

	if(f == 0) {
		ea->nlAddTab(-1); ea->nlPrint(L"%ls: %ls", L"stb_image_plgLoad()", L"File not founded");
		return false;
	}

	// Чтение файла в память
	length_ll = ea->nFileLength(f);
	if(length_ll <= INT_MAX)
		length = (int)length_ll;
	else {
		ea->nlAddTab(-1); ea->nlPrint(L"%ls: %ls", L"stb_image_plgLoad()", L"file is too large");
		ea->nFileClose(f);
		return false;
	}

	buf = ea->nAllocMemory(length);

	if(!buf) {
		ea->nlAddTab(-1); ea->nlPrint(L"%ls: %ls", L"stb_image_plgLoad()", L"false");
		ea->nFileClose(f);
		return false;
	}

	ea->nFileRead(f, buf, length);

	ea->nFileClose(f);

	// Читаем изображение из памяти
	data = stbi_load_from_memory(buf, length, &sizex, &sizey, &noc, 0);

	ea->nFreeMemory(buf);

	if(!data) {
		ea->nlAddTab(-1); ea->nlPrint(L"%ls: %ls", L"stb_image_plgLoad()", L"can\'t load image from memory");
		return false;
	}

	if(noc == 0 || noc > 4) {
		ea->nlAddTab(-1); ea->nlPrint(L"%ls: %ls", L"stb_image_plgLoad()", L"unknown color format");
		stbi_image_free(data);
		return false;
	}

	tex->sizex = sizex;
	tex->sizey = sizey;
	switch(noc) {
		case 1:
			tex->nglcolorformat = NGL_COLORFORMAT_L8;
			break;
		case 2:
			tex->nglcolorformat = NGL_COLORFORMAT_L8A8;
			break;
		case 3:
			tex->nglcolorformat = NGL_COLORFORMAT_R8G8B8;
			break;
		case 4:
			tex->nglcolorformat = NGL_COLORFORMAT_R8G8B8A8;
			break;
	}
	tex->nglrowalignment = 1;
	tex->buffer = ea->nAllocMemory(sizex*sizey*noc);

	if(!tex->buffer) {
		ea->nlAddTab(-1); ea->nlPrint(L"%ls: %ls", L"stb_image_plgLoad()", L"false");
		stbi_image_free(data);
		return false;
	}

	p_in = data+(sizey-1)*sizex*noc;
	p_out = tex->buffer;
	for(i = 0; i < sizey; i++) {
		memcpy(p_out, p_in, sizex*noc);
		p_out += sizex*noc;
		p_in -= sizex*noc;
	}

	stbi_image_free(data);

	ea->nlAddTab(-1); ea->nlPrint(L"%ls: %ls", L"stb_image_plgLoad()", L"ok");

	return true;
}

