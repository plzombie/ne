/*
	Файл	: cl_score.с

	Описание: Меню

	История	: 03.11.13	Создан

*/

#include "../../nyan/nyan_publicapi.h"

#include "cl_main.h"
#include "cl_menu.h"
#include "cl_game.h"
#include "cl_score.h"

typedef struct {
	wchar_t name[16];
	unsigned int result;
} cl_score_type;

static cl_score_type cl_scores[6];

/*
	Функция	: clAddScore

	Описание: Дополнение списка лучших игроков.

	История	: 03.11.13	Создан

*/
void clScoreAdd(wchar_t *name, unsigned int score)
{
	unsigned int i;
	
	for(i = 0; i < 6; i++)
		if(cl_scores[i].result < score) break;
	
	if(i != 6) {
		unsigned int j;
		
		for(j = 5; j > i; j--) {
			wcscpy(cl_scores[j].name, cl_scores[j-1].name);
			cl_scores[j].result = cl_scores[j-1].result;
		}
		
		wcscpy(cl_scores[i].name, name);
		cl_scores[i].result = score;
	}
}

/*
	Функция	: clScoreLoad

	Описание: Загрузка списка лучших игроков.

	История	: 03.11.13	Создан

*/
void clScoreLoad(void)
{
	unsigned int f;
	
	if(!nFileCreateDir(L"ColorLines2013", NF_PATH_GAMESAVES)) goto DEFAULT;

	if(!nFileCreate(L"ColorLines2013/colorlines2013.dat", false, NF_PATH_GAMESAVES)) goto DEFAULT;
	
	f = nFileOpen(L"ColorLines2013/colorlines2013.dat");
	
	if(!f) goto DEFAULT;
	
	if(nFileRead(f, cl_scores, 6*sizeof(cl_score_type)) <= 0) { nFileClose(f); goto DEFAULT; }
	
	nFileClose(f);
	
	return;
	
DEFAULT:
	wcscpy(cl_scores[0].name, L"First");
	cl_scores[0].result = 100;
	wcscpy(cl_scores[1].name, L"Second");
	cl_scores[1].result = 90;
	wcscpy(cl_scores[2].name, L"Third");
	cl_scores[2].result = 80;
	wcscpy(cl_scores[3].name, L"Fourth");
	cl_scores[3].result = 70;
	wcscpy(cl_scores[4].name, L"Fifth");
	cl_scores[4].result = 60;
	wcscpy(cl_scores[5].name, L"Sixth");
	cl_scores[5].result = 50;
}

/*
	Функция	: clScoreSave

	Описание: Загрузка списка лучших игроков.

	История	: 03.11.13	Создан

*/
void clScoreSave(void)
{
	unsigned int f;
	
	if(!nFileCreate(L"ColorLines2013/colorlines2013.dat", false, NF_PATH_GAMESAVES)) return;
	
	f = nFileOpen(L"ColorLines2013/colorlines2013.dat");
	
	if(!f) return;
	
	nFileWrite(f, cl_scores, 6*sizeof(cl_score_type));
	
	nFileClose(f);
}

/*
	Функция	: clProcessScore

	Описание: Вывод списка лучших игроков. Возвращает состояние, в которое должна перейти игра

	История	: 03.11.13	Создан

*/
int clProcessScore(void)
{
	wchar_t temp[64];
	static int old_cur_item = 0, current_item = 0;
	int mbl, ret = 0, i;
	
	if(nvGetStatusi(NV_STATUS_WIN_EXITMSG)) return 0;
	if(nvGetKey(27) == 2) return 1;
	
	mbl = nvGetStatusi(NV_STATUS_WINMBL);
	
	nvBegin2d();
		nvDraw2dText(L"Color Lines '13", 28, 56, cl_fontid, cl_fonttexid, 4.05f, 4.05f, CBLACK);
		nvDraw2dText(L"Color Lines '13", 32, 60, cl_fontid, cl_fonttexid, 4.0, 4.0, CWHITE);
		
		nvDraw2dText(L"Hall of fame", 32, 180, cl_fontid, cl_fonttexid, 2.0, 2.0, CWHITE);
		
		for(i = 0; i < 6; i++) {
			swprintf(temp, 64, L"%u %ls", cl_scores[i].result, cl_scores[i].name);
			nvDraw2dText(temp, 32, 240+20*i, cl_fontid, cl_fonttexid, 1.0, 1.0, CLBLUE);
		}

		if(nvDraw2dText(L"Back", 32, 380, cl_fontid, cl_fonttexid, 2.0, 2.0, (old_cur_item==1)?CPINK:CWHITE))
			current_item = 1;
		
		ret = 4;
		if(mbl == 2) {
			switch(current_item) {
				case 1:
					ret = 1;
					clInitGame();
					break;
			}
		}			
		
		old_cur_item = current_item;
		current_item = 0;
	nvEnd2d();
	
	return ret;
}
