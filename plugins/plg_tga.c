/*
	Файл	: plg_tga.c

	Описание: Плагин для загрузки tga

	История	: 15.06.12	Создан

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

#include "plg_tga.h"

#pragma pack (push, 1)
typedef struct {
	unsigned char IdLeight;		//Длина информации после заголовка
	unsigned char ColorMap;		//Идентификатор наличия цветовой карты (0 - нет, 1 - есть)
	unsigned char DataType;		//Тип сжатия
								//   0 - No Image Data Included
								//   1 - Uncompressed, Color-mapped Image
								//   2 - Uncompressed, True-color Image
								//   3 - Uncompressed, Black-and-white Image
								//   9 - Run-length encoded, Color-mapped Image
								//   10 - Run-length encoded, True-color Image
								//   11 - Run-length encoded, Black-and-white Image
	unsigned short CmapStart;	//Начало палитры
	unsigned short CmapLength;	//Длина палитры
	unsigned char CmapDepth;	//Глубина элементов палитры (15, 16, 24, 32)
	unsigned short X_Origin;	//Начало изображения по оси X
	unsigned short Y_Origin;	//Начало изображения по оси Y
	unsigned short TGAWidth;	//Ширина изображения
	unsigned short TGAHeight;	//Высота изображения
	unsigned char BitPerPel;	//Кол-во бит на пиксель (8, 16, 24, 32)
	unsigned char Description;	//Описание
} TGAHEADER;
#pragma pack (pop)

bool N_APIENTRY plgTGASupportExt(const wchar_t *fname, const wchar_t *fext);
bool N_APIENTRY plgTGALoad(const wchar_t *fname, nv_texture_type *tex);

nv_tex_plugin_type plgTGA = {sizeof(nv_tex_plugin_type), &plgTGASupportExt, &plgTGALoad};

/*
	Функция	: plgTGASupportExt

	Описание: Проверяет поддержку типа файла

	История	: 16.06.12	Создан

*/
bool N_APIENTRY plgTGASupportExt(const wchar_t *fname, const wchar_t *fext)
{
	(void)fname; // Неиспользуемая переменная

	if(_wcsicmp(fext,L"tga") == 0) return true;
	if(_wcsicmp(fext,L"tpic") == 0) return true;
	return false;
}

