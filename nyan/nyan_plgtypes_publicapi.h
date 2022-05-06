/*
	Файл	: nyan_plgtypes_publicapi.h

	Описание: Типы плагинов

	История	: 19.08.12	Создан

*/

#ifndef NYAN_PLGTYPES_PUBLICAPI_H
#define NYAN_PLGTYPES_PUBLICAPI_H

#include <stddef.h>
#include <wchar.h>

typedef struct {
	size_t size; // Размер структуры plg_api_type
	unsigned int type; // Тип плагина
	const wchar_t *name; // Имя плагина
	void *api;
} plg_api_type;

#define N_PLG_TYPE_TEXLOADER 1 // Плагин для загрузки текстур
#define N_PLG_TYPE_AFLOADER 2 // Плагин для загрузки аудиофайлов
#define N_PLG_TYPE_FILESYS 3 // Плагин для работы с файлами

#endif
