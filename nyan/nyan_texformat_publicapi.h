/*
	Файл	: nyan_texformat_publicapi.h

	Описание: Параметры для задания текстуры

	История	: 05.08.12	Создан

*/

#ifndef NYAN_TEXFORMAT_PUBLICAPI_H
#define NYAN_TEXFORMAT_PUBLICAPI_H

#include <stdbool.h>
#include <wchar.h>

#ifndef N_APIENTRY
#include "nyan_decls_publicapi.h"
#endif

#define NGL_COLORFORMAT_R8G8B8		0x0001
#define NGL_COLORFORMAT_R8G8B8A8	0x0002
#define NGL_COLORFORMAT_B8G8R8		0x0003
#define NGL_COLORFORMAT_B8G8R8A8	0x0004
#define NGL_COLORFORMAT_A8B8G8R8	0x0005
#define NGL_COLORFORMAT_L8			0x0006
#define NGL_COLORFORMAT_L8A8		0x0007
#define NGL_COLORFORMAT_X1R5G5B5	0x0008
#define NGL_COLORFORMAT_R5G6B5		0x0009

//Флаги текстуры (public)
#define NGL_TEX_FLAGS_LINEARMIN 0x1 // Линейная фильтрация
#define NGL_TEX_FLAGS_LINEARMAG 0x2 // Линейная фильтрация
#define NGL_TEX_FLAGS_MIPMAP 0x4
#define NGL_TEX_FLAGS_LINEARMIPMAP 0x8 // Линейная фильтрация между мипмапами
#define NGL_TEX_FLAGS_ANISOTROPY 0x10
#define NGL_TEX_FLAGS_FOR2D 0x20 // Текстура будет использоваться для 2d графики
#define NGL_TEX_FLAGS_CLAMP_TO_EDGE 0x40 // При использовании пикселем текстурных координат за пределами диапазона [0;1], они приводятся к этому диапазону
#define NGL_TEX_FLAGS_ALL (NGL_TEX_FLAGS_LINEARMIN|NGL_TEX_FLAGS_LINEARMAG|NGL_TEX_FLAGS_MIPMAP|NGL_TEX_FLAGS_LINEARMIPMAP|NGL_TEX_FLAGS_ANISOTROPY|NGL_TEX_FLAGS_FOR2D)

// Структура, описывающая текстуру
typedef struct{
	unsigned int sizex;
	unsigned int sizey;
	unsigned int nglcolorformat;
	int nglrowalignment; // Выравнивание строки
	unsigned char *buffer; // Временное хранилище текстуры
} nv_texture_type;

// Структура, описывающая плагин для загрузки текстуры
typedef bool (N_APIENTRY * nv_tex_plg_supportext_type)(const wchar_t *fname, const wchar_t *fext);
typedef bool (N_APIENTRY * nv_tex_plg_load_type)(const wchar_t *fname, nv_texture_type *tex);
typedef struct {
	size_t size; // Размер структуры nv_tex_plugin_type
	nv_tex_plg_supportext_type SupportExt; // Функции передаётся тип файла. Возвращает true, если тип поддерживается
	nv_tex_plg_load_type Load; // Загружает изображение fname в tex
} nv_tex_plugin_type; // Плагин для загрузки изображений из файла

#endif
