/*
	Файл	: unix_ngl_init.c

	Описание: Инициализация графической библиотеки. Платформа unix, X11
		По статье https://msdn.microsoft.com/en-us/library/windows/desktop/dd318252(v=vs.85).aspx и урокам от nehe для x11

	История	: 26.10.12	Создан

*/

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <wchar.h>
#include <stdlib.h>

#include <unistd.h>
#include <time.h>
#include <signal.h>

#include <GL/glx.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <X11/extensions/xf86vmode.h>
#include "../forks/gl/glxext.h"

#include "ngl.h"
#include "ngl_text.h"

#include "../nyan/nyan_vismodes_publicapi.h"
#include "../nyan/nyan_keycodes_publicapi.h"

#include "../unixsupport/keysym2ucs.h"
#include "../extclib/mbstowcsl.h"
#include "../extclib/wcstombsl.h"

#include "ngl_init.h"
#include "ngl_batch.h"
#include "unix_ngl_init.h"

#ifndef SA_RESTART
	#define SA_RESTART 0
#endif

// GLX_MESA_swap_control
typedef int (* glXSwapIntervalMESA_type)(int interval);
static glXSwapIntervalMESA_type funcptr_glXSwapIntervalMESA;

bool ngl_win_swapc_ext = false; // GLX_EXT_swap_control

static bool ngl_win_xf86vidmode_ext = false; // XF86VIDMODE

static Display *ngl_win_dpy;
static Window ngl_win_glwin;
static Colormap ngl_win_cmap;
static XVisualInfo *ngl_win_vi;
static GLXContext ngl_win_glcontext;
static bool ngl_win_isResolutionChanged = false;
static XF86VidModeModeInfo ngl_win_olddescmode; // Старый видеорежим (до смены разрешения)
static int ngl_win_screen; // Screen number
static int ngl_win_attributes[] = {GLX_RGBA, GLX_DEPTH_SIZE, 16, GLX_DOUBLEBUFFER, None};
static unsigned char ngl_win_keycodes_map[256];

static GLXContext *ngl_win_separate_hRCs = NULL; // Separate Thread Rendering Contexts
static unsigned int ngl_win_nof_separate_hRCs = 0; // Количество отдельных контекстов отрисовки
static unsigned int ngl_win_nof_used_separate_hRCs = 0; // Количество отдельных __используемых__ контекстов отрисовки
static uintptr_t ngl_win_separate_hRCs_synch_mutex = 0;

static timer_t ngl_win_updatetimerid; // Таймер для отрисовки окна через заданный промежуток времени
static bool ngl_win_timerneedcancel = false;
static struct sigevent ngl_win_sevp;
static sigset_t ngl_win_updatesigset; // Набор сигналов при вызове таймера обновления
static struct sigaction ngl_win_updatesigaction, ngl_win_oldsigaction;

static Bool WaitForMapNotify(Display *d, XEvent *e, char *arg)
{
	(void)d; // Неиспользуемая переменная

	if ((e->type == MapNotify) && (e->xmap.window == (Window)arg)) {
		return GL_TRUE;
	}

	return GL_FALSE;
}

/*
	Функция : nglMakeKeycodeMap

	Описание: Заполняет массив ngl_win_keycodes_map, содержащий номера NK_* для соответствующих клавищ клавиатуры

	История	: 18.04.14	Создан

*/
static void nglMakeKeycodeMap(void)
{
	unsigned int i;

	ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_BackSpace)] = NK_BACK;
	ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_Tab)] = NK_TAB;
	ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_Return)] = NK_RETURN;
	ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_KP_Enter)] = NK_RETURN;

	// NK_SHITF, NK_CONTROL, NK_MENU устанавливаются в nglUpdateWindow()

	ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_Pause)] = NK_PAUSE;

	ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_Caps_Lock)] = NK_CAPSLOCK;

	ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_Escape)] = NK_ESCAPE;

	ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_space)] = NK_SPACE;
	ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_Page_Up)] = NK_PRIOR;
	ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_Page_Down)] = NK_NEXT;
	ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_End)] = NK_END;
	ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_Home)] = NK_HOME;

	ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_Left)] = NK_LEFT;
	ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_Up)] = NK_UP;
	ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_Right)] = NK_RIGHT;
	ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_Down)] = NK_DOWN;

	ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_Print)] = NK_PRINTSCREEN;

	ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_Insert)] = NK_INSERT;
	ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_Delete)] = NK_DELETE;

	for(i = 0; i < 10; i++)
		ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_0+i)] = NK_0+i;

	for(i = 0; i < 26; i++)
		ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_A+i)] = NK_A+i;

	ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_Super_L)] = NK_LWIN;
	ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_Super_R)] = NK_RWIN;
	ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_Menu)] = NK_APP;

	for(i = 0; i < 10; i++)
		ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_KP_0+i)] = NK_NUMPAD0+i;

	ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_KP_Multiply)] = NK_MULTIPLY;
	ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_KP_Add)] = NK_ADD;
	ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_KP_Separator)] = NK_SEPARATOR; // Работает на моём нетбуке
	ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_KP_Decimal)] = NK_SEPARATOR; // Написано в нете. По факту, возвращает другой keycode
	ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_KP_Delete)] = NK_SEPARATOR; // Должно работать везде
	ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_KP_Subtract)] = NK_SUBSTRACT;
	ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_KP_Decimal)] = NK_DECIMAL;
	ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_KP_Divide)] = NK_DIVIDE;

	ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_F1)] = NK_F1;
	ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_F2)] = NK_F2;
	ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_F3)] = NK_F3;
	ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_F4)] = NK_F4;
	ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_F5)] = NK_F5;
	ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_F6)] = NK_F6;
	ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_F7)] = NK_F7;
	ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_F8)] = NK_F8;
	ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_F9)] = NK_F9;
	ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_F10)] = NK_F10;
	ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_F11)] = NK_F11;
	ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_F12)] = NK_F12;

	ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_Num_Lock)] = NK_NUMLOCK;
	ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_Scroll_Lock)] = NK_SCROLL;

	ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_Shift_L)] = NK_LSHIFT;
	ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_Shift_R)] = NK_RSHIFT;
	ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_Control_L)] = NK_LCONTROL;
	ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_Control_R)] = NK_RCONTROL;
	ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_Alt_L)] = NK_LMENU;
	ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_Alt_R)] = NK_RMENU;

	ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_semicolon)] = NK_OEM_1; // VK_OEM_1, ; or :
	ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_equal)] = NK_OEM_PLUS; // VK_OEM_PLUS, = or +
	ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_comma /*XK_less*/)] = NK_OEM_COMMA; // VK_OEM_COMMA, , or <
	ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_underscore)] = NK_OEM_MINUS; // VK_OEM_MINUS, - or _
	ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_period /*XK_greater*/)] = NK_OEM_PERIOD; // VK_OEM_PERIOD, . or >
	ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_question)] = NK_OEM_2; // VK_OEM_2, / or ?
	ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_grave)] = NK_OEM_3; // VK_OEM_3, ` or ~

	ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_bracketleft)] = NK_OEM_4; // VK_OEM_4, [ or {
	ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_backslash)] = NK_OEM_5; // VK_OEM_5, \ or |
	ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_bracketright)] = NK_OEM_6; // VK_OEM_6, ] or }
	ngl_win_keycodes_map[XKeysymToKeycode(ngl_win_dpy, XK_apostrophe)] = NK_OEM_7; // VK_OEM_7, ' or "
}

