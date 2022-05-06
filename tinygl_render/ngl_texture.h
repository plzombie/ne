/*
	Файл	: ngl_texture.h

	Описание: Заголовок для ngl_texture.c

	История	: 5.10.12	Создан

*/

extern int ngl_tex_maxanis;
extern int ngl_tex_anis;
extern int ngl_tex_enabledflags;

extern N_API bool N_APIENTRY_EXPORT nglIsTex(unsigned int id);
extern N_API bool N_APIENTRY_EXPORT nglFreeAllTextures(void);
