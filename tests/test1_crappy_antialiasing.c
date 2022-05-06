
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#include "../nyan/nyan_publicapi.h"

const int CWHITE = NV_COLOR(255,255,255,255);
const int CRED = NV_COLOR(255,0,0,255);
const int CGREEN = NV_COLOR(0,255,0,255);
const int CBLUE = NV_COLOR(0,0,255,255);

void nvDraw2dLineAntialiased(unsigned int batch_currenttex, nv_2dvertex_type *varray)
{
	nv_2dvertex_type _varray[2];
	int alpha[2], sub_alpha[2], sub_alpha2[2];
	int sub_rgb[2];
	
	_varray[0] = varray[0];
	_varray[1] = varray[1];
	
	alpha[0] = ((_varray[0].colorRGBA>>24)&0xff);
	sub_alpha[0] = alpha[0]/2;
	sub_alpha2[0] = alpha[0]-sub_alpha[0];
	sub_rgb[0] = _varray[0].colorRGBA&0xffffff;
	alpha[1] = ((_varray[1].colorRGBA>>24)&0xff);
	sub_alpha[1] = alpha[1]/2;
	sub_alpha2[1] = alpha[1]-sub_alpha[1];
	sub_rgb[1] = _varray[1].colorRGBA&0xffffff;
	
	_varray[0].x -= 0.25;
	_varray[0].y -= 0.25;
	_varray[0].colorRGBA = (sub_alpha[0]<<24)|(sub_rgb[0]);
	_varray[1].x -= 0.25;
	_varray[1].y -= 0.25;
	_varray[1].colorRGBA = (sub_alpha[1]<<24)|(sub_rgb[1]);
	
	nvDraw2dLine(batch_currenttex, _varray);
	
	_varray[0].x += 0.5;
	_varray[0].colorRGBA = (sub_alpha2[0]<<24)|(sub_rgb[0]);
	_varray[1].x += 0.5;
	_varray[1].colorRGBA = (sub_alpha2[1]<<24)|(sub_rgb[1]);
	
	nvDraw2dLine(batch_currenttex, _varray);
	
	_varray[0].y += 0.5;
	_varray[0].colorRGBA = (sub_alpha[0]<<24)|(sub_rgb[0]);
	_varray[1].y += 0.5;
	_varray[1].colorRGBA = (sub_alpha[1]<<24)|(sub_rgb[1]);
	
	nvDraw2dLine(batch_currenttex, _varray);
	
	_varray[0].x -= 0.5;
	_varray[0].colorRGBA = (sub_alpha2[0]<<24)|(sub_rgb[0]);
	_varray[1].x -= 0.5;
	_varray[1].colorRGBA = (sub_alpha2[1]<<24)|(sub_rgb[1]);
	
	nvDraw2dLine(batch_currenttex, _varray);
}

