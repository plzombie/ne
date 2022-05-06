#ifndef NYAN_PUBLICAPI_H
#define NYAN_PUBLICAPI_H

#include <stdbool.h>
#include <time.h>
#include <wchar.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "nyan_decls_publicapi.h"

#include "nyan_entry_publicapi.h"
#include "nyan_texformat_publicapi.h"
#include "nyan_audiofile_publicapi.h"
#include "nyan_file_publicapi.h"
#include "nyan_vismodes_publicapi.h"
#include "nyan_keycodes_publicapi.h"
#include "nyan_draw_publicapi.h"
#include "nyan_plgtypes_publicapi.h"

// Работа с потоками
#include "nyan_threads_publicapi.h"

// Инициализация движка
#include "nyan_init_publicapi.h"

// Инициализация рендера
#include "nyan_vis_init_publicapi.h"

// Журнал сообщений
#include "nyan_log_publicapi.h"

// Подсчёт кадров в секунду
#include "nyan_fps_publicapi.h"

// Работа с файлами
#include "nyan_filesys_publicapi.h"
#include "nyan_filesys_dirpaths_publicapi.h"

// Текстуры
#include "nyan_vis_texture_publicapi.h"

// Рисование
#include "nyan_vis_draw_publicapi.h"

// Матрицы
#include "nyan_vis_matrixes_publicapi.h"

// Сцена и 3d
#include "nyan_vis_3dmodel_publicapi.h"

// Шрифты и текст
#include "nyan_vis_fonts_publicapi.h"

// Простой вывод графики
#include "nyan_vis_justdraw_publicapi.h"

// Спрайты и текстурные атласы
#include "nyan_vis_spritesheets_publicapi.h"

// Работа с памятью
#include "nyan_mem_publicapi.h"

// Загрузка плагинов из dll
#include "nyan_plgloader_publicapi.h"

// Работа со звуком
#include "nyan_au_init_publicapi.h"
#include "nyan_au_main_publicapi.h"

#ifdef __cplusplus
}
#endif

#endif /* NYAN_PUBLICAPI_H */