/*
	Функция	: plgTGALoad

	Описание: Загружает tga файл

	История	: 15.06.12	Создан

*/
bool N_APIENTRY plgTGALoad(const wchar_t *fname, nv_texture_type *tex)
{
	unsigned int f;
	TGAHEADER head;
	unsigned int bpp;
	unsigned char *pal = 0;
	unsigned char *databuf = 0, *pdb = 0;
	uint64_t databuf_size, databuf_pos;
	uint64_t texture_size;
	unsigned char *temp, *p, *p2, *p3;
	unsigned char b;
	unsigned int i, j, k;

	nlPrint(LOG_FDEBUGFORMAT7,F_PLGTGALOAD,N_FNAME,fname); nlAddTab(1);

	f = nFileOpen(fname);

	if(f == 0) {
		nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT,F_PLGTGALOAD,ERR_FILENOTFOUNDED);
		return false;
	}

	nFileRead(f,&head,sizeof(TGAHEADER));

	switch(head.DataType) { // Проверка правильности\поддержки файла
		case 1: // Проверяю изображения с палитрой
		case 9:
			if((head.ColorMap != 1) || (head.BitPerPel != 8)) {
				nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT,F_PLGTGALOAD,ERR_FILEISDAMAGED);
				nFileClose(f);
				return false;
			}
			if(!((head.CmapDepth == 24) || (head.CmapDepth == 32))) {
				nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT3,F_PLGTGALOAD,ERR_UNSUPPORTEDPALETTE,head.BitPerPel);
				nFileClose(f);
				return false;
			}
			break;
		case 2: // Проверяю изображения без палитры (24,32 bpp)
		case 10:
			if(head.ColorMap != 0) {
				nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT,F_PLGTGALOAD,ERR_FILEISDAMAGED);
				nFileClose(f);
				return false;
			}
			if(!((head.BitPerPel == 24) || (head.BitPerPel == 32))) {
				nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT3,F_PLGTGALOAD,ERR_UNSUPPORTEDCOLOR,head.BitPerPel);
				nFileClose(f);
				return false;
			}
			break;
		case 3: // Проверяю изображения без палитры (8 bpp, greyscale)
		case 11:
			if(head.ColorMap != 0) {
				nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT,F_PLGTGALOAD,ERR_FILEISDAMAGED);
				nFileClose(f);
				return false;
			}
			if(head.BitPerPel != 8) {
				nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT3,F_PLGTGALOAD,ERR_UNSUPPORTEDCOLOR,head.BitPerPel);
				nFileClose(f);
				return false;
			}
			break;
		default:
			nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT3,F_PLGTGALOAD,ERR_UNSUPPORTEDDATATYPE,head.DataType);
			nFileClose(f);
			return false;
	}

	if(head.TGAWidth == 0 || head.TGAHeight == 0) {
		nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_PLGTGALOAD, ERR_FILEISDAMAGED);
		nFileClose(f);
		return false;
	}

	if((head.BitPerPel == 8) && (head.ColorMap == 0)) { // Настройка для greyscale-изображений
		tex->nglcolorformat = NGL_COLORFORMAT_L8;
		bpp = 1;
	} else if((head.BitPerPel == 32) || ((head.BitPerPel == 8) && (head.CmapDepth == 32))) { // Настройка 32bpp изображений (и палитрой или без)
		tex->nglcolorformat = NGL_COLORFORMAT_B8G8R8A8;
		bpp = 4;
	} else { // Настройка 24bpp изображений (и палитрой или без)
		tex->nglcolorformat = NGL_COLORFORMAT_B8G8R8;
		bpp = 3;
	}

	tex->sizex = head.TGAWidth;
	tex->sizey = head.TGAHeight;

	if((tex->sizex*bpp)%4 == 0) tex->nglrowalignment = 4; else tex->nglrowalignment = 1;

	texture_size = (uint64_t)tex->sizex*(uint64_t)tex->sizey*(uint64_t)bpp;

	if(texture_size > SIZE_MAX) {
		nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_PLGTGALOAD, ERR_FILEISTOOLARGE);
		nFileClose(f);
		return false;
	}

	tex->buffer = nAllocMemory((size_t)texture_size);
	if(!tex->buffer) {
		nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT,F_PLGTGALOAD,N_FALSE);
		nFileClose(f);
		return false;
	}
	if(head.ColorMap == 1) {
		pal = nAllocMemory(head.CmapLength*bpp);
		if(!pal) {
			nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT,F_PLGTGALOAD,N_FALSE);
			nFreeMemory(tex->buffer);
			nFileClose(f);
			return false;
		}
		nFileRead(f, pal, head.CmapLength*bpp);
	}

	// Пропускаю идентификатор
	nFileSeek(f, head.IdLeight, FILE_SEEK_CUR);

	switch(head.DataType) { // Чтение изображения
		case 1: // Чтение изображения с палитрой
			temp = nAllocMemory(tex->sizex*tex->sizey); // для tga - макс. 65535*65535<UINT_MAX, проверка не нужна
			if(!temp) {
				nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT,F_PLGTGALOAD,N_FALSE);
				nFreeMemory(tex->buffer);
				nFreeMemory(pal);
				nFileClose(f);
				return false;
			}
			nFileRead(f,temp,tex->sizex*tex->sizey);
			p = tex->buffer;
			p3 = temp;
			for(i = 0;i < tex->sizex*tex->sizey;i++) {
				unsigned int cmap_offset;

				cmap_offset = *p3-head.CmapStart;
				if(cmap_offset >= head.CmapLength || *p3 < head.CmapStart) {
					nlPrint(LOG_FDEBUGFORMAT, F_PLGTGALOAD, ERR_FILEISDAMAGED);
					break;
				}
				p2 = pal+((cmap_offset)*bpp);
				*p = *p2; p++; p2++;
				*p = *p2; p++; p2++;
				*p = *p2; p++;
				if(bpp == 4) {  p2++; *p = *p2; p++; }
				p3++;
			}
			nFreeMemory(temp);
			break;
		case 2: // Чтение 8(greyscale),24,32bit изображений
		case 3:
			nFileRead(f, tex->buffer, (size_t)texture_size);
			break;
		case 9: // Декодирование изображения с палитрой
			databuf_pos = 0;
			databuf_size = (unsigned int)nFileLength(f)-(unsigned int)nFileTell(f);
			if(databuf_size > SIZE_MAX) {
				nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_PLGTGALOAD, ERR_FILEISTOOLARGE);
				nFreeMemory(tex->buffer);
				nFreeMemory(pal);
				nFileClose(f);
				return false;
			}
			databuf = nAllocMemory((size_t)databuf_size);
			if(!databuf) {
				nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT,F_PLGTGALOAD,N_FALSE);
				nFreeMemory(tex->buffer);
				nFreeMemory(pal);
				nFileClose(f);
				return false;
			}
			nFileRead(f, databuf, (size_t)databuf_size);

			pdb = databuf;
			i = 0;
			p = tex->buffer;
			while(i < tex->sizex*tex->sizey) {
				if(databuf_pos >= databuf_size) { // databuf_pos+1 > databuf_size
					nlPrint(LOG_FDEBUGFORMAT, F_PLGTGALOAD, ERR_FILEISDAMAGED);
					break;
				}
				b = *pdb; pdb++;
				databuf_pos += 1;
				if((i+(b&0x7F)+1) > tex->sizex*tex->sizey) {
					nlPrint(LOG_FDEBUGFORMAT,F_PLGTGALOAD,ERR_FILEISDAMAGED);
					break;
				}
				if(b & 0x80) { // the packet is a Run-length Packet
					unsigned int cmap_offset;

					if(databuf_pos >= databuf_size) { // databuf_pos+1 > databuf_size
						nlPrint(LOG_FDEBUGFORMAT, F_PLGTGALOAD, ERR_FILEISDAMAGED);
						break;
					}

					cmap_offset = (*pdb)-head.CmapStart;
					if(cmap_offset >= head.CmapLength || (*pdb) < head.CmapStart) {
						nlPrint(LOG_FDEBUGFORMAT, F_PLGTGALOAD, ERR_FILEISDAMAGED);
						break;
					}

					for(j = 0;j < (unsigned int)((b&0x7F)+1);j++) {
						p2 = pal+((cmap_offset)*bpp);
						*p = *p2; p++; p2++;
						*p = *p2; p++; p2++;
						*p = *p2; p++; p2++;
						if(bpp == 4) { *p = *p2; p++; p2++; }
					}
					pdb++;
					databuf_pos += 1;
				} else { // the packet is a Raw Packet
					if(databuf_pos+((b&0x7F)+1) > databuf_size) {
						nlPrint(LOG_FDEBUGFORMAT, F_PLGTGALOAD, ERR_FILEISDAMAGED);
						break;
					}
					for(j = 0;j < (unsigned int)((b&0x7F)+1);j++) {
						unsigned int cmap_offset;

						cmap_offset = pdb[j]-head.CmapStart;
						if(cmap_offset >= head.CmapLength || pdb[j] < head.CmapStart) {
							nlPrint(LOG_FDEBUGFORMAT, F_PLGTGALOAD, ERR_FILEISDAMAGED);
							break;
						}
						p2 = pal+((cmap_offset)*bpp);
						*p = *p2; p++; p2++;
						*p = *p2; p++; p2++;
						*p = *p2; p++; p2++;
						if(bpp == 4) { *p = *p2; p++; p2++; }
					}
					if(j < (unsigned int)((b&0x7F)+1))
						break;
					pdb += (b&0x7F)+1;
					databuf_pos += (b&0x7F)+1;
				}
				i += (b&0x7F)+1;
			}
			nFreeMemory(databuf);
			break;
		case 10: // Чтение 8(greyscale),24,32bit изображений
		case 11:
			databuf_pos = 0;
			databuf_size = (unsigned int)nFileLength(f)-(unsigned int)nFileTell(f);
			if(databuf_size > SIZE_MAX) {
				nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_PLGTGALOAD, ERR_FILEISTOOLARGE);
				nFreeMemory(tex->buffer);
				nFileClose(f);
				return false;
			}
			databuf = nAllocMemory((size_t)databuf_size);
			if(!databuf) {
				nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT,F_PLGTGALOAD,N_FALSE);
				nFreeMemory(tex->buffer);
				nFileClose(f);
				return false;
			}
			nFileRead(f, databuf, (size_t)databuf_size);

			pdb = databuf;
			i = 0;
			p = tex->buffer;
			while(i < tex->sizex*tex->sizey) {
				if(databuf_pos >= databuf_size) { // databuf_pos+1 > databuf_size
					nlPrint(LOG_FDEBUGFORMAT, F_PLGTGALOAD, ERR_FILEISDAMAGED);
					break;
				}
				b = *pdb; pdb++;
				databuf_pos += 1;
				if((i+(b&0x7F)+1) > tex->sizex*tex->sizey) {
					nlPrint(LOG_FDEBUGFORMAT,F_PLGTGALOAD,ERR_FILEISDAMAGED);
					break;
				}
				if(b & 0x80) { // the packet is a Run-length Packet
					if(databuf_pos+bpp > databuf_size) {
						nlPrint(LOG_FDEBUGFORMAT, F_PLGTGALOAD, ERR_FILEISDAMAGED);
						break;
					}
					for(j = 0;j < (unsigned int)((b&0x7F)+1);j++) {
						p2 = pdb;
						for(k = 0;k < bpp;k++) {
							*p = *p2; p++; p2++; }
					}
					pdb += bpp;
					databuf_pos += bpp;
				} else { // the packet is a Raw Packet
					if(databuf_pos+bpp*((b&0x7F)+1) > databuf_size) {
						nlPrint(LOG_FDEBUGFORMAT, F_PLGTGALOAD, ERR_FILEISDAMAGED);
						break;
					}
					memcpy(p, pdb, bpp*((b&0x7F)+1)); pdb += bpp*((b&0x7F)+1); p += bpp*((b&0x7F)+1);
					databuf_pos += bpp*((b&0x7F)+1);
				}
				i += (b&0x7F)+1;
			}
			nFreeMemory(databuf);
			break;
	}

	// Переворот по оси y (если необходимо, гимп может сохранять с этим флагом)
	if(head.Description & 0x20) {
		p = tex->buffer;
		temp = nAllocMemory(tex->sizex*bpp);
		if(temp) { // Можно здесь завершить работу функции, если !temp -^_^-
			p2 = &tex->buffer[(size_t)tex->sizex*(size_t)bpp*(size_t)(tex->sizey-1)];
			for(i = 0;i < tex->sizey/2;i++) {
				memcpy(temp,p,tex->sizex*bpp);
				memcpy(p,p2,tex->sizex*bpp);
				memcpy(p2,temp,tex->sizex*bpp);
				p += tex->sizex*bpp;
				p2 -= tex->sizex*bpp;
			}
			nFreeMemory(temp);
		}
	}

	if(head.ColorMap == 1) nFreeMemory(pal);
	nFileClose(f);

	nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT,F_PLGTGALOAD,N_OK);

	return true;
}
