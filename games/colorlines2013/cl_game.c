/*
	Файл	: cl_game.с

	Описание: Собственно, игра

	История	: 29.09.13	Создан

*/

#include <string.h>
#include <stdlib.h>

#include "../../nyan/nyan_publicapi.h"

#include "cl_main.h"
#include "cl_menu.h"
#include "cl_game.h"
#include "cl_score.h"

typedef struct {
	unsigned int color;
	unsigned int isball;
} cl_ball_type;

static cl_ball_type cl_balls[100];
static int cl_ballspathtable[10000];
static unsigned int cl_nofballs;

unsigned int cl_difficulty = 0;

static unsigned int cl_gamescore;

static bool cl_isgameover;

static unsigned int cl_selectedball;

static bool clAddNewBalls(void);

static wchar_t cl_your_name[16] = L"You";

/*
	Функция	: clInitGame

	Описание: Устанавливает начальные параметры игры

	История	: 29.09.13	Создан

*/
void clInitGame(void)
{
	cl_gamescore = 0;

	cl_selectedball = 0;

	cl_nofballs = 0;

	cl_isgameover = false;

	if(cl_difficulty > 3) cl_difficulty = 3;
	if(cl_difficulty == 0) cl_difficulty = 1;

	memset(cl_balls, 0, 100*sizeof(cl_ball_type));

	clAddNewBalls();
	//printf("cl_nofballs %d\n", cl_nofballs);
}

/*
	Функция	: clCheckPath

	Описание: Проверяет наличие пути из ball1 в ball2

	История	: 03.11.13	Создан

*/
static bool clCheckPath(unsigned int ball1, unsigned int ball2)
{
	unsigned int i, j, k;

	ball1--;
	ball2--;

	memset(cl_ballspathtable, 0, 10000*sizeof(int));

	for(i = 0; i < 100; i++)
		cl_ballspathtable[i*100+i] = 1;

	for(i = 0; i < 10; i++) // y
		for(j = 0; j < 10; j++) { //x
			if(i > 0) {
				if((cl_balls[(i-1)*10+j].isball == 0) && ((cl_balls[i*10+j].isball == 0) || ((i*10+j) == ball1))) {
					cl_ballspathtable[((i-1)*10+j)*100 + (i*10+j)] = 1;
					cl_ballspathtable[(i*10+j)*100 + ((i-1)*10+j)] = 1;
				}
			}
			if(i < 9) {
				if((cl_balls[(i+1)*10+j].isball == 0) && ((cl_balls[i*10+j].isball == 0) || ((i*10+j) == ball1))) {
					cl_ballspathtable[((i+1)*10+j)*100 + (i*10+j)] = 1;
					cl_ballspathtable[(i*10+j)*100 + ((i+1)*10+j)] = 1;
				}
			}
			if(j > 0) {
				if((cl_balls[i*10+(j-1)].isball == 0) && ((cl_balls[i*10+j].isball == 0) || ((i*10+j) == ball1))) {
					cl_ballspathtable[(i*10+(j-1))*100 + (i*10+j)] = 1;
					cl_ballspathtable[(i*10+j)*100 + (i*10+(j-1))] = 1;
				}
			}
			if(j < 9) {
				if((cl_balls[i*10+(j+1)].isball == 0) && ((cl_balls[i*10+j].isball == 0) || ((i*10+j) == ball1))) {
					cl_ballspathtable[(i*10+(j+1))*100 + (i*10+j)] = 1;
					cl_ballspathtable[(i*10+j)*100 + (i*10+(j+1))] = 1;
				}
			}
		}

	for(k = 0; k < 100; k++)
		for(i = 0; i < 100; i++)
			for(j = 0; j < 100; j++)
				cl_ballspathtable[i*100+j] = cl_ballspathtable[i*100+j] || (cl_ballspathtable[i*100+k] && cl_ballspathtable[k*100+j]);

	if(cl_ballspathtable[ball1*100+ball2])
		return true;
	else
		return false;
}

