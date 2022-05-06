/*
	Файл	: ss_main.h

	Описание: Заголовок для ss_main.c . Большинство глобальных переменных будут объявлены именно в этом файле

	История	: 24.05.14	Создан

*/

enum {
	SS_GAME_STATE_EXIT,
	SS_GAME_STATE_MAINMENU,
	SS_GAME_STATE_GAMEPROCESS
};

#define SS_DEFAULT_SCREEN_X

extern const int ORIG_SCREENW;
extern const int ORIG_SCREENH;

extern const int CWHITE;

extern unsigned int ss_cloudtexid;

extern unsigned int ss_fonttexid;
extern unsigned int ss_fontid;

extern float ss_xmult, ss_xoffset, ss_ymult, ss_yoffset;
extern float ss_winx, ss_winy;
