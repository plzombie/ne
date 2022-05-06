
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#include "../nyan/nyan_publicapi.h"
#include "../forks/stb/stb_easy_font.h"

bool nyan_stb_easy_font_draw(float x, float y, char* text, float scalex, float scaley, unsigned int color);

NYAN_MAIN
{
	NYAN_INIT

	if(!nvAttachRender(L"ngl")) return 0;
	if(!naAttachLib(L"nullal")) return 0;

	nvSetStatusi(NV_STATUS_WINUPDATEINTERVAL, 16);
	nvSetScreen(512, 384, 32, NV_MODE_WINDOWED, 0);

	if(!nInit()) return 0;
	
	stb_easy_font_spacing(0);

	while(!nvGetStatusi(NV_STATUS_WIN_EXITMSG) && !nvGetKey(27)) {
		nvBegin2d();
			if(nyan_stb_easy_font_draw(10, 10, "Hello", 20.0, 10.0, NV_COLOR(255, 0, 0, 255)))
				nyan_stb_easy_font_draw(10, 110, "World", 17.5, 10.0, NV_COLOR(0, 255, 0, 255));
		nvEnd2d();
		nUpdate();
	}

	NYAN_CLOSE

	return 0;
}

typedef struct {
	float x;
	float y;
	float z;
	unsigned int color;
} nyan_stb_easy_font_vertex_type;

#define NYAN_STB_EASY_FONT_MAXVERTICES 1024

bool nyan_stb_easy_font_draw(float x, float y, char* text, float scalex, float scaley, unsigned int color)
{
	static nyan_stb_easy_font_vertex_type vertices[NYAN_STB_EASY_FONT_MAXVERTICES]; // Not thread-safe
	nv_2dvertex_type nvertices[4];
	int i, j, nof_quads, sizex, sizey, mx, my;

	nof_quads = stb_easy_font_print(0, 0, text, (unsigned char *)(&color), vertices, sizeof(nyan_stb_easy_font_vertex_type)*NYAN_STB_EASY_FONT_MAXVERTICES);

	j = 0;
	for(i = 0; i < nof_quads; i++) {
		for(j = 0; j < 4; j++) {
			nvertices[j].x = x+scalex*vertices[i*4+j].x;
			nvertices[j].y = y+scaley*vertices[i*4+j].y;
			nvertices[j].z = 0;
			nvertices[j].tx = nvertices[j].ty = 0;
			nvertices[j].colorRGBA = vertices[i*4+j].color;
		}
		
		nvDraw2dQuad(0, nvertices);
	}

	sizex = (int)(scalex*stb_easy_font_width(text));
	sizey = (int)(scaley*stb_easy_font_height(text));
	mx = nvGetStatusi(NV_STATUS_WINMX);
	my = nvGetStatusi(NV_STATUS_WINMY);

	if(mx >= x && my >= y && mx < x+sizex && my < y+sizey) {
		int cr_sx, cr_sy, cr_ex, cr_ey;

		cr_sx = nvGetStatusi(NV_STATUS_WINCLIPPINGREGIONSX);
		cr_sy = nvGetStatusi(NV_STATUS_WINCLIPPINGREGIONSY);
		cr_ex = nvGetStatusi(NV_STATUS_WINCLIPPINGREGIONEX);
		cr_ey = nvGetStatusi(NV_STATUS_WINCLIPPINGREGIONEY);

		if(mx >= cr_sx && my >= cr_sy && mx < cr_ex && my < cr_ey)
			return true;
		else
			return false;
	} else
		return false;

}
