/*
	Файл	: ss_main.с

	Описание: Инициализация игры, основной цикл

	История	: 24.05.14	Создан

*/

#include <stdlib.h>

#include "../../nyan/nyan_publicapi.h"

#include "ss_main.h"
#include "ss_background.h"
#include "ss_game.h"

const int CWHITE = NV_COLOR(255,255,255,255);

const int ORIG_SCREENW = 640;
const int ORIG_SCREENH = 480;

unsigned int ss_cloudtexid = 0;

unsigned int ss_fonttexid = 0;
unsigned int ss_fontid = 0;

float ss_xmult = 1, ss_xoffset = 0, ss_ymult = 1, ss_yoffset = 0;
float ss_winx = 640, ss_winy = 480;

/*
	Функция	: ssLoadGameResources

	Описание: Загружает ресурсы (изображения, звуки), которые понадобятся игре

	История	: 24.05.14	Создан

*/
bool ssLoadGameResources(void)
{
	// Загрузка шрифтов
	ss_fonttexid = nvCreateTextureFromFile(L"deffont.tga", NGL_TEX_FLAGS_FOR2D);
	if(!ss_fonttexid) return false;

	if(!nvLoadTexture(ss_fonttexid)) return false;

	ss_fontid = nvCreateFont(L"deffont.nek1");
	if(!ss_fontid) return false;

	// Загрузка текстур фона
	ss_cloudtexid = nvCreateTextureFromFile(L"ss_cloud.tga", NGL_TEX_FLAGS_LINEARMIN | NGL_TEX_FLAGS_LINEARMAG | NGL_TEX_FLAGS_FOR2D);
	if(!ss_cloudtexid) return false;

	if(!nvLoadTexture(ss_cloudtexid)) return false;

	return true;
}

void ssSetWorkingSet(void)
{
	ss_winx = (float)nvGetStatusi(NV_STATUS_WINX);
	ss_winy = (float)nvGetStatusi(NV_STATUS_WINY);
	
	if((ss_winx/ss_winy) > ((float)ORIG_SCREENW/(float)ORIG_SCREENH)) {
		ss_ymult = ss_winy/(float)ORIG_SCREENH;
		ss_yoffset = 0;
		ss_xmult = ss_ymult;
		ss_xoffset = (ss_winx-ss_xmult*(float)ORIG_SCREENW)/2.0f;
	} else {
		ss_xmult = ss_winx/(float)ORIG_SCREENW;
		ss_xoffset = 0;
		ss_ymult = ss_xmult;
		ss_yoffset = (ss_winy-ss_ymult*(float)ORIG_SCREENH)/2.0f;
	}
}

NYAN_MAIN
{
	int game_state = SS_GAME_STATE_MAINMENU;

	NYAN_INIT

	nlPrint(L"%ls", L" Unnamed sidescroller game\t\t\t\t\t\t  2014");
	nlPrint(L"");

	srand((unsigned int)time(NULL));

	if(!nvAttachRender(L"ngl")) return 0;
	if(!naAttachLib(L"nullal")) return 0;

	nvSetStatusw(NV_STATUS_WINTITLE, L"Unnamed sidescroller game");

	nvSetStatusi(NV_STATUS_WINUPDATEINTERVAL, 16);

	nvSetScreen(ORIG_SCREENW, ORIG_SCREENH, 32, NV_MODE_WINDOWED, 0);

	nMountDir(L"media");
	nMountDir(L"media/fonts");
	nMountDir(L"../media");
	nMountDir(L"../media/fonts");
	nMountDir(L"../../media");
	nMountDir(L"../../media/fonts");
	nMountDir(L"../../../media");
	nMountDir(L"../../../media/fonts");

	if(!nInit()) return 0;

	if(ssLoadGameResources() == false) { nClose(); return 0; }

	ssBkgSet(SS_BKG_TYPE1);

	while(game_state != SS_GAME_STATE_EXIT) {
		if(nvGetStatusi(NV_STATUS_WIN_EXITMSG) || nvGetKey(27))
			game_state = SS_GAME_STATE_EXIT;

		nvBegin2d();
			ssSetWorkingSet();
			ssBkgUpdate();

			switch(game_state) {
				case SS_GAME_STATE_MAINMENU:
					game_state = SS_GAME_STATE_GAMEPROCESS;
					break;
				case SS_GAME_STATE_GAMEPROCESS:
					game_state = ssProcessGame();
					break;
				default:
					game_state = SS_GAME_STATE_EXIT;
			}
			ssFogUpdate();
		nvEnd2d();
		nUpdate();
	}

	nClose();

	NYAN_CLOSE

	return 0;
}
