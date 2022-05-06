#include "../../nyan/nyan_filesys_publicapi.h"
#include "../../nyan/nyan_mem_publicapi.h"
#include "ufmod_misc.h"

/*
	Функция	: uFMOD_PlaySong2

	Описание: Версия uFMOD_PlaySong для использования с файловой системой движка

	История	: 04.08.14	Создан

*/
#ifdef N_WINDOWS
HWAVEOUT* uFMOD_PlaySong2(wchar_t *fname, int fdwSong)
#else
int uFMOD_PlaySong2(wchar_t *fname, int fdwSong)
#endif
{
#ifdef N_WINDOWS
	HWAVEOUT* ret;
#else
	int ret;
#endif
	unsigned int f, size;
	void *mem;

	// Если fname указывает на 0, то останавливаем воспроизведение
	if(fname == 0)
		return uFMOD_StopSong();

	// Иначе, пытаемся открыть файл
	f = nFileOpen(fname);

	if(!f)
		return 0;

	size = (unsigned int)nFileLength(f); // Получаем размер файла

	mem = nAllocMemory(size); // Выделяем память под него

	if(!mem) {
		nFileClose(f);

		return 0;
	}

	nFileRead(f, mem, size); // Читаем файл в память

	fdwSong = fdwSong & (XM_NOLOOP | XM_SUSPENDED); // Удаляем все аргументы fdwSong, кроме XM_NOLOOP и XM_SUSPENDED

#ifdef N_WINDOWS
	ret = uFMOD_PlaySong(mem, (void *)size, fdwSong | XM_MEMORY); // Открываем файл из памяти
#else
	ret = uFMOD_PlaySong(mem, (int)size, fdwSong | XM_MEMORY); // Открываем файл из памяти
#endif

	// Освобождаем память и закрываем файл
	nFreeMemory(mem);
	nFileClose(f);

	return ret;
}
