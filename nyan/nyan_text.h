/*
	Файл	: nyan_text.h

	Описание: Весь текст

	История	: 04.08.12	Создан

*/

#ifndef NYAN_TEXT_H
#define NYAN_TEXT_H

#define N_NAME L"Engine"

#define N_OK L"ok"
#define N_TRUE L"true"
#define N_FALSE L"false"
#define N_YES L"yes"
#define N_NO L"no"
#define N_BROKEN L"broken"
#define N_ID L"id"
#define N_SUCCESS L"success"
#define N_FNAME L"filename"
#define N_FNAMES L"filenames"

#define N_MOUNTDIR L"Mount dir"
#define N_MOUNTARCHIVE L"Mount archive"

#define N_ADDFILEPLUGIN L"Add file plugin"

#define NV_ADDTEXTUREPLUGIN L"Add texture plugin"

#define NV_SCREENINFO L"%ls: Screen mode %dx%dx%d, fullscreen %ls, dpi %d%%x%d%%"

#define NA_ADDAUDIOFILEPLUGIN L"Add audiofile plugin"

// Шаблоны
#define LOG_OPENED L"Log opened at %ls\n"
#define LOG_CLOSED L"Log closed at %ls\n"

#define LOG_LINEFORMAT L"%6d %ls %f: " // Формат строки в журнале сообщений

#define LOG_FDEBUGFORMAT L"%ls: %ls" // Шаблон для вывода отладочной информации
#define LOG_FDEBUGFORMAT2 L"%ls: %ls: %ls" // Шаблон для вывода отладочной информации
#define LOG_FDEBUGFORMAT3 L"%ls: %ls: %d" // Шаблон для вывода отладочной информации

#define LOG_FDEBUGFORMAT4 L"%ls %ls %d" // Шаблон для вывода отладочной информации
#define LOG_FDEBUGFORMAT5 L"%ls: %ls %ls %d" // Шаблон для вывода отладочной информации
#define LOG_FDEBUGFORMAT6 L"%ls %ls %ls" // Шаблон для вывода отладочной информации
#define LOG_FDEBUGFORMAT7 L"%ls %ls '%ls'" // Шаблон для вывода отладочной информации
#define LOG_FDEBUGFORMAT8 L"%ls: %ls '%ls'" // Шаблон для вывода отладочной информации
#define LOG_FDEBUGFORMAT9 L"%ls: %ls: %lld" // Шаблон для вывода отладочной информации
#define LOG_FDEBUGFORMAT10 L"%ls: %ls %ls %lld" // Шаблон для вывода отладочной информации

#if defined(__MINGW32__) || (defined(_MSC_VER) && !defined(__POCC__))
	#define LOG_TIME1 L"%Y-%m-%d %H:%M:%S"
	#define LOG_TIME2 L"%H:%M:%S"
#else
	#define LOG_TIME1 L"%F %T"
	#define LOG_TIME2 L"%T"
#endif

// Названия функции

#define F_NINIT L"nInit()"
#define F_NCLOSE L"nClose()"

#define F_NCLOSEALLFILES L"nCloseAllFiles()"

#define F_NVATTACHRENDER L"nvAttachRender()"

#define F_NVINIT L"nvInit()"
#define F_NVCLOSE L"nvClose()"

#define F_NVMAKESCREENSHOT L"nvMakeScreenshot()"

#define F_NVDRAWINIT L"nvDrawInit()"
#define F_NVDRAWCLOSE L"nvDrawClose()"

#define F_NVCREATE3DMODEL L"nvCreate3dModel()"
#define F_NVLOAD3DMODEL L"nvLoad3dModel()"
#define F_NVUNLOAD3DMODEL L"nvUnload3dModel()"
#define F_NVDESTROY3DMODEL L"nvDestroy3dModel()"

#define F_NVCREATE3DLEVEL L"nvCreate3dLevel()"
#define F_NVLOAD3DLEVEL L"nvLoad3dLevel()"
#define F_NVUNLOAD3DLEVEL L"nvUnload3dLevel()"
#define F_NVDESTROY3DLEVEL L"nvDestroy3dLevel()"

#define F_NVSETSTATUSI L"nvSetStatusi()"
#define F_NVGETSTATUSI L"nvGetStatusi()"
#define F_NVSETSTATUSF L"nvSetStatusf()"
#define F_NVGETSTATUSF L"nvGetStatusf()"
#define F_NVSETSTATUSW L"nvSetStatusw()"
#define F_NVGETSTATUSW L"nvGetStatusw()"
#define F_NVGETKEY L"nvGetKey()"
#define F_NVSETSCREEN L"nvSetScreen()"
#define F_NVSETCLIPPINGREGION L"nvSetClippingRegion()"

#define F_PLGNEK0LOAD L"plgNEK0Load()"
#define F_PLGTGALOAD L"plgTGALoad()"
#define F_PLGPCXLOAD L"plgPCXLoad()"
#define F_PLGBMPLOAD L"plgBMPLoad()"
#define F_PLGWAVLOAD L"plgWAVLoad()"

#define F_NMOUNTDIR L"nMountDir()"
#define F_NMOUNTARCHIVE L"nMountArchive()"

#define F_NADDFILEPLUGIN L"nAddFilePlugin()"
#define F_NDELETEALLFILEPLUGINS L"nDeleteAllFilePlugins()"

#define F_NVADDTEXTUREPLUGIN L"nvAddTexturePlugin()"
#define F_NVDELETEALLTEXTUREPLUGINS L"nvDeleteAllTexturePlugins()"

