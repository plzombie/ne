/*
	Файл	: nyan_audiofile_publicapi.h

	Описание: Параметры для аудиофайла

	История	: 01.09.12	Создан

*/

#ifndef NYAN_AUDIOFILE_PUBLICAPI_H
#define NYAN_AUDIOFILE_PUBLICAPI_H

#include <stdbool.h>
#include <wchar.h>

#ifndef N_APIENTRY
#include "nyan_decls_publicapi.h"
#endif

// Значения для sf
#define NA_SOUND_8BIT_MONO 0
#define NA_SOUND_8BIT_STEREO 1
#define NA_SOUND_16BIT_MONO 2
#define NA_SOUND_16BIT_STEREO 3

typedef struct {
	unsigned int nos; // Количество семплов
	unsigned int noc; // Кол-во каналов
	unsigned int freq; // Частота
	unsigned int sf; // Формат звука
	unsigned int bps; // байт в одном семпле
	unsigned int selplg; // Плагин, выбранный для чтения файла
	unsigned int selplgusecounter; // Счётчик использования для id выбранного плагина
	void * plgdata; // Данные, которые необходимы плагину для прочтения файла
} na_audiofile_type;

// Структура, описывающая плагин для загрузки аудиофайлов
typedef bool (N_APIENTRY *na_audiofile_plg_supportext_type)(const wchar_t *fname, const wchar_t *fext);
typedef bool (N_APIENTRY *na_audiofile_plg_load_type)(const wchar_t *fname, na_audiofile_type *tex);
typedef bool (N_APIENTRY *na_audiofile_plg_read_type)(unsigned int offset, unsigned int nofs, void *buf, na_audiofile_type *tex);
typedef void (N_APIENTRY *na_audiofile_plg_unload_type)(na_audiofile_type *tex);
typedef struct {
	size_t size; // Размер структуры na_audiofile_plugin_type
	na_audiofile_plg_supportext_type SupportExt; // Функции передаётся тип файла. Возвращает true, если тип поддерживается
	na_audiofile_plg_load_type Load; // Загружает аудиофайл 
	na_audiofile_plg_read_type Read; // Загружает аудиофайл 
	na_audiofile_plg_unload_type Unload; // Загружает аудиофайл 
} na_audiofile_plugin_type; // Плагин для загрузки аудиофайлов

#endif
