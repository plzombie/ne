/*
	Файл	: cl_menu.с

	Описание: Меню

	История	: 28.09.13	Создан

*/

#include "../../nyan/nyan_publicapi.h"

#include "cl_main.h"
#include "cl_menu.h"
#include "cl_game.h"

/*
	Функция	: clProcessMainMenu

	Описание: Вывод и обработка главного меню. Возвращает состояние, в которое должна перейти игра

	История	: 28.09.13	Создан

*/
int clProcessMainMenu(void)
{
	static int old_cur_item = 0, current_item = 0;
	int mbl, ret = 0;
	
	if(nvGetStatusi(NV_STATUS_WIN_EXITMSG)) return 0;
	if(nvGetKey(27)) return 0;
	
	mbl = nvGetStatusi(NV_STATUS_WINMBL);
	
	nvBegin2d();
		nvDraw2dText(L"Color Lines '13", 28, 56, cl_fontid, cl_fonttexid, 4.05f, 4.05f, CBLACK);
		nvDraw2dText(L"Color Lines '13", 32, 60, cl_fontid, cl_fonttexid, 4.0, 4.0, CWHITE);
		
		if(nvDraw2dText(L"Start Game", 32, 200, cl_fontid, cl_fonttexid, 2.0, 2.0, (old_cur_item==1)?CPINK:CWHITE))
			current_item = 1;
		if(nvDraw2dText(L"Hall of fame", 32, 240, cl_fontid, cl_fonttexid, 2.0, 2.0, (old_cur_item==2)?CPINK:CWHITE))
			current_item = 2;
		//if(nvDraw2dText(L"Credits", 32, 280, cl_fontid, cl_fonttexid, 2.0, 2.0, (old_cur_item==3)?CPINK:CWHITE))
			//current_item = 3;
		if(nvDraw2dText(L"Exit", 32, 320, cl_fontid, cl_fonttexid, 2.0, 2.0, (old_cur_item==4)?CPINK:CWHITE))
			current_item = 4;
		
		ret = 1;
		if(mbl == 2) {
			switch(current_item) {
				case 1:
					ret = 3;
					clInitGame();
					break;
				case 2:
					ret = 4;
					break;
				case 4:
					ret = 0;
					break;
			}
		}			
		
		old_cur_item = current_item;
		current_item = 0;
	nvEnd2d();
	
	return ret;
}
