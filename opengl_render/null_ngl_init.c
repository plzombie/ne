/*
	Файл	: null_ngl_init.c

	Описание: Инициализация графической библиотеки. Шаблон

	История	: 05.10.12	Создан

*/

#include <stdio.h>
#include <stdbool.h>

#include <GL/glu.h>

#include "ngl.h"
#include "ngl_text.h"

#include "../nyan/nyan_vismodes.h"

#include "ngl_init.h"
#include "ngl_batch.h"
#include "null_ngl_init.h"

bool ngl_win_swapc_ext = false; // Поддержка вертикальной синхронизации

/*
	Функция : nglIsMainRContext

	Описание: Возвращает true, если текущий контекст - главный (контекст рендеринга).

	История	: 15.11.17	Создан

*/
bool nglIsMainRContext(void)
{
	return true;
}

/*
	Функция : nglAddSeparateRContextIfNeeded

	Описание: Если для потока не выбран контекст, создаёт отдельный констекст OpenGL, добавляет его в массив.
		В данной реализации контексты уже созданы и добавлены, нужно только увеличить счётчик ngl_win_nof_used_separate_hRCs.

	История	: 15.11.17	Создан

*/
bool nglAddSeparateRContextIfNeeded(void)
{
	return true;
}

/*
	Функция	: nglInitWindow

	Описание: Создаёт окно

	История	: 5.10.12	Создан

*/
bool nglInitWindow(void)
{
	ngl_ea->nlPrint(NGL_SCREENINFO, F_NGLINITWINDOW, ngl_winx, ngl_winy, ngl_winbpp, pfd.cDepthBits, (ngl_winmode == NV_MODE_FULLSCREEN)?N_YES:N_NO);

	// Поддержка вертикальной синхронизации
	ngl_win_swapc_ext = false;
	ngl_ea->nlPrint(LOG_FDEBUGFORMAT2,F_NGLINITWINDOW,NGL_SUPPORTVSYNC,ngl_win_swapc_ext?N_YES:N_NO);

	// Установка вертикальной синхронизации
	ngl_win_vsync = 0;
	ngl_ea->nlPrint(LOG_FDEBUGFORMAT2,F_NGLINITWINDOW,NGL_VSYNC,ngl_win_vsync?N_YES:N_NO);

	return true;
}

/*
	Функция	: nglCloseWindow

	Описание: Уничтожает окно

	История	: 5.10.12	Создан

*/
bool nglCloseWindow(void)
{
	return true;
}

/*
	Функция	: nglUpdateWindow

	Описание: Обновление окна

	История	: 5.10.12	Создан

*/
void nglUpdateWindow(void)
{

}

/*
	Функция	: nglChangeVSYNC

	Описание: Меняет вертикальную синхронизацию

	История	: 5.10.12	Создан

*/
void nglChangeVSYNC(void)
{
	ngl_ea->nlPrint(LOG_FDEBUGFORMAT,F_NGLSETSTATUSI,ERR_CANTCHANGEVSYNC); ngl_win_vsync = 0;
}

/*
	Функция	: nglChangeUpdateInterval

	Описание: Меняет интервал обновления экрана

	История	: 23.03.17	Создан

*/
void nglChangeUpdateInterval(unsigned int updateinterval)
{
	(void)updateinterval; // Неиспользуемая переменная

	ngl_ea->nlPrint(LOG_FDEBUGFORMAT, F_NGLSETSTATUSI, ERR_CANTCHANGEUPDATEINTERVAL); ngl_win_updateinterval = 0;
}

/*
	Функция	: nglChangeWindowMode

	Описание: Устанавливает режим окна (полноэкранный, оконный...)
		Разрешение экрана меняется в nglChangeWindowSize

	История	: 09.03.18	Создан

*/
void nglChangeWindowMode(int winmode)
{
	if(ngl_winmode == winmode) return;

	/*if (ngl_winmode == NV_MODE_FULLSCREEN) {
		// Восстанавливаем оригинальное разрешение экрана
	}*/

	// Устанавливаем стили окна

	ngl_winmode = winmode;
}

/*
	Функция	: nglChangeWindowSize

	Описание: Меняет размер окна

	История	: 27.08.17	Создан

*/
void nglChangeWindowSize(unsigned int winx, unsigned int winy)
{
	if(ngl_winx == winx && ngl_winy == winy) return;

	if(ngl_isinit) {
		//if(Failed to change window size to ngl_windpix*winx/100, ngl_windpiy*winy/100)
			return;
	}

	ngl_winx = winx;
	ngl_winy = winy;
}

/*
	Функция	: nglGetProcAddress

	Описание: Возращает процедуру по имени

	История	: 5.10.12	Создан

*/
ngl_func_type nglGetProcAddress(const char * name)
{
    return 0;
}
