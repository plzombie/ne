# Ken's Picture LIBrary fork

## Description(ru)

Модификация kplib.c Кена Силвермана. Оригинальный файл можно взять из http://www.advsys.net/ken/voxlap/voxlap_src.zip

Новые функции:

## Функция kzaddstack2

### Синтаксис

`int kzaddstack2 (const char *filnam, int adddirs);`

### Описание

Если файл filnam найден, то загружает внутреннюю папку (internal directory) из ZIP/GRP в память (hash) чтобы обеспечить быстрый доступ (к ней) позже.

Если файл не найден, считает, что это папка и, если adddirs не равен нулю, то добавляет её во внутренний список (internal list).

### Возвращаемое значение

Возвращает -1 в случае успеха, иначе возвращает 0.

## Функция kzaccess

### Синтаксис

`int kzaccess(const char *filnam);`

### Описание

Проверяет наличие файла filnam. Аналог _access(filnam, 0).

### Возвращаемое значение

Возвращает 0, если файл найден, иначе возвращает -1.

## Original description

KPLIB.C: Ken's Picture LIBrary written by Ken Silverman

Copyright (c) 1998-2008 Ken Silverman

Ken Silverman's official web site: http://advsys.net/ken


Features of KPLIB.C:

	* Routines for decoding JPG/PNG/GIF/PCX/TGA/BMP/DDS/CEL. See kpgetdim(), kprender(), and optional helper function: kpzload().

	* Routines for reading files out of ZIP/GRP files. All ZIP/GRP functions start with "kz".

	* Multi-platform support: Dos/Windows/Linux/Mac/etc..

	* Compact code, all in a single source file. Yeah, bad design on my part... but makes life easier for everyone else - you simply add a single C file to your project, throw a few externs in there, add the function calls, and you're done!


Brief history:

1998?: Wrote KPEG, a JPEG viewer for DOS

2000: Wrote KPNG, a PNG viewer for DOS

2001: Combined KPEG & KPNG, ported to Visual C, and made it into a library called KPLIB.C

2002: Added support for TGA,GIF,CEL,ZIP

2003: Added support for BMP

05/18/2004: Added support for 8&24 bit PCX

12/09/2005: Added support for progressive JPEG

01/05/2006: Added support for DDS

07/28/2007: Added support for GRP (Build Engine archive)


I offer this code to the community for free use - all I ask is that my name be included in the credits.


-Ken S.
