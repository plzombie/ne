/*
	Файл	: nyan_vis_matrixes_publicapi.h

	Описание: Публичные функции для работы с матрицами

	История	: 15.01.16	Создан

*/

#ifndef NYAN_VIS_MATRIXES_PUBLICAPI_H
#define NYAN_VIS_MATRIXES_PUBLICAPI_H

#include <stdbool.h>

#include "nyan_decls_publicapi.h"

NYAN_FUNC(void, nvMatrixSetTranslate, (double x, double y, double z, double *matrix))
NYAN_FUNC(void, nvMatrixSetRotateX, (double rx, double *matrix))
NYAN_FUNC(void, nvMatrixSetRotateY, (double ry, double *matrix))
NYAN_FUNC(void, nvMatrixSetRotateZ, (double rz, double *matrix))
NYAN_FUNC(void, nvMatrixSetScale, (double sx, double sy, double sz, double *matrix))
NYAN_FUNC(void, nvMatrixSetMult, (double *m1, double *m2, double *dest))
NYAN_FUNC(void, nvMatrixMult, (double *m1, double *m2))
NYAN_FUNC(void, nvMatrixSetCamera, (double x, double y, double z, double rx, double ry, double rz, double *matrix))
NYAN_FUNC(bool, nvSetMatrix, (double *matrix))

#endif