/*
	Функция	: nglGetDisplayDPI

	Описание: Получает DPI монитора

	История	: 12.08.17	Создан

*/
static void nglGetDisplayDPI(void)
{
	int dpy_dpix = 96, dpy_dpiy = 96;
	char *rmstr;
	Display *display;

	// XResourceManagerString(display) создаётся один раз при вызове XOpenDisplay(NULL)
	// А мне надо проверять строку каждый раз при смене dpi
	// Не нашёл ничего лучше, чем пересоздавать дисплей каждый раз
	display = XOpenDisplay(NULL);

	if(display) {
		rmstr = XResourceManagerString(display);
		if(rmstr) {
			XrmDatabase xrmdb;

			//wprintf(L"rmstr:\n%s\n", rmstr);

			xrmdb = XrmGetStringDatabase(rmstr);
			if(xrmdb) {
				char *str_type_return = NULL;
				XrmValue value_return;

				// Согласно этому https://developer.blender.org/rBfe3fb236970c#change-RmJT6cPdGkij коммиту нужно указывать класс "Xft.Dpi".
				// Здесь https://github.com/glfw/glfw/issues/1019#issuecomment-302772498 написано, что надо указывать "String"
				// Но первое выглядит как-то убедительнее, так как String - это тип возвращаемого значения.
				if(XrmGetResource(xrmdb, "Xft.dpi", "Xft.Dpi", &str_type_return, &value_return)) {
					//wprintf(L"XrmGetResource returned true\n");
					//wprintf(L"type ptr %lld\n", (long long)str_type_return);
					if(str_type_return != NULL) {
						//wprintf(L"restype: %s\n", str_type_return);
						if(!strcmp(str_type_return, "String")) {
							//wprintf(L"dpi returned by x11: %s\n", (char *)(value_return.addr));

							dpy_dpix = dpy_dpiy = (int)(atof((char *)(value_return.addr))+0.5);
						}
					}
				} /*else
					wprintf(L"XrmGetResource returned false\n");*/

				XrmDestroyDatabase(xrmdb);
			}
		}

		XCloseDisplay(display);
	}

	ngl_windpix = 100*dpy_dpix/96; // Вычисление относительной величины dpi, где 96 dpi составляют 100%
	ngl_windpiy = 100*dpy_dpiy/96;

	if(!ngl_windpix)
		ngl_windpix = 100;

	if(!ngl_windpiy)
		ngl_windpiy = 100;

	//wprintf(L"xdpi %d(%d\%) ydpi %d(%d\%)\n", dpy_dpix, ngl_windpix, dpy_dpiy, ngl_windpiy);
}

/*
	Функция	: nglSetWindowMinMaxSize

	Описание: Устанавливает фиксированный размер окна

	История	: 13.01.18	Создан

*/
static void nglSetWindowMinMaxSize(void)
{
	XSizeHints sh;

	sh.flags = PMinSize | PMaxSize;
	sh.min_width = sh.max_width = ngl_windpix*ngl_winx/100;
	sh.min_height = sh.max_height = ngl_windpiy*ngl_winy/100;

	XSetWMNormalHints(ngl_win_dpy, ngl_win_glwin, &sh);
}

/*
	Функция : nglIsMainRContext

	Описание: Возвращает true, если текущий контекст - главный (контекст рендеринга).

	История	: 15.11.17	Создан

*/
bool nglIsMainRContext(void)
{
	if(glXGetCurrentContext() == ngl_win_glcontext)
		return true;
	else
		return false;
}

