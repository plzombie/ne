
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "../nyan/nyan_publicapi.h"

#include <math.h>

//#define USE_ANIMATEDMODEL2
#define UNCOMPLETE_MIRROR_TEST

#ifdef UNCOMPLETE_MIRROR_TEST
	// Пока только 0, так как иначе часть здания будет загораживать вид зеркала.
	// Надо или отбрасывать часть за зеркалом при отрисовке обратной стороны, или не рисовать задние грани
	#define MIRROR_Z_POS 0
#endif

const int CWHITE = NV_COLOR(255,255,255,255);
const int CRED = NV_COLOR(255,0,0,255);
const int CGREEN = NV_COLOR(0,255,0,255);
const int CBLUE = NV_COLOR(0,0,255,255);

unsigned int modelid = 0, weaponid = 0, mapid = 0, standanimid = 0, runanimid = 0;

typedef struct {
	float camx, camy, camz, camrx, camry, camrz;
	double camera_matrix[16];
} camera_type;

typedef struct {
#ifdef USE_ANIMATEDMODEL2
	int64_t anim_start_time;
#else
	float frame;
#endif
	unsigned int animid, status;
	float x, y, z, m_yangle;
} scene_type;

void InitScene(scene_type *scene)
{
#ifdef USE_ANIMATEDMODEL2
	scene->anim_start_time = 0;
#else
	scene->frame = 0;
#endif
	scene->animid = 0; scene->status = 0;
	scene->x = 0;
	scene->y = 24;
	scene->z = 0;
	scene->m_yangle = 0;
}

void UpdateScene(scene_type *scene)
{
	if(nvGetKey('A')) {
		scene->m_yangle += (float)(nGetspf()*60);
	}
	if(nvGetKey('D')) {
		scene->m_yangle -= (float)(nGetspf()*60);
	}
	if(nvGetKey('W')) { // Модель направлена на ось x
		scene->x += (float)(cos(scene->m_yangle/57.295779513)*nGetspf()*48);
		scene->z += (float)(-sin(scene->m_yangle/57.295779513)*nGetspf()*48);
		if(scene->status != 1) {
			scene->status = 1;
			scene->animid = runanimid;
		#ifdef USE_ANIMATEDMODEL2
			scene->anim_start_time = nFrameStartClock();
		#else
			scene->frame = 0;
		#endif
		}
	} else if(nvGetKey('S')) {
		scene->x -= (float)(cos(scene->m_yangle/57.295779513)*nGetspf()*48);
		scene->z -= (float)(-sin(scene->m_yangle/57.295779513)*nGetspf()*48);
		if(scene->status != 1) {
			scene->status = 1;
			scene->animid = runanimid;
		#ifdef USE_ANIMATEDMODEL2
			scene->anim_start_time = nFrameStartClock();
		#else
			scene->frame = 0;
		#endif
		}
	} else {
		scene->animid = standanimid;
		if(scene->status != 0) {
			scene->status = 0;
		#ifdef USE_ANIMATEDMODEL2
			scene->anim_start_time = nFrameStartClock();
		#else
			scene->frame = 0;
		#endif
		}
	}
}

void InitCamera(camera_type *camera)
{
	camera->camx = 0;
	camera->camy = 24;
	camera->camz = 100;
	camera->camrx = 0;
	camera->camry = 0;
	camera->camrz = 0;

	memset(&camera->camera_matrix, 0, 16*sizeof(double));
}

