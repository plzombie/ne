#include "softgl.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

void SaveRawPic(char *filename)
{
	unsigned int sx, sy, scolortype;
	unsigned char *surface;
	FILE *f;

	if(!sglGetSurface(&sx, &sy, &scolortype, &surface)) return;

	if(scolortype != SGL_COLORFORMAT_R8G8B8A8) return;

	f = fopen(filename, "wb");
	if(!f) return;
	fwrite(surface, 4, sx*sy, f);
	fclose(f);
}

const int CWHITE = SGL_COLOR(255,255,255,255);
const int CRED = SGL_COLOR(255,0,0,255);
const int CGREEN = SGL_COLOR(0,255,0,255);
const int CBLUE = SGL_COLOR(0,0,255,255); 

int main(void)
{
	unsigned int texid = 0, winx = 640, winy = 480, i;
	float angle;
	unsigned int timer;
	unsigned char *buffer;
	sgl_2dvertex_type vs[4];
	FILE *f;

	sglCreateSurface(winx, winy, SGL_COLORFORMAT_R8G8B8A8);
	sglClearSurface(0.1, 0, 0.1, 1.0);

	f = fopen("Cover.raw", "rb");
	buffer = malloc(2878*2864*4);
	fread(buffer, 4, 2878*2864, f);
	texid = sglCreateTexture(2878, 2864, SGL_COLORFORMAT_R8G8B8, 1, buffer);
	free(buffer);
	fclose(f);

	timer = clock();
for(i=0;i<1000;i++) {	
	// Вывод линий
	vs[0].x = (float)winx/16; vs[0].y = (float)winy/16; vs[0].z = 0; vs[0].colorRGBA = CWHITE; vs[0].tx = 0; vs[0].ty = 1;
	vs[1].x = (float)winx/2-(float)winx/16; vs[1].y = (float)winy/2-(float)winy/16; vs[1].z = 0; vs[1].colorRGBA = CRED; vs[1].tx = 1; vs[1].ty = 1;
	sglDraw2d(SGL_DRAWLINE, 0, 2, vs);
	vs[0].x = (float)winx/2-(float)winx/16; vs[0].y = (float)winy/16; vs[0].z = 0; vs[0].colorRGBA = CGREEN; vs[0].tx = 1; vs[0].ty = 0;
	vs[1].x = (float)winx/16; vs[1].y = (float)winy/2-(float)winy/16; vs[1].z = 0; vs[1].colorRGBA = CBLUE; vs[1].tx = 0; vs[1].ty = 0;
	sglDraw2d(SGL_DRAWLINE, 0, 2, vs);

	// Вывод треугольника
	vs[0].x = (float)winx/16; vs[0].y = (float)winy/2+(float)winy/16; vs[0].z = 0; vs[0].colorRGBA = CWHITE; vs[0].tx = 0; vs[0].ty = 1;
	vs[1].x = (float)winx/2-(float)winx/16; vs[1].y = (float)winy/2+(float)winy/16; vs[1].z = 0; vs[1].colorRGBA = CRED; vs[1].tx = 1; vs[1].ty = 1;
	vs[2].x = (float)winx/2-(float)winx/16; vs[2].y = (float)winy-(float)winy/16;	vs[2].z = 0; vs[2].colorRGBA = CGREEN; vs[2].tx = 1; vs[2].ty = 0;
	sglDraw2d(SGL_DRAWTRIANGLE, 0, 3, vs);

	// Вывод квадрата
	vs[0].x = (float)winx/2+(float)winx/16; vs[0].y = (float)winy/16; vs[0].z = 0; vs[0].colorRGBA = CWHITE; vs[0].tx = 0; vs[0].ty = 1;
	vs[1].x = (float)winx-(float)winx/16; vs[1].y = (float)winy/16; vs[1].z = 0; vs[1].colorRGBA = CRED; vs[1].tx = 1; vs[1].ty = 1;
	vs[2].x = (float)winx-(float)winx/16; vs[2].y = (float)winy/2-(float)winy/16; vs[2].z = 0; vs[2].colorRGBA = CGREEN; vs[2].tx = 1; vs[2].ty = 0;
	sglDraw2d(SGL_DRAWTRIANGLE, 0, 3, vs);
	vs[1] = vs[2];
	vs[2].x = (float)winx/2+(float)winx/16; vs[2].y = (float)winy/2-(float)winy/16; vs[2].z = 0; vs[2].colorRGBA = CBLUE; vs[2].tx = 0; vs[2].ty = 0;
	sglDraw2d(SGL_DRAWTRIANGLE, 0, 3, vs);

	// Вывод текстурированного квадрата
	vs[0].x = (float)winx/2+(float)winx/16; vs[0].y = (float)winy/2+(float)winy/16; vs[0].z = 0; vs[0].colorRGBA = CWHITE; vs[0].tx = 0; vs[0].ty = 1;
	vs[1].x = (float)winx-(float)winx/16; vs[1].y = (float)winy/2+(float)winy/16; vs[1].z = 0; vs[1].colorRGBA = CWHITE; vs[1].tx = 1; vs[1].ty = 1;
	vs[2].x = (float)winx-(float)winx/16; vs[2].y = (float)winy-(float)winy/16; vs[2].z = 0; vs[2].colorRGBA = CWHITE; vs[2].tx = 1; vs[2].ty = 0;
	sglDraw2d(SGL_DRAWTRIANGLE, texid, 3, vs);
	vs[1] = vs[2];
	vs[2].x = (float)winx/2+(float)winx/16; vs[2].y = (float)winy-(float)winy/16; vs[2].z = 0; vs[2].colorRGBA = CWHITE; vs[2].tx = 0; vs[2].ty = 0;
	sglDraw2d(SGL_DRAWTRIANGLE, texid, 3, vs);
		

	// Вывод треугольника
	vs[0].x = (float)0; vs[0].y = (float)0; vs[0].z = 0; vs[0].colorRGBA = CWHITE; vs[0].tx = 1; vs[0].ty = 1;
	vs[1].x = (float)16; vs[1].y = (float)0; vs[1].z = 0; vs[1].colorRGBA = CWHITE; vs[1].tx = 0; vs[1].ty = 1;
	vs[2].x = (float)16; vs[2].y = (float)16;	vs[2].z = 0; vs[2].colorRGBA = CWHITE; vs[2].tx = 0; vs[2].ty = 0;
	sglDraw2d(SGL_DRAWTRIANGLE, texid, 3, vs);
	sglDraw2d(SGL_DRAWLINE, texid, 2, vs);
	sglDraw2d(SGL_DRAWLINE, texid, 2, vs+1);

	//vs[0].x = (float)1; vs[0].y = (float)17; vs[0].z = 0; vs[0].colorRGBA = CWHITE; vs[0].tx = 0; vs[0].ty = 1;
	//vs[1].x = (float)1; vs[1].y = (float)17; vs[1].z = 0; vs[1].colorRGBA = CRED; vs[1].tx = 1; vs[1].ty = 1;
	//sglDraw2d(SGL_DRAWLINE, 0, 2, vs);
}

	// Вывод точек
	angle = 0;
	for(i=0;i<20;i++) {
		vs[0].x = (float)winx/2+(float)winx/16*cos(angle); vs[0].y = (float)winy/2+(float)winy/16*sin(angle); vs[0].z = 0; vs[0].colorRGBA = SGL_COLOR(255*fabs(sin(angle)),0,255*fabs(cos(angle)),255); vs[0].tx = 0; vs[0].ty = 1;
		sglDraw2d(SGL_DRAWPOINT, 0, 1, vs);
		angle += 0.31415278; // += 2*PI/20
	}

	sglFlipBuffers();

	timer = clock()-timer;
	printf("rendering time: %u ms", timer);

	SaveRawPic("example1.raw");
	sglDestroySurface();
	sglDestroyAllTextures();

	return 0;
}