/*
	Функция : nglMakeCurrentRContextNull

	Описание: Сбрасывает текущий используемый контекст.

	История	: 07.12.17	Создан

*/
void N_APIENTRY nglMakeCurrentRContextNull(void *param)
{
	(void)param; // Unused

	if(glXGetCurrentContext())
		glXMakeCurrent(ngl_win_dpy, None, NULL);
}

/*
	Функция : nglAddSeparateRContextIfNeeded

	Описание: Если для потока не выбран контекст, создаёт отдельный констекст OpenGL, добавляет его в массив.
		В данной реализации контексты уже созданы и добавлены, нужно только увеличить счётчик ngl_win_nof_used_separate_hRCs.

	История	: 15.11.17	Создан

*/
bool nglAddSeparateRContextIfNeeded(void)
{
	if(glXGetCurrentContext() != NULL)
		return true;

	ngl_ea->nLockMutex(ngl_win_separate_hRCs_synch_mutex);
		if(ngl_win_nof_used_separate_hRCs < ngl_win_nof_separate_hRCs) {
			glXMakeCurrent(ngl_win_dpy, ngl_win_glwin, ngl_win_separate_hRCs[ngl_win_nof_used_separate_hRCs]);
			ngl_win_nof_used_separate_hRCs++;
		} else {
			ngl_ea->nUnlockMutex(ngl_win_separate_hRCs_synch_mutex);
			ngl_ea->nlPrint(LOG_FDEBUGFORMAT, F_NGLADDSEPARATERCONTEXTIFNEEDED, ERR_FAILEDTOCREATESEPARATERC);
			return false;
		}
	ngl_ea->nUnlockMutex(ngl_win_separate_hRCs_synch_mutex);

	return true;

}

/*
	Функция	: nUpdateSAHandler

	Описание: Обработка сигналов, относящихся к обновлению окна

	История	: 01.01.17	Создан

*/
static void nUpdateSAHandler(int signo)
{
	(void)signo; // Unused

	return;
}

