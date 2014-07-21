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

EXPORT   signed char  schar (   signed char  x) { return x; }
EXPORT unsigned char  uchar ( unsigned char  x) { return x; }
EXPORT   signed short sshort(   signed short x) { return x; }
EXPORT unsigned short ushort( unsigned short x) { return x; }
EXPORT   signed int   sint  (   signed int   x) { return x; }
EXPORT unsigned int   uint  ( unsigned int   x) { return x; }
EXPORT   signed long  slong (   signed long  x) { return x; }
EXPORT unsigned long  ulong ( unsigned long  x) { return x; }
EXPORT  int64_t       sint64( int64_t        x) { return x; }
EXPORT uint64_t       uint64( uint64_t       x) { return x; }