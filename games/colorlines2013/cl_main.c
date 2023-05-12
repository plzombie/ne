/*
	Файл	: cl_main.с

	Описание: Инициализация игры, основной цикл

	История	: 28.09.13	Создан

*/

#include "../../nyan/nyan_publicapi.h"

#include "cl_main.h"
#include "cl_menu.h"
#include "cl_game.h"
#include "cl_sdifficulty.h"
#include "cl_score.h"

#ifdef N_WINDOWS
	#include <Windows.h>
#endif

const int CBLACK = NV_COLOR(0,0,0,255);
const int CWHITE = NV_COLOR(255,255,255,255);
const int CRED = NV_COLOR(255,0,0,255);
const int CGREEN = NV_COLOR(0,255,0,255);
const int CBLUE = NV_COLOR(0,0,255,255);
const int CPINK = NV_COLOR(255,128,192,255);
const int CLBLUE = NV_COLOR(0,162,255,255);
const int CYELLOW = NV_COLOR(255,255,0,255);

unsigned int cl_fonttexid = 0;
unsigned int cl_fontid = 0;

unsigned int cl_ball1texid = 0;
unsigned int cl_ball2texid = 0;

/*
	Функция	: clLoadGameResources

	Описание: Загружает ресурсы (изображения, звуки), которые понадобятся игре

	История	: 28.09.13	Создан

*/
bool clLoadGameResources(void)
{
	cl_fonttexid = nvCreateTextureFromFile(L"deffont.tga", NGL_TEX_FLAGS_FOR2D);
	if(!cl_fonttexid) return false;

	cl_ball1texid = nvCreateTextureFromFile(L"ball1.tga", NGL_TEX_FLAGS_LINEARMIN | NGL_TEX_FLAGS_LINEARMAG | NGL_TEX_FLAGS_FOR2D);
	if(!cl_ball1texid) return false;

	cl_ball2texid = nvCreateTextureFromFile(L"ball2.tga", NGL_TEX_FLAGS_LINEARMIN | NGL_TEX_FLAGS_LINEARMAG | NGL_TEX_FLAGS_FOR2D);
	if(!cl_ball2texid) return false;

	if(!nvLoadTexture(cl_fonttexid)) return false;
	if(!nvLoadTexture(cl_ball1texid)) return false;
	if(!nvLoadTexture(cl_ball2texid)) return false;

	cl_fontid = nvCreateFont(L"deffont.nek1");
	if(!cl_fontid) return false;
	nvSetFontEmptySymFromDefault(cl_fontid);

	return true;
}

NYAN_MAIN
{
	int game_state = 1; // 0 - выход из игры, 1 - главное меню, 2 - игра, 3 - выбор сложности
				// 4 - таблица результатов
#ifdef N_WINDOWS
	HANDLE console_handle;
	CONSOLE_SCREEN_BUFFER_INFO console_sbinfo;
	WORD console_original_color;
	bool console_can_change_colors = false;
#endif
				
	NYAN_INIT

#ifdef N_WINDOWS
	console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
	if(console_handle != INVALID_HANDLE_VALUE && console_handle != NULL) {
		if(GetConsoleScreenBufferInfo(console_handle, &console_sbinfo)) {
			console_original_color = console_sbinfo.wAttributes;
			console_can_change_colors = true;
			SetConsoleTextAttribute(console_handle, 
				BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE |
				FOREGROUND_RED | FOREGROUND_BLUE);
		}
	}
#endif
	nlPrint(L"%ls", L" Color Lines\t\t\t\t\t\t\t\t   2013 ");
#ifdef N_WINDOWS
	if(console_can_change_colors)
		SetConsoleTextAttribute(console_handle, console_original_color);
#endif
	nlPrint(L"");

	srand((unsigned int)time(NULL));

	if(!nvAttachRender(L"ngl")) return 0;
	if(!naAttachLib(L"nullal")) return 0;

	nvSetStatusw(NV_STATUS_WINTITLE, L"Color Lines 2013");

	nvSetStatusi(NV_STATUS_WINUPDATEINTERVAL, 16);

	nvSetScreen(640, 480, 32, NV_MODE_WINDOWED_FIXED, 0);

	nvSetStatusf(NV_STATUS_WINBCRED,0.2f);
	nvSetStatusf(NV_STATUS_WINBCGREEN,0.2f);
	nvSetStatusf(NV_STATUS_WINBCBLUE,0.2f);

	nMountDir(L"media");
	nMountDir(L"media/fonts");
	nMountDir(L"../media");
	nMountDir(L"../media/fonts");
	nMountDir(L"../../media");
	nMountDir(L"../../media/fonts");
	nMountDir(L"../../../media");
	nMountDir(L"../../../media/fonts");

	if(!nInit()) return 0;

	if(clLoadGameResources() == false) { nClose(); return 0; }

	clScoreLoad();

	while(game_state) {
		switch(game_state) {
			case 1:
				game_state = clProcessMainMenu();
				break;
			case 2:
				game_state = clProcessGame();
				break;
			case 3:
				game_state = clProcessSDifficulty();
				break;
			case 4:
				game_state = clProcessScore();
				break;
			default:
				game_state = 0;
		}
		nUpdate();
	}

	clScoreSave();

	nClose();

	NYAN_CLOSE

	return 0;
}