void nvDraw2dTriangleAntialiased(unsigned int batch_currenttex, nv_2dvertex_type *varray)
{
	nv_2dvertex_type _varray[3];
	int alpha[3], sub_alpha[3], sub_alpha2[3];
	int sub_rgb[3];
	
	_varray[0] = varray[0];
	_varray[1] = varray[1];
	_varray[2] = varray[2];
	
	alpha[0] = ((_varray[0].colorRGBA>>24)&0xff);
	sub_alpha[0] = alpha[0]/2;
	sub_alpha2[0] = alpha[0]-sub_alpha[0];
	sub_rgb[0] = _varray[0].colorRGBA&0xffffff;
	alpha[1] = ((_varray[1].colorRGBA>>24)&0xff);
	sub_alpha[1] = alpha[1]/2;
	sub_alpha2[1] = alpha[1]-sub_alpha[1];
	sub_rgb[1] = _varray[1].colorRGBA&0xffffff;
	alpha[2] = ((_varray[2].colorRGBA>>24)&0xff);
	sub_alpha[2] = alpha[2]/2;
	sub_alpha2[2] = alpha[2]-sub_alpha[2];
	sub_rgb[2] = _varray[2].colorRGBA&0xffffff;
	
	_varray[0].x -= 0.25;
	_varray[0].y -= 0.25;
	_varray[0].colorRGBA = (sub_alpha[0]<<24)|(sub_rgb[0]);
	_varray[1].x -= 0.25;
	_varray[1].y -= 0.25;
	_varray[1].colorRGBA = (sub_alpha[1]<<24)|(sub_rgb[1]);
	_varray[2].x -= 0.25;
	_varray[2].y -= 0.25;
	_varray[2].colorRGBA = (sub_alpha[2]<<24)|(sub_rgb[2]);

	nvDraw2dTriangle(batch_currenttex, _varray);
	
	_varray[0].x += 0.5;
	_varray[0].colorRGBA = (sub_alpha2[0]<<24)|(sub_rgb[0]);
	_varray[1].x += 0.5;
	_varray[1].colorRGBA = (sub_alpha2[1]<<24)|(sub_rgb[1]);
	_varray[2].x += 0.5;
	_varray[2].colorRGBA = (sub_alpha2[2]<<24)|(sub_rgb[2]);
	
	nvDraw2dTriangle(batch_currenttex, _varray);
	
	_varray[0].y += 0.5;
	_varray[0].colorRGBA = (sub_alpha[0]<<24)|(sub_rgb[0]);
	_varray[1].y += 0.5;
	_varray[1].colorRGBA = (sub_alpha[1]<<24)|(sub_rgb[1]);
	_varray[2].y += 0.5;
	_varray[2].colorRGBA = (sub_alpha[2]<<24)|(sub_rgb[2]);
	
	nvDraw2dTriangle(batch_currenttex, _varray);

	_varray[0].x -= 0.5;
	_varray[0].colorRGBA = (sub_alpha2[0]<<24)|(sub_rgb[0]);
	_varray[1].x -= 0.5;
	_varray[1].colorRGBA = (sub_alpha2[1]<<24)|(sub_rgb[1]);
	_varray[2].x -= 0.5;
	_varray[2].colorRGBA = (sub_alpha2[2]<<24)|(sub_rgb[2]);
	
	nvDraw2dTriangle(batch_currenttex, _varray);
}

