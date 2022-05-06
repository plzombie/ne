/*
	Файл	: plg_pcx.c

	Описание: Плагин для загрузки pcx

	История	: 17.07.12	Создан

*/

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "../extclib/_wcsicmp.h"

#include "../nyan/nyan_text.h"
#include "../nyan/nyan_log_publicapi.h"
#include "../nyan/nyan_filesys_publicapi.h"
#include "../nyan/nyan_mem_publicapi.h"
#include "../nyan/nyan_file_publicapi.h"
#include "../nyan/nyan_texformat_publicapi.h"
#include "../nyan/nyan_plgtypes_publicapi.h"

#include "plg_pcx.h"

typedef struct {
	unsigned char Manufacturer;	// Постоянный флаг 10 = ZSoft .PCX
	unsigned char Version;		// 0 = Версия 2.5
					// 2 = Версия 2.8 с информацией о палитре
					// 3 = Версия 2.8 без информации о палитре
					// 5 = Версия 3.0
	unsigned char Enc;		// 1 = .PCX кодирование длинными сериями
	unsigned char BitsPerLayer;	// Число бит на пиксел в слое
	short Xmin;			// Размеры изображения
	short Ymin;			// Размеры изображения
	short Xmax;			// Размеры изображения
	short Ymax;			// Размеры изображения
	short HRes;			// Горизонтальное разрешение создающего устройства
	short VRes;			// Вертикальное разрешение создающего устройства
	unsigned char Colormap[48];	// Набор цветовой палитры (см. далее текст)
	unsigned char Reserved;
	unsigned char NPlanes;		// Число цветовых слоев
	short BytesPerLine;		// Число байт на строку в цветовом слое
					// (для PCX-файлов всегда должно быть четным)
	short Pal;			// Как интерпретировать палитру:
					// 1 = цветная/черно-белая,
					// 2 = градации серого
	unsigned char Filler[58];	// Заполняется нулями до конца заголовка
} PCX_HEADER;

bool N_APIENTRY plgPCXSupportExt(const wchar_t *fname, const wchar_t *fext);
bool N_APIENTRY plgPCXLoad(const wchar_t *fname, nv_texture_type *tex);

nv_tex_plugin_type plgPCX = {sizeof(nv_tex_plugin_type), &plgPCXSupportExt, &plgPCXLoad};

/*
	Функция	: plgPCXSupportExt

	Описание: Проверяет поддержку типа файла

	История	: 17.07.12	Создан

*/
bool N_APIENTRY plgPCXSupportExt(const wchar_t *fname, const wchar_t *fext)
{
	(void)fname; // Неиспользуемая переменная

	if(_wcsicmp(fext,L"pcx") == 0) return true;
	return false;
}

