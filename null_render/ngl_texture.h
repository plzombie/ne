/*
	Файл	: ngl_texture.h

	Описание: Заголовок для ngl_texture.c

	История	: 5.10.12	Создан

*/

//Флаги текстуры (private)
#define NGL_TEX_FLAGS_SCALECOORD 0x40

extern int ngl_tex_maxanis;
extern int ngl_tex_anis;
extern int ngl_tex_enabledflags;

// Структура, описывающая текстуру
typedef struct{
	bool isused; // true, если текстура занята
	bool bufmayfree; // true, если память под buffer управляется этой программой(nullgl.dll), false - если сторонней (nya.dll например)
	unsigned int glid; // id OpenGL текстуры
	unsigned int flags; // Флаги текстуры
	unsigned int oasizex; // original active size x
	unsigned int oasizey; // original active size y
	unsigned int osizex; // original size x
	unsigned int osizey; // original size y
	unsigned int sizex;
	unsigned int sizey;
	float cscalex; // Числа, на которые надо домножить координаты текстуры при флаге NGL_TEX_FLAGS_SCALECOORD 
	float cscaley;
	unsigned int nglcolorformat;
	int nglrowalignment; // Параметр для GL_UNPACK_ALIGNMENT и GL_PACK_ALIGNMENT
	unsigned char *buffer; // Хранилище текстуры
} ngl_texture_type;

extern ngl_texture_type *ngl_textures;
extern unsigned int ngl_maxtextures;

extern uintptr_t ngl_textures_sync_mutex;

extern N_API bool N_APIENTRY_EXPORT nglIsTex(unsigned int id);
extern void nglTexGetScale(unsigned int id,unsigned int *flags, float *scalex, float *scaley);
extern N_API bool N_APIENTRY_EXPORT nglFreeAllTextures(void);
