/*
	Файл	: dll_plg_rix.c

	Описание: Плагин для загрузки rix файлов

	История	: 25.07.12	Создан

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

#include "dll_plg_rix.h"

#pragma pack (push, 2)
typedef struct {
	unsigned int Sign;
	unsigned short Width; //ширина картинки (640 пикселей, 0x280)
    unsigned short Height; //высота картинки (480 пикселей, 0x1e0)
    short Unknown;
} RIX_HEADER;
#pragma pack (pop)

#define RIX_ID 0x33584952

engapi_type *ea = 0;

bool N_APIENTRY rix_plgSupportExt(const wchar_t *fname, const wchar_t *fext);
bool N_APIENTRY rix_plgLoad(const wchar_t *fname, nv_texture_type *tex);

nv_tex_plugin_type plg = {sizeof(nv_tex_plugin_type), &rix_plgSupportExt, &rix_plgLoad};

plg_api_type plg_api = {sizeof(plg_api_type), N_PLG_TYPE_TEXLOADER, L"ColorRix *.rix files", &plg};

/*
	Функция	: plgGetApi

	Описание: Возвращает API плагина

	История	: 14.05.13	Создан

*/
N_API plg_api_type * N_APIENTRY plgGetApi(void)
{
	if(!ea) return 0;

	return &plg_api;
}

/*
	Функция	: plgSetupDll

	Описание: Настройка dll плагина

	История	: 19.08.12	Создан

*/
N_API bool N_APIENTRY plgSetupDll(engapi_type *engapi)
{
	if(engapi) {
		if(engapi->mysize == sizeof(engapi_type)) {
			ea = engapi;

			return true;
		}
	}

	return false;
}

/*
	Функция	: rix_plgSupportExt

	Описание: Проверяет поддержку типа файла

	История	: 25.07.12	Создан

*/
bool N_APIENTRY rix_plgSupportExt(const wchar_t *fname, const wchar_t *fext)
{
	(void)fname; // Неиспользуемая переменная

	if(_wcsicmp(fext,L"rix") == 0) return true;
	return false;
}

/*
	Функция	: rix_plgLoad

	Описание: Загружает rix файл

	История	: 25.07.12	Создан

*/
bool N_APIENTRY rix_plgLoad(const wchar_t *fname, nv_texture_type *tex)
{
	unsigned int f;
	unsigned char pal[768];
	unsigned char *temp, *p, *p2, *p3;
	unsigned int i, j;
	RIX_HEADER head;

	if(!ea) return false;

	ea->nlPrint(L"%ls %ls %ls", L"rix_plgLoad()", L"fname", fname); ea->nlAddTab(1);

	f = ea->nFileOpen(fname);

	if(f == 0) {
		ea->nlAddTab(-1); ea->nlPrint(L"%ls: %ls", L"rix_plgLoad()", L"File not founded");
		return false;
	}

	ea->nFileRead(f,&head,sizeof(RIX_HEADER));

	if(head.Sign != RIX_ID) {
		ea->nlAddTab(-1); ea->nlPrint(L"%ls: %ls", L"rix_plgLoad()", L"Wrong signature");
		ea->nFileClose(f);
		return false;
	}

	if(head.Unknown != 175) {
		ea->nlAddTab(-1); ea->nlPrint(L"%ls: %ls", L"rix_plgLoad()", L"Inknown file type");
		ea->nFileClose(f);
		return false;
	}

	tex->nglcolorformat = NGL_COLORFORMAT_R8G8B8;

	tex->sizex = head.Width;
	tex->sizey = head.Height;

	if((tex->sizex*3)%4 == 0) tex->nglrowalignment = 4; else tex->nglrowalignment = 1;

	tex->buffer = ea->nAllocMemory(tex->sizex*tex->sizey*3);

	if(!tex->buffer) {
		ea->nlAddTab(-1); ea->nlPrint(L"%ls: %ls", L"rix_plgLoad()", L"false");
		ea->nFileClose(f);
		return false;
	}

	ea->nFileRead(f,pal,768);

	p = pal;
	for(i=0;i<768;i++) {
		*p = (*p)<<2; p++;
	}

	temp = ea->nAllocMemory(tex->sizex*tex->sizey);

	if(!temp) {
		ea->nFreeMemory(tex->buffer);
		ea->nlAddTab(-1); ea->nlPrint(L"%ls: %ls", L"rix_plgLoad()", L"false");
		ea->nFileClose(f);
		return false;
	}

	ea->nFileRead(f,temp,tex->sizex*tex->sizey);
	p3 = temp;
	p = &tex->buffer[tex->sizex*(tex->sizey-1)*3];
	for(i=tex->sizey;i>0;i--) {
		for(j=0;j<tex->sizex;j++) {
			p2 = pal+((*p3)*3); p3++;
			*p = *p2; p++; p2++;
			*p = *p2; p++; p2++;
			*p = *p2; p++; p2++;
		}
		p -= tex->sizex*3*2;
	}
	ea->nFreeMemory(temp);

	ea->nFileClose(f);

	ea->nlAddTab(-1); ea->nlPrint(L"%ls: %ls", L"rix_plgLoad()", L"ok");

	return true;
}

