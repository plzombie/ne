/*
	Файл	: ss_background.h

	Описание: Заголовок для ss_background.c .

	История	: 24.05.14	Создан

*/

#define SS_BKG_TYPE1 1

#define SS_BKG_TYPE_MIN SS_BKG_TYPE1
#define SS_BKG_TYPE_MAX SS_BKG_TYPE1

extern void ssBkgSet(int bkgtype);
extern void ssBkgUpdate(void);
extern void ssFogUpdate(void);
