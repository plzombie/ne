/*
	Файл	: demo_dna.с

	Описание: DNA demo

	История	: 31.08.14	Создан

*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "../nyan/nyan_publicapi.h"

const int CWHITE = NV_COLOR(255,255,255,255);

typedef struct {
	double oldx;
	double y;
	double a;
	int cr;
	int cg;
	int cb;
	int ca;
	int ca2;
} ball_type;

const double PI = 3.1415926535897932;
const int ORIG_SCREENW = 800;
const int ORIG_SCREENH = 600;

double frand()
{
	return (double)rand()/RAND_MAX;
}

NYAN_MAIN
{
	unsigned int texid, i, j, colorRGBA;
	ball_type ball[96];

	NYAN_INIT

	if(!nvAttachRender(L"ngl")) return 0;
	if(!naAttachLib(L"nullal")) return 0;

	nvSetStatusw(NV_STATUS_WINTITLE, L"DNA");

	nvSetScreen(ORIG_SCREENW, ORIG_SCREENH, 32, NV_MODE_WINDOWED, 0);

	nMountDir(L"media");
	nMountDir(L"../media");
	nMountDir(L"../../media");
	nMountDir(L"../../../media");

	if(!nInit()) return 0;


	texid = nvCreateTextureFromFile(L"ball.tga", NGL_TEX_FLAGS_LINEARMIN | NGL_TEX_FLAGS_LINEARMAG | NGL_TEX_FLAGS_FOR2D);
	nvLoadTexture(texid);

	for(i = 0; i < 96; i++) {
		ball[i].oldx = 0;
		ball[i].y = (int)i*7-36;
		ball[i].a = (int)i*7.5*PI/180;
		ball[i].cr = 255;
		ball[i].cg = (int)(255*frand());
		ball[i].cb = (int)(255*frand());
	}

	while(!nvGetStatusi(NV_STATUS_WIN_EXITMSG) && !nvGetKey(27)) {
		int w, h, halfw;

		w = nvGetStatusi(NV_STATUS_WINX);
		h = nvGetStatusi(NV_STATUS_WINY);
		halfw = w/2;
		if(nGetspf() > 0) {
			for(i = 0; i < 96; i++) {
				ball[i].y += 100*nGetspf();

				if(ball[i].y >= 636)
					ball[i].y -= 672;

				ball[i].a += nGetspf();

				if((ball[i].oldx - (400-256*sin(ball[i].a))) > 0) {
					ball[i].ca = (int)( 127*fabs(sin(ball[i].a)) );
					ball[i].ca2 = (int)( 255-128*fabs(sin(ball[i].a)) );
				} else {
					ball[i].ca = (int)( 255-128*fabs(sin(ball[i].a)) );
					ball[i].ca2 = (int)( 127*fabs(sin(ball[i].a)) );
				}
				ball[i].oldx = 400-256*sin(ball[i].a);
			}
		}

		nvBegin2d();
			for(j = 0; j < 256; j++) {
				for(i = 0; i < 96; i++) {
					if(ball[i].ca == (int)j) {
						colorRGBA = NV_COLOR(ball[i].cr*ball[i].ca/255,ball[i].cg*ball[i].ca/255,ball[i].cb*ball[i].ca/255,255);
						nvDraw2dPicture(halfw-(int)(256*sin(ball[i].a)), (int)(ball[i].y*h/ORIG_SCREENH), texid, 1.0, 1.0, colorRGBA);
					}
					if(ball[i].ca2 == (int)j) {
						colorRGBA = NV_COLOR(ball[i].cr*ball[i].ca2/255,ball[i].cg*ball[i].ca2/255,ball[i].cb*ball[i].ca2/255,255);
						nvDraw2dPicture(halfw+(int)(256*sin(ball[i].a)), (int)(ball[i].y*h/ORIG_SCREENH), texid, 1.0, 1.0, colorRGBA);
					}
				}
			}
		nvEnd2d();
		nUpdate();
	}
	nClose();

	NYAN_CLOSE

	return 0;
}
