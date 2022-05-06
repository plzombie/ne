/*
	Файл	: ngl_init.h

	Описание: Заголовок для ngl_init.c

	История	: 05.10.12	Создан

*/

#include "../nyan/nyan_engapi.h"
#include "../nyan/nyan_vismodes_publicapi.h"

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

extern int ngl_glrowalignment;

extern GLContext *ngl_tinygl_context;
extern unsigned int ngl_glvpx, ngl_glvpy;

extern bool nglInitWindow(void);
extern bool nglCloseWindow(void);
extern void nglUpdateWindow(void);
extern void nglChangeVSYNC(int vsync);
extern void nglChangeUpdateInterval(unsigned int updateinterval);
extern void nglChangeWindowMode(unsigned int winx, unsigned int winy, int winmode);
extern void nglChangeWindowTitle(const wchar_t *title);
extern void *nglGetProcAddress(const char * name);
