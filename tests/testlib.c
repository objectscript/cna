#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#ifdef _WIN32
  #define EXPORT __declspec(dllexport)
  #include <windows.h>
#else /* UNIX */
  #define EXPORT
#endif /* _WIN32 */

inline void
logger(const char *format, ...)
{
  #ifdef DEBUG
    va_list args;
    va_start(args, format);
    
    #ifdef _WIN32
      char path[MAX_PATH];
      HMODULE hm;
      GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR) &logger, &hm);
      GetModuleFileNameA(hm, path, sizeof(path));
      strcpy(strrchr(path, '\\') + 1, "log.txt");

      FILE *fd = fopen(path, "a");
      if (!fd) {
        return;
      }
      time_t now;
      time(&now);
      fprintf(fd, "%s\t", ctime(&now));
      vfprintf(fd, format, args);
      fclose(fd);
    #else
      vprintf(format, args);
    #endif /* _WIN32 */
    
    va_end(args);
  #endif /* DEBUG */
}

EXPORT double 
square(double a)
{
  return a * a;
}

EXPORT unsigned
sum(unsigned a, unsigned b, unsigned c, unsigned d, unsigned e)
{
  return a + b + c + d + e;
}

EXPORT double
mul(unsigned long long a, double b)
{
  return a * b;
}

EXPORT int64_t
x(int64_t x)
{
  return x;
}

EXPORT unsigned int
cstrlen(const char *s)
{
  unsigned int i;
  for (i = 0; *s; ++i, ++s) {}
  return i;
}

EXPORT void *
mal(size_t size)
{
  return malloc(size);
}

EXPORT void
fr(void *mem)
{
  free(mem);
}

EXPORT int
five(void)
{
  return 5;
}

EXPORT   signed char  _schar (   signed char  x) { return x; }
EXPORT unsigned char  _uchar ( unsigned char  x) { return x; }
EXPORT   signed short _sshort(   signed short x) { return x; }
EXPORT unsigned short _ushort( unsigned short x) { return x; }
EXPORT   signed int   _sint  (   signed int   x) { return x; }
EXPORT unsigned int   _uint  ( unsigned int   x) { return x; }
EXPORT   signed long  _slong (   signed long  x) { return x; }
EXPORT unsigned long  _ulong ( unsigned long  x) { return x; }
EXPORT  int64_t       _sint64( int64_t        x) { return x; }
EXPORT uint64_t       _uint64( uint64_t       x) { return x; }


struct st {
  char a;
  long long b;
  char c;
};

EXPORT long long
sum_st(struct st x)
{
  return x.a + x.b + x.c;
}

EXPORT struct st
create_st(void)
{
  struct st a;
  a.a = 1;
  a.b = 2;
  a.c = 3;
  return a;
}

struct A {
  signed char a1;
  long long a2;
  signed char a3;
};

struct B {
  signed char b1;
  struct A b2;
  signed char b3;
};

EXPORT long long
sum_nested(struct B x)
{
  return x.b1 + x.b2.a1 + x.b2.a2 + x.b2.a3 + x.b3;
}

EXPORT float
sum_float(float a, float b)
{
  return a + b;
}

EXPORT long double
sum_long_double(long double a, long double b)
{
  return a + b;
}

EXPORT int
compare_string_and_ulysses(const char *s, const char *path)
{
  FILE *fd = fopen(path, "rb");
  if (!fd) {
    logger("Can't open Ulysses for reading\n");
    return -1;
  }
  fseek(fd, 0ll, SEEK_END);
  size_t size = ftell(fd);
  rewind(fd);
  char *buf = malloc(sizeof(char) * size);

  if (!buf) { 
    logger("Can't allocate memory\n");
    return -1;
  }
  size_t res = fread(buf, sizeof(char), size, fd);
  if (res != size) { 
    logger("Bytes readed: %llu\n", res);
    logger("Is EOF: %d\n", feof(fd));
    logger("Is error: %d\n", ferror(fd));
    return -1; 
  }

  int cmp = strncmp(s, buf, size);

  fclose(fd);
  if (cmp == 0) {
    return 1;
  } else {
    return 0;
  }
}