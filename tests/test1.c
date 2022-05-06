
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#include "../nyan/nyan_publicapi.h"

const int CWHITE = NV_COLOR(255,255,255,255);
const int CRED = NV_COLOR(255,0,0,255);
const int CGREEN = NV_COLOR(0,255,0,255);
const int CBLUE = NV_COLOR(0,0,255,255);

NYAN_MAIN
{
	unsigned int texid, i;
	nv_2dvertex_type vs[4];

	NYAN_INIT

#if defined(N_CAUSEWAY)
	if(!nvAttachRender(L"nullgl")) return 0;
#else
	if(!nvAttachRender(L"ngl")) return 0;
#endif
	if(!naAttachLib(L"nullal")) return 0;

	nvSetStatusi(NV_STATUS_WINUPDATEINTERVAL, 16);

	nvSetStatusi(NV_STATUS_WINMODE, NV_MODE_WINDOWED);

	nMountDir(L"media");
	nMountDir(L"../media");
	nMountDir(L"../../media");
	nMountDir(L"../../../media");

	if(!nInit()) return 0;
	texid = nvCreateTextureFromFile(L"chihiro.tga", NGL_TEX_FLAGS_LINEARMIN|NGL_TEX_FLAGS_LINEARMAG|NGL_TEX_FLAGS_FOR2D);
	nvLoadTexture(texid);
	nvSetStatusf(NV_STATUS_WINBCRED,0.1f);
	nvSetStatusf(NV_STATUS_WINBCGREEN,0.0);
	nvSetStatusf(NV_STATUS_WINBCBLUE,0.1f);
	while(!nvGetStatusi(NV_STATUS_WIN_EXITMSG) && !nvGetKey(27)) {
		unsigned int winx, winy;
		double angle;

		winx = nvGetStatusi(NV_STATUS_WINX);
		winy = nvGetStatusi(NV_STATUS_WINY);
		nvBegin2d();
			// Вывод линий
			vs[0].x = (float)winx/16; vs[0].y = (float)winy/16; vs[0].z = 0; vs[0].colorRGBA = CWHITE; vs[0].tx = 0; vs[0].ty = 1;
			vs[1].x = (float)winx/2-(float)winx/16; vs[1].y = (float)winy/2-(float)winy/16; vs[1].z = 0; vs[1].colorRGBA = CRED; vs[1].tx = 1; vs[1].ty = 1;
			nvDraw2dLine(0, vs);
			vs[0].x = (float)winx/2-(float)winx/16; vs[0].y = (float)winy/16; vs[0].z = 0; vs[0].colorRGBA = CGREEN; vs[0].tx = 1; vs[0].ty = 0;
			vs[1].x = (float)winx/16; vs[1].y = (float)winy/2-(float)winy/16; vs[1].z = 0; vs[1].colorRGBA = CBLUE; vs[1].tx = 0; vs[1].ty = 0;
			nvDraw2dLine(0, vs);

			// Вывод треугольника
			vs[0].x = (float)winx/16; vs[0].y = (float)winy/2+(float)winy/16; vs[0].z = 0; vs[0].colorRGBA = CWHITE; vs[0].tx = 0; vs[0].ty = 1;
			vs[1].x = (float)winx/2-(float)winx/16; vs[1].y = (float)winy/2+(float)winy/16; vs[1].z = 0; vs[1].colorRGBA = CRED; vs[1].tx = 1; vs[1].ty = 1;
			vs[2].x = (float)winx/2-(float)winx/16; vs[2].y = (float)winy-(float)winy/16;	vs[2].z = 0; vs[2].colorRGBA = CGREEN; vs[2].tx = 1; vs[2].ty = 0;
			nvDraw2dTriangle(0, vs);

			// Вывод квадрата
			vs[0].x = (float)winx/2+(float)winx/16; vs[0].y = (float)winy/16; vs[0].z = 0; vs[0].colorRGBA = CWHITE; vs[0].tx = 0; vs[0].ty = 1;
			vs[1].x = (float)winx-(float)winx/16; vs[1].y = (float)winy/16; vs[1].z = 0; vs[1].colorRGBA = CRED; vs[1].tx = 1; vs[1].ty = 1;
			vs[2].x = (float)winx-(float)winx/16; vs[2].y = (float)winy/2-(float)winy/16; vs[2].z = 0; vs[2].colorRGBA = CGREEN; vs[2].tx = 1; vs[2].ty = 0;
			vs[3].x = (float)winx/2+(float)winx/16; vs[3].y = (float)winy/2-(float)winy/16; vs[3].z = 0; vs[3].colorRGBA = CBLUE; vs[3].tx = 0; vs[3].ty = 0;
			nvDraw2dQuad(0, vs);

			// Вывод текстурированного квадрата
			vs[0].x = (float)winx/2+(float)winx/16; vs[0].y = (float)winy/2+(float)winy/16; vs[0].z = 0; vs[0].colorRGBA = CWHITE; vs[0].tx = 0; vs[0].ty = 1;
			vs[1].x = (float)winx-(float)winx/16; vs[1].y = (float)winy/2+(float)winy/16; vs[1].z = 0; vs[1].colorRGBA = CWHITE; vs[1].tx = 1; vs[1].ty = 1;
			vs[2].x = (float)winx-(float)winx/16; vs[2].y = (float)winy-(float)winy/16; vs[2].z = 0; vs[2].colorRGBA = CWHITE; vs[2].tx = 1; vs[2].ty = 0;
			vs[3].x = (float)winx/2+(float)winx/16; vs[3].y = (float)winy-(float)winy/16; vs[3].z = 0; vs[3].colorRGBA = CWHITE; vs[3].tx = 0; vs[3].ty = 0;
			nvDraw2dQuad(texid, vs);

			// Вывод точек
			angle = 0;
			for(i=0;i<20;i++) {
				vs[0].x = (float)winx/2+(float)winx/16*(float)cos(angle); vs[0].y = (float)winy/2+(float)winy/16*(float)sin(angle); vs[0].z = 0; vs[0].colorRGBA = NV_COLOR(255*fabs(sin(angle)),0,255*fabs(cos(angle)),255); vs[0].tx = 0; vs[0].ty = 1;
				nvDraw2dPoint(0, vs);
				angle += 0.31415278; // += 2*PI/20
			}
		nvEnd2d();
		nUpdate();
	}
	nClose();

	NYAN_CLOSE

	return 0;
}
