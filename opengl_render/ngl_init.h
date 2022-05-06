/*
	Файл	: ngl_init.h

	Описание: Заголовок для ngl_init.c

	История	: 05.08.12	Создан

*/

#include "../nyan/nyan_engapi.h"
#include "../nyan/nyan_vismodes_publicapi.h"

#ifndef __glext_h_
	#include "../forks/gl/glext.h"
#endif

extern engapi_type *ngl_ea;

extern bool ngl_isinit;

extern bool ngl_win_swapc_ext;

extern unsigned int ngl_winx, ngl_winy, ngl_winbpp;
extern int ngl_winmbl, ngl_winmbr, ngl_winmbm, ngl_winmx, ngl_winmy, ngl_winmwheel;
extern int ngl_winmode, ngl_win_vsync;
extern int ngl_win_exitmsg;
extern unsigned int ngl_win_updateinterval;
extern char ngl_winkeys[256];

extern bool ngl_win_havetouch;
extern int ngl_wintouchcount, ngl_wintouchmaxcount;
extern int ngl_wintouchx[NV_STATUS_WINTOUCHMAX_X-NV_STATUS_WINTOUCH0_X+1];
extern int ngl_wintouchy[NV_STATUS_WINTOUCHMAX_Y-NV_STATUS_WINTOUCH0_Y+1];
extern int ngl_wintouchpressed[NV_STATUS_WINTOUCHMAX_PRESSED-NV_STATUS_WINTOUCH0_PRESSED+1];

extern const wchar_t *ngl_wintitle; // Имя окна
extern wchar_t ngl_wintextinputbuf[256];
extern int ngl_wintextinputbufsize;

extern int ngl_windpix;
extern int ngl_windpiy;

extern float ngl_winbcred, ngl_winbcgreen, ngl_winbcblue;
extern bool ngl_winbcchanged;

extern unsigned int ngl_glrowalignment;

extern int ngl_win_maxvertexattribs, ngl_win_maxprogramlocalparams, ngl_win_maxprogramenvparams;
extern int ngl_win_maxprogrammatrices, ngl_win_maxprogramtemporaries, ngl_win_maxprogramparams, ngl_win_maxprogramaddressregs;
extern bool ngl_win_vertexprogram_ext;
extern bool ngl_win_texture_edge_clamp;

extern PFNGLPROGRAMSTRINGARBPROC funcptr_glProgramStringARB;
extern PFNGLBINDPROGRAMARBPROC funcptr_glBindProgramARB;
extern PFNGLDELETEPROGRAMSARBPROC funcptr_glDeleteProgramsARB;
extern PFNGLGENPROGRAMSARBPROC funcptr_glGenProgramsARB;
extern PFNGLGETPROGRAMIVARBPROC funcptr_glGetProgramivARB;
extern PFNGLPROGRAMLOCALPARAMETER4FARBPROC funcptr_glProgramLocalParameter4fARB;

typedef void (APIENTRYP ngl_func_type)(void);

extern bool nglIsMainRContext(void);
extern void N_APIENTRY nglMakeCurrentRContextNull(void *param);
extern bool nglAddSeparateRContextIfNeeded(void);
extern bool nglInitWindow(void);
extern bool nglCloseWindow(void);
extern void nglUpdateWindow(void);
extern void nglChangeVSYNC(int vsync);
extern void nglChangeUpdateInterval(unsigned int updateinterval);
extern void nglChangeWindowMode(unsigned int winx, unsigned int winy, int winmode);
extern void nglChangeWindowTitle(const wchar_t *title);
extern ngl_func_type nglGetProcAddress(const char * name);
extern GLenum nglCatchOpenGLError(wchar_t *caller);