void UpdateCamera(camera_type *camera)
{
	double mult;

	if(nvGetKey(NK_SHIFT))
		mult = 2.5;
	else
		mult = 1.0;
	if(nvGetKey('K'))
		camera->camz += (float)(nGetspf()*8*mult);
	if(nvGetKey('M'))
		camera->camz -= (float)(nGetspf()*8*mult);
	if(nvGetKey('O'))
		camera->camx += (float)(nGetspf()*8*mult);
	if(nvGetKey('I'))
		camera->camx -= (float)(nGetspf()*8*mult);
	if(nvGetKey('P') || nvGetKey(NK_PRIOR)) {
		camera->camy += (float)(nGetspf()*8*mult);
	}
	if(nvGetKey('L') || nvGetKey(NK_NEXT)) {
		camera->camy -= (float)(nGetspf()*8*mult);
	}
	if(nvGetKey(NK_UP)) {
		camera->camz += (float)(-cos(camera->camry/57.295779513)*nGetspf()*32*mult);
		camera->camx += (float)(-sin(camera->camry/57.295779513)*nGetspf()*32*mult);
	}
	if(nvGetKey(NK_DOWN)) {
		camera->camz -= (float)(-cos(camera->camry/57.295779513)*nGetspf()*32*mult);
		camera->camx -= (float)(-sin(camera->camry/57.295779513)*nGetspf()*32*mult);
	}
	if(nvGetKey(NK_LEFT)) {
		camera->camry += (float)(nGetspf()*45);
	}
	if(nvGetKey(NK_RIGHT)) {
		camera->camry -= (float)(nGetspf()*45);
	}
}

void SetCamera(camera_type *camera)
{
	nvMatrixSetCamera(camera->camx, camera->camy, camera->camz, camera->camrx, camera->camry, camera->camrz, camera->camera_matrix);
	nvSetMatrix(camera->camera_matrix);
	//nvSet3dCamera(camera->camx, camera->camy, camera->camz, camera->camrx, camera->camry, camera->camrz);
}

void DrawColoredBox(float x_min, float x_max, float y_min, float y_max, float z_min, float z_max, float cr, float cg, float cb, float ca)
{
	nv_3dvertex_type v[8];
	unsigned int indices[36] = {
		0, 1, 2, 
		0, 2, 3, 
		4, 5, 6,
		4, 6, 7,
		0, 1, 5,
		0, 5, 4,
		3, 2, 6,
		3, 6, 7,
		0, 4, 7,
		0, 7, 3,
		1, 5, 6,
		1, 6, 2};

	v[0].cr = v[1].cr = v[2].cr = v[3].cr = v[4].cr = v[5].cr = v[6].cr = v[7].cr = cr;
	v[0].cg = v[1].cg = v[2].cg = v[3].cg = v[4].cg = v[5].cg = v[6].cg = v[7].cg = cg;
	v[0].cb = v[1].cb = v[2].cb = v[3].cb = v[4].cb = v[5].cb = v[6].cb = v[7].cb = cb;
	v[0].ca = v[1].ca = v[2].ca = v[3].ca = v[4].ca = v[5].ca = v[6].ca = v[7].ca = ca;

	v[0].nx = v[1].nx = v[2].nx = v[3].nx = v[4].nx = v[5].nx = v[6].nx = v[7].nx = (x_min+x_max)/2; // Не знаю, насколько правильно
	v[0].ny = v[1].ny = v[2].ny = v[3].ny = v[4].ny = v[5].ny = v[6].ny = v[7].ny = (y_min +y_max)/2; // Не знаю, насколько правильно
	v[0].nz = v[1].nz = v[2].nz = v[3].nz = v[4].nz = v[5].nz = v[6].nz = v[7].nz = (z_min +z_max)/2; // Не знаю, насколько правильно

	v[0].x = x_min;
	v[0].y = y_min;
	v[0].z = z_min;
	
	v[1].x = x_max;
	v[1].y = y_min;
	v[1].z = z_min;

	v[2].x = x_max;
	v[2].y = y_max;
	v[2].z = z_min;

	v[3].x = x_min;
	v[3].y = y_max;
	v[3].z = z_min;

	v[4].x = x_min;
	v[4].y = y_min;
	v[4].z = z_max;

	v[5].x = x_max;
	v[5].y = y_min;
	v[5].z = z_max;

	v[6].x = x_max;
	v[6].y = y_max;
	v[6].z = z_max;

	v[7].x = x_min;
	v[7].y = y_max;
	v[7].z = z_max;

	nvDraw3dIndexedMesh(0, 8, v, 12, indices);
}

