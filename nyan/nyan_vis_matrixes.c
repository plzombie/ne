/*
	Файл	: nyan_vis_matrixes.c

	Описание: Работа с матрицами

	История	: 15.01.16	Создан

*/

#include <string.h>
#include <stdbool.h>
#include <math.h>

#include "nyan_decls_publicapi.h"

#include "nyan_vis_matrixes_publicapi.h"
#include "nyan_text.h"

#include "nyan_vis_init.h"
#include "nyan_vis_draw.h"
#include "nyan_vis_matrixes.h"

#include "nyan_nglapi.h"

#define NYAN_PI 3.14159265358979323846

/*
	Функция	: nvMatrixSetTranslate

	Описание: Задаёт матрицу сдвига

	История	: 15.01.16	Создан

*/
N_API void N_APIENTRY_EXPORT nvMatrixSetTranslate(double x, double y, double z, double *matrix)
{
	memset(matrix, 0, 16*sizeof(double));

	matrix[0] = 1.;
	matrix[5] = 1.;
	matrix[10] = 1.;
	matrix[15] = 1.;
	matrix[12] = x;
	matrix[13] = y;
	matrix[14] = z;
}

/*
Функция	: nvMatrixSetRotateX

Описание: Задаёт матрицу поворота вокруг x
	http://www.songho.ca/opengl/gl_anglestoaxes.html

История	: 15.01.16	Создан

*/
N_API void N_APIENTRY_EXPORT nvMatrixSetRotateX(double rx, double *matrix)
{
	rx = rx*NYAN_PI/180;

	memset(matrix, 0, 16 * sizeof(double));

	matrix[0] = 1.;

	matrix[5] = cos(rx);
	matrix[6] = sin(rx);

	matrix[9] = -sin(rx);
	matrix[10] = cos(rx);

	matrix[15] = 1.;
}

/*
Функция	: nvMatrixSetRotateY

Описание: Задаёт матрицу поворота вокруг y
	http://www.songho.ca/opengl/gl_anglestoaxes.html

История	: 15.01.16	Создан

*/
N_API void N_APIENTRY_EXPORT nvMatrixSetRotateY(double ry, double *matrix)
{
	ry = ry*NYAN_PI / 180;

	memset(matrix, 0, 16 * sizeof(double));

	matrix[0] = cos(ry);
	matrix[2] = -sin(ry);

	matrix[5] = 1.;

	matrix[8] = sin(ry);
	matrix[10] = cos(ry);;

	matrix[15] = 1.;
}

/*
Функция	: nvMatrixSetRotateZ

Описание: Задаёт матрицу поворота вокруг z
	http://www.songho.ca/opengl/gl_anglestoaxes.html

История	: 15.01.16	Создан

*/
N_API void N_APIENTRY_EXPORT nvMatrixSetRotateZ(double rz, double *matrix)
{
	rz = rz*NYAN_PI / 180;

	memset(matrix, 0, 16 * sizeof(double));

	matrix[0] = cos(rz);
	matrix[1] = sin(rz);

	matrix[4] = -sin(rz);
	matrix[5] = cos(rz);

	matrix[10] = 1.;
	matrix[15] = 1.;
}

/*
Функция	: nvMatrixSetScale

Описание: Задаёт матрицу масштабирования

История	: 15.01.16	Создан

*/
N_API void N_APIENTRY_EXPORT nvMatrixSetScale(double sx, double sy, double sz, double *matrix)
{
	memset(matrix, 0, 16 * sizeof(double));

	matrix[0] = sx;
	matrix[5] = sy;
	matrix[10] = sz;
	matrix[15] = 1.;
}

/*
Функция	: nvMatrixSetMult

Описание: Перемножает матрицу m1 на m2, записывает результат в dest

История	: 15.01.16	Создан

*/
N_API void N_APIENTRY_EXPORT nvMatrixSetMult(double *m1, double *m2, double *dest)
{
	unsigned int c, r, i;

	for(r= 0; r < 4; r++)
		for(c = 0; c < 4; c++) {
			dest[c*4+r] = 0;
			for(i = 0; i < 4; i++)
				dest[c*4+r] += m1[i*4+r]*m2[c*4+i];
		}
}

/*
Функция	: nvMatrixMult

Описание: Перемножает матрицу m1 на m2, записывает результат в m1

История	: 15.01.16	Создан

*/
N_API void N_APIENTRY_EXPORT nvMatrixMult(double *m1, double *m2)
{
	double temp_matrix[16];

	nvMatrixSetMult(m1, m2, temp_matrix);

	memcpy(m1, temp_matrix, 16*sizeof(double));
}

/*
	Функция	: nvMatrixSetCamera

	Описание: Задаёт матрицу камеры

	История	: 15.01.16	Создан

*/
N_API void N_APIENTRY_EXPORT nvMatrixSetCamera(double x, double y, double z, double rx, double ry, double rz, double *matrix)
{
	double m[16];

	nvMatrixSetRotateZ(-rz, matrix);

	nvMatrixSetRotateY(-ry, m);
	nvMatrixMult(matrix, m);

	nvMatrixSetRotateX(-rx, m);
	nvMatrixMult(matrix, m);

	nvMatrixSetTranslate(-x, -y, -z, m);
	nvMatrixMult(matrix, m);
}

/*
	Функция	: nvSetMatrix

	Описание: Установка матрицы преобразования

	История	: 15.01.16	Создан

*/
N_API bool N_APIENTRY_EXPORT nvSetMatrix(double *matrix)
{
	if(!nv_isinit) return false;
	if (nv_draw_state == NV_DRAW_STATE_NO) return false;

	return funcptr_nglBatch3dSetModelviewMatrix(matrix);
}
