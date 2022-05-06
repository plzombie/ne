/*
part of md2tonek2
Copyright (C) 2016 plzombie.
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "../../extclib/mbstowcsl.h"

#include "../../nyan/nyan_draw_publicapi.h"

#include "../../nyan_container/nyan_container.h"

#define MD2TONEK2_READERVERSION 0
#define MD2TONEK2_WRITERVERSION 0

#include "1Q2PART.h"

typedef struct {
	char animname[20];
	unsigned int startframe;
	unsigned int endframe;
	unsigned int fps;
} nv_3dmodel_anim_type;

// Значения nv_3dmodel_header_type.trisformat
#define NV_3DMODEL_TRISFORMAT_PLAIN 0 // Каждый треугольник имеет три вершины: vertices[i*3], vertices[i*3+1] и vertices[i*3+2], где i - номер треугольника. Всего noftriangles*3 вершин.
#define NV_3DMODEL_TRISFORMAT_INDEXED 1 // Каждый треугольник имеет три индекса, указывающие номера используемых вершин. Индексы хранятся в массиве vertexindexes[i*3], vertexindexes[i*3+1] и vertexindexes[i*3+2], где i - номер треугольника.

// Значения nv_3dmodel_header_type.animformat
#define NV_3DMODEL_ANIMFORMAT_VERTEX 0 // Вершинная анимация

typedef struct {
	unsigned int nofframes; // Количество кадров
	unsigned int noftriangles; // Количество треугольников в кадре
	unsigned int nofvertices; // Количество вершин
	unsigned int nofskins; // Количество скинов
	unsigned int nofanims; // Количество анимаций
	unsigned int trisformat; // Формат в котором хранятся треугольники
	unsigned int animformat; // Формат анимации
} nv_3dmodel_header_type;

typedef struct {
	nv_3dmodel_header_type header;
	//unsigned int status;
	//unsigned int refcount; // Количество ссылок на модель
	unsigned int *vertexindexes; // Индексы вершин (для trisformat == NV_3DMODEL_TRISFORMAT_INDEXED)
	nv_3dvertex_type *vertices; // Данные вершин для статической модели (без анимации)
	nv_3ddeltavertex_type *deltavertices; // Разница между статической моделью и кадром анимации. Кадры идут последовательно (0, 1, ..., frames-1)
	//nv_3dvertex_type *vertices_cash; // (для анимации, предварительно вычисленные вершины)
	nv_3dmodel_anim_type *anims; // Данные об анимациях
	//unsigned int *skintexids; // ngl id текстур
	wchar_t **skintexnames; // Имена текстур
	//char *modname; // Имя модели
} nv_3dmodel_type;

int main(int argc, char **argv)
{
	unsigned int j, k, maxnamelen;
	FILE *f;
	MD2HEAD md2head;
	MD2ST *md2st;
	MD2TRIS *md2tris;
	MD2FRAMEHEAD *md2framehead;
	MD2VERTICES *md2vertices;
	nv_3dmodel_type nv_3dmodel;
	char *input_filename, *output_filename;
	int keep_animations;

	nyan_filetypechunk_type nek2; // Чанк, содержащий тип файла
	nyan_chunkhead_type chunkhead; // Заголовок чанка

	if(argc < 3) {
		printf("%s", "  RAW to NEK0 converter\n");
		printf("%s", "    rawtonek0.exe input.md2 output.nek2 [triangles_type=0] [keep_animations=1]\n\n");
		printf("%s", "      triangles_type == 0 - triangles is array of vertices (3 vertices per triangle)\n");
		printf("%s", "      triangles_type == 1 - triangles is divided into vertices and indexes to vertices (each triangle have 3 vertices, one vectice can be a part of many triangles)\n");
		printf("%s", "      keep_animations != 0 - import animations from original file\n");
		printf("%s", "      keep_animations == 0 - delete them\n\n");
		if(argc != 1)
			printf("%s", "error: need 2 args\n");

		return 0;
	}

	input_filename = argv[1];
	output_filename = argv[2];
	if(argc > 3)
		nv_3dmodel.header.trisformat = atoi(argv[3]);
	else
		nv_3dmodel.header.trisformat = NV_3DMODEL_TRISFORMAT_PLAIN;

	if(argc > 4)
		keep_animations = atoi(argv[4]);
	else
		keep_animations = 1;

	if(nv_3dmodel.header.trisformat > NV_3DMODEL_TRISFORMAT_INDEXED) {
		printf("%s", "error: unknown triangles_type\n");
		return 0;
	}

	nv_3dmodel.header.animformat = NV_3DMODEL_ANIMFORMAT_VERTEX;

	// Чтение файла
	f = fopen(input_filename, "rb");
	if(!f)
		{ printf("%s", "error: can't open input file\n"); return 0; }

	if(fread(&md2head, 1, sizeof(MD2HEAD), f) != sizeof(MD2HEAD)) {
		printf("%s", "error: file is damaged\n");
		fclose(f);
		return 0;
	}

	printf("sign %u\n", md2head.sign);
	printf("version %u\n", md2head.version);
	printf("texwid %u\n", md2head.texwid);
	printf("texhei %u\n", md2head.texhei);
	printf("framesize %u\n", md2head.framesize);
	printf("num_skins %u\n", md2head.num_skins);
	printf("num_vertices %u\n", md2head.num_vertices);
	printf("num_texcoord %u\n", md2head.num_texcoord);
	printf("num_tris %u\n", md2head.num_tris);
	printf("num_glcmds %u\n", md2head.num_glcmds);
	printf("num_frames %u\n", md2head.num_frames);
	printf("ofs_skins %u\n", md2head.ofs_skins);
	printf("ofs_texcoord %u\n", md2head.ofs_texcoord);
	printf("ofs_tri %u\n", md2head.ofs_tris);
	printf("ofs_frames %u\n", md2head.ofs_frames);
	printf("fs_end %u\n", md2head.ofs_end);

	nv_3dmodel.header.noftriangles = md2head.num_tris;
	
	if(nv_3dmodel.header.trisformat == NV_3DMODEL_TRISFORMAT_INDEXED) {
		nv_3dmodel.header.nofvertices = md2head.num_vertices;
		
		printf("%s", "Allocating memory for vertex indexes...\n");
		nv_3dmodel.vertexindexes = malloc(nv_3dmodel.header.noftriangles*3*sizeof(unsigned int));
		if(!nv_3dmodel.vertexindexes) {
			fclose(f);
			printf("%s", "error: can't allocate memory\n");
			return 0;
		}
	} else
		nv_3dmodel.header.nofvertices = nv_3dmodel.header.noftriangles*3;
	
	if(keep_animations)
		nv_3dmodel.header.nofframes = md2head.num_frames;
	else
		nv_3dmodel.header.nofframes = 0;
	
	printf("%s", "Allocating memory for vertices...\n");
	nv_3dmodel.vertices = malloc(nv_3dmodel.header.nofvertices*sizeof(nv_3dvertex_type));
	if(!nv_3dmodel.vertices) {
		if(nv_3dmodel.header.trisformat == NV_3DMODEL_TRISFORMAT_INDEXED)
			free(nv_3dmodel.vertexindexes);
		fclose(f);
		printf("%s", "error: can't allocate memory\n");
		return 0;
	}
	
	printf("%s", "Allocating memory for deltavertices...\n");
	nv_3dmodel.deltavertices = malloc(md2head.num_frames*nv_3dmodel.header.nofvertices*sizeof(nv_3ddeltavertex_type));
	if(!nv_3dmodel.deltavertices) {
		if(nv_3dmodel.header.trisformat == NV_3DMODEL_TRISFORMAT_INDEXED)
			free(nv_3dmodel.vertexindexes);
		free(nv_3dmodel.vertices);
		fclose(f);
		printf("%s", "error: can't allocate memory\n");
		return 0;
	}

	nv_3dmodel.header.nofskins = md2head.num_skins;

	nv_3dmodel.skintexnames = malloc(md2head.num_skins*sizeof(wchar_t *));
	if(!nv_3dmodel.skintexnames) {
		if(nv_3dmodel.header.trisformat == NV_3DMODEL_TRISFORMAT_INDEXED)
			free(nv_3dmodel.vertexindexes);
		free(nv_3dmodel.vertices);
		free(nv_3dmodel.deltavertices);
		fclose(f);
		printf("%s", "error: can't allocate memory\n");
		return 0;
	}

	// Чтение скинов
	fseek(f, md2head.ofs_skins, SEEK_SET);
	for(j = 0; j < md2head.num_skins; j++) {
		char skinname[64];
		nv_3dmodel.skintexnames[j] = malloc(69*sizeof(wchar_t)); // 64+4+1 for .pcx and \0
		if(!nv_3dmodel.skintexnames[j]) {
			for(k = 0; k < j; k++)
				free(nv_3dmodel.skintexnames[k]);
			free(nv_3dmodel.skintexnames);
			if(nv_3dmodel.header.trisformat == NV_3DMODEL_TRISFORMAT_INDEXED)
				free(nv_3dmodel.vertexindexes);
			free(nv_3dmodel.vertices);
			free(nv_3dmodel.deltavertices);
			fclose(f);
			printf("%s", "error: can't allocate memory\n");
			return 0;
		}
		fread(skinname, 1, 64, f);
		mbstowcsl(nv_3dmodel.skintexnames[j], skinname, 64);
		if(!strrchr(skinname, '.')) wcscat(nv_3dmodel.skintexnames[j], L".pcx");
	}

	md2st = malloc(md2head.num_texcoord*sizeof(MD2ST));
	md2tris = malloc(md2head.num_tris*sizeof(MD2TRIS));
	md2framehead = malloc(md2head.num_frames*sizeof(MD2FRAMEHEAD));
	md2vertices = malloc(md2head.num_vertices*sizeof(MD2VERTICES));
	if(!md2st || !md2tris || !md2framehead || !md2vertices) {
		if(md2st)
			free(md2st);
		if(md2tris)
			free(md2tris);
		if(md2framehead)
			free(md2framehead);
		if(md2vertices)
			free(md2vertices);

		for(j = 0; j < md2head.num_skins; j++)
			free(nv_3dmodel.skintexnames[j]);
		free(nv_3dmodel.skintexnames);
		if(nv_3dmodel.header.trisformat == NV_3DMODEL_TRISFORMAT_INDEXED)
			free(nv_3dmodel.vertexindexes);
		free(nv_3dmodel.vertices);
		free(nv_3dmodel.deltavertices);

		fclose(f);
		printf("%s", "error: can't allocate memory\n");
		return 0;
	}

	// Чтение и заполнение вершин
	fseek(f, md2head.ofs_texcoord, SEEK_SET);
	fread(md2st, 1, md2head.num_texcoord*sizeof(MD2ST), f);

	fseek(f, md2head.ofs_tris, SEEK_SET);
	fread(md2tris, 1, md2head.num_tris*sizeof(MD2TRIS), f);

	if(nv_3dmodel.header.trisformat == NV_3DMODEL_TRISFORMAT_INDEXED) {
		for(j = 0; j < nv_3dmodel.header.noftriangles; j++) {
			nv_3dmodel.vertexindexes[j*3] = md2tris[j].vertexindex[0];
			nv_3dmodel.vertexindexes[j*3+1] = md2tris[j].vertexindex[1];
			nv_3dmodel.vertexindexes[j*3+2] = md2tris[j].vertexindex[2];
		}
		
		for(j = 0; j < nv_3dmodel.header.nofvertices; j++) {
			nv_3dmodel.vertices[j].cr = 1.0;
			nv_3dmodel.vertices[j].cg = 1.0;
			nv_3dmodel.vertices[j].cb = 1.0;
			nv_3dmodel.vertices[j].ca = 1.0;
			nv_3dmodel.vertices[j].tx = 0.0;
			nv_3dmodel.vertices[j].ty = 0.0;
		}
		
		for(j = 0; j < md2head.num_tris*3; j++) {
			k = md2tris[j/3].vertexindex[j%3];
			if(nv_3dmodel.vertices[k].tx != ((float)md2st[md2tris[j/3].textureindex[j%3]].s/md2head.texwid) ||
				nv_3dmodel.vertices[k].ty != ((float)(md2head.texhei-md2st[md2tris[j/3].textureindex[j%3]].t)/md2head.texhei)) {
				printf("%u == k %f %f to %f %f\n", k, nv_3dmodel.vertices[k].tx, nv_3dmodel.vertices[k].ty,
					(float)md2st[md2tris[j/3].textureindex[j%3]].s/md2head.texwid,
					(float)(md2head.texhei-md2st[md2tris[j/3].textureindex[j%3]].t)/md2head.texhei);
			}
			nv_3dmodel.vertices[k].tx = (float)md2st[md2tris[j/3].textureindex[j%3]].s/md2head.texwid;
			nv_3dmodel.vertices[k].ty = (float)(md2head.texhei-md2st[md2tris[j/3].textureindex[j%3]].t)/md2head.texhei;
		}
	} else { // NV_3DMODEL_TRISFORMAT_PLAIN
		for(j = 0; j < md2head.num_tris*3; j++) {
			nv_3dmodel.vertices[j].cr = 1.0;
			nv_3dmodel.vertices[j].cg = 1.0;
			nv_3dmodel.vertices[j].cb = 1.0;
			nv_3dmodel.vertices[j].ca = 1.0;
			nv_3dmodel.vertices[j].tx = (float)md2st[md2tris[j/3].textureindex[j%3]].s/md2head.texwid;
			nv_3dmodel.vertices[j].ty = (float)(md2head.texhei-md2st[md2tris[j/3].textureindex[j%3]].t)/md2head.texhei;
		}
	}

	fseek(f, md2head.ofs_frames, SEEK_SET);
	for(j = 0; j < md2head.num_frames; j++) {
		//nlPrint(L"Read anim %d, sizeof(MD2FRAMEHEAD) %d, md2head.num_vertices*sizeof(MD2VERTICES) %d", j, sizeof(MD2FRAMEHEAD), md2head.num_vertices*sizeof(MD2VERTICES));
		fread(md2framehead+j, 1, sizeof(MD2FRAMEHEAD), f);
		fread(md2vertices, 1, md2head.num_vertices*sizeof(MD2VERTICES), f);
		//nlPrint(L"md2framehead->scale %f %f %f", md2framehead[j].scale[0], md2framehead[j].scale[1], md2framehead[j].scale[2]);
		//nlPrint(L"md2framehead->translate %f %f %f", md2framehead[j].translate[0], md2framehead[j].translate[1], md2framehead[j].translate[2]);
		if(j == 0) {
			if(nv_3dmodel.header.trisformat == NV_3DMODEL_TRISFORMAT_INDEXED) {
				for(k = 0; k < nv_3dmodel.header.nofvertices; k++) {
					nv_3dmodel.vertices[k].nx = anorms[md2vertices[k].lightnormalindex].x;
					nv_3dmodel.vertices[k].ny = anorms[md2vertices[k].lightnormalindex].y;
					nv_3dmodel.vertices[k].nz = anorms[md2vertices[k].lightnormalindex].z;
					nv_3dmodel.vertices[k].x = md2vertices[k].v[0]*md2framehead[0].scale[0]+md2framehead[0].translate[0];
					nv_3dmodel.vertices[k].y = md2vertices[k].v[1]*md2framehead[0].scale[1]+md2framehead[0].translate[1];
					nv_3dmodel.vertices[k].z = md2vertices[k].v[2]*md2framehead[0].scale[2]+md2framehead[0].translate[2];
				}
			} else { // NV_3DMODEL_TRISFORMAT_PLAIN
				for(k = 0; k < md2head.num_tris*3; k++) {
					nv_3dmodel.vertices[k].nx = anorms[md2vertices[md2tris[k/3].vertexindex[k%3]].lightnormalindex].x;
					nv_3dmodel.vertices[k].ny = anorms[md2vertices[md2tris[k/3].vertexindex[k%3]].lightnormalindex].y;
					nv_3dmodel.vertices[k].nz = anorms[md2vertices[md2tris[k/3].vertexindex[k%3]].lightnormalindex].z;
					nv_3dmodel.vertices[k].x = md2vertices[md2tris[k/3].vertexindex[k%3]].v[0]*md2framehead[0].scale[0]+md2framehead[0].translate[0];
					nv_3dmodel.vertices[k].y = md2vertices[md2tris[k/3].vertexindex[k%3]].v[1]*md2framehead[0].scale[1]+md2framehead[0].translate[1];
					nv_3dmodel.vertices[k].z = md2vertices[md2tris[k/3].vertexindex[k%3]].v[2]*md2framehead[0].scale[2]+md2framehead[0].translate[2];
				}
			}
		}

		if(nv_3dmodel.header.trisformat == NV_3DMODEL_TRISFORMAT_INDEXED) {
			for(k = 0; k < nv_3dmodel.header.nofvertices; k++) {
				nv_3dmodel.deltavertices[j*nv_3dmodel.header.nofvertices+k].nx = anorms[md2vertices[k].lightnormalindex].x-nv_3dmodel.vertices[k].nx;
				nv_3dmodel.deltavertices[j*nv_3dmodel.header.nofvertices+k].ny = anorms[md2vertices[k].lightnormalindex].y-nv_3dmodel.vertices[k].ny;
				nv_3dmodel.deltavertices[j*nv_3dmodel.header.nofvertices+k].nz = anorms[md2vertices[k].lightnormalindex].z-nv_3dmodel.vertices[k].nz;
				nv_3dmodel.deltavertices[j*nv_3dmodel.header.nofvertices+k].x = md2vertices[k].v[0]*md2framehead[j].scale[0]+md2framehead[j].translate[0]-nv_3dmodel.vertices[k].x;
				nv_3dmodel.deltavertices[j*nv_3dmodel.header.nofvertices+k].y = md2vertices[k].v[1]*md2framehead[j].scale[1]+md2framehead[j].translate[1]-nv_3dmodel.vertices[k].y;
				nv_3dmodel.deltavertices[j*nv_3dmodel.header.nofvertices+k].z = md2vertices[k].v[2]*md2framehead[j].scale[2]+md2framehead[j].translate[2]-nv_3dmodel.vertices[k].z;
			}
		} else { // NV_3DMODEL_TRISFORMAT_PLAIN
			for(k = 0; k < md2head.num_tris*3; k++) {
				nv_3dmodel.deltavertices[j*md2head.num_tris*3+k].nx = anorms[md2vertices[md2tris[k/3].vertexindex[k%3]].lightnormalindex].x-nv_3dmodel.vertices[k].nx;
				nv_3dmodel.deltavertices[j*md2head.num_tris*3+k].ny = anorms[md2vertices[md2tris[k/3].vertexindex[k%3]].lightnormalindex].y-nv_3dmodel.vertices[k].ny;
				nv_3dmodel.deltavertices[j*md2head.num_tris*3+k].nz = anorms[md2vertices[md2tris[k/3].vertexindex[k%3]].lightnormalindex].z-nv_3dmodel.vertices[k].nz;
				nv_3dmodel.deltavertices[j*md2head.num_tris*3+k].x = md2vertices[md2tris[k/3].vertexindex[k%3]].v[0]*md2framehead[j].scale[0]+md2framehead[j].translate[0]-nv_3dmodel.vertices[k].x;
				nv_3dmodel.deltavertices[j*md2head.num_tris*3+k].y = md2vertices[md2tris[k/3].vertexindex[k%3]].v[1]*md2framehead[j].scale[1]+md2framehead[j].translate[1]-nv_3dmodel.vertices[k].y;
				nv_3dmodel.deltavertices[j*md2head.num_tris*3+k].z = md2vertices[md2tris[k/3].vertexindex[k%3]].v[2]*md2framehead[j].scale[2]+md2framehead[j].translate[2]-nv_3dmodel.vertices[k].z;
			}
		}
	}

	fclose(f);

	// Заполнение анимаций
	nv_3dmodel.header.nofanims = 0;
	if(keep_animations) {
		for(j = 0; j < md2head.num_frames; j++) {
			for(k = 0; k < 16; k++) {
				if(isdigit(md2framehead[j].name[k]))
					break;
			}
			if(j == 0) {
				nv_3dmodel.header.nofanims++;
				maxnamelen = k;
			} else if(strncmp(md2framehead[j-1].name, md2framehead[j].name, maxnamelen)) {
				nv_3dmodel.header.nofanims++;
				maxnamelen = k;
			}
		}
		nv_3dmodel.anims = malloc(nv_3dmodel.header.nofanims * sizeof(nv_3dmodel_anim_type));
		if(!nv_3dmodel.anims) {
			free(md2st);
			free(md2tris);
			free(md2framehead);
			free(md2vertices);

			for(j = 0; j < md2head.num_skins; j++)
				free(nv_3dmodel.skintexnames[j]);
			free(nv_3dmodel.skintexnames);
			if(nv_3dmodel.header.trisformat == NV_3DMODEL_TRISFORMAT_INDEXED)
				free(nv_3dmodel.vertexindexes);
			free(nv_3dmodel.vertices);
			free(nv_3dmodel.deltavertices);

			printf("%s", "error: can't allocate memory for anims\n");
			return 0;
		}
		nv_3dmodel.header.nofanims = 0;
		for(j = 0; j < md2head.num_frames; j++) {
			for(k = 0; k < 16; k++) {
				if(isdigit(md2framehead[j].name[k]))
					break;
			}
			if(j == 0) {
				memset(nv_3dmodel.anims[0].animname, 0, 20);
				memcpy(nv_3dmodel.anims[0].animname, md2framehead[j].name, k);
				nv_3dmodel.anims[0].animname[k] = 0;
				nv_3dmodel.anims[0].startframe = 0;
				nv_3dmodel.anims[0].endframe = md2head.num_frames - 1;
				nv_3dmodel.anims[0].fps = 10; // Я не виноват, что Кармак не добавил скорость воспроизведения в свой формат \_(^-^)_/
				nv_3dmodel.header.nofanims++;
				//MessageBoxA(NULL, nv_3dmodel.anims[0].animname, "New animation", 0);

				maxnamelen = k;
			} else if(strncmp(md2framehead[j-1].name, md2framehead[j].name, maxnamelen)) {
				memset(nv_3dmodel.anims[nv_3dmodel.header.nofanims].animname, 0, 20);
				memcpy(nv_3dmodel.anims[nv_3dmodel.header.nofanims].animname, md2framehead[j].name, k);
				nv_3dmodel.anims[nv_3dmodel.header.nofanims].animname[k] = 0;
				nv_3dmodel.anims[nv_3dmodel.header.nofanims].startframe = j;
				nv_3dmodel.anims[nv_3dmodel.header.nofanims].endframe = md2head.num_frames-1;
				nv_3dmodel.anims[nv_3dmodel.header.nofanims].fps = 10;
				nv_3dmodel.anims[nv_3dmodel.header.nofanims-1].endframe = j-1;
				//MessageBoxA(NULL, nv_3dmodel.anims[nv_3dmodel.header.nofanims].animname, "New animation", 0);

				nv_3dmodel.header.nofanims++;
				maxnamelen = k;
			}
		}
	}

	free(md2st);
	free(md2tris);
	free(md2framehead);
	free(md2vertices);

	printf("Model loaded\n");

	// Открытие выходного файла
	f = fopen(output_filename, "wb");
	if(!f)
		{ printf("%s", "error: can't open output file\n"); goto EXIT; }

	// Запись чанка, содержащего тип файла
	ncfSetFileType(&nek2, "NEK2", MD2TONEK2_READERVERSION, MD2TONEK2_WRITERVERSION);
	fwrite(&nek2, 1, sizeof(nek2), f);
	
	// Запись чанка, содержащего заголовок файла
	memcpy(chunkhead.cname, "FILEHEAD", 8);
	chunkhead.csize = sizeof(nv_3dmodel_header_type);
	fwrite(&chunkhead, 1, sizeof(chunkhead), f);
	fwrite(&(nv_3dmodel.header), 1, sizeof(nv_3dmodel_header_type), f);
	
	// Запись чанка, содержащего вершины и треугольники
	memcpy(chunkhead.cname, "V&TRDATA", 8);
	chunkhead.csize = nv_3dmodel.header.nofvertices*sizeof(nv_3dvertex_type);
	if(nv_3dmodel.header.trisformat == NV_3DMODEL_TRISFORMAT_INDEXED)
		chunkhead.csize += nv_3dmodel.header.noftriangles*3*sizeof(unsigned int);
	fwrite(&chunkhead, 1, sizeof(chunkhead), f);
	fwrite(nv_3dmodel.vertices, 1, nv_3dmodel.header.nofvertices*sizeof(nv_3dvertex_type), f);
	if(nv_3dmodel.header.trisformat == NV_3DMODEL_TRISFORMAT_INDEXED)
		fwrite(nv_3dmodel.vertexindexes, 1, nv_3dmodel.header.noftriangles*3*sizeof(unsigned int), f);
	printf("V&TRDATA chunk size: %llu\n", chunkhead.csize);

	// Запись чанка, содержащего скины
	if(nv_3dmodel.header.nofskins) {
		memcpy(chunkhead.cname, "SKINDATA", 8);
		chunkhead.csize = nv_3dmodel.header.nofskins*sizeof(unsigned int);
		for(j = 0; j < nv_3dmodel.header.nofskins; j++)
			chunkhead.csize += wcslen(nv_3dmodel.skintexnames[j])*2;
		fwrite(&chunkhead, 1, sizeof(chunkhead), f);
		for(j = 0; j < nv_3dmodel.header.nofskins; j++) {
			unsigned int name_len;
		
			name_len = wcslen(nv_3dmodel.skintexnames[j]);
			fwrite(&name_len, 1, sizeof(unsigned int), f);
			fwrite(nv_3dmodel.skintexnames[j], 1, wcslen(nv_3dmodel.skintexnames[j])*2, f);
		}
		printf("SKINDATA chunk size: %llu\n", chunkhead.csize);
	}

	// Запись чанка, содержащего список анимаций
	if(nv_3dmodel.header.nofanims) {
		memcpy(chunkhead.cname, "ANIMDATA", 8);
		chunkhead.csize = nv_3dmodel.header.nofanims*sizeof(nv_3dmodel_anim_type);
		fwrite(&chunkhead, 1, sizeof(chunkhead), f);
		fwrite(nv_3dmodel.anims, 1, nv_3dmodel.header.nofanims*sizeof(nv_3dmodel_anim_type), f);
		printf("ANIMDATA chunk size: %llu\n", chunkhead.csize);
	}

	// Запись чанка, содержащего кадры вершинной анимации
	if(nv_3dmodel.header.nofframes) {
		memcpy(chunkhead.cname, "VAFRDATA", 8);
		chunkhead.csize = nv_3dmodel.header.nofframes*nv_3dmodel.header.nofvertices*sizeof(nv_3ddeltavertex_type);
		fwrite(&chunkhead, 1, sizeof(chunkhead), f);
		fwrite(nv_3dmodel.deltavertices, 1, nv_3dmodel.header.nofframes*nv_3dmodel.header.nofvertices*sizeof(nv_3ddeltavertex_type), f);
		printf("VAFRDATA chunk size: %llu\n", chunkhead.csize);
	}

	fclose(f);

	printf("%s", "Model saved\n");

EXIT:

	for (j = 0; j < md2head.num_skins; j++)
		free(nv_3dmodel.skintexnames[j]);
	free(nv_3dmodel.skintexnames);
	free(nv_3dmodel.vertices);
	free(nv_3dmodel.deltavertices);
	free(nv_3dmodel.anims);
	if(nv_3dmodel.header.trisformat == NV_3DMODEL_TRISFORMAT_INDEXED)
		free(nv_3dmodel.vertexindexes);

	return 0;
}
