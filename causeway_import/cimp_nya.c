/*
	Файл	: cimp_nya.c

	Описание: Модуль, импортирующий функции для CauseWay Dos Extender

	История	: 05.08.12	Создан

*/

#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <cwdll.h>

#define N_CAUSEWAY_IMPLIB

#include "../nyan/nyan_publicapi.h"

void *cwnyadllhandle = 0;

static void *ImportFunc(char *name)
{
	void *ptr;

	ptr = GetProcAddress(cwnyadllhandle, name);
	if(!ptr) {
		wchar_t temp[1024];
		mbstowcs(temp, name, 1023); temp[1023] = 0;
		wprintf(L"Can't import %ls()\n", temp);
		exit(EXIT_FAILURE);
	}

	return ptr;
}

void nyanImportCauseWayDll(void)
{
	wprintf(L"Loading nya.dll\n");
	cwnyadllhandle = LoadLibrary("nya.dll");
	if(!cwnyadllhandle) {
		wprintf(L"Can't load nya.dll\n");
		exit(EXIT_FAILURE);
	}

	// В nyan_threads_publicapi.h
	// Работа с потоками
	nCreateTask = ImportFunc("_nCreateTask");
	nDestroyTask = ImportFunc("_nDestroyTask");
	nRunTaskOnAllThreads = ImportFunc("_nRunTaskOnAllThreads");
	nGetMaxThreadsForTasks = ImportFunc("_nGetMaxThreadsForTasks");
	nGetNumberOfTaskFunctionCalls = ImportFunc("_nGetNumberOfTaskFunctionCalls");
	nSetNumberOfTaskFunctionCalls = ImportFunc("_nSetNumberOfTaskFunctionCalls");

	nCreateMutex = ImportFunc("_nCreateMutex");
	nDestroyMutex = ImportFunc("_nDestroyMutex");
	nLockMutex = ImportFunc("_nLockMutex");
	nUnlockMutex = ImportFunc("_nUnlockMutex");

	nSleep = ImportFunc("_nSleep");

	// В nyan_init_publicapi.h
	// Инициализация движка
	nIsInit = ImportFunc("_nIsInit");
	nInit = ImportFunc("_nInit");
	nClose = ImportFunc("_nClose");
	nUpdate = ImportFunc("_nUpdate");

	// Инициализация рендера
	// В nyan_vis_init_publicapi.h
	// Прикрепление рендера
	nvAttachRender = ImportFunc("_nvAttachRender");
	// Очистка экрана
	nvClear = ImportFunc("_nvClear");
	// Снятие скриншота
	nvMakeScreenshot = ImportFunc("_nvMakeScreenshot");
	// Получение информации о графической подсистеме
	nvGetStatusi = ImportFunc("_nvGetStatusi");
	nvSetStatusi = ImportFunc("_nvSetStatusi");
	nvGetStatusf = ImportFunc("_nvGetStatusf");
	nvSetStatusf = ImportFunc("_nvSetStatusf");
	nvGetStatusw = ImportFunc("_nvGetStatusw");
	nvSetStatusw = ImportFunc("_nvSetStatusw");
	nvGetKey = ImportFunc("_nvGetKey");
	nvSetScreen = ImportFunc("_nvSetScreen");
	nvSetClippingRegion = ImportFunc("_nvSetClippingRegion");

	// В nyan_log_publicapi.h
	// Журнал сообщений
	nlEnable = ImportFunc("_nlEnable");
	nlAddTab = ImportFunc("_nlAddTab");
	nlPrint = ImportFunc("_nlPrint");

	// В nyan_fps_publicapi.h
	// Подсчёт кадров в секунду
	nClock = ImportFunc("_nClock");
	nFrameStartClock = ImportFunc("_nFrameStartClock");
	nGetfps = ImportFunc("_nGetfps");
	nGetafps = ImportFunc("_nGetafps");
	nGetspf = ImportFunc("_nGetspf");
	nGetaspf = ImportFunc("_nGetaspf");

	// В nyan_filesys_dirpaths_publicapi.h
	nFileGetDir = ImportFunc("_nFileGetDir");
	nFileAppendFilenameToDir = ImportFunc("_nFileAppendFilenameToDir");
	nFileGetAbsoluteFilename = ImportFunc("_nFileGetAbsoluteFilename");
	// В nyan_filesys_publicapi.h
	// Работа с файлами
	nFileCreateDir = ImportFunc("_nFileCreateDir");
	nFileDeleteDir = ImportFunc("_nFileDeleteDir");
	nFileCreate = ImportFunc("_nFileCreate");
	nFileDelete = ImportFunc("_nFileDelete");
	nFileRead = ImportFunc("_nFileRead");\
	nFileWrite = ImportFunc("_nFileWrite");
	nFileCanWrite = ImportFunc("_nFileCanWrite");
	nFileLength = ImportFunc("_nFileLength");
	nFileTell = ImportFunc("_nFileTell");
	nFileSeek = ImportFunc("_nFileSeek");
	nFileOpen = ImportFunc("_nFileOpen");
	nFileClose = ImportFunc("_nFileClose");
	nCloseAllFiles = ImportFunc("_nCloseAllFiles");
	nMountDir = ImportFunc("_nMountDir");
	nMountArchive = ImportFunc("_nMountArchive");

	nAddFilePlugin = ImportFunc("_nAddFilePlugin");
	nDeleteAllFilePlugins = ImportFunc("_nDeleteAllFilePlugins");

	//Текстуры
	// В nyan_vis_texture_publicapi.h
	nvCreateTexture = ImportFunc("_nvCreateTexture");\
	nvCreateTextureFromFile = ImportFunc("_nvCreateTextureFromFile");
	nvCreateTextureFromMemory = ImportFunc("_nvCreateTextureFromMemory");
	nvSetTextureLoadTypeVoid = ImportFunc("_nvSetTextureLoadTypeVoid");
	nvSetTextureLoadTypeFromFile = ImportFunc("_nvSetTextureLoadTypeFromFile");
	nvSetTextureLoadTypeFromMemory = ImportFunc("_nvSetTextureLoadTypeFromMemory");
	nvLoadImageToMemory = ImportFunc("_nvLoadImageToMemory");
	nvUpdateTextureFromMemory = ImportFunc("_nvUpdateTextureFromMemory");
	nvLoadTexture = ImportFunc("_nvLoadTexture");
	nvDestroyTexture = ImportFunc("_nvDestroyTexture");
	nvDestroyAllTextures = ImportFunc("_nvDestroyAllTextures");
	nvGetTextureStatus = ImportFunc("_nvGetTextureStatus");
	nvGetTextureLType = ImportFunc("_nvGetTextureLType");
	nvGetTextureWH = ImportFunc("_nvGetTextureWH");
	nvGetTextureFormat = ImportFunc("_nvGetTextureFormat");

	nvAddTexturePlugin = ImportFunc("_nvAddTexturePlugin");
	nvDeleteAllTexturePlugins = ImportFunc("_nvDeleteAllTexturePlugins");

	// Рисование
	// В nyan_vis_draw_publicapi.h
	nvBegin2d = ImportFunc("_nvBegin2d");
	nvEnd2d = ImportFunc("_nvEnd2d");
	nvSetAmbientLight = ImportFunc("_nvSetAmbientLight");
	nvGetAmbientLight = ImportFunc("_nvGetAmbientLight");
	nvBegin3d = ImportFunc("_nvBegin3d");
	nvEnd3d = ImportFunc("_nvEnd3d");
	nvDraw2dPoint = ImportFunc("_nvDraw2dPoint");
	nvDraw2dLine = ImportFunc("_nvDraw2dLine");
	nvDraw2dTriangle = ImportFunc("_nvDraw2dTriangle");
	nvDraw2dQuad = ImportFunc("_nvDraw2dQuad");
	nvDraw2d = ImportFunc("_nvDraw2d");
	nvDraw2dPicture = ImportFunc("_nvDraw2dPicture");
	nvDraw2dButton = ImportFunc("_nvDraw2dButton");
	nvDraw3dMesh = ImportFunc("_nvDraw3dMesh");
	nvDraw3dIndexedMesh = ImportFunc("_nvDraw3dIndexedMesh");

	// В nyan_vis_matrixes_publicapi.h
	// Матрицы
	nvMatrixSetTranslate = ImportFunc("_nvMatrixSetTranslate");
	nvMatrixSetRotateX = ImportFunc("_nvMatrixSetRotateX");
	nvMatrixSetRotateY = ImportFunc("_nvMatrixSetRotateY");
	nvMatrixSetRotateZ = ImportFunc("_nvMatrixSetRotateZ");
	nvMatrixSetScale = ImportFunc("_nvMatrixSetScale");
	nvMatrixSetMult = ImportFunc("_nvMatrixSetMult");
	nvMatrixMult = ImportFunc("_nvMatrixMult");
	nvMatrixSetCamera = ImportFunc("_nvMatrixSetCamera");
	nvSetMatrix = ImportFunc("_nvSetMatrix");

	// Сцена и 3d
	// В nyan_vis_3dmodel_publicapi.h
	nvCreate3dModel = ImportFunc("_nvCreate3dModel");
	nvLoad3dModel = ImportFunc("_nvLoad3dModel");
	nvUnload3dModel = ImportFunc("_nvUnload3dModel");
	nvDestroy3dModel = ImportFunc("_nvDestroy3dModel");
	nvDestroyAll3dModels = ImportFunc("_nvDestroyAll3dModels");
	nvDrawStatic3dModel = ImportFunc("_nvDrawStatic3dModel");
	nvDraw3dModel = ImportFunc("_nvDraw3dModel");
	nvDrawAnimated3dModel = ImportFunc("_nvDrawAnimated3dModel");
	nvDrawAnimated3dModel2 = ImportFunc("_nvDrawAnimated3dModel2");
	nvGet3dModelNumberOfFrames = ImportFunc("_nvGet3dModelNumberOfFrames");
	nvGet3dModelNumberOfSkins = ImportFunc("_nvGet3dModelNumberOfSkins");
	nvGet3dModelNumberOfAnims = ImportFunc("_nvGet3dModelNumberOfAnims");
	nvGet3dModelRefCount = ImportFunc("_nvGet3dModelRefCount");
	nvGet3dModelAnimIdByName = ImportFunc("_nvGet3dModelAnimIdByName");
	nvGet3dModelIdByName = ImportFunc("_nvGet3dModelIdByName");
	nvGet3dModelName = ImportFunc("_nvGet3dModelName");
	nvIncrease3dModelRefCount = ImportFunc("_nvIncrease3dModelRefCount");
	nvDecrease3dModelRefCount = ImportFunc("_nvDecrease3dModelRefCount");

	// В nyan_vis_fonts_publicapi.h
	// Шрифты и текст
	nvCreateFont = ImportFunc("_nvCreateFont");
	nvDestroyFont = ImportFunc("_nvDestroyFont");
	nvDestroyAllFonts = ImportFunc("_nvDestroyAllFonts");
	nvDraw2dText = ImportFunc("_nvDraw2dText");
	nvDraw2dTextbox = ImportFunc("_nvDraw2dTextbox");

	// В nyan_vis_justdraw_publicapi.h
	// Простой вывод графики
	nvJustDraw2dLine = ImportFunc("_nvJustDraw2dLine");
	nvJustDraw2dPoint = ImportFunc("_nvJustDraw2dPoint");

	// В nyan_vis_spritesheets_publicapi.h
	// Спрайты и текстурные атласы
	nvCreateSpriteSheetFromTileset = ImportFunc("_nvCreateSpriteSheetFromTileset");
	nvCreateSpriteSheetFromFile = ImportFunc("_nvCreateSpriteSheetFromFile");
	nvCreateSpriteSheetFromPreallocMemory = ImportFunc("_nvCreateSpriteSheetFromPreallocMemory");
	nvGetSpriteNameById = ImportFunc("_nvGetSpriteNameById");
	nvGetSpriteIdByName = ImportFunc("_nvGetSpriteIdByName");
	nvDrawSprite = ImportFunc("_nvDrawSprite");
	nvGetSpriteSheetSize = ImportFunc("_nvGetSpriteSheetSize");
	nvDestroySpriteSheet = ImportFunc("_nvDestroySpriteSheet");
	nvDestroyAllSpriteSheets = ImportFunc("_nvDestroyAllSpriteSheets");
	nvGetSpriteWH = ImportFunc("_nvGetSpriteWH");

	// В nyan_mem_publicapi.h
	// Работа с памятью
	nAllocMemory = ImportFunc("_nAllocMemory");
	nFreeMemory = ImportFunc("_nFreeMemory");
	nReallocMemory = ImportFunc("_nReallocMemory");

	// Загрузка плагинов из dll
	// В nyan_plgloader_publicapi.h
	nAddPlugin = ImportFunc("_nAddPlugin");
	nDeleteAllPlugins = ImportFunc("_nDeleteAllPlugins");

	// Работа со звуком
	// Функции для инициализации аудиодвижка
	// В nyan_au_init_publicapi.h
	naAttachLib = ImportFunc("_naAttachLib");
	naLoadAudioFile = ImportFunc("_naLoadAudioFile");
	naReadAudioFile = ImportFunc("_naReadAudioFile");
	naUnloadAudioFile = ImportFunc("_naUnloadAudioFile");
	naAddAudioFilePlugin = ImportFunc("_naAddAudioFilePlugin");
	naDeleteAllAudioFilePlugins = ImportFunc("_naDeleteAllAudioFilePlugins");
	// Функции аудиодвижка
	// В nyan_au_main_publicapi.h
	naCreateBuffer = ImportFunc("_naCreateBuffer");
	naDestroyAllBuffers = ImportFunc("_naDestroyAllBuffers");
	naDestroyBuffer = ImportFunc("_naDestroyBuffer");
	naLoadBuffer = ImportFunc("_naLoadBuffer");
	naUnloadBuffer = ImportFunc("_naUnloadBuffer");
	naCreateSource = ImportFunc("_naCreateSource");
	naDestroyAllSources = ImportFunc("_naDestroyAllSources");
	naDestroySource = ImportFunc("_naDestroySource");
	naPlaySource = ImportFunc("_naPlaySource");
	naPauseSource = ImportFunc("_naPauseSource");
	naStopSource = ImportFunc("_naStopSource");
	naReplaySource = ImportFunc("_naReplaySource");
	naGetSourceLoop = ImportFunc("_naGetSourceLoop");
	naSetSourceLoop = ImportFunc("_naSetSourceLoop");
	naGetSourceSecOffset = ImportFunc("_naGetSourceSecOffset");
	naSetSourceSecOffset = ImportFunc("_naSetSourceSecOffset");
	naGetSourceLength = ImportFunc("_naGetSourceLength");
	naGetSourceGain = ImportFunc("_naGetSourceGain");
	naSetSourceGain = ImportFunc("_naSetSourceGain");
	naGetSourceStatus = ImportFunc("_naGetSourceStatus");
	naCreateAudioStream = ImportFunc("_naCreateAudioStream");
	naCreateAudioStreamEx = ImportFunc("_naCreateAudioStreamEx");
	naPlayAudioStream = ImportFunc("_naPlayAudioStream");
	naPauseAudioStream = ImportFunc("_naPauseAudioStream");
	naStopAudioStream = ImportFunc("_naStopAudioStream");
	naReplayAudioStream = ImportFunc("_naReplayAudioStream");
	naGetAudioStreamLoop = ImportFunc("_naGetAudioStreamLoop");
	naSetAudioStreamLoop = ImportFunc("_naSetAudioStreamLoop");
	naGetAudioStreamSecOffset = ImportFunc("_naGetAudioStreamSecOffset");
	naSetAudioStreamSecOffset = ImportFunc("_naSetAudioStreamSecOffset");
	naGetAudioStreamLength = ImportFunc("_naGetAudioStreamLength");
	naGetAudioStreamGain = ImportFunc("_naGetAudioStreamGain");
	naSetAudioStreamGain = ImportFunc("_naSetAudioStreamGain");
	naGetAudioStreamStatus = ImportFunc("_naGetAudioStreamStatus");
	naDestroyAllAudioStreams = ImportFunc("_naDestroyAllAudioStreams");
	naDestroyAudioStream = ImportFunc("_naDestroyAudioStream");
}

void nyanUnloadCauseWayDll(void)
{
	FreeLibrary(cwnyadllhandle);
}
