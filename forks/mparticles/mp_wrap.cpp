//-----------------------------------------------------------------------------
// File: mp_wrap.cpp
//
// Copyright (c) Astralax. All rights reserved.
// Author: Sedov Alexey
// Adapted from HGE to nyan engine by plzombie
//-----------------------------------------------------------------------------

#include "mp_wrap.h"

/*#pragma comment(lib, "hge.lib")
#pragma comment(lib, "hgehelp.lib")*/


// eng: Class storing the textural atlas
// rus: Класс, который хранит текстурный атлас
MP_Atlas_WRAP::MP_Atlas_WRAP(int width, int height, const char* file) : MP_Atlas(width, height, file)
{
	if (file_name.empty())
	{
		nv_texture_type tex;

		tex.sizex = width;
		tex.sizey = height;
		tex.nglrowalignment = 4;
		tex.nglcolorformat = NGL_COLORFORMAT_R8G8B8A8;
		tex.buffer = (unsigned char *)nAllocMemory(tex.sizex*tex.sizey*4);

		if(tex.buffer) {
			memset(tex.buffer, 0, tex.sizex*tex.sizey * 4);

			texture = nvCreateTextureFromMemory(&tex, NGL_TEX_FLAGS_FOR2D | NGL_TEX_FLAGS_LINEARMIN | NGL_TEX_FLAGS_LINEARMAG);
			if(!nvLoadTexture(texture)) {
				nvDestroyTexture(texture);
				texture = 0;
			}
			
			nFreeMemory(tex.buffer);
		} else {
			texture = 0;
		}
	}
	else
	{
		texture=device->LoadTextureFromFile(file_name.c_str());
	}

	quad[0].z = 0;
	quad[1].z = 0;
	quad[2].z = 0;
	quad[3].z = 0;
}

// eng: Destroy atlas texture
// rus: Уничтожить текстуру атласа
void MP_Atlas_WRAP::Destroy()
{
	if (texture)
	{
		nvDestroyTexture(texture);

		texture = 0;
	}
}

// eng: Loading of frame texture
// rus: Загрузка текстурного кадра
void MP_Atlas_WRAP::LoadTexture(const MAGIC_CHANGE_ATLAS* c)
{
	/*HGE* hge=device->GetHGE();

	HTEXTURE texture_from;
	if (c->data)
		texture_from=hge->Texture_Load(c->data,c->length,false);
	else
	{
		texture_from=device->LoadTextureFromFile(c->file);
	}

	int texture_from_width=hge->Texture_GetWidth(texture_from,true);
	int texture_from_height=hge->Texture_GetHeight(texture_from,true);
	hgeSprite* spr_from=new hgeSprite(texture_from,0.f,0.f,(float)texture_from_width,(float)texture_from_height);

	int frame_width=c->width;
	int frame_height=c->height;

	float frame_from_width=spr_from->GetWidth();
	float frame_from_height=spr_from->GetHeight();

	float scale_x=((float)frame_width)/frame_from_width;
	float scale_y=((float)frame_height)/frame_from_height;

	int x=c->x;
	int y=c->y;

	int pitch_to=hge->Texture_GetWidth(texture);
	int pitch_from=hge->Texture_GetWidth(texture_from);

	unsigned long* to=hge->Texture_Lock(texture, false, x, y, frame_width, frame_height);
	unsigned long* from=hge->Texture_Lock(texture_from, true, 0, 0, (int)frame_from_width, (int)frame_from_height);

	for (int i=0;i<frame_width;i++)
	{
		for (int j=0;j<frame_height;j++)
		{
			int i2=(int)(((float)i)/scale_x);
			int j2=(int)(((float)j)/scale_y);

			unsigned long color=from[j2*pitch_from+i2];
			to[j*pitch_to+i]=color;
		}
	}

	hge->Texture_Unlock(texture_from);
	hge->Texture_Unlock(texture);

	if (texture_from)
	{
		hge->Texture_Free(texture_from);
		texture_from=NULL;
	}

	if (spr_from)
	{
		delete spr_from;
		spr_from=NULL;
	}*/
}

