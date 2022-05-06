/*
	Файл	: nyan_au_init.h

	Описание: Заголовок для nyan_au_init.c

	История	: 30.08.12	Создан

*/

#ifndef NYAN_AU_INIT_H
#define NYAN_AU_INIT_H

#include "nyan_threads.h"

extern unsigned int na_isinit;

extern bool naInit(void);
extern bool naClose(void);

#endif
