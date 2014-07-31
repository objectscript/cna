#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef _WIN32
  #define EXPORT __declspec(dllexport)
#else /* UNIX */
  #define EXPORT
#endif /* _WIN32 */

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

EXPORT unsigned int
print(const char *s)
{
  FILE *fd = fopen("C:\\Users\\dd\\Dropbox\\is\\CNA\\log.txt", "a");
  if (!fd) { return 25; }
  fprintf(fd, "\n\t%s\n", s);
  fclose(fd);
  return 99;
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
  char s[100];
  print(s);
  return a + b;
}