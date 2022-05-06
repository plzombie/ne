/*
	Файл	: cl_sdifficulty.с

	Описание: Меню

	История	: 03.11.13	Создан

*/

#include "../../nyan/nyan_publicapi.h"

#include "cl_main.h"
#include "cl_menu.h"
#include "cl_game.h"

/*
	Функция	: clProcessSDifficulty

	Описание: Вывод и обработка меню выбора сложности. Возвращает состояние, в которое должна перейти игра

	История	: 03.11.13	Создан

*/
int clProcessSDifficulty(void)
{
	static int old_cur_item = 0, current_item = 0;
	int mbl, ret = 0;
	
	if(nvGetStatusi(NV_STATUS_WIN_EXITMSG)) return 0;
	if(nvGetKey(27) == 2) return 1;
	
	mbl = nvGetStatusi(NV_STATUS_WINMBL);
	
	nvBegin2d();
		nvDraw2dText(L"Color Lines '13", 28, 56, cl_fontid, cl_fonttexid, 4.05f, 4.05f, CBLACK);
		nvDraw2dText(L"Color Lines '13", 32, 60, cl_fontid, cl_fonttexid, 4.0, 4.0, CWHITE);
		
		nvDraw2dText(L"Select Difficulty:", 32, 180, cl_fontid, cl_fonttexid, 2.0, 2.0, CWHITE);
		
		if(nvDraw2dText(L"Slow", 32, 240, cl_fontid, cl_fonttexid, 2.0, 2.0, (old_cur_item==1)?CLBLUE:CWHITE))
			current_item = 1;
		if(nvDraw2dText(L"Deep", 32, 280, cl_fontid, cl_fonttexid, 2.0, 2.0, (old_cur_item==2)?CLBLUE:CWHITE))
			current_item = 2;
		if(nvDraw2dText(L"Hard", 32, 320, cl_fontid, cl_fonttexid, 2.0, 2.0, (old_cur_item==3)?CLBLUE:CWHITE))
			current_item = 3;
		if(nvDraw2dText(L"Back", 32, 380, cl_fontid, cl_fonttexid, 2.0, 2.0, (old_cur_item==4)?CPINK:CWHITE))
			current_item = 4;
		
		ret = 3;
		if(mbl == 2) {
			switch(current_item) {
				case 1:
				case 2:
				case 3:
					cl_difficulty = current_item;
					ret = 2;
					clInitGame();
					break;
				case 4:
					ret = 1;
					break;
			}
		}			
		
		old_cur_item = current_item;
		current_item = 0;
	nvEnd2d();
	
	return ret;
}
