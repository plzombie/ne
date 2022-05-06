/*
	Файл	: nyan_vismodes_publicapi.h

	Описание: Константы для определения режимов модуля визуализации

	История	: 12.08.12	Создан

*/

#ifndef NYAN_VISMODES_PUBLICAPI_H
#define NYAN_VISMODES_PUBLICAPI_H

// Параметры NV_STATUS_WINMODE
#define NV_MODE_FULLSCREEN 0
#define NV_MODE_WINDOWED 1
#define NV_MODE_WINDOWED_FIXED 2

// Параметры nv(ngl)SetStatusi и nvGetStatusi
#define NV_STATUS_WINMODE 1
#define NV_STATUS_WINBPP 2
#define NV_STATUS_WINX 3
#define NV_STATUS_WINY 4
#define NV_STATUS_WINMBL 5
#define NV_STATUS_WINMBR 6
#define NV_STATUS_WINMBM 7
#define NV_STATUS_WINMX 8
#define NV_STATUS_WINMY 9
#define NV_STATUS_WIN_EXITMSG 10
#define NV_STATUS_WINVSYNC 11
#define NV_STATUS_SUPPORT_WINVSYNC 12
#define NV_STATUS_WINCLIPPINGREGION 13
#define NV_STATUS_WINCLIPPINGREGIONSX 14
#define NV_STATUS_WINCLIPPINGREGIONSY 15
#define NV_STATUS_WINCLIPPINGREGIONEX 16
#define NV_STATUS_WINCLIPPINGREGIONEY 17
#define NV_STATUS_WINUPDATEINTERVAL 18
#define NV_STATUS_SUPPORT_WINTOUCH 19
#define NV_STATUS_WINTOUCHCOUNT 20
#define NV_STATUS_WINTOUCHMAXCOUNT 21
#define NV_STATUS_IS_WINDPINONSTANDARD 22
#define NV_STATUS_WINDPIX 23
#define NV_STATUS_WINDPIY 24
#define NV_STATUS_WINMWHEEL 25

// Параметры nv(ngl)GetStatusi, относящиеся к Window Touch input
#define NV_STATUS_WINTOUCH0_X 0x100
#define NV_STATUS_WINTOUCH255_X 0x1ff
#define NV_STATUS_WINTOUCHMAX_X NV_STATUS_WINTOUCH255_X
#define NV_STATUS_WINTOUCH0_Y 0x200
#define NV_STATUS_WINTOUCH255_Y 0x2ff
#define NV_STATUS_WINTOUCHMAX_Y NV_STATUS_WINTOUCH255_Y
#define NV_STATUS_WINTOUCH0_PRESSED 0x300
#define NV_STATUS_WINTOUCH255_PRESSED 0x3ff
#define NV_STATUS_WINTOUCHMAX_PRESSED NV_STATUS_WINTOUCH255_PRESSED

// Параметры nv(ngl)SetStatusf и nvGetStatusf
#define NV_STATUS_WINBCRED 1
#define NV_STATUS_WINBCGREEN 2
#define NV_STATUS_WINBCBLUE 3

// Параметры nv(ngl)SetStatusw и nvGetStatusw
#define NV_STATUS_WINTITLE 1
#define NV_STATUS_WINTEXTINPUTBUF 2

// Параметры (флаги) nv(ngl)Clear
#define NV_CLEAR_COLOR_BUFFER 1
#define NV_CLEAR_DEPTH_BUFFER 2

// Значения nv(ngl)GetKey и nv(ngl)GetStatusi с параметрами NV_STATUS_WINMB(L,B,R), NV_STATUS_WINTOUCH(0-255)_PRESSED
#define NV_KEYSTATUS_UNTOUCHED 0
#define NV_KEYSTATUS_PRESSED 1
#define NV_KEYSTATUS_RELEASED 2

#endif