/*
	Функция	: clDetectLines

	Описание: Обнаруживает и удаляет линии, начисляет очки

	История	: 29.09.13	Создан

*/
static bool clDetectLines(void)
{
	unsigned int i, j, k;
	bool result = false;

	for(i = 0; i < 10; i++) {
		unsigned int linesize, currentcolor = 0; // Вообще-то, ставить currentcolor = 0; не особо нужно, но студия ругается, так что...

		// Проверяем горизонтальные линии. i - y, j - x
		linesize = 0;
		for(j = 0; j < 10; j++) {
			if( (linesize == 0) && (cl_balls[j+i*10].isball == 1) ) {
				linesize = 1;
				currentcolor = cl_balls[j+i*10].color;
			} else {
				if( (currentcolor == cl_balls[j+i*10].color) && (cl_balls[j+i*10].isball == 1) ) {
					linesize++;
					if(j == 9 && linesize > 4) {
						cl_gamescore += linesize;
						result = true;
						cl_nofballs -= linesize;
						for(k = j+1-linesize; k <= j; k++)
							cl_balls[k+i*10].isball = 0;
					}
				} else {
					if(linesize > 4) {
						cl_gamescore += linesize;
						result = true;
						cl_nofballs -= linesize;
						for(k = j-linesize; k < j; k++)
							cl_balls[k+i*10].isball = 0;
					}

					if(cl_balls[j+i*10].isball == 1) {
						currentcolor = cl_balls[j+i*10].color;
						linesize = 1;
					} else {
						linesize = 0;
					}
				}
			}
		}

		// Проверяем вертикальные линии. i - x, j - y
		linesize = 0;
		for(j = 0; j < 10; j++) {
			if( (linesize == 0) && (cl_balls[i+j*10].isball == 1) ) {
				linesize = 1;
				currentcolor = cl_balls[i+j*10].color;
			} else {
				if( (currentcolor == cl_balls[i+j*10].color) && (cl_balls[i+j*10].isball == 1) ) {
					linesize++;
					if(j == 9 && linesize > 4) {
						cl_gamescore += linesize;
						result = true;
						cl_nofballs -= linesize;
						for(k = j+1-linesize; k <= j; k++)
							cl_balls[i+k*10].isball = 0;
					}
				} else {
					if(linesize > 4) {
						cl_gamescore += linesize;
						result = true;
						cl_nofballs -= linesize;
						for(k = j-linesize; k < j; k++)
							cl_balls[i+k*10].isball = 0;
					}

					if(cl_balls[i+j*10].isball == 1) {
						currentcolor = cl_balls[i+j*10].color;
						linesize = 1;
					} else {
						linesize = 0;
					}
				}
			}
		}
	}

	return result;
}

/*
	Функция	: clDrawBorder

	Описание: Рисует границу

	История	: 29.09.13	Создан

*/
static void clDrawBorder(void)
{
	unsigned int i;

	for(i = 0; i < 12; i++) {
		nvDraw2dPicture(128+32*i, 64, cl_ball2texid, 1.0, 1.0, CWHITE);
		nvDraw2dPicture(128+32*i, 64+384-32, cl_ball2texid, 1.0, 1.0, CWHITE);
	}

	for(i = 0; i < 10; i++) {
		nvDraw2dPicture(128, 64+32+32*i, cl_ball2texid, 1.0, 1.0, CWHITE);
		nvDraw2dPicture(128+384-32, 64+32+32*i, cl_ball2texid, 1.0, 1.0, CWHITE);
	}
}

/*
	Функция	: clGetRandomColor

	Описание: Возвращает рандомный цвет

	История	: 29.09.13	Создан

*/
static unsigned int clGetRandomColor(void)
{
	switch(rand()%7) {
		case 0:
			return CWHITE;
		case 1:
			return CRED;
		case 2:
			return CGREEN;
		case 3:
			return CBLUE;
		case 4:
			return CPINK;
		case 5:
			return CLBLUE;
		case 6:
			return CYELLOW;
	}

	return CWHITE;
}

/*
	Функция	: clAddNewBalls

	Описание: Добавляет новые шары на карту. Возвращает 1, если шарики добавлены, иначе 0

	История	: 29.09.13	Создан

*/
static bool clAddNewBalls(void)
{
	unsigned int nofemptyballids;
	unsigned int emptyballids[3]; // Номера пустых шаров, куда будут помещены новые шары
	unsigned int i, j, k;

	// Считаем количество id пустых шаров
	nofemptyballids = 0;
	for(i = 0; i < 100; i++)
		if(cl_balls[i].isball == 0)
			nofemptyballids++;

	if(nofemptyballids < cl_difficulty)
		return false;

	// Получаем значения для пустых шаров
	for(i = 0; i < cl_difficulty; i++) {
		while(1) {
			emptyballids[i] = rand()%nofemptyballids;

			for(j = 0; j < i; j++) {
				if(emptyballids[j] == emptyballids[i]) break;
			}

			if(j == i) break;
		}
	}

	//printf("emptyballids:");
	//for(i = 0; i < cl_difficulty; i++)
	//	printf(" %d",emptyballids[i]);
	//printf("\n");

	// Добавляем новые шары к старым шарам
	k = 0; // Номер текущего пустого шара
	for(i = 0; i < 100; i++) {
		if(cl_balls[i].isball == 0) {
			for(j = 0; j < cl_difficulty; j++)
				if(k == emptyballids[j]) {
					cl_balls[i].isball = 1;
					cl_balls[i].color = clGetRandomColor();
					break;
				}
			k++;
		}
	}

	cl_nofballs += cl_difficulty;

	return true;
}

