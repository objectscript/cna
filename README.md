#Caché Native Access (CNA)#

CNA provides an interface for using native shared libraries without anything but Caché ObjectScript code.

CNA — связка класса Caché и библиотеки на C, позволяющая вызывать функции из внешних библиотек, написанных на C или бинарно с ним совместимых. CNA основана на [libffi](https://sourceware.org/libffi/).

##Установка##

###Linux###

1. Устанавливаем libffi — из репо (для deb-based) `apt-get install libffi-dev`, или собираем из [исходников](ftp://sourceware.org/pub/libffi/libffi-3.1.tar.gz).
2. Делаем `make`. Если ругается на отстуствие переменной `GLOBALS_HOME` — прописываем  

```sh
export GLOBALS_HOME='путь к корневой папке Caché'
```
3. Готово! Нужный нам файл — libcna.so. Осталось только импортировать класс cna.xml в любую удобную область Caché:

```M
do $system.OBJ.Load("путь к cna.xml", "c")
```

###Windows###

Для Windows-систем можно повторить указанные выше действия (для сборки использовать [mingw32](http://www.mingw.org/) или [mingw32-w64](http://mingw-w64.sourceforge.net/)). Или использовать уже готовые [бинарные версии](https://github.com/intersystems-ru/cna/releases) (пока только для x86-64). Импорт cna.xml делаем аналогично, или через студию.

##Запуск тестов##

Для запуска тестов необходима тестовая библиотека libtest. Если вы собирали libcna сами, то она уже лежит где надо. Если нет — необходимо поместить ее в папку `tests/`, рядом с libcna. Далее, присваиваем глобалу `^UnitTestRoot` нужное значение и запускаем тесты: 

```M
set ^UnitTestRoot="путь к папке с libcna"  
do ##class(%UnitTest.Manager).RunTest()
```

##Простой пример##

Воспользуемся библиотекой libtest. В ней определена такая функция:

```C
unsigned int cstrlen(const char *s);
```

Которая возвращает длину строки. Посмотрим, что нужно сделать, чтобы вызвать ее из Caché:

```M
set cna = ##class(CNA.CNA).%New("путь к libcna") // Создаем объект класса CNA.CNA. В аргументах указываем путь к libcna.dll или libcna.so
do cna.LoadLibrary("путь к libtest")             // Загружаем библиотеку libtest в CNA

set string = cna.ConvertStringToPointer("Hello") // Конвертируем нужную строку в массив типа char, и сохраняем указатель на первый
                                                 // элемент — именно в таком виде хранятся строки в C 

set argTypes = ##class(%ListOfDataTypes).%New()  // Создаем список, в котором будем передавать список аргументов
do argTypes.Insert(cna.#POINTER)                 // Вставляем в список параметр класса CNA.CNA, который обозначает тип "указатель"
                                                 // Аргумент в нашей функции только один, поэтому переходим к ее вызову

set result = cna.CallFunction("cstrlen",         // Передаем имя вызываемой функции,
                               cna.#UINT,        // тип возвращаемого значения,
                               argTypes,         // список типов аргументов функции
                               string)           // и все аргументы через запятую

w result, !                                      // Проверяем рузультат (должно получиться 5)
do cna.FreeLibrary()                             // Освобождаем загруженную библиотеку
```