/*
	Функция	: nglInitWindow

	Описание: Создаёт окно

	История	: 26.10.12	Создан

*/
bool nglInitWindow(void)
{
	wchar_t *tempt = 0;
	char *wintitle; int wintitle_size;
	XSetWindowAttributes swa;
	XEvent event;
	Atom wmDelete;
	int dummy;
	int vidmode_majorversion, vidmode_minorversion;
	int vidmode_bestmode = 0, vidmode_bestdiff;
	int vidmode_nofmodes;
	int xf86_event_base, xf86_error_base;
	XF86VidModeModeInfo **vidmode_modes;
	char *glx_extstr = 0;
	int i;

	XInitThreads(); // Не проверяем ошибку, т.к. система может не поддерживать многопоточность. И не понятно что будет, если вызвать её несколько раз подряд в программе

	ngl_win_dpy = XOpenDisplay(NULL);
	if(ngl_win_dpy == NULL){
		ngl_ea->nlPrint(LOG_FDEBUGFORMAT, F_NGLINITWINDOW, ERR_COULDNTOPENDISPLAY);
		return false;
	}

	ngl_win_screen = DefaultScreen(ngl_win_dpy);

	XrmInitialize();

	nglGetDisplayDPI();

	if(XF86VidModeQueryExtension(ngl_win_dpy, &xf86_event_base, &xf86_error_base) == 0){
		ngl_ea->nlPrint(LOG_FDEBUGFORMAT, F_NGLINITWINDOW, ERR_FAILEDTOQUERYXF86VIDMODEEXT);
		ngl_win_xf86vidmode_ext = false;
	} else
		ngl_win_xf86vidmode_ext = true;

	//===Смена разрешения===
	if(ngl_winmode == NV_MODE_FULLSCREEN && ngl_win_xf86vidmode_ext == true) {
		XF86VidModeQueryVersion(ngl_win_dpy, &vidmode_majorversion, &vidmode_minorversion);
		ngl_ea->nlPrint(L"%ls: %ls %d.%d", F_NGLINITWINDOW, L"XF86VidModeQueryVersion returned", vidmode_majorversion, vidmode_minorversion);

		XF86VidModeGetAllModeLines(ngl_win_dpy, ngl_win_screen, &vidmode_nofmodes, &vidmode_modes);
		ngl_win_olddescmode = *vidmode_modes[0];

		ngl_ea->nlPrint(LOG_FDEBUGFORMAT, F_NGLINITWINDOW, L"XF86VidModeGetAllModeLines returned");
		vidmode_bestdiff = abs((int)(vidmode_modes[vidmode_bestmode]->hdisplay)-(int)(ngl_winx))+abs((int)(vidmode_modes[vidmode_bestmode]->vdisplay)-(int)(ngl_winy));
		for(i = 0; i < vidmode_nofmodes; i++) {
			ngl_ea->nlPrint(L"%ls: %ls %d %ls %dx%d", F_NGLINITWINDOW,
				L"\tmode", i,
				L"res", vidmode_modes[i]->hdisplay, vidmode_modes[i]->vdisplay);

			if(vidmode_modes[i]->hdisplay == ngl_winx && vidmode_modes[i]->vdisplay == ngl_winy) {
				vidmode_bestmode = i;
				vidmode_bestdiff = 0;
			} else if((abs((int)(vidmode_modes[i]->hdisplay)- (int)(ngl_winx))+abs((int)(vidmode_modes[i]->vdisplay)- (int)(ngl_winy))) < vidmode_bestdiff) {
				vidmode_bestmode = i;
				vidmode_bestdiff = abs((int)(vidmode_modes[i]->hdisplay)- (int)(ngl_winx))+abs((int)(vidmode_modes[i]->vdisplay)- (int)(ngl_winy));
			}
		}

		if(vidmode_bestmode) {
			if(XF86VidModeSwitchToMode(ngl_win_dpy, ngl_win_screen, vidmode_modes[vidmode_bestmode])) {
				XF86VidModeSetViewPort(ngl_win_dpy, ngl_win_screen, 0, 0);
				XFlush(ngl_win_dpy);
				ngl_win_isResolutionChanged = true;
			} else
				vidmode_bestmode = 0;
		}

		ngl_winx = vidmode_modes[vidmode_bestmode]->hdisplay;
		ngl_winy = vidmode_modes[vidmode_bestmode]->vdisplay;

		XFree(vidmode_modes);
	}

	if(ngl_winmode == NV_MODE_FULLSCREEN) {
		ngl_windpix = 100;
		ngl_windpiy = 100;
	}

	//===Создания контекста OpenGL===
	if(!glXQueryExtension(ngl_win_dpy, &dummy, &dummy)){
		ngl_ea->nlPrint(LOG_FDEBUGFORMAT, F_NGLINITWINDOW, ERR_FAILEDTOQUERYGLXEXT);
		return false;
	}

	/* find an OpenGL-capable Color Index visual with depth buffer */
	ngl_win_vi = glXChooseVisual(ngl_win_dpy, DefaultScreen(ngl_win_dpy), ngl_win_attributes);
	if(ngl_win_vi == NULL) {
		ngl_ea->nlPrint(LOG_FDEBUGFORMAT, F_NGLINITWINDOW, ERR_COULDNTGETVISUAL);
		return false;
	}

	// Получение списка расширений glX
	glx_extstr = (char *)glXQueryExtensionsString(ngl_win_dpy, ngl_win_vi->screen);
	tempt = ngl_ea->nAllocMemory(sizeof(wchar_t)*(strlen(glx_extstr)+1));
	if(tempt) {
		mbstowcsl(tempt, glx_extstr, strlen(glx_extstr)+1);
		ngl_ea->nlPrint(LOG_FDEBUGFORMAT2, F_NGLINITWINDOW, L"glXQueryExtensionsString", tempt);
		ngl_ea->nFreeMemory(tempt);
	}

	// Создание основного контекста OpenGL
	ngl_win_glcontext = glXCreateContext(ngl_win_dpy, ngl_win_vi,  None, GL_TRUE);
	if(ngl_win_glcontext == NULL) {
		ngl_ea->nlPrint(LOG_FDEBUGFORMAT, F_NGLINITWINDOW, ERR_FAILEDTOCREATERC);
		return false;
	}

	// Создания мьютекса для синхронизации добавления отдельных контекстов OpenGL
	ngl_win_separate_hRCs_synch_mutex = ngl_ea->nCreateMutex();
	if(!ngl_win_separate_hRCs_synch_mutex) {
		ngl_ea->nlPrint(LOG_FDEBUGFORMAT, F_NGLINITWINDOW, ERR_FAILEDTOCREATESEPARATERC);
		glXDestroyContext(ngl_win_dpy, ngl_win_glcontext);
		return false;
	}

	// Добавление отдельных контекстов OpenGL
	ngl_win_nof_separate_hRCs = ngl_ea->nGetMaxThreadsForTasks();
	if(ngl_win_nof_separate_hRCs) {
		unsigned int i;

		ngl_win_separate_hRCs = ngl_ea->nAllocMemory(ngl_win_nof_separate_hRCs*sizeof(GLXContext));
		if(!ngl_win_separate_hRCs) {
			ngl_ea->nlPrint(LOG_FDEBUGFORMAT, F_NGLINITWINDOW, ERR_FAILEDTOCREATESEPARATERC);
			ngl_win_nof_separate_hRCs = 0;
			ngl_ea->nDestroyMutex(ngl_win_separate_hRCs_synch_mutex);
			ngl_win_separate_hRCs_synch_mutex = 0;
			glXDestroyContext(ngl_win_dpy, ngl_win_glcontext);
			return false;
		}

		memset(ngl_win_separate_hRCs, 0, ngl_win_nof_separate_hRCs*sizeof(GLXContext));

		for(i = 0; i < ngl_win_nof_separate_hRCs; i++) {
			ngl_win_separate_hRCs[i] = glXCreateContext(ngl_win_dpy, ngl_win_vi, ngl_win_glcontext, GL_TRUE);
			if(!(ngl_win_separate_hRCs[i])){
				unsigned int j;

				ngl_ea->nlPrint(LOG_FDEBUGFORMAT, F_NGLINITWINDOW, ERR_FAILEDTOCREATESEPARATERC);
				for(j = 0; j < i; j++)
					glXDestroyContext(ngl_win_dpy, ngl_win_separate_hRCs[i]);
				ngl_win_nof_separate_hRCs = 0;
				ngl_ea->nFreeMemory(ngl_win_separate_hRCs);
				ngl_ea->nDestroyMutex(ngl_win_separate_hRCs_synch_mutex);
				ngl_win_separate_hRCs_synch_mutex = 0;
				glXDestroyContext(ngl_win_dpy, ngl_win_glcontext);
				return false;
			}

			//wglShareLists(ngl_win_hRC, ngl_win_separate_hRCs[i]);
		}
	}

	/* create an X colormap since probably not using default visual */
	ngl_win_cmap = XCreateColormap(ngl_win_dpy, RootWindow(ngl_win_dpy, ngl_win_vi->screen),
                                ngl_win_vi->visual, AllocNone);

	//===Создание окна===
	swa.colormap = ngl_win_cmap;
	swa.border_pixel = 0;
	swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | StructureNotifyMask;
	/*if(ngl_winmode == NV_MODE_FULLSCREEN) {
		// Только в случае написания fullscreen с нуля
		swa.override_redirect = True;

		ngl_win_glwin = XCreateWindow(ngl_win_dpy, RootWindow(ngl_win_dpy, ngl_win_vi->screen), 0, 0, ngl_winx,
			ngl_winy, 0, ngl_win_vi->depth, InputOutput, ngl_win_vi->visual,
			CWBorderPixel | CWColormap | CWEventMask | CWOverrideRedirect, &swa);
	} else*/
		ngl_win_glwin = XCreateWindow(ngl_win_dpy, RootWindow(ngl_win_dpy, ngl_win_vi->screen), 0, 0, ngl_windpix*ngl_winx/100,
			ngl_windpiy*ngl_winy/100, 0, ngl_win_vi->depth, InputOutput, ngl_win_vi->visual,
			CWBorderPixel | CWColormap | CWEventMask, &swa);

	if(ngl_winmode == NV_MODE_WINDOWED_FIXED) {
		nglSetWindowMinMaxSize();
	} else if(ngl_winmode == NV_MODE_FULLSCREEN) {
		Atom atoms[2] = { XInternAtom(ngl_win_dpy, "_NET_WM_STATE_FULLSCREEN", False), None };

		XChangeProperty(
			ngl_win_dpy, /* display */
			ngl_win_glwin, /* w */
			XInternAtom(ngl_win_dpy, "_NET_WM_STATE", False), /* property */
			XA_ATOM, /* type */
			32, /* format */
			PropModeReplace, /* mode */
			(unsigned char *)atoms, /* data */
			1 /* nelements */
			);
	}

	// Перехватывание WM_DELETE_WINDOW
	wmDelete = XInternAtom(ngl_win_dpy, "WM_DELETE_WINDOW", True);
	if(wmDelete != None)
		XSetWMProtocols(ngl_win_dpy, ngl_win_glwin, &wmDelete, 1);

	// Установка имени окна
	wintitle_size = wcslen(ngl_wintitle)+1;
	wintitle = malloc(wintitle_size*sizeof(char));
	if(!wintitle) {
		XSetStandardProperties(ngl_win_dpy, ngl_win_glwin, "Unnamed", "Unnamed", None, 0/*argv*/,
									0/*argc*/, NULL);
	} else {
		wcstombsl(wintitle, ngl_wintitle, wintitle_size);
		XSetStandardProperties(ngl_win_dpy, ngl_win_glwin, wintitle, wintitle, None, 0/*argv*/,
									0/*argc*/, NULL);
		free(wintitle);
	}

	glXMakeCurrent(ngl_win_dpy, ngl_win_glwin, ngl_win_glcontext);

	XMapRaised(ngl_win_dpy, ngl_win_glwin);
	XIfEvent(ngl_win_dpy,  &event,  WaitForMapNotify,  (char *)ngl_win_glwin);

	//===Дополнительная настройка окна и OpenGL===

	// Создание карты для преобразования из кейкодов в NK_*
	nglMakeKeycodeMap();

	// Поддержка вертикальной синхронизации
	ngl_win_swapc_ext = false;
	if(strstr(glx_extstr, "GLX_MESA_swap_control"))
		ngl_win_swapc_ext = true;
	ngl_ea->nlPrint(LOG_FDEBUGFORMAT2, F_NGLINITWINDOW, NGL_SUPPORTVSYNC, ngl_win_swapc_ext?N_YES:N_NO);

	// Установка вертикальной синхронизации
	if(ngl_win_swapc_ext) {
		funcptr_glXSwapIntervalMESA = (glXSwapIntervalMESA_type)glXGetProcAddress((GLubyte *)"glXSwapIntervalMESA");
		if(funcptr_glXSwapIntervalMESA(ngl_win_vsync)) {
			ngl_ea->nlPrint(LOG_FDEBUGFORMAT, F_NGLINITWINDOW, ERR_CANTCHANGEVSYNC); ngl_win_vsync = 0; ngl_win_swapc_ext = false; }
	} else
		ngl_win_vsync = 0;
	ngl_ea->nlPrint(LOG_FDEBUGFORMAT2, F_NGLINITWINDOW, NGL_VSYNC, ngl_win_vsync?N_YES:N_NO);

	// Установка таймера, который будет использоваться для обновления окна
	// Установка обработчика SIGUSR1
	ngl_win_updatesigaction.sa_flags = SA_RESTART;
	sigemptyset(&(ngl_win_updatesigaction.sa_mask));
	ngl_win_updatesigaction.sa_handler = nUpdateSAHandler;
	if(sigaction(SIGUSR1, &ngl_win_updatesigaction, &ngl_win_oldsigaction) == 0) {
		// Перечисление сигналов (SIGUSR1), которые будут интерпретироваться как срабатывание таймера
		sigemptyset(&ngl_win_updatesigset);
		sigaddset(&ngl_win_updatesigset, SIGUSR1);
		// Определение уведомления при срабатывании таймера (сигнал SIGUSR1)
		ngl_win_sevp.sigev_notify = SIGEV_SIGNAL;
		ngl_win_sevp.sigev_signo = SIGUSR1;
		ngl_win_sevp.sigev_value.sival_int = 0;
		// Собственно создание таймера
		if(timer_create(CLOCK_MONOTONIC, &ngl_win_sevp, &ngl_win_updatetimerid) != 0) {
			ngl_win_updatetimerid = 0;
			ngl_win_updateinterval = 0;
			ngl_ea->nlPrint(LOG_FDEBUGFORMAT, F_NGLINITWINDOW, ERR_FAILEDTOCREATETIMER);
		} /*else
			ngl_ea->nlPrint(LOG_FDEBUGFORMAT3, F_NGLINITWINDOW, L"Created timer, id", ngl_win_updatetimerid);*/
	} else {
		ngl_win_updatetimerid = 0;
		ngl_win_updateinterval = 0;
		ngl_ea->nlPrint(LOG_FDEBUGFORMAT, F_NGLINITWINDOW, ERR_FAILEDTOCREATETIMER);
	}
	ngl_win_timerneedcancel = false;
	if(ngl_win_updateinterval != 0) {
		struct itimerspec new_value;

		//ngl_ea->nlPrint(L"Set timer to %d", ngl_win_updateinterval);

		new_value.it_value.tv_sec = new_value.it_interval.tv_sec = ngl_win_updateinterval/1000;
		new_value.it_value.tv_nsec = new_value.it_interval.tv_nsec = 1000000*(long)(ngl_win_updateinterval%1000);

		if(timer_settime(ngl_win_updatetimerid, 0, &new_value, NULL) != 0)
			ngl_win_updateinterval = 0;

		//ngl_ea->nlPrint(L"Timer set to %d values %d %d", ngl_win_updateinterval, (int)(new_value.it_value.tv_sec), (int)(new_value.it_value.tv_nsec));
	}

	return true;
}

