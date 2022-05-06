/*
	Файл	: nyan_fps.h

	Описание: Заголовок для nyan_fps.c

	История	: 01.07.12	Создан

*/

#include <stdint.h>

extern int64_t fps_deltaclocks;
extern int64_t fps_lastclocks;

extern double afps_deltaclocks;
extern int64_t afps_lastnulldeltaclockslen;
extern unsigned int afps_lastnulldeltas;
extern unsigned int afps_nulldeltas;
