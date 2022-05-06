# Функция nRunTaskOnAllThreads

## Синтаксис

```c
bool nRunTaskOnAllThreads(void N_APIENTRY func(void *param), void *args, bool wait_for_running);
```

## Объявлена в

> "nyan_threads_publicapi.h"

> "nyan_publicapi.h"

## Описание

Запускает задачу один раз на всех потоках.

* **func** - указатель на функцию, вызываемую во время выполнения задачи.

* **args** - аргумент, передаваемый функции.

* **wait_for_running** - если *true*, то ждёт выполнения всех задач. В противном случае, возвращается сразу же.

## Возвращаемое значение

В случае успеха возвращает *true*. В случае ошибки возвращает *false*.

## Потокобезопасная

Да

## Пример

-
