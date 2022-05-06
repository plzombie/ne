/*
	Файл	: null_ngl_init.c

	Описание: Инициализация графической библиотеки. Шаблон

	История	: 05.10.12	Создан

*/

#include <stdio.h>
#include <stdbool.h>

#include "ngl.h"
#include "ngl_text.h"

#include "../nyan/nyan_vismodes_publicapi.h"

#include "ngl_init.h"
#include "ngl_batch.h"
#include "null_ngl_init.h"

bool ngl_win_swapc_ext = false; // Поддержка вертикальной синхронизации

/*
	Функция	: nglInitWindow

	Описание: Создаёт окно

	История	: 5.10.12	Создан

*/
bool nglInitWindow(void)
{
	ngl_ea->nlPrint(NGL_SCREENINFO, F_NGLINITWINDOW, ngl_winx, ngl_winy, ngl_winbpp, 32, (ngl_winmode == NV_MODE_FULLSCREEN)?N_YES:N_NO);

	// Установка вертикальной синхронизации
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
void nglChangeVSYNC(int vsync)
{
	(void)vsync; // Неиспользуемая переменная

	ngl_ea->nlPrint(LOG_FDEBUGFORMAT, F_NGLSETSTATUSI, ERR_CANTCHANGEVSYNC);
	ngl_win_vsync = 0;
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
void nglChangeWindowMode(unsigned int winx, unsigned int winy, int winmode)
{
	if(!ngl_isinit) return;

	// Устанавливаем стили окна
	// ...
	
	// Меняем разрешение экрана
	//if(winmode == NV_MODE_FULLSCREEN) {
		//if((ngl_winmode != winmode) || (ngl_winx != winx) || (ngl_winy != winy)) {
			// Ставим новое разрешение экрана
			
			// В полноэкранном режиме не учитываем DPI
			//ngl_windpix = 100;
			//ngl_windpiy = 100;
		//}
	//} else if(ngl_winmode == NV_MODE_FULLSCREEN) { // winmode != NV_MODE_FULLSCREEN, а ngl_winmode == NV_MODE_FULLSCREEN
		// Восстанавливаем оригинальное разрешение экрана
	//}
	
	
	if(ngl_winx != winx || ngl_winy != winy) {
		//if(Failed to change window size to ngl_windpix*winx/100, ngl_windpiy*winy/100)
			return;
	}
	
	ngl_winmode = winmode;
	ngl_winx = winx;
	ngl_winy = winy;
}

/*
	Функция	: nglChangeWindowTitle

	Описание: Устанавливает заголовок окна

	История	: 20.06.18	Создан

*/
void nglChangeWindowTitle(const wchar_t *title)
{
	(void)title;

	return;
}