/*
	Функция	: nglCloseWindow

	Описание: Уничтожает окно

	История	: 26.10.12	Создан

*/
bool nglCloseWindow(void)
{
	glXMakeCurrent(ngl_win_dpy, None, NULL);
	glXDestroyContext(ngl_win_dpy, ngl_win_glcontext);

	if(ngl_win_nof_separate_hRCs) {
		unsigned int i;

		for(i = 0; i < ngl_win_nof_separate_hRCs; i++) {
			glXDestroyContext(ngl_win_dpy, ngl_win_separate_hRCs[i]);
		}

		ngl_win_nof_separate_hRCs = 0;
	}

	ngl_win_nof_used_separate_hRCs = 0;

	if(ngl_win_separate_hRCs) {
		ngl_ea->nFreeMemory(ngl_win_separate_hRCs);
		ngl_win_separate_hRCs = 0;
	}

	ngl_ea->nDestroyMutex(ngl_win_separate_hRCs_synch_mutex);

	if(ngl_win_isResolutionChanged == true && ngl_win_xf86vidmode_ext == true) {
		XF86VidModeSwitchToMode(ngl_win_dpy, ngl_win_screen, &ngl_win_olddescmode);
		XF86VidModeSetViewPort(ngl_win_dpy, ngl_win_screen, 0, 0);
	}

	XDestroyWindow(ngl_win_dpy, ngl_win_glwin);

	XFreeColormap(ngl_win_dpy, ngl_win_cmap); // Закомментировал, так как уже происходит в XCloseDisplay

	XFree(ngl_win_vi);

	XCloseDisplay(ngl_win_dpy);

	if(ngl_win_updatetimerid != 0) {
		//ngl_ea->nlPrint(L"Free timer (id %d)", ngl_win_updatetimerid);
		timer_delete(ngl_win_updatetimerid);
		ngl_win_updatetimerid = 0;
		ngl_win_updateinterval = 0;

		sigaction(SIGUSR1, &ngl_win_oldsigaction, NULL);
	}

	return true;
}

