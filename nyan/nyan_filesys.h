/*
	Файл	: nyan_filesys.h

	Описание: Заголовок для nyan_filesys.c

	История	: 15.07.12	Создан

*/

#ifndef NYAN_FILESYS_H
#define NYAN_FILESYS_H

#include <wchar.h>

#include "nyan_threads.h"

// Функции для работы с fs_fsmutex
// Инициализация fs_fsmutex происходит в nInit() сразу перед вызовом nInitThreadsLib()
// Деинициализация - после вызова nDestroyThreadsLib()
extern bool nFileInitMutex(void);
extern bool nFileDestroyMutex(void);
extern void nFileLockMutex(void);
extern void nFileUnlockMutex(void);

#endif
