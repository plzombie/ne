/*
	Файл	: ngl_texture.h

	Описание: Заголовок для ngl_texture.c

	История	: 14.08.12	Создан

*/

//Флаги текстуры (private)
#define NGL_TEX_FLAGS_SCALECOORD 0x10000

extern unsigned int ngl_tex_maxsize;
extern int ngl_tex_maxanis;
extern int ngl_tex_anis;
extern bool ngl_tex_s_npot;
extern bool ngl_tex_s_bgra_ext;
extern bool ngl_tex_s_abgr_ext;
extern bool ngl_tex_s_cmyka_ext;
extern bool ngl_tex_s_packed_pixels;
extern int ngl_tex_enabledflags;

enum {
	NGL_TEX_STATUS_FREE = 0,
	NGL_TEX_STATUS_PENDING,
	NGL_TEX_STATUS_LOADED
};

// Структура, описывающая текстуру
typedef struct{
	unsigned int status;
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
	unsigned int nglrowalignment; // Параметр для GL_UNPACK_ALIGNMENT и GL_PACK_ALIGNMENT
	unsigned char *buffer; // Временное хранилище текстуры (существует только во время исполнения nglLoadTexture)
	bool bufmayfree; // true, если память под buffer управляется этой программой(ngl.dll), false - если сторонней (nya.dll например)
} ngl_texture_type;

extern ngl_texture_type *ngl_textures;
extern unsigned int ngl_maxtextures;

extern uintptr_t ngl_textures_sync_mutex;

extern N_API bool N_APIENTRY_EXPORT nglIsTex(unsigned int id);
extern void nglTexGetScale(unsigned int id,unsigned int *flags, float *scalex, float *scaley);
extern unsigned int nglTexGetGLid(unsigned int id);
extern N_API bool N_APIENTRY_EXPORT nglFreeAllTextures(void);
