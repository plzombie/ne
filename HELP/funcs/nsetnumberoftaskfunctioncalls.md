# Функция nSetNumberOfTaskFunctionCalls

## Синтаксис

```c
bool nSetNumberOfTaskFunctionCalls(unsigned int taskid, int noffunccalls);
```

## Объявлена в

> "nyan_threads_publicapi.h"

> "nyan_publicapi.h"

## Описание

Устанавливает количество вызовов функции задачи.

* **taskid** - номер задачи.

* **noffunccalls** - новое значение для количества вызовов её функции.

## Возвращаемое значение

Возвращает *true* в случае успеха и *false* в случае ошибки.

## Потокобезопасная

Да

## Пример

-
