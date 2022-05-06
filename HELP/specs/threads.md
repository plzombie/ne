# Спецификация многопоточности в Nyan Engine

## Основные сведения

Для реализации многопоточности в движке используются задачи. Каждая задача имеет функцию, которая вызывается в фоне во время работы движка. Между вызовами функции происходит задержка не менее N миллисекунд (число N задаётся при создании задачи). Количество вызовов функции можно задать от 0 до бесконечности.

Функции вызываются в основном потоке (во время вызова nUpdate), если платформа не поддерживает многопоточность, или в параллельных основному, если поддерживает. 

Для синхронизации между задачами (в случаях, когда задачи и основной цикл выполняются в разных потоках) используются мьютексы. Мьютекс можно повторно захватывать одним и тем же потоком несколько раз. Мьютексы не удалятся при вызове nClose и должны быть удалены самостоятельно вызовом функции nDestroyMutex.

Задачи в движке аналогичны таймерам. Основное отличие заключается в том, что для каждой задачи движка обычно выделяется отдельный поток системы (в текущей реализации - только один, на все задачи). Сходство с таймерами было получено из-за желания обеспечить поддержку платформ, которые не поддерживают многопоточность.

## Список функций для работы с задачами и мьютексами

* nCreateTask
* nDestroyTask
* nRunTaskOnAllThreads
* nGetMaxThreadsForTasks
* nGetNumberOfTaskFunctionCalls
* nSetNumberOfTaskFunctionCalls
* nCreateMutex
* nDestroyMutex
* nLockMutex
* nUnlockMutex

## Список потокобезопасных функций

* nCreateTask
* nDestroyTask
* nRunTaskOnAllThreads
* nGetMaxThreadsForTasks
* nGetNumberOfTaskFunctionCalls
* nSetNumberOfTaskFunctionCalls
* nCreateMutex
* nDestroyMutex
* nLockMutex
* nUnlockMutex
* nlPrint
* nlAddTab
* nSleep
* nClock
* nAllocMemory
* nFreeMemory
* nReallocMemory
* nFileCreate
* nFileDelete
* nFileRead
* nFileWrite
* nFileCanWrite
* nFileLength
* nFileTell
* nFileSeek
* nFileOpen
* nFileClose
* nCloseAllFiles
* nMountDir
* nMountArchive
* nAddFilePlugin
* nDeleteAllFilePlugins
* nvLoadImageToMemory
* nvCreateTexture
* nvCreateTextureFromFile
* nvCreateTextureFromMemory
* nvSetTextureLoadTypeVoid
* nvSetTextureLoadTypeFromFile
* nvSetTextureLoadTypeFromMemory
* nvUpdateTextureFromMemory
* nvLoadTexture
* nvUnloadTexture
* nvDestroyTexture
* nvDestroyAllTextures
* nvGetTextureStatus
* nvGetTextureLType
* nvGetTextureWH
* nvGetTextureFormat
* nvAddTexturePlugin
* nvDeleteAllTexturePlugins
* аргумент na_audiofile_plg_read_type readaf функции nalCreateStreamingSource
* nglIsTex (в ngl и nullgl)
* nglFreeTexture (в ngl и nullgl)
* nglFreeAllTextures (в ngl и nullgl)
* nglLoadTexture (в ngl и nullgl)
* nglUpdateTexture (в ngl и nullgl)
