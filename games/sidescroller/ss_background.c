/*
	Файл	: ss_background.с

	Описание: Прорисовка фона игры

	История	: 24.05.14	Создан

*/

#include <stdlib.h>

#include "../../nyan/nyan_publicapi.h"

#include "ss_main.h"
#include "ss_background.h"

static int ss_bkg_type = SS_BKG_TYPE1;
static int ss_background_color = NV_COLOR(0,0,0,255);
static int ss_transp_bkg_color = NV_COLOR(0,0,0,0);
static int ss_fog_backgr_color = NV_COLOR(0,0,0,255);

#define SS_BKG_MAXOBJS 8

static float ss_bkg_objs_x[SS_BKG_MAXOBJS], ss_bkg_objs_y[SS_BKG_MAXOBJS], ss_bkg_objs_speed[SS_BKG_MAXOBJS];

/*
	Функция	: ssBkgSet

	Описание: Установка фона

	История	: 24.05.14	Создан

*/
void ssBkgSet(int bkgtype)
{
	unsigned int i;

	ss_bkg_type = bkgtype;

	if(ss_bkg_type < SS_BKG_TYPE_MIN) ss_bkg_type = SS_BKG_TYPE_MIN;
	if(ss_bkg_type > SS_BKG_TYPE_MAX) ss_bkg_type = SS_BKG_TYPE_MAX;

	switch(ss_bkg_type) {
		case SS_BKG_TYPE1:
			nvSetStatusf(NV_STATUS_WINBCRED,0.3f);
			nvSetStatusf(NV_STATUS_WINBCGREEN,0.3f);
			nvSetStatusf(NV_STATUS_WINBCBLUE,0.8f);
			ss_background_color = NV_COLOR(77,77,204,255);
			ss_transp_bkg_color = NV_COLOR(77,77,204,0);
			ss_fog_backgr_color = NV_COLOR(38,38,102,255);

			for(i = 0; i < SS_BKG_MAXOBJS; i++) {
				ss_bkg_objs_x[i] = (float)(-32+rand()%641);
				ss_bkg_objs_y[i] = (float)(-64+rand()%481);
				ss_bkg_objs_speed[i] = 10+(float)rand()/RAND_MAX*20;
			}

			break;
	}
}

/*
	Функция	: ssBkgUpdate

	Описание: Обновление фона и вывод на экран

	История	: 24.05.14	Создан

*/
void ssBkgUpdate(void)
{
	unsigned int i;

	for(i = 0; i < SS_BKG_MAXOBJS; i++) {
		nvDraw2dPicture((int)(ss_bkg_objs_x[i]*ss_xmult+ss_xoffset), (int)(ss_bkg_objs_y[i]*ss_ymult+ss_yoffset), ss_cloudtexid, ss_xmult, ss_ymult, CWHITE);

		ss_bkg_objs_y[i] += ss_bkg_objs_speed[i]*(float)nGetspf();

		if(ss_bkg_objs_y[i] > ORIG_SCREENH+32) {
			ss_bkg_objs_x[i] = (float)(-32+rand()%ORIG_SCREENW);
			ss_bkg_objs_y[i] = -64.f;
			ss_bkg_objs_speed[i] = 10+(float)rand()/RAND_MAX*10;
		}
	}
}

/*
	Функция	: ssFogUpdate

	Описание: Отрисовка "тумана войны" по бокам экрана для случаев, когда размер окна не соотвенствует ORIG_SCREENWxORIG_SCREENH

	История	: 16.03.18	Создан

*/
void ssFogUpdate(void)
{
	if(ss_xoffset != 0) {
		float x1, x2;
		
		x1 = ss_xoffset-32;
		x2 = ss_xoffset;
		
		if(ss_xoffset > 32) {
			nvJustDraw2dTriangle(0, 0, x1,       0, x1, ss_winy, ss_fog_backgr_color, ss_background_color, ss_background_color);
			nvJustDraw2dTriangle(0, 0, x1, ss_winy,  0, ss_winy, ss_fog_backgr_color, ss_background_color, ss_fog_backgr_color);
			
			nvJustDraw2dTriangle(ss_winx, 0, ss_winx-x1,       0, ss_winx-x1, ss_winy, ss_fog_backgr_color, ss_background_color, ss_background_color);
			nvJustDraw2dTriangle(ss_winx, 0, ss_winx-x1, ss_winy,    ss_winx, ss_winy, ss_fog_backgr_color, ss_background_color, ss_fog_backgr_color);
		}
		nvJustDraw2dTriangle(x1, 0, x2,       0, x2, ss_winy, ss_background_color, ss_transp_bkg_color, ss_transp_bkg_color);
		nvJustDraw2dTriangle(x1, 0, x2, ss_winy, x1, ss_winy, ss_background_color, ss_transp_bkg_color, ss_background_color);
		
		nvJustDraw2dTriangle(ss_winx-x1, 0, ss_winx-x2,       0, ss_winx-x2, ss_winy, ss_background_color, ss_transp_bkg_color, ss_transp_bkg_color);
		nvJustDraw2dTriangle(ss_winx-x1, 0, ss_winx-x2, ss_winy, ss_winx-x1, ss_winy, ss_background_color, ss_transp_bkg_color, ss_background_color);
	}
	
	if(ss_yoffset != 0) {
		float y1, y2;
		
		y1 = ss_yoffset-32;
		y2 = ss_yoffset;
		
		if(ss_yoffset > 32) {
			nvJustDraw2dTriangle(0, 0,       0, y1, ss_winx, y1, ss_fog_backgr_color, ss_background_color, ss_background_color);
			nvJustDraw2dTriangle(0, 0, ss_winx, y1, ss_winx,  0, ss_fog_backgr_color, ss_background_color, ss_fog_backgr_color);
			
			nvJustDraw2dTriangle(0, ss_winy,       0, ss_winy-y1, ss_winx, ss_winy-y1, ss_fog_backgr_color, ss_background_color, ss_background_color);
			nvJustDraw2dTriangle(0, ss_winy, ss_winx, ss_winy-y1, ss_winx, ss_winy, ss_fog_backgr_color, ss_background_color, ss_fog_backgr_color);
		}
		nvJustDraw2dTriangle(0, y1,       0, y2, ss_winx, y2, ss_background_color, ss_transp_bkg_color, ss_transp_bkg_color);
		nvJustDraw2dTriangle(0, y1, ss_winx, y2, ss_winx, y1, ss_background_color, ss_transp_bkg_color, ss_background_color);
		
		nvJustDraw2dTriangle(0, ss_winy-y1,       0, ss_winy-y2, ss_winx, ss_winy-y2, ss_background_color, ss_transp_bkg_color, ss_transp_bkg_color);
		nvJustDraw2dTriangle(0, ss_winy-y1, ss_winx, ss_winy-y2, ss_winx, ss_winy-y1, ss_background_color, ss_transp_bkg_color, ss_background_color);
	}
}