/*
	Функция	: nglUpdateWindow

	Описание: Обновление окна

	История	: 26.10.12	Создан

*/
void nglUpdateWindow(void)
{
	XEvent event;
	KeySym key;
	int ucskey;
	char *message_atom_name;
	GLsizei virtua_x, virtua_y;
	GLint start_y;

	glXSwapBuffers(ngl_win_dpy, ngl_win_glwin);

	if(ngl_win_updateinterval) {
		int sig = 0;

		if(sigwait(&ngl_win_updatesigset, &sig) == 0) { // Успех
			if(ngl_win_timerneedcancel == true) {
				struct itimerspec new_value;

				ngl_win_timerneedcancel = false;

				//ngl_ea->nlPrint(L"Cancel timer");

				new_value.it_value.tv_sec = new_value.it_interval.tv_sec = 0;
				new_value.it_value.tv_nsec = new_value.it_interval.tv_nsec = 0;

				if(timer_settime(ngl_win_updatetimerid, 0, &new_value, NULL) == 0) // Таймер успешно выключен
					ngl_win_updateinterval = 0;
				else
					ngl_ea->nlPrint(LOG_FDEBUGFORMAT, F_NGLUPDATEWINDOW, ERR_CANTCHANGEUPDATEINTERVAL);

				//ngl_ea->nlPrint(L"Timer set to %d values %d %d", ngl_win_updateinterval, (int)(new_value.it_value.tv_sec), (int)(new_value.it_value.tv_nsec));
			}
		}
	}

	ngl_win_exitmsg = false;

	while (XPending(ngl_win_dpy)) {
		XNextEvent(ngl_win_dpy, &event);
		switch (event.type) {
			case MotionNotify:
				ngl_winmx = 100*event.xmotion.x/ngl_windpix;
				ngl_winmy = 100*event.xmotion.y/ngl_windpiy;
				//ngl_ea->nlPrint(L"MotionNotify event.xmotion. x %d y %d",event.xmotion.x,event.xmotion.y);
				break;
			case ButtonPress: // Тут должна быть обработка нажатия кнопок мыши
				if(event.xbutton.button == Button1)
					ngl_winmbl = 1;
				else if(event.xbutton.button == Button2)
					ngl_winmbm = 1;
				else if(event.xbutton.button == Button3)
					ngl_winmbr = 1;
				else if(event.xbutton.button == Button4)
					ngl_winmwheel++;
				else if(event.xbutton.button == Button5)
					ngl_winmwheel--;
				//ngl_ea->nlPrint(L"ButtonPress event.xbutton.button %u",event.xbutton.button);
				break;
			case ButtonRelease:
				if(event.xbutton.button == Button1)
					ngl_winmbl = 2;
				else if(event.xbutton.button == Button2)
					ngl_winmbm = 2;
				else if(event.xbutton.button == Button3)
					ngl_winmbr = 2;
				//ngl_ea->nlPrint(L"ButtonRelease event.xbutton.button %u",event.xbutton.button);
				break;
			case KeyPress:
				XLookupString((XKeyEvent *)&event, NULL, 0, &key, NULL);
				ucskey = keysym2ucs(key);
				if(ucskey > -1 && ucskey < 65536)
					if(ngl_wintextinputbufsize < 255) {
						*(ngl_wintextinputbuf+ngl_wintextinputbufsize) = ucskey;
						ngl_wintextinputbufsize++;
					}
				ngl_winkeys[ngl_win_keycodes_map[event.xkey.keycode]] = 1;
				//ngl_ea->nlPrint(L"KeyPress key %d/%x ucskey %d/%x", (int)key, (int)key, ucskey, ucskey);
				//ngl_ea->nlPrint(L"KeyPress keycode %d NK_*** %d", event.xkey.keycode, ngl_win_keycodes_map[event.xkey.keycode]);
				break;
			case KeyRelease:
				ngl_winkeys[ngl_win_keycodes_map[event.xkey.keycode]] = 2;
				//ngl_ea->nlPrint(L"KeyRelease keycode %d NK_*** %d %x key %d/%x ucskey %d/%x", event.xkey.keycode, ngl_win_keycodes_map[event.xkey.keycode], ngl_win_keycodes_map[event.xkey.keycode], (int)key, (int)key, ucskey, ucskey);
				break;
			case ClientMessage:
				message_atom_name = XGetAtomName(ngl_win_dpy, event.xclient.message_type);
				if(message_atom_name) {
					if(!strcmp(message_atom_name, "WM_PROTOCOLS")) {
						char *data_atom_name;

						data_atom_name = XGetAtomName(ngl_win_dpy, event.xclient.data.l[0]);
						if(data_atom_name) {
							if(!strcmp(data_atom_name, "WM_DELETE_WINDOW"))
								ngl_win_exitmsg = true;
							XFree(data_atom_name);
						}
					}
					XFree(message_atom_name);
				}
				break;
			case ConfigureNotify:
				ngl_winx = 100*event.xconfigure.width/ngl_windpix;
				ngl_winy = 100*event.xconfigure.height/ngl_windpiy;
				
				virtua_x = ngl_windpix*ngl_winx/100;
				virtua_y = ngl_windpiy*ngl_winy/100;

				if(virtua_y < event.xconfigure.height)
					start_y = event.xconfigure.height-virtua_y;
				else
					start_y = 0;

				//wprintf(L"real %d %d win %d %d start_y %d\n", (int)event.xconfigure.width, (int)event.xconfigure.height, (int)virtua_x, (int)virtua_y, (int)start_y);

				if(ngl_winmode != NV_MODE_FULLSCREEN) {
					int olddpix, olddpiy;

					olddpix = ngl_windpix;
					olddpiy = ngl_windpiy;

					nglGetDisplayDPI();

					if(olddpix != ngl_windpix || olddpiy != ngl_windpiy) {
						if(ngl_winmode == NV_MODE_WINDOWED_FIXED)
							nglSetWindowMinMaxSize();
						else
							XResizeWindow(ngl_win_dpy, ngl_win_glwin, ngl_windpix*ngl_winx/100, ngl_windpiy*ngl_winy/100);
					}
				}

				glViewport(0, start_y, virtua_x, virtua_y);
				if(ngl_batch_state == NGL_BATCH_STATE_3D) {
					glMatrixMode(GL_PROJECTION);
					glLoadIdentity();
					gluPerspective(45.0, (ngl_winx != 0 && ngl_winy != 0)?((GLdouble)ngl_winx/(GLdouble)ngl_winy):(1.0), 2.0, 4000.0);
					glMatrixMode(GL_MODELVIEW);
					glLoadIdentity();
				} else if(ngl_batch_state == NGL_BATCH_STATE_2D) {
					glMatrixMode(GL_PROJECTION);
					glLoadIdentity();
					glOrtho(0.0,(ngl_winx)?ngl_winx:1,(ngl_winy)?ngl_winy:1,0.0,-1.0,1.0);
					glMatrixMode(GL_MODELVIEW);
					glLoadIdentity();
				}

				nglCatchOpenGLError(0);

				break;
		}
	}

	ngl_winkeys[NK_SHIFT] = ngl_winkeys[NK_LSHIFT] | ngl_winkeys[NK_RSHIFT];
	if(ngl_winkeys[NK_SHIFT] == 3) ngl_winkeys[NK_SHIFT] = 1;
	
	ngl_winkeys[NK_CONTROL] = ngl_winkeys[NK_LCONTROL] | ngl_winkeys[NK_RCONTROL];
	if(ngl_winkeys[NK_CONTROL] == 3) ngl_winkeys[NK_CONTROL] = 1;
	
	ngl_winkeys[NK_MENU] = ngl_winkeys[NK_LMENU] | ngl_winkeys[NK_RMENU];
	if(ngl_winkeys[NK_MENU] == 3) ngl_winkeys[NK_MENU] = 1;
}