/*
	Функция	: plgPCXLoad

	Описание: Загружает tga файл

	История	: 17.07.12	Создан

*/
bool N_APIENTRY plgPCXLoad(const wchar_t *fname, nv_texture_type *tex)
{
	unsigned int f;
	PCX_HEADER head;
	unsigned char *databuf = 0, *pdb = 0, *paldata = 0;
	uint64_t databuf_size, databuf_pos;
	uint64_t texture_size;
	unsigned char *planeBuf = 0;
	unsigned int i, len;

	nlPrint(LOG_FDEBUGFORMAT7, F_PLGPCXLOAD, N_FNAME, fname); nlAddTab(1);

	f = nFileOpen(fname);

	if(f == 0) {
		nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_PLGPCXLOAD, ERR_FILENOTFOUNDED);
		return false;
	}

	nFileRead(f,&head,sizeof(PCX_HEADER));

	if(head.Manufacturer != 10) {
		nFileClose(f);
		nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_PLGPCXLOAD, ERR_FILEISDAMAGED);
		return false;
	}

	if(head.Enc != 1) {
		nFileClose(f);
		nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT3,F_PLGPCXLOAD,ERR_UNSUPPORTEDENCODING,(int)head.Enc);
		return false;
	}

	if(head.Version != 5) {
		nFileClose(f);
		nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT3,F_PLGPCXLOAD,ERR_UNSUPPORTEDVERSION,(int)head.Version);
		return false;
	}

	if((head.BitsPerLayer != 8) || ((head.NPlanes != 1) && (head.NPlanes != 3))) {
		nFileClose(f);
		nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT3,F_PLGPCXLOAD,ERR_UNSUPPORTEDCOLOR,(int)head.NPlanes);
		return false;
	}

	if(((unsigned short)head.BytesPerLine <= (int)(unsigned short)head.Xmax-(int)(unsigned short)head.Xmin) || ((unsigned short)head.Xmax < (unsigned short)head.Xmin) || ((unsigned short)head.Ymax < (unsigned short)head.Ymin)) {
		nFileClose(f);
		nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_PLGPCXLOAD, ERR_FILEISDAMAGED);
		return false;
	}

	databuf_size = (uint64_t)nFileLength(f)-sizeof(PCX_HEADER);
	if(databuf_size > SIZE_MAX) {
		nFileClose(f);
		nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_PLGPCXLOAD, ERR_FILEISTOOLARGE);
		return false;
	}
	databuf_pos = 1;
	databuf = nAllocMemory((size_t)databuf_size);
	if(!databuf) {
		nFileClose(f);
		nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT,F_PLGPCXLOAD,N_FALSE);
		return false;
	}
	nFileRead(f, databuf, (size_t)databuf_size);
	nFileClose(f);

	if(head.NPlanes == 1) {
		if(databuf_size < 769) {
			nFreeMemory(databuf);
			nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_PLGPCXLOAD, ERR_FILEISDAMAGED);
			return false;
		}
		paldata = &databuf[databuf_size-768];
		if(databuf[databuf_size-769] == 10) {
			unsigned char *p = 0;
			p = paldata;
			for(i=0;i<768;i++) { *p = *p<<2; p++; }
		}
	}

	tex->nglcolorformat = NGL_COLORFORMAT_R8G8B8;

	tex->sizex = (int)(unsigned short)head.Xmax-(int)(unsigned short)head.Xmin+1;
	tex->sizey = (int)(unsigned short)head.Ymax-(int)(unsigned short)head.Ymin+1;

	if((tex->sizex*head.NPlanes)%4 == 0) tex->nglrowalignment = 4; else tex->nglrowalignment = 1;

	texture_size = (uint64_t)tex->sizex*(uint64_t)tex->sizey*(uint64_t)3;
	if(texture_size > SIZE_MAX) {
		nFreeMemory(databuf);
		nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_PLGPCXLOAD, ERR_FILEISTOOLARGE);
		return false;
	}
	tex->buffer = nAllocMemory((size_t)texture_size);
	if(!tex->buffer) {
		nFreeMemory(databuf);
		nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT,F_PLGPCXLOAD,N_FALSE);
		return false;
	}

	planeBuf = nAllocMemory((size_t)head.NPlanes*(size_t)(unsigned short)head.BytesPerLine);
	if(!planeBuf) {
		nFreeMemory(databuf);
		nFreeMemory(tex->buffer);
		nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT,F_PLGPCXLOAD,N_FALSE);
		return false;
	}

	pdb = databuf;
	for(i=1;i<=tex->sizey;i++) {
		unsigned char *pB = 0, *pB2 = 0, *pB3 = 0, *p = 0;
		unsigned int j, k;
		j = 0;
		pB = planeBuf;

		while(j < (size_t)head.NPlanes*(size_t)(unsigned short)head.BytesPerLine) {
			if(databuf_pos > databuf_size) {
				nFreeMemory(planeBuf);
				nFreeMemory(databuf);
				nFreeMemory(tex->buffer);
				nlPrint(L"%s: pos %d size %d",F_PLGPCXLOAD,databuf_pos,databuf_size);
				nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT,F_PLGPCXLOAD,ERR_FILEISDAMAGED);
				return false;
			}
			if(*pdb > 192) {
				len = *pdb-192;
				if(j+len > (unsigned int)head.NPlanes*head.BytesPerLine || 1+databuf_pos > databuf_size) {
					nFreeMemory(planeBuf);
					nFreeMemory(databuf);
					nFreeMemory(tex->buffer);
					nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT,F_PLGPCXLOAD,ERR_FILEISDAMAGED);
					return false;
				}
				pdb++;
				for(k=0;k<len;k++) {
					*pB = *pdb; pB++;
				}
				j += len;
				databuf_pos+=2;
			} else {
				*pB = *pdb; pB++; databuf_pos++;
				j++;
			}
			pdb++;
		}

		p = tex->buffer+(size_t)tex->sizex*3*(tex->sizey-i);
		pB = planeBuf;
		if(head.NPlanes == 1) // 256 цветов
			for(j=0;j<tex->sizex;j++) {
				*p = paldata[(*pB)*3]; p++;
				*p = paldata[(*pB)*3+1]; p++;
				*p = paldata[(*pB)*3+2]; p++;
				pB++;
			}
		else if(head.NPlanes == 3) { // truecolor
			pB2 = planeBuf+(size_t)(unsigned short)head.BytesPerLine;
			pB3 = planeBuf+2*(size_t)(unsigned short)head.BytesPerLine;
			for(j=0;j<tex->sizex;j++) {
				*p = *pB; p++; pB++;
				*p = *pB2; p++; pB2++;
				*p = *pB3; p++; pB3++;
			}
		}
	}

	nFreeMemory(planeBuf);
	nFreeMemory(databuf);

	nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT,F_PLGPCXLOAD,N_OK);

	return true;
}