/*
	Функция	: clDrawBalls

	Описание: Рисует шары. Возвращает номер нажатого шара, иначе 0

	История	: 29.09.13	Создан

*/
static unsigned int clDrawBalls(void)
{
	unsigned int i, j, ballid;

	unsigned int mx, my, mbl;

	mx = nvGetStatusi(NV_STATUS_WINMX);
	my = nvGetStatusi(NV_STATUS_WINMY);
	mbl = nvGetStatusi(NV_STATUS_WINMBL);

	for(i = 0; i < 10; i++) {
		for(j = 0; j < 10; j++) {
			if(cl_balls[j+i*10].isball == 1) {
				nvDraw2dPicture(128+32+32*j, 64+32+32*i, ((i*10+j+1) == cl_selectedball)?cl_ball2texid:cl_ball1texid, 1.0, 1.0, cl_balls[j+i*10].color);
			}
		}
	}

	if( (mbl != 2) || (mx < (128+32)) || (my < (64+32)) || (mx > (128+32+320)) || (my > (64+32+320)) ) {
		ballid = 0;
	} else {
		mx -= 128+32;
		my -= 64+32;
		mx /= 32;
		my /= 32;
		ballid = mx+my*10+1;
		//printf("%d\n",ballid);
	}

	return ballid;
}

/*
	Функция	: clProcessGame

	Описание: Обрабатывет игру

	История	: 29.09.13	Создан

*/
int clProcessGame(void)
{
	wchar_t stemp[80];
	unsigned int newselectedball;
	int name_edit_status;

	if(nvGetStatusi(NV_STATUS_WIN_EXITMSG)) return 0;
	if(cl_isgameover == true)
		 if((nvGetKey(27) == 2) || wcschr(cl_your_name, 13) || wcschr(cl_your_name, 10) ) {
			wchar_t *temp;
			temp = wcschr(cl_your_name, 13); if(temp) *temp = 0;
			temp = wcschr(cl_your_name, 10); if(temp) *temp = 0;

			clScoreAdd(cl_your_name, cl_gamescore);

			return 4;
		}

	nvBegin2d();
		swprintf(stemp, 80, L"%ls%u", L"Your score: ", cl_gamescore);
		nvDraw2dText(stemp, 29, 5, cl_fontid, cl_fonttexid, 3.04f, 3.04f, CBLACK);
		nvDraw2dText(stemp, 32, 8, cl_fontid, cl_fonttexid, 3.0, 3.0, CWHITE);

		clDrawBorder();
		newselectedball = clDrawBalls();
		if(cl_isgameover) {
			nvDraw2dText(L"Game Over", 165, 157, cl_fontid, cl_fonttexid, 3.04f, 3.04f, CBLACK);
			nvDraw2dText(L"Game Over", 168, 160, cl_fontid, cl_fonttexid, 3.0, 3.0, CWHITE);
			nvDraw2dText(L"Enter your name:", 168, 460, cl_fontid, cl_fonttexid, 1.0, 1.0, CWHITE);

			name_edit_status = 1;
			nvDraw2dTextbox(cl_your_name, &name_edit_status, 16, '_', 500, 336, 460, cl_fontid, cl_fonttexid, 1.0, 1.0, CWHITE, CWHITE);
		} else {
			if(newselectedball) {
				if(cl_selectedball) {
					if(cl_balls[newselectedball-1].isball == 1) { // Меняем номер выбранного шара
						cl_selectedball = newselectedball;
					} else { // Перемещаем шары, если возможно
						if(clCheckPath(cl_selectedball, newselectedball)) {
							unsigned int need_more_balls;

							//printf("move %d to %d\n",cl_selectedball,newselectedball);
							cl_balls[newselectedball-1] = cl_balls[cl_selectedball-1];
							cl_balls[cl_selectedball-1].isball = 0;
							cl_selectedball = 0;

							// Добавляем новые шары только если в процессе хода не образовалась новая линия
							need_more_balls = !clDetectLines();

							if(need_more_balls) {
								cl_isgameover = !clAddNewBalls();

								// Проверяем, возможно новые шарики дали новую линию
								if(!cl_isgameover)
									clDetectLines();
							}
							//printf("cl_nofballs %d\n", cl_nofballs);
						}
					}
				} else if(cl_balls[newselectedball-1].isball == 1) {
					cl_selectedball = newselectedball;
					//printf("set cl_selectedball to %d\n",newselectedball);
				}
			}

			if(nvGetKey(27) == 2) cl_isgameover = true;
		}

		if(cl_nofballs == 100) cl_isgameover = true;
	nvEnd2d();

	return 2;
}