// eng: Cleaning up of rectangle
// rus: Очистка прямоугольника
void MP_Atlas_WRAP::CleanRectangle(const MAGIC_CHANGE_ATLAS* c)
{
	float left = (float)c->x;
	float top = (float)c->y;
	float right = left+(float)(c->width-1);
	float bottom = top+(float)(c->height-1);

	quad[0].x = left; quad[0].y = top; quad[0].z = 0; quad[0].colorRGBA = NV_COLOR(0, 0, 0, 255); quad[0].tx = 0; quad[0].ty = 1;
	quad[1].x = right; quad[1].y = top; quad[1].z = 0; quad[1].colorRGBA = NV_COLOR(0, 0, 0, 255); quad[1].tx = 1; quad[1].ty = 1;
	quad[2].x = right; quad[2].y = bottom; quad[2].z = 0; quad[2].colorRGBA = NV_COLOR(0, 0, 0, 255); quad[2].tx = 1; quad[2].ty = 0;
	quad[3].x = left; quad[3].y = bottom; quad[3].z = 0; quad[3].colorRGBA = NV_COLOR(0, 0, 0, 255); quad[3].tx = 0; quad[3].ty = 0;
	nvDraw2dQuad(0, quad);
}

// eng: Particle drawing
// rus: Отрисовка частицы
void MP_Atlas_WRAP::Draw(MAGIC_PARTICLE_VERTEXES* vertexes)
{
	quad[0].x = vertexes->vertex1.x; quad[0].y = vertexes->vertex1.y; quad[0].colorRGBA = vertexes->color; quad[0].tx = vertexes->u1; quad[0].ty = 1.f-vertexes->v1;
	quad[1].x = vertexes->vertex2.x; quad[1].y = vertexes->vertex2.y; quad[1].colorRGBA = vertexes->color; quad[1].tx = vertexes->u2; quad[1].ty = 1.f-vertexes->v2;
	quad[2].x = vertexes->vertex3.x; quad[2].y = vertexes->vertex3.y; quad[2].colorRGBA = vertexes->color; quad[2].tx = vertexes->u3; quad[2].ty = 1.f-vertexes->v3;
	quad[3].x = vertexes->vertex4.x; quad[3].y = vertexes->vertex4.y; quad[3].colorRGBA = vertexes->color; quad[3].tx = vertexes->u4; quad[3].ty = 1.f-vertexes->v4;
	nvDraw2dQuad(texture, quad);
}

// eng: Setting of intense
// rus: Установить интенсивность
void MP_Atlas_WRAP::SetIntense(bool intense)
{
	/*if (intense)
		quad.blend=(BLEND_COLORMUL | BLEND_ALPHAADD | BLEND_NOZWRITE);
	else
		quad.blend=(BLEND_COLORMUL | BLEND_ALPHABLEND | BLEND_NOZWRITE);*/
}

// --------------------------------------------------------------------------------

// eng: Class controlling drawing
// rus: Класс, который управляет рисованием
MP_Device_WRAP::MP_Device_WRAP(int width, int height) : MP_Device(width, height)
{
	is_lost=false;
}

// eng: Beginning of scene drawing
// rus: Начало отрисовки сцены
void MP_Device_WRAP::BeginScene()
{
	MP_Device::BeginScene();

	nvBegin2d();
}

// eng: End of scene drawing
// rus: Конец отрисовки сцены
void MP_Device_WRAP::EndScene()
{
	nvEnd2d();
}

// eng: Indicates that device is lost
// rus: Проверка на потерю устройства рисования
bool MP_Device_WRAP::IsLost()
{
	if (is_lost)
	{
		is_lost=false;
		return true;
	}
	return false;
}

// eng: Loading texture from file
// rus: Загрузка текстуры из файла
unsigned int MP_Device_WRAP::LoadTextureFromFile(const char* file_name)
{
#if 0
	std::string texture_file=MP->GetPathToTexture();
	texture_file+=file_name;

	const wchar_t* texture_file_utf16=(const wchar_t*)Magic_UTF8to16((const unsigned char *)texture_file.c_str());
#else
	// Если требуется искать текстуры в папке textures, можно добавить строчку nMountDir(L"textures"); в код. Не зачем так хардкодить путь
	const wchar_t* texture_file_utf16 = (const wchar_t*)Magic_UTF8to16((const unsigned char *)file_name);
#endif

	unsigned int texture = nvCreateTextureFromFile((wchar_t *)texture_file_utf16, NGL_TEX_FLAGS_FOR2D | NGL_TEX_FLAGS_LINEARMIN | NGL_TEX_FLAGS_LINEARMAG);

	nvLoadTexture(texture);

	return texture;
}

