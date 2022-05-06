/*
	Файл	: plg_bmp.c

	Описание: Плагин для загрузки bmp

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

#include "plg_bmp.h"

#define BITMAP_ID 0x4D42

typedef struct {
	int biSize;
	int biWidth;
	int biHeight;
	short biPlanes;
	short biBitCount;
	int biCompression;
	int biSizeImage;
	int biXPelsPerMeter;
	int biYPelsPerMeter;
	int biClrUsed;
	int biClrImportant;
} BMP_BITMAPINFOHEADER;

#pragma pack (push, 2)
typedef struct {
	short bfType;
	int bfSize;
	short bfReserved1;
	short bfReserved2;
	int bfOffBits;
} BMP_BITMAPFILEHEADER ;
#pragma pack (pop)

bool N_APIENTRY plgBMPSupportExt(const wchar_t *fname, const wchar_t *fext);
bool N_APIENTRY plgBMPLoad(const wchar_t *fname, nv_texture_type *tex);

nv_tex_plugin_type plgBMP = {sizeof(nv_tex_plugin_type), &plgBMPSupportExt, &plgBMPLoad};

/*
	Функция	: plgBMPSupportExt

	Описание: Проверяет поддержку типа файла

	История	: 01.08.12	Создан

*/
bool N_APIENTRY plgBMPSupportExt(const wchar_t *fname, const wchar_t *fext)
{
	(void)fname; // Неиспользуемая переменная

	if(_wcsicmp(fext,L"bmp") == 0) return true;
	return false;
}

