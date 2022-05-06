/*
	Файл	: nyan_fps_publicapi.h

	Описание: Публичные функции для подсчёта fps

	История	: 18.10.15	Создан

*/

#ifndef NYAN_FPS_PUBLICAPI_H
#define NYAN_FPS_PUBLICAPI_H

#include <stdint.h>
#include <time.h>

#include "nyan_decls_publicapi.h"

#define N_CLOCKS_PER_SEC 1000000

NYAN_FUNC(int64_t, nClock, (void))
NYAN_FUNC(int64_t, nFrameStartClock, (void))
NYAN_FUNC(double, nGetfps, (void))
NYAN_FUNC(double, nGetafps, (void))
NYAN_FUNC(double, nGetspf, (void))
NYAN_FUNC(double, nGetaspf, (void))

#endif
