/*
	Файл	: cl_score.h

	Описание: Заголовок для cl_score.c

	История	: 03.11.13	Создан

*/

extern int clProcessScore(void);
extern void clScoreLoad(void);
extern void clScoreSave(void);
extern void clScoreAdd(wchar_t *name, unsigned int score);
