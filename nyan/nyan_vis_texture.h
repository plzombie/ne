/*
	Файл	: nyan_vis_texture.h

	Описание: Заголовок для nyan_vis_texture.c

	История	: 15.08.12	Создан

*/

#include <stdint.h>
#include <wchar.h>

#include "nyan_texformat_publicapi.h"

typedef struct {
	unsigned int status;
	unsigned int ltype; // Тип текстуры (как загружать текстуру)
	unsigned int nglid; // id ngl текстуры
	unsigned int flags; // Флаги текстуры
	wchar_t *fname; // Имя
	wchar_t *subfname; // Второе имя (к примеру, альфаканал)
	unsigned int sub1; // Дополнительные параметры
	unsigned int sub2;
	unsigned int sub3;
	unsigned int sub4;
	nv_texture_type tex;
} nv_texobj_type;

extern nv_texobj_type *nv_texobjs;
extern unsigned int nv_maxtexobjs;

extern uintptr_t nv_textures_sync_mutex;

extern unsigned int nvGetNGLTextureId(unsigned int id);
extern bool nvGetTextureNGLIdAndWH(unsigned int id, unsigned int *nglid, unsigned int *wid, unsigned int *hei);
