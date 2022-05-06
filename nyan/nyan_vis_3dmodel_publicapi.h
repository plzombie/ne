/*
	Файл	: nyan_vis_3dmodel_publicapi.h

	Описание: Публичные функции для вывода 3d моделей

	История	: 05.07.17	Создан

*/

#ifndef NYAN_VIS_3DMODEL_PUBLICAPI_H
#define NYAN_VIS_3DMODEL_PUBLICAPI_H

#include <stdbool.h>
#include <wchar.h>

#include "nyan_decls_publicapi.h"

NYAN_FUNC(unsigned int, nvCreate3dModel, (const wchar_t *filename, const char *modname))
NYAN_FUNC(bool, nvLoad3dModel, (unsigned int id))
NYAN_FUNC(bool, nvUnload3dModel, (unsigned int id))
NYAN_FUNC(bool, nvDestroy3dModel, (unsigned int id))
NYAN_FUNC(void, nvDestroyAll3dModels, (void))
NYAN_FUNC(void, nvDrawStatic3dModel, (unsigned int id, unsigned int skinid))
NYAN_FUNC(void, nvDraw3dModel, (unsigned int id, unsigned int skinid, unsigned int frame_start, unsigned int frame_end, float frame_mid))
NYAN_FUNC(void, nvDrawAnimated3dModel, (unsigned int id, unsigned int skinid, unsigned int animid, float frame, float *nextframe))
NYAN_FUNC(void, nvDrawAnimated3dModel2, (unsigned int id, unsigned int skinid, unsigned int animid, int64_t anim_start))
NYAN_FUNC(unsigned int, nvGet3dModelNumberOfFrames, (unsigned int id))
NYAN_FUNC(unsigned int, nvGet3dModelNumberOfSkins, (unsigned int id))
NYAN_FUNC(unsigned int, nvGet3dModelNumberOfAnims, (unsigned int id))
NYAN_FUNC(unsigned int, nvGet3dModelRefCount, (unsigned int id))
NYAN_FUNC(unsigned int, nvGet3dModelAnimIdByName, (unsigned int id, const char *animname))
NYAN_FUNC(unsigned int, nvGet3dModelIdByName, (const char *name))
NYAN_FUNC(const char *, nvGet3dModelName, (unsigned int id))
NYAN_FUNC(void, nvIncrease3dModelRefCount, (unsigned int id))
NYAN_FUNC(void, nvDecrease3dModelRefCount, (unsigned int id))

#endif