/*
	Функция	: plgBMPLoad

	Описание: Загружает bmp файл

	История	: 01.08.12	Создан

*/
bool N_APIENTRY plgBMPLoad(const wchar_t *fname, nv_texture_type *tex)
{
	unsigned int f, i, j;
	int bitpos;
	uint64_t texsize = 0; // Размер текстуры в байтах
	uint64_t ptexsize = 0, ptexal = 0; // Размер изображения в байтах и количество байт, необходимых для выравнивания строки
	unsigned char *temp, *p, *p2, *p3;
	bool reset_alpha = false; // true, если альфаканал надо заполнить значением 0xff

	BMP_BITMAPFILEHEADER fheader;
	BMP_BITMAPINFOHEADER iheader;
	unsigned char pal[1024];

	nlPrint(LOG_FDEBUGFORMAT7,F_PLGBMPLOAD,N_FNAME,fname); nlAddTab(1);

	f = nFileOpen(fname);

	if(f == 0) {
		nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT,F_PLGBMPLOAD,ERR_FILENOTFOUNDED);
		return false;
	}

	nFileRead(f,&fheader,sizeof(fheader));

	if(fheader.bfType != BITMAP_ID) {
		nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT,F_PLGBMPLOAD,ERR_WRONGSIGNIN);
		nFileClose(f);
		return false;
	}

	nFileRead(f,&iheader,sizeof(iheader));

	if(iheader.biCompression != 0 && iheader.biCompression != 3 && iheader.biCompression != 6) {
		nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT,F_PLGBMPLOAD,ERR_UNSUPPORTEDCOLOR);
		nFileClose(f);
		return false;
	}

	//Загрузка палитры и установка параметров текстуры
	switch(iheader.biBitCount) {
		case 1:
		case 4:
		case 8:
			texsize = (uint64_t)iheader.biWidth*(uint64_t)iheader.biHeight*3;

			tex->nglcolorformat = NGL_COLORFORMAT_B8G8R8;

			if(((int)iheader.biWidth*3)%4 == 0) tex->nglrowalignment = 4; else tex->nglrowalignment = 1;

			ptexsize = (uint64_t)iheader.biBitCount*(uint64_t)iheader.biWidth;

			if(ptexsize%8)
				ptexsize = ptexsize/8+1;
			else
				ptexsize /= 8;

			ptexal = (4-ptexsize%4)%4; // Выравнивание строки в bmp файле

			ptexsize = (ptexsize+ptexal)*(uint64_t)iheader.biHeight; // Размер изображения в bmp файле

			if(ptexsize > SIZE_MAX) {
				nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_PLGBMPLOAD, ERR_FILEISTOOLARGE);
				nFileClose(f);
				return false;
			}

			if(iheader.biSize > 40) // Версия bmp выше 3-й
				nFileSeek(f,iheader.biSize-40,FILE_SEEK_CUR);
			else if(iheader.biCompression == 3) // 3-я версия bmp 
				nFileSeek(f,12,FILE_SEEK_CUR);
			else if(iheader.biCompression == 6)
				nFileSeek(f,16,FILE_SEEK_CUR);

			nFileRead(f,pal,(1<<iheader.biBitCount)*4); // Чтение палитры

			break;
		case 16:
			if(iheader.biCompression == 0) {
				tex->nglcolorformat = NGL_COLORFORMAT_X1R5G5B5;
			} else {
				unsigned int mask[4];
				
				if(iheader.biSize > 40 || iheader.biCompression == 6) { // Версия bmp выше 3-й
					nFileRead(f,mask,4*sizeof(unsigned int));
				} else {
					nFileRead(f,&mask,3*sizeof(unsigned int));
					mask[3] = 0;
				}
				
				if(mask[0] == 0x7c00 && mask[1] == 0x03e0 && mask[2] == 0x001f && mask[3] == 0x8000)
					tex->nglcolorformat = NGL_COLORFORMAT_X1R5G5B5; // NGL_COLORFORMAT_A1R5G5B5;
				else if(mask[0] == 0x7c00 && mask[1] == 0x03e0 && mask[2] == 0x001f && mask[3] == 0x0000)
					tex->nglcolorformat = NGL_COLORFORMAT_X1R5G5B5;
				else if(mask[0] == 0xf800 && mask[1] == 0x07e0 && mask[2] == 0x001f && mask[3] == 0x0000)
					tex->nglcolorformat = NGL_COLORFORMAT_R5G6B5;
				else {
					nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT,F_PLGBMPLOAD,ERR_UNSUPPORTEDCOLOR);
					nFileClose(f);
					return false;
				}
			}
			tex->nglrowalignment = 4;
			texsize = (uint64_t)iheader.biWidth*2;
			if(texsize%4)
				texsize += 4-texsize%4;
			texsize *= (uint64_t)iheader.biHeight;
			break;
		case 24:
			tex->nglcolorformat = NGL_COLORFORMAT_B8G8R8;
			tex->nglrowalignment = 4;
			texsize = (uint64_t)iheader.biWidth*3;
			if(texsize%4)
				texsize += 4-texsize%4;
			texsize *= (uint64_t)iheader.biHeight;
			break;
		case 32:
			if(iheader.biCompression == 0) {
				tex->nglcolorformat = NGL_COLORFORMAT_B8G8R8A8;
			} else {
				unsigned int mask[4]; 
				// mask[0] - красный, mask[1] - зелёный, mask[2] - синий, mask[3] - альфа
				// Маска записывается в little-endian
				// 0x000000FF - 1-й байт, 0x0000FF00 - 2-й байт, 0x00FF0000 - 3-й байт, 0xFF000000 - 4-й байт
				// Т.е. если mask[0] = 0xFF000000, то красный будет в 4-м байте
				
				if(iheader.biSize > 40 || iheader.biCompression == 6) { // Версия bmp выше 3-й
					nFileRead(f,mask,4*sizeof(unsigned int));
				} else {
					nFileRead(f,&mask,3*sizeof(unsigned int));
					mask[3] = 0;
				}
				
				//nlPrint(L"mask[0]=%d mask[1]=%d mask[2]=%d mask[3]=%d", (int)mask[0], (int)mask[1], (int)mask[2], (int)mask[3]);

				if(mask[0] == 0x00FF0000 && mask[1] == 0x0000FF00 && mask[2] == 0x000000FF && mask[3] == 0) {
					tex->nglcolorformat = NGL_COLORFORMAT_B8G8R8A8;
					reset_alpha = true;
				} else if (mask[0] == 0x00FF0000 && mask[1] == 0x0000FF00 && mask[2] == 0x000000FF && mask[3] == 0)
					tex->nglcolorformat = NGL_COLORFORMAT_B8G8R8A8;
				else if(mask[0] == 0xFF000000 && mask[1] == 0x00FF0000 && mask[2] == 0x0000FF00 && mask[3] == 0x000000FF)
					tex->nglcolorformat = NGL_COLORFORMAT_A8B8G8R8; // В предыдущей версии кода был стандартом для RGBA
				else {
					nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT,F_PLGBMPLOAD,ERR_UNSUPPORTEDCOLOR);
					nFileClose(f);
					return false;
				}
			}
			
			tex->nglrowalignment = 4;
			texsize = 4*(uint64_t)iheader.biWidth*(uint64_t)iheader.biHeight;
			break;
		default:
			nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT,F_PLGBMPLOAD,ERR_UNSUPPORTEDCOLOR);
			nFileClose(f);
			return false;
	}

	if(texsize > SIZE_MAX) {
		nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT, F_PLGBMPLOAD, ERR_FILEISTOOLARGE);
		nFileClose(f);
		return false;
	}

	tex->sizex = iheader.biWidth;
	tex->sizey = iheader.biHeight;

	nFileSeek(f,fheader.bfOffBits,FILE_SEEK_SET);

	tex->buffer = nAllocMemory((size_t)texsize);
	if(!tex->buffer) {
		nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT,F_PLGBMPLOAD,N_FALSE);
		nFileClose(f);
		return false;
	}

	// Загрузка текстуры
	switch(iheader.biBitCount) {
		case 1:
			temp = nAllocMemory((size_t)ptexsize);
			if(!temp) {
				nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT,F_PLGBMPLOAD,N_FALSE);
				nFileClose(f);
				nFreeMemory(tex->buffer);
				return false;
			}
			nFileRead(f,temp,(size_t)ptexsize);
			p = tex->buffer;
			p3 = temp;
			bitpos = 7;
			for(i = 0;i < tex->sizey;i++) {
				for(j = 0;j < tex->sizex;j++) {
					//if(_bittest((const long int *)p3,bitpos)) // Можно через биттест, а можно так
					if(((*p3)>>bitpos)&0x01)
						p2 = pal+4;
					else
						p2 = pal;
					*p = *p2; p++; p2++;
					*p = *p2; p++; p2++;
					*p = *p2; p++;
					bitpos--;
					if(bitpos==-1) { bitpos = 7; p3++; }
				}
				if(bitpos!=7) {
					p3++;
					bitpos = 7;
				}
				p3 += ptexal;
			}
			nFreeMemory(temp);
			break;
		case 4:
			temp = nAllocMemory((size_t)ptexsize);
			if(!temp) {
				nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT,F_PLGBMPLOAD,N_FALSE);
				nFileClose(f);
				nFreeMemory(tex->buffer);
				return false;
			}
			nFileRead(f,temp,(size_t)ptexsize);
			p = tex->buffer;
			p3 = temp;
			for(i = 0;i < tex->sizey;i++) {
				for(j = 0;j < tex->sizex;j+=2) {
					p2 = pal+(((*p3)>>4)&0x0F)*4;
					*p = *p2; p++; p2++;
					*p = *p2; p++; p2++;
					*p = *p2; p++;
					if(j+1<tex->sizex) {
						p2 = pal+((*p3)&0x0F)*4;
						*p = *p2; p++; p2++;
						*p = *p2; p++; p2++;
						*p = *p2; p++;
					}
					p3++;
				}
				p3 += ptexal;
			}
			nFreeMemory(temp);
			break;
		case 8:
			temp = nAllocMemory((size_t)ptexsize);
			if(!temp) {
				nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT,F_PLGBMPLOAD,N_FALSE);
				nFileClose(f);
				nFreeMemory(tex->buffer);
				return false;
			}
			nFileRead(f,temp,(size_t)ptexsize);
			p = tex->buffer;
			p3 = temp;
			for(i = 0;i < tex->sizey;i++) {
				for(j = 0;j < tex->sizex;j++) {
					p2 = pal+(*p3)*4;
					*p = *p2; p++; p2++;
					*p = *p2; p++; p2++;
					*p = *p2; p++;
					p3++;
				}
				p3 += ptexal;
			}
			nFreeMemory(temp);
			break;
		case 16:
		case 24:
		case 32:
			nFileRead(f,tex->buffer,(size_t)texsize);
			if(reset_alpha) {
				switch(tex->nglcolorformat) {
					case NGL_COLORFORMAT_B8G8R8A8:
						p = tex->buffer+3;
						for(i = 0;i < tex->sizey;i++) {
							for(j = 0;j < tex->sizex;j++) {
								*p = 255; p += 4;
							}
							p += ptexal;
						}
				}
			}
			break;
	}

	nFileClose(f);

	nlAddTab(-1); nlPrint(LOG_FDEBUGFORMAT,F_PLGBMPLOAD,N_OK);

	return true;
}