#define F_NVCREATETEXTURE L"nvCreateTexture()"
#define F_NVCREATETEXTUREFROMFILE L"nvCreateTextureFromFile()"
#define F_NVCREATETEXTUREFROMMEMORY L"nvCreateTextureFromMemory()"
#define F_NVLOADTEXTURE L"nvLoadTexture()"
#define F_NVUNLOADTEXTURE L"nvUnloadTexture()"
#define F_NVDESTROYTEXTURE L"nvDestroyTexture()"
//#define F_NVDESTROYALLTEXTURES L"nvDestroyAllTextures()"
#define F_NVCREATEFONT L"nvCreateFont()"
#define F_NVDESTROYFONT L"nvDestroyFont()"
//#define F_NVDESTROYALLFONTS L"nvDestroyAllFonts()"
#define F_NVCREATEEMPTYSPRITESHEET L"nvCreateEmptySpriteSheet()"
#define F_NVCREATESPRITESHEETFROMTILESET L"nvCreateSpriteSheetFromTileSet()"
#define F_NVCREATESPRITESHEETFROMPREALLOCMEMORY L"nvCreateSpriteSheetFromPreallocMemory()"
#define F_NVCREATESPRITESHEETFROMFILE L"nvCreateSpriteSheetFromFile()"
#define F_NVDESTROYSPRITESHEET L"nvDestroySpriteSheet()"
//#define F_NVDESTROYALLSPRITESHEETS L"nvDestroyAllSpriteSheets()"

#define F_NADDPLUGIN L"nAddPlugin()"
#define F_NDELETEALLPLUGINS L"nDeleteAllPlugins()"

#define F_NAINIT L"naInit()"
#define F_NACLOSE L"naClose()"

#define F_NAATTACHLIB L"naAttachLib()"

#define F_NACREATEBUFFER L"naCreateBuffer()"
#define F_NADESTROYBUFFER L"naDestroyBuffer()"
#define F_NALOADBUFFER L"naLoadBuffer()"
#define F_NAUNLOADBUFFER L"naUnloadBuffer()"
#define F_NACREATESOURCE L"naCreateSource()"
#define F_NACREATEAUDIOSTREAM L"naCreateAudioStream()"
#define F_NACREATEAUDIOSTREAMEX L"naCreateAudioStreamEx()"
#define F_NADESTROYSOURCE L"naDestroySource()"
#define F_NADESTROYAUDIOSTREAM L"naDestroyAudioStream()"

#define F_NADELETEALLAUDIOFILEPLUGINS L"naDeleteAllAudioFilePlugins()"
#define F_NAADDAUDIOFILEPLUGIN L"naAddAudioFilePlugin()"

#define F_NALLOCMEMORY L"nAllocMemory()"
#define F_NREALLOCMEMORY L"nReallocMemory()"
#define F_NFREEMEMORY L"nFreeMemory()"

#define F_NINITTHREADSLIB L"nInitThreadsLib()"
#define F_NDESTROYTHREADSLIB L"nDestroyThreadsLib()"
#define F_NCREATETASK L"nCreateTask()"
#define F_NDESTROYTASK L"nDestroyTask()"
#define F_NRUNTASKONALLTHREADS L"nRunTaskOnAllThreads()"
#define F_NCREATEMUTEX L"nCreateMutex()"
#define F_NDESTROYMUTEX L"nDestroyMutex()"
#define F_NDESTROYSYSTEMMUTEX L"nDestroySystemMutex()"

// Сообщения об ошибках

#define ERR_TRYINGTOUSENULLOBJECT L"Trying to use null object"

#define ERR_CANTALLOCMEM L"Can't allocate memory, size"
#define ERR_TRYINGTOFREENULLPTR L"Warning: Trying to free null pointer"

#define ERR_FILEISTOOLARGE L"File is too large"

#define ERR_UMUSTATTACHRENDERBSTE L"You must attach render before start engine"
#define ERR_UMUSTATTACHRENDERBCRP L"You must attach render before change render parameters"
#define ERR_UMUSTATTACHALIBBSTE L"You must attach audio library before start engine"

#define ERR_CANTLOADDLL L"Can't load dll"
#define ERR_CANTIMPORTFUNC L"Can't import function"
#define ERR_RETURNSFALSE L"returns false"

#define ERR_FILENOTFOUNDED L"File not founded"
#define ERR_FILEISDAMAGED L"File is damaged"
#define ERR_WRONGSIGNIN L"Wrong signature(magic number)"

#define ERR_CANTFINDFILETYPE L"Can't find filetype"
#define ERR_CANTFINDFILEHEAD L"Can't find filehead"
#define ERR_CANTFINDFILEDATA L"Can't find filedata"
#define ERR_CANTFINDCHUNK L"Can't find chunk"
#define ERR_WRONGFILETYPE L"Wrong filetype"
#define ERR_WRONGFILEHEAD L"Wrong filehead"
#define ERR_WRONGFILEDATA L"Wrong filedata"
#define ERR_WRONGCHUNK L"Wrong chunk"

#define ERR_UNSUPPORTEDCOLOR L"Unsupported color mode"
#define ERR_UNSUPPORTEDPALETTE L"Unsupported palette mode"
#define ERR_UNSUPPORTEDDATATYPE L"Unsupported data type"
#define ERR_UNSUPPORTEDVERSION L"Unsupported version"
#define ERR_UNSUPPORTEDENCODING L"Unsupported encoding"

#endif