/*
	Функция	: nglChangeVSYNC

	Описание: Меняет вертикальную синхронизацию

	История	: 26.10.12	Создан

*/
void nglChangeVSYNC(int vsync)
{
	if(ngl_isinit) {
		if(ngl_win_swapc_ext) {
			if(vsync == ngl_win_vsync)
				return;

			if(funcptr_glXSwapIntervalMESA(vsync)) {
				ngl_ea->nlPrint(LOG_FDEBUGFORMAT,F_NGLSETSTATUSI,ERR_CANTCHANGEVSYNC);
				ngl_win_vsync = 0;
			} else
				ngl_win_vsync = vsync;
		}
	} else
		ngl_win_vsync = vsync;
}

/*
	Функция	: nglChangeUpdateInterval

	Описание: Меняет интервал обновления экрана

	История	: 23.03.17	Создан

*/
void nglChangeUpdateInterval(unsigned int updateinterval)
{
	if(ngl_win_updateinterval == updateinterval) return;

	if(ngl_isinit) {
		if(ngl_win_updatetimerid != 0) {
			if(updateinterval == 0)
				ngl_win_timerneedcancel = true;
			else {
				struct itimerspec new_value;

				ngl_win_updateinterval = updateinterval;

				//ngl_ea->nlPrint(L"Set timer to %d", ngl_win_updateinterval);

				new_value.it_value.tv_sec = new_value.it_interval.tv_sec = ngl_win_updateinterval/1000;
				new_value.it_value.tv_nsec = new_value.it_interval.tv_nsec = 1000000*(long)(ngl_win_updateinterval%1000);

				if(timer_settime(ngl_win_updatetimerid, 0, &new_value, NULL) != 0) {
					ngl_ea->nlPrint(LOG_FDEBUGFORMAT, F_NGLSETSTATUSI, ERR_CANTCHANGEUPDATEINTERVAL);
					ngl_win_updateinterval = 0;
				}

				//ngl_ea->nlPrint(L"Timer set to %d values %d %d", ngl_win_updateinterval, (int)(new_value.it_value.tv_sec), (int)(new_value.it_value.tv_nsec));
			}
		} else {
			ngl_ea->nlPrint(LOG_FDEBUGFORMAT, F_NGLSETSTATUSI, ERR_CANTCHANGEUPDATEINTERVAL); ngl_win_updateinterval = 0;
		}
	} else {
		ngl_win_updateinterval = updateinterval;
	}
}