void DrawScene(scene_type *scene, camera_type *camera)
{
	double m1[16], m2[16];

#ifndef UNCOMPLETE_MIRROR_TEST
	nvMatrixSetTranslate(-scene->x, scene->y, -scene->z, m1);
	nvMatrixSetRotateY(scene->m_yangle + 180, m2);
	nvMatrixMult(m1, m2);
	nvMatrixSetRotateX(-90.0, m2);
	nvMatrixMult(m1, m2);
	nvMatrixSetMult(camera->camera_matrix, m1, m2);
	nvSetMatrix(m2);
#ifdef USE_ANIMATEDMODEL2
	nvDrawAnimated3dModel2(modelid, 0, scene->animid, scene->anim_start_time);
	nvDrawAnimated3dModel2(weaponid, 0, scene->animid, scene->anim_start_time);
#else
	nvDrawAnimated3dModel(modelid, 0, scene->animid, scene->frame, 0);
	nvDrawAnimated3dModel(weaponid, 0, scene->animid, scene->frame, 0);
#endif

	nvClear(NV_CLEAR_COLOR_BUFFER);
#endif

	nvMatrixSetRotateX(-90.0, m1);
	nvMatrixSetMult(camera->camera_matrix, m1, m2);
	nvSetMatrix(m2);
	nvDrawStatic3dModel(mapid, 0);

	nvMatrixSetTranslate(scene->x, scene->y, scene->z, m1);
	nvMatrixSetRotateY(scene->m_yangle, m2);
	nvMatrixMult(m1, m2);
	nvMatrixSetRotateX(-90.0, m2);
	nvMatrixMult(m1, m2);
	nvMatrixSetMult(camera->camera_matrix, m1, m2);
	nvSetMatrix(m2);
#ifdef USE_ANIMATEDMODEL2
	nvDrawAnimated3dModel2(modelid, 0, scene->animid, scene->anim_start_time);
	nvDrawAnimated3dModel2(weaponid, 0, scene->animid, scene->anim_start_time);
#else
	nvDrawAnimated3dModel(modelid, 0, scene->animid, scene->frame, 0);
	nvDrawAnimated3dModel(weaponid, 0, scene->animid, scene->frame, &scene->frame);
#endif
}

void MirrorSetRevZMatrix(double rev_z[16])
{
	double matrix_temp[16], matrix_temp2[16];


	nvMatrixSetTranslate(0, 0, MIRROR_Z_POS, matrix_temp);

	nvMatrixSetScale(1.0, 1.0, -1.0, matrix_temp2);
	nvMatrixMult(matrix_temp, matrix_temp2);

	nvMatrixSetTranslate(0, 0, -(MIRROR_Z_POS), matrix_temp2);
	nvMatrixSetMult(matrix_temp, matrix_temp2, rev_z);
}


