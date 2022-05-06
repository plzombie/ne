//-----------------------------------------------------------------------------
// File: mp_wrap.h
//
// Copyright (c) Astralax. All rights reserved.
// Author: Sedov Alexey
// Adapted from HGE to nyan engine by plzombie
//-----------------------------------------------------------------------------

#ifndef MAGIC_PARTICLES_WRAPPER
#define MAGIC_PARTICLES_WRAPPER

#include "mp.h"
#include "../../nyan/nyan_publicapi.h"
/*#include "hge.h"
#include "hgefont.h"*/

// eng: Class storing the textural atlas
// rus: Класс, который хранит текстурный атлас
class MP_Atlas_WRAP : public MP_Atlas
{
protected:
	unsigned int texture;

	nv_2dvertex_type quad[4];
	//hgeQuad quad;
	
public:
	MP_Atlas_WRAP(int width, int height, const char* file);
	virtual ~MP_Atlas_WRAP(){}

	// eng: Returns texture
	// rus: Возвращает текстуру
	unsigned int GetTexture(){return texture;}

	// eng: Destroy atlas texture
	// rus: Уничтожить текстуру атласа
	virtual void Destroy();

	// eng: Loading of frame texture
	// rus: Загрузка текстурного кадра
	virtual void LoadTexture(const MAGIC_CHANGE_ATLAS* c);

	// eng: Cleaning up of rectangle
	// rus: Очистка прямоугольника
	virtual void CleanRectangle(const MAGIC_CHANGE_ATLAS* c);

	// eng: Particle drawing
	// rus: Отрисовка частицы
	virtual void Draw(MAGIC_PARTICLE_VERTEXES* vertexes);

	// eng: Setting of intense
	// rus: Установить интенсивность
	virtual void SetIntense(bool intense);
};

// eng: Class controlling drawing
// rus: Класс, который управляет рисованием
class MP_Device_WRAP : public MP_Device
{
public:
	//HGE* hge;
	bool is_lost;

public:

	MP_Device_WRAP(int width, int height);

	//HGE* GetHGE(){return hge;}

	// eng: Creating
	// rus: Создание
	virtual bool Create(){return true;}

	// eng: Destroying
	// rus: Уничтожение
	virtual void Destroy(){}

	// eng: Beginning of scene drawing
	// rus: Начало отрисовки сцены
	virtual void BeginScene();

	// eng: End of scene drawing
	// rus: Конец отрисовки сцены
	virtual void EndScene();

	// eng: Indicates that device is lost
	// rus: Проверка на потерю устройства рисования
	virtual bool IsLost();

	// eng: Loading texture from file
	// rus: Загрузка текстуры из файла
	unsigned int LoadTextureFromFile(const char* file);
};


#endif