/*
	Функция	: nglChangeWindowMode

	Описание: Устанавливает режим окна (полноэкранный, оконный...)
		Разрешение экрана меняется в nglChangeWindowSize

	История	: 09.03.18	Создан

*/
void nglChangeWindowMode(unsigned int winx, unsigned int winy, int winmode)
{
	(void)winmode; // unused

	if(!ngl_isinit) return;

	// Устанавливаем стили окна
	// ...

	// Меняем разрешение экрана
	//if(winmode == NV_MODE_FULLSCREEN) {
		//if((ngl_winmode != winmode) || (ngl_winx != winx) || (ngl_winy != winy)) {
			// Ставим новое разрешение экрана

			// В полноэкранном режиме не учитываем DPI
			//ngl_windpix = 100;
			//ngl_windpiy = 100;
		//}
	//} else if(ngl_winmode == NV_MODE_FULLSCREEN) { // winmode != NV_MODE_FULLSCREEN, а ngl_winmode == NV_MODE_FULLSCREEN
		// Восстанавливаем оригинальное разрешение экрана
	//}


	if(ngl_winx != winx || ngl_winy != winy) {
		XResizeWindow(ngl_win_dpy, ngl_win_glwin, ngl_windpix*winx/100, ngl_windpiy*winy/100);
	}

	//ngl_winmode = winmode;

	// Установка происходит при обработке события ConfigureNotify
	//ngl_winx = winx;
	//ngl_winy = winy;
}

/*
	Функция	: nglChangeWindowTitle

	Описание: Устанавливает заголовок окна

	История	: 20.06.18	Создан

*/
void nglChangeWindowTitle(const wchar_t *title)
{
	(void)title;

	return;
}

/*
	Функция	: nglGetProcAddress

	Описание: Возращает процедуру по имени

	История	: 26.10.12	Создан

*/
ngl_func_type nglGetProcAddress(const char * name)
{
	return glXGetProcAddress((GLubyte *)name);
}