NYAN_MAIN
{
	unsigned int texid, fontid;
	bool gameloop = true;
	wchar_t fpst[16];
	scene_type scene;
	camera_type camera;
#ifdef UNCOMPLETE_MIRROR_TEST
	nv_3dvertex_type mirror[4];
	unsigned int mirror_indexes[6];
	scene_type mirrored_scene;
	camera_type mirrored_camera;
	double matrix_rev_z[16];
#endif

	NYAN_INIT

	if(!nvAttachRender(L"ngl")) return 0;
	if(!naAttachLib(L"nullal")) return 0;

	nvSetStatusi(NV_STATUS_WINMODE, NV_MODE_WINDOWED);

	nMountDir(L"media");
	nMountDir(L"media/fonts");
	nMountDir(L"media/players/ogro");
	nMountDir(L"../media");
	nMountDir(L"../media/fonts");
	nMountDir(L"../media/players/ogro");
	nMountDir(L"../../media");
	nMountDir(L"../../media/fonts");
	nMountDir(L"../../media/players/ogro");
	nMountDir(L"../../../media");
	nMountDir(L"../../../media/fonts");
	nMountDir(L"../../../media/players/ogro");

	if(!nInit()) return 0;
	texid = nvCreateTextureFromFile(L"deffont.tga", NGL_TEX_FLAGS_LINEARMIN|NGL_TEX_FLAGS_LINEARMAG|NGL_TEX_FLAGS_FOR2D);
	nvLoadTexture(texid);
	fontid = nvCreateFont(L"deffont.nek1");

	modelid = nvCreate3dModel(L"tris.nek2", "ogro");
	standanimid = nvGet3dModelAnimIdByName(modelid, "stand");
	runanimid = nvGet3dModelAnimIdByName(modelid, "run");
	weaponid = nvCreate3dModel(L"weapon.nek2", "weapon");
	mapid = nvCreate3dModel(L"city.nek2", "map");
	nvLoad3dModel(modelid);
	nvLoad3dModel(weaponid);
	nvLoad3dModel(mapid);

#ifdef UNCOMPLETE_MIRROR_TEST
	mirror[0].x = -20;
	mirror[0].y = 10;
	mirror[0].z = MIRROR_Z_POS; // ТУДУ: Разобраться как сдвигать камеру для зеркала
	mirror[0].nx = mirror[0].ny = mirror[0].nz = 0;
	mirror[0].cr = mirror[0].cg = mirror[0].cb = mirror[0].ca = 0.5f;

	mirror[1].x = -20;
	mirror[1].y = 90;
	mirror[1].z = MIRROR_Z_POS;
	mirror[1].nx = mirror[1].ny = mirror[1].nz = 0;
	mirror[1].cr = mirror[1].cg = mirror[1].cb = mirror[1].ca = 0.5f;

	mirror[2].x = 20;
	mirror[2].y = 90;
	mirror[2].z = MIRROR_Z_POS;
	mirror[2].nx = mirror[2].ny = mirror[2].nz = 0;
	mirror[2].cr = mirror[2].cg = mirror[2].cb = mirror[2].ca = 0.5f;

	mirror[3].x = 20;
	mirror[3].y = 10;
	mirror[3].z = MIRROR_Z_POS;
	mirror[3].nx = mirror[3].ny = mirror[3].nz = 0;
	mirror[3].cr = mirror[3].cg = mirror[3].cb = mirror[3].ca = 0.5f;

	mirror_indexes[0] = 0;
	mirror_indexes[1] = 1;
	mirror_indexes[2] = 2;
	mirror_indexes[3] = 0;
	mirror_indexes[4] = 2;
	mirror_indexes[5] = 3;

	MirrorSetRevZMatrix(matrix_rev_z);
#endif

	InitCamera(&camera);
	InitScene(&scene);

	//nvSetAmbientLight(0.5, 1.0, 0.5, 1.0);

	while(gameloop) {

		if(nvGetStatusi(NV_STATUS_WIN_EXITMSG)) gameloop = false;
		if(nvGetKey(27)) gameloop = false;
		nvBegin3d();
			UpdateCamera(&camera);
			UpdateScene(&scene);

#ifdef UNCOMPLETE_MIRROR_TEST
			mirrored_camera = camera;
			mirrored_scene = scene;
			SetCamera(&mirrored_camera);
			nvMatrixMult(mirrored_camera.camera_matrix, matrix_rev_z);
			DrawScene(&mirrored_scene, &mirrored_camera);


			// Отрисовка зеркала
			nvClear(NV_CLEAR_DEPTH_BUFFER);
			SetCamera(&camera);
			nvDraw3dIndexedMesh(0, 4, mirror, 2, mirror_indexes);
			DrawColoredBox(-256, 256, -10, 200, -115, 115, 0, 0, 0, 1);
#endif

			// Отрисовка сцены
			DrawScene(&scene, &camera);
			SetCamera(&camera);
		nvEnd3d();
		nvBegin2d();
			swprintf(fpst, 16, L"fps %f", nGetafps());
			nvDraw2dText(fpst, 0, 0, fontid, texid, 1.00, 1.00, CBLUE);
		nvEnd2d();
		nUpdate();
	}
	nClose();

	NYAN_CLOSE

	return 0;
}
