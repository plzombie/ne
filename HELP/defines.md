﻿# Список дефайнов

## Общие

* N_WCHAR32 - wchar_t имеет размер в 32 бита
* N_NODYLIB - Движок, аудиодвижок и рендер компилируются как одна библиотека, а не
	как набор библиотек (исходники аудиодвижка и рендера компилируются вместе с исходниками движка как одна программа).
	При этом всё ещё необходимо вызывать ф-и nvAttachRender и naAttachLib (для вызова функций nglSetupDll и nalSetupDll).
	Не влияет на загрузку плагинов.
	
## Сборка библиотеки движка

* N_EXPORT - Следует указывать при компиляции библиотеки движка
* N_STATIC - Библиотека движка собирается(была собрана) как статическая (т.е. nya.lib вместо nya.dll)

## Платформа

* N_WINDOWS - Платформа Windows
* N_DOS - Платформа ДОС
* N_CAUSEWAY - Платформа ДОС через Causeway extender
* N_POSIX - Posix-совместимая платформа (+Linux, +FreeBsd, -Android)
* N_ANDROID - Android

! Если платформа не указана, то движок собирается (был собран) как статическая библиотека.

! Платформозависимые функции при этом не работают (возвращают 0).

! Дефайны для платформ и N_NODYLIB применимы также к рендеру и аудиодвижку
