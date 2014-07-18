#include <stdio.h>
#include <stdint.h>

typedef unsigned int uint; 

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

EXPORT uint
sum(uint a, uint b, uint c, uint d, uint e)
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