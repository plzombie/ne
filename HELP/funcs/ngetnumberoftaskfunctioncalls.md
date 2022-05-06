# Функция nGetNumberOfTaskFunctionCalls

## Синтаксис

```c
bool nGetNumberOfTaskFunctionCalls(unsigned int taskid, int *noffunccalls);
```

## Объявлена в

> "nyan_threads_publicapi.h"

> "nyan_publicapi.h"

## Описание

Возвращает количество вызовов функции задачи.

* **taskid** - номер задачи.

* **noffunccalls** - указатель на переменную, в которую нужно поместить количество вызовов функции.

## Возвращаемое значение

Возвращает *true* в случае успеха и *false* в случае ошибки.

## Потокобезопасная

Да

## Пример

-
