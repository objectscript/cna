#Caché Native Access (CNA)#

CNA provides an interface for using native C-compatible shared libraries without anything but Caché ObjectScript code. CNA is a wrapper for [libffi](https://sourceware.org/libffi/). CNA consists of native library (libcna) and Caché class (CNA.CNA).

##Installation##

###Linux###

- Build libfii: `make libffi`. If make says that `GLOBALS_HOME` is undefined, then define it.

```sh
 export GLOBALS_HOME='путь к корневой папке Caché'
```

- Build libcna: `make`
- Done! File we are looking for — libcna.so. After all you need to import "cna.xml" in any Caché namespace:

```lisp
 do $system.OBJ.Load("путь к cna.xml", "c")
```

###Windows###

For Windows-based systems you can try to do the same as for Linux (for building you may use [mingw32](http://www.mingw.org/) or [mingw32-w64](http://mingw-w64.sourceforge.net/)). Or you can just download [binary files](https://github.com/intersystems-ru/cna/releases). **Note: types (32-bit or 64-bit) of libcna, current Caché installation and external libraries must be equal**

##Runnning the Tests##

If you want to check if CNA works correctly on your system you may run the unit-tests. You will need native library with test functions 'libtest'. If you build CNA by yourself, it is already in the right place. If no — it must be placed in directory `cna/tests/`. Than you should set `^UnitTestRoot` global to the right value and run the tests.

```lisp
 set ^UnitTestRoot="path to 'cna/tests/' directory"  
 do ##class(%UnitTest.Manager).RunTest()
```

##Simple Example##

Let's try to call `strlen` function from C standard lubrary. In Windows it should be located in `C:\Windows\System32\msvcrt.dll`. In Linux — `/usr/lib/libc.so`. `strlen` returns the length of the string:

```C
 size_t strlen ( const char * str );
```
Let's see what you need to do to call it from Caché:

```lisp
 set cna = ##class(CNA.CNA).%New("path to libcna"); Creates object of CNA.CNA class.
 do  cna.LoadLibrary("path to libc")              ; Loads C standard library in CNA

 set string = cna.ConvertStringToPointer("Hello") ; Converts the string into char array, and saves pointer to the first element
                                                  ; That's how string are stored in C

 set argTypes = $lb(cna.#POINTER)                 ; Creates list of CNA.CNA pararmeters. Each parameter stands for the type of the function argument

 set result = cna.CallFunction("strlen",          ; Passes the name of the function,
                                cna.#SIZET,       ; type of return value,
                                argTypes,         ; list of argument types,
                                string)           ; and comma-separated arguments 

 write result, !                                  ; Writes the result (it should be 5)
 do cna.FreeLibrary()                             ; Frees the library
```