void nvDraw2dQuadAntialiased(unsigned int batch_currenttex, nv_2dvertex_type *varray)
{
	nv_2dvertex_type _varray[4];
	int alpha[4], sub_alpha[4], sub_alpha2[4];
	int sub_rgb[4];
	
	_varray[0] = varray[0];
	_varray[1] = varray[1];
	_varray[2] = varray[2];
	_varray[3] = varray[3];
	
	alpha[0] = ((_varray[0].colorRGBA>>24)&0xff);
	sub_alpha[0] = alpha[0]/2;
	sub_alpha2[0] = alpha[0]-sub_alpha[0];
	sub_rgb[0] = _varray[0].colorRGBA&0xffffff;
	alpha[1] = ((_varray[1].colorRGBA>>24)&0xff);
	sub_alpha[1] = alpha[1]/2;
	sub_alpha2[1] = alpha[1]-sub_alpha[1];
	sub_rgb[1] = _varray[1].colorRGBA&0xffffff;
	alpha[2] = ((_varray[2].colorRGBA>>24)&0xff);
	sub_alpha[2] = alpha[2]/2;
	sub_alpha2[2] = alpha[2]-sub_alpha[2];
	sub_rgb[2] = _varray[2].colorRGBA&0xffffff;
	alpha[3] = ((_varray[3].colorRGBA>>24)&0xff);
	sub_alpha[3] = alpha[3]/2;
	sub_alpha2[3] = alpha[3]-sub_alpha[3];
	sub_rgb[3] = _varray[3].colorRGBA&0xffffff;
	
	_varray[0].x -= 0.25;
	_varray[0].y -= 0.25;
	_varray[0].colorRGBA = (sub_alpha[0]<<24)|(sub_rgb[0]);
	_varray[1].x -= 0.25;
	_varray[1].y -= 0.25;
	_varray[1].colorRGBA = (sub_alpha[1]<<24)|(sub_rgb[1]);
	_varray[2].x -= 0.25;
	_varray[2].y -= 0.25;
	_varray[2].colorRGBA = (sub_alpha[2]<<24)|(sub_rgb[2]);
	_varray[3].x -= 0.25;
	_varray[3].y -= 0.25;
	_varray[3].colorRGBA = (sub_alpha[3]<<24)|(sub_rgb[3]);
	
	nvDraw2dQuad(batch_currenttex, _varray);
	
	_varray[0].x += 0.5;
	_varray[0].colorRGBA = (sub_alpha2[0]<<24)|(sub_rgb[0]);
	_varray[1].x += 0.5;
	_varray[1].colorRGBA = (sub_alpha2[1]<<24)|(sub_rgb[1]);
	_varray[2].x += 0.5;
	_varray[2].colorRGBA = (sub_alpha2[2]<<24)|(sub_rgb[2]);
	_varray[3].x += 0.5;
	_varray[3].colorRGBA = (sub_alpha2[3]<<24)|(sub_rgb[3]);
	
	nvDraw2dQuad(batch_currenttex, _varray);
	
	_varray[0].y += 0.5;
	_varray[0].colorRGBA = (sub_alpha[0]<<24)|(sub_rgb[0]);
	_varray[1].y += 0.5;
	_varray[1].colorRGBA = (sub_alpha[1]<<24)|(sub_rgb[1]);
	_varray[2].y += 0.5;
	_varray[2].colorRGBA = (sub_alpha[2]<<24)|(sub_rgb[2]);
	_varray[3].y += 0.5;
	_varray[3].colorRGBA = (sub_alpha[3]<<24)|(sub_rgb[3]);
	
	nvDraw2dQuad(batch_currenttex, _varray);
	
	_varray[0].x -= 0.5;
	_varray[0].colorRGBA = (sub_alpha2[0]<<24)|(sub_rgb[0]);
	_varray[1].x -= 0.5;
	_varray[1].colorRGBA = (sub_alpha2[1]<<24)|(sub_rgb[1]);
	_varray[2].x -= 0.5;
	_varray[2].colorRGBA = (sub_alpha2[2]<<24)|(sub_rgb[2]);
	_varray[3].x -= 0.5;
	_varray[3].colorRGBA = (sub_alpha2[3]<<24)|(sub_rgb[3]);
	
	nvDraw2dQuad(batch_currenttex, _varray);
}

