#Caché Native Access (CNA)#

CNA provides an interface for using native shared libraries without anything but Caché ObjectScript code.

CNA — связка класса Caché и библиотеки на C, позволяющая вызывать функции из внешних библиотек, написанных на C или бинарно с ним совместимых. CNA основана на [libffi](https://sourceware.org/libffi/).

##Установка##

###Linux###

- Собираем libffi: `make libffi`. Если ругается на отстуствие переменной `GLOBALS_HOME` — прописываем

```sh
 export GLOBALS_HOME='путь к корневой папке Caché'
```

- Собираем libcna: `make`
- Готово! Нужный нам файл — libcna.so. Осталось только импортировать класс cna.xml в любую удобную область Caché:

```lisp
 do $system.OBJ.Load("путь к cna.xml", "c")
```

###Windows###

Для Windows-систем можно повторить указанные выше действия (для компиляции и сборки использовать [mingw32](http://www.mingw.org/) или [mingw32-w64](http://mingw-w64.sourceforge.net/)). Или можно загрузить уже готовые [бинарные версии](https://github.com/intersystems-ru/cna/releases) CNA. **Внимание: разрядности libcna, инсталяции Caché и внешних библиотек доллжны совпадать.** Импорт cna.xml делаем аналогично или через студию.

##Запуск тестов##

Для проверки работоспособности CNA можно запустить юнит-тесты. Для запуска тестов необходима библиотека с тестовыми функциями libtest. Если вы собирали libcna сами, то она уже лежит где надо. Если нет — ее нужно поместить в папку `cna/tests/`. Далее, присваиваем глобалу `^UnitTestRoot` нужное значение и запускаем тесты: 

```lisp
 set ^UnitTestRoot="путь к папке tests"  
 do ##class(%UnitTest.Manager).RunTest()
```

##Простой пример##

Попробуем вызвать функцию `strlen` из стандартной библиотеки языка C. В Windows она скорее всего находится по адресу `C:\Windows\System32\msvcrt.dll`. В Linux — `/usr/lib/libc.so`. Функция `strlen` возвращает длину строки:

```C
 size_t strlen ( const char * str );
```

Посмотрим, что нужно сделать, чтобы вызвать ее из Caché:

```lisp
 set cna = ##class(CNA.CNA).%New("путь к libcna") ; Создаем объект класса CNA.CNA. В аргументах указываем путь к libcna.dll или libcna.so
 do  cna.LoadLibrary("путь к libc")               ; Загружаем стандартную библиотеку С в CNA

 set string = cna.ConvertStringToPointer("Hello") ; Конвертируем нужную строку в массив типа char, и сохраняем указатель на первый
                                                  ; элемент — именно в таком виде хранятся строки в C 

 set argTypes = ##class(%ListOfDataTypes).%New()  ; Создаем объект, в котором будем передавать типы аргументов функции
 do  argTypes.Insert(cna.#POINTER)                ; Вставляем в список параметр класса CNA.CNA, который обозначает тип "указатель"
                                                  ; Аргумент в нашей функции только один, поэтому переходим к ее вызову

 set result = cna.CallFunction("strlen",          ; Передаем имя вызываемой функции,
                                cna.#SIZET,       ; тип возвращаемого значения,
                                argTypes,         ; список типов аргументов функции
                                string)           ; и все значения аргументов через запятую

 w result, !                                      ; Проверяем результат (должно получиться 5)
 do cna.FreeLibrary()                             ; Освобождаем загруженную библиотеку
```