void nvDraw2dPointAntialiased(unsigned int batch_currenttex, nv_2dvertex_type *varray)
{
	nv_2dvertex_type _varray;
	int alpha, sub_alpha, sub_alpha2;
	int sub_rgb;
	
	_varray = *varray;
	
	alpha = ((_varray.colorRGBA>>24)&0xff);
	sub_alpha = alpha/2;
	sub_alpha2 = alpha-sub_alpha*2;
	sub_rgb = _varray.colorRGBA&0xffffff;
	
	_varray.colorRGBA = (sub_alpha<<24)|(sub_rgb);
	
	nvDraw2dPoint(batch_currenttex, &_varray);
	
	_varray.x += 0.5;
	_varray.colorRGBA = (sub_alpha2<<24)|(sub_rgb);
	
	nvDraw2dPoint(batch_currenttex, &_varray);
	
	_varray.y += 0.5;
	_varray.colorRGBA = (sub_alpha<<24)|(sub_rgb);
	
	nvDraw2dPoint(batch_currenttex, &_varray);
	
	_varray.x -= 0.5;
	_varray.colorRGBA = (sub_alpha2<<24)|(sub_rgb);
	
	nvDraw2dPoint(batch_currenttex, &_varray);
}

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
			nvDraw2dLineAntialiased(0, vs);
			vs[0].x = (float)winx/2-(float)winx/16; vs[0].y = (float)winy/16; vs[0].z = 0; vs[0].colorRGBA = CGREEN; vs[0].tx = 1; vs[0].ty = 0;
			vs[1].x = (float)winx/16; vs[1].y = (float)winy/2-(float)winy/16; vs[1].z = 0; vs[1].colorRGBA = CBLUE; vs[1].tx = 0; vs[1].ty = 0;
			nvDraw2dLineAntialiased(0, vs);

			// Вывод треугольника
			vs[0].x = (float)winx/16; vs[0].y = (float)winy/2+(float)winy/16; vs[0].z = 0; vs[0].colorRGBA = CWHITE; vs[0].tx = 0; vs[0].ty = 1;
			vs[1].x = (float)winx/2-(float)winx/16; vs[1].y = (float)winy/2+(float)winy/16; vs[1].z = 0; vs[1].colorRGBA = CRED; vs[1].tx = 1; vs[1].ty = 1;
			vs[2].x = (float)winx/2-(float)winx/16; vs[2].y = (float)winy-(float)winy/16;	vs[2].z = 0; vs[2].colorRGBA = CGREEN; vs[2].tx = 1; vs[2].ty = 0;
			nvDraw2dTriangleAntialiased(0, vs);

			// Вывод квадрата
			vs[0].x = (float)winx/2+(float)winx/16; vs[0].y = (float)winy/16; vs[0].z = 0; vs[0].colorRGBA = CWHITE; vs[0].tx = 0; vs[0].ty = 1;
			vs[1].x = (float)winx-(float)winx/16; vs[1].y = (float)winy/16; vs[1].z = 0; vs[1].colorRGBA = CRED; vs[1].tx = 1; vs[1].ty = 1;
			vs[2].x = (float)winx-(float)winx/16; vs[2].y = (float)winy/2-(float)winy/16; vs[2].z = 0; vs[2].colorRGBA = CGREEN; vs[2].tx = 1; vs[2].ty = 0;
			vs[3].x = (float)winx/2+(float)winx/16; vs[3].y = (float)winy/2-(float)winy/16; vs[3].z = 0; vs[3].colorRGBA = CBLUE; vs[3].tx = 0; vs[3].ty = 0;
			nvDraw2dQuadAntialiased(0, vs);

			// Вывод текстурированного квадрата
			vs[0].x = (float)winx/2+(float)winx/16; vs[0].y = (float)winy/2+(float)winy/16; vs[0].z = 0; vs[0].colorRGBA = CWHITE; vs[0].tx = 0; vs[0].ty = 1;
			vs[1].x = (float)winx-(float)winx/16; vs[1].y = (float)winy/2+(float)winy/16; vs[1].z = 0; vs[1].colorRGBA = CWHITE; vs[1].tx = 1; vs[1].ty = 1;
			vs[2].x = (float)winx-(float)winx/16; vs[2].y = (float)winy-(float)winy/16; vs[2].z = 0; vs[2].colorRGBA = CWHITE; vs[2].tx = 1; vs[2].ty = 0;
			vs[3].x = (float)winx/2+(float)winx/16; vs[3].y = (float)winy-(float)winy/16; vs[3].z = 0; vs[3].colorRGBA = CWHITE; vs[3].tx = 0; vs[3].ty = 0;
			nvDraw2dQuadAntialiased(texid, vs);

			// Вывод точек
			angle = 0;
			for(i=0;i<20;i++) {
				vs[0].x = (float)winx/2+(float)winx/16*(float)cos(angle); vs[0].y = (float)winy/2+(float)winy/16*(float)sin(angle); vs[0].z = 0; vs[0].colorRGBA = NV_COLOR(255*fabs(sin(angle)),0,255*fabs(cos(angle)),255); vs[0].tx = 0; vs[0].ty = 1;
				nvDraw2dPointAntialiased(0, vs);
				angle += 0.31415278; // += 2*PI/20
			}
		nvEnd2d();
		nUpdate();
	}
	nClose();

	NYAN_CLOSE

	return 0;
